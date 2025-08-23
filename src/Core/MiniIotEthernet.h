#pragma once
#include <ArduinoJson.h>
#include <LittleFS.h>

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>

#include "MiniIotUtils.h"


class MiniIotEthernet
{

private:
    uint32_t init_time = 0;
    uint32_t connect_time = 0;
    uint32_t connect_led_time = 0;

    String configName = "/ethernetConfig.json";
    // String jsonData = "{\"ssid\":\"miniiot.top\",\"passwd\":\"88888888\"}";

    // wifi信息
    // String WifiSsid;
    // String WifiPasswd;

    String mac;

    // 初始化 LittleFS
    bool initializeLittleFS()
    {
#ifdef ESP32
        if (!LittleFS.begin(true))
#else
        if (!LittleFS.begin())
#endif
        {
            MiniIot_LOG_LN(F("[WIFI] LittleFS 初始化失败"));
            return false;
        }

        return true;
    }

    // 读取本地wifi信息
    // bool loadConfig()
    // {
    //     if (!initializeLittleFS())
    //     {
    //         return false;
    //     }

    //     File config = LittleFS.open(this->configName, "r");
    //     if (!config)
    //     {
    //         MiniIot_LOG(F("[WIFI] 无法打开配置文件："));
    //         MiniIot_LOG_LN(this->configName);
    //         MiniIot_LOG(F("[WIFI] 使用默认配置："));
    //         MiniIot_LOG_LN(this->jsonData);
    //     }
    //     else
    //     {
    //         this->jsonData = config.readString();
    //         config.close();
    //         LittleFS.end();

    //         MiniIot_LOG(F("[WIFI] 成功读取配置："));
    //         MiniIot_LOG_LN(this->jsonData);
    //     }

    //     DynamicJsonDocument JSON_Buffer(this->jsonData.length() + 20);
    //     if (deserializeJson(JSON_Buffer, this->jsonData) != DeserializationError::Ok)
    //     {
    //         MiniIot_LOG_LN(F("[WIFI] 配置解析JSON错误"));
    //         return false;
    //     }

    //     this->WifiSsid = JSON_Buffer["ssid"].as<String>();
    //     this->WifiPasswd = JSON_Buffer["passwd"].as<String>();

    //     MiniIot_LOG(F("[WIFI] 成功解析配置："));
    //     MiniIot_LOG_LN(this->jsonData);
    //     return true;
    // }

    // 配置写入
    // bool write()
    // {
    //     if (!initializeLittleFS())
    //     {
    //         return false;
    //     }

    //     File config = LittleFS.open(this->configName, "w");
    //     if (!config)
    //     {
    //         MiniIot_LOG(F("[WIFI] 无法打开配置文件："));
    //         MiniIot_LOG_LN(this->configName);
    //         LittleFS.end();
    //         return false;
    //     }

    //     String data = "{\"ssid\":\"" + this->WifiSsid + "\",\"passwd\":\"" + this->WifiPasswd + "\"}";
    //     if (config.print(data) != data.length())
    //     {
    //         config.close();
    //         LittleFS.end();
    //         MiniIot_LOG(F("[WIFI] 配置写入失败："));
    //         MiniIot_LOG_LN(this->configName);
    //         return false;
    //     }

    //     MiniIot_LOG(F("[WIFI] 配置写入成功："));
    //     MiniIot_LOG_LN(data);
    //     config.close();
    //     LittleFS.end();

    //     return true;
    // }

    bool status(){
        int code1 = Ethernet.hardwareStatus();
        if(code1 != 3){
            MiniIot_LOG(F("[Ethernet] Ethernet模块初始化失败："));
            MiniIot_LOG_LN(code1);
            return false;
        }

        MiniIot_LOG_LN(F("[Ethernet] Ethernet模块初始化成功"));

        int code2 = Ethernet.linkStatus();
        if(code2 != 1){
            MiniIot_LOG(F("[Ethernet] 网线未连接："));
            MiniIot_LOG_LN(code2);
            return false;
        }

        MiniIot_LOG_LN(F("[Ethernet] 网线已连接"));
        return true;
    }

    void begin(){
        String macStr = MiniIotUtils::getMacByChipId();
        this->mac = macStr;
        byte mac[6];
        sscanf(macStr.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        
        IPAddress ip(192, 168, 0, 29);
        IPAddress myDns(119, 29, 29, 29);

        MiniIot_LOG_LN();
        MiniIot_LOG_LN(F("[Ethernet] 初始化Ethernet模块"));

        digitalWrite(MiniIot_ETH_RST, HIGH);
        delay(200);
        digitalWrite(MiniIot_ETH_RST, LOW);
        delay(200);
        digitalWrite(MiniIot_ETH_RST, HIGH);
        delay(200);

        MiniIot_LOG_LN(F("[Ethernet] 开始获取IP..."));
        if (Ethernet.begin(mac) == 0)
        {
            MiniIot_LOG_LN(F("[Ethernet] DHCP获取IP失败,使用静态IP"));
            Ethernet.begin(mac, ip, myDns);
        }

        MiniIot_LOG(F("[Ethernet] IP: "));
        MiniIot_LOG(Ethernet.localIP());
        MiniIot_LOG(F(",MAC: "));
        MiniIot_LOG_LN(this->mac);

        this->init_time = millis();
    }

public:

    void init(){
        // 初始化Ethernet板子
        pinMode(MiniIot_ETH_RST, OUTPUT);
        Ethernet.init(SS);
    }

    // 连接网络（LED常量）
    bool connect()
    {
        if(this->connect_led_time == 0){
            this->connect_led_time = millis();
            #ifdef MiniIot_STATE_LED
                digitalWrite(MiniIot_STATE_LED, !digitalRead(MiniIot_STATE_LED));
            #endif
        }
        
        // 1s刷新一次LED
        if(millis() - this->connect_led_time >= 1000){
            this->connect_led_time = 0;
        }

        
        if(this->connect_time == 0){
            this->connect_time = millis();
            if(this->status()){
                this->connect_time = 0;
                return true;
            }
        }

        // 3s检测一次网络连接
        if(millis() - this->connect_time >= 3000){
            this->connect_time = 0;
        }


        if(this->init_time == 0){
            this->init_time = millis();
            if(!this->status()){
                this->begin();
            }
        }
        
        // 9s未连接，重新初始化
        if(millis() - this->init_time >= 9000){
            this->init_time = 0;
        }

        return false;
    }

    // 获取MAC地址
    String getMac()
    {
        return this->mac;
    }

    // 获取网线连接状态
    int getStatus()
    {
        return Ethernet.hardwareStatus() == 3 && Ethernet.linkStatus() == 1 ? 1 : 0;
    }

    // 清除配置信息
    void clear()
    {
        if (!initializeLittleFS())
        {
            return;
        }

        if (LittleFS.remove(this->configName))
        {
            MiniIot_LOG(F("[Ethernet] 配置清除成功："));
        }
        else
        {
            MiniIot_LOG(F("[Ethernet] 配置清除失败："));
        }
        MiniIot_LOG_LN(this->configName);

        LittleFS.end();
    }

    // 更新配置
    // void update(String wifiName, String wifiPassword)
    // {
    //     this->WifiSsid = wifiName;
    //     this->WifiPasswd = wifiPassword;
    //     this->write();
    // }
};
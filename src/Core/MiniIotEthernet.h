#pragma once
#include <ArduinoJson.h>

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>

#include "MiniIotUtils.h"

class MiniIotEthernet
{

private:
    uint32_t init_time = 0;
    uint32_t connect_time = 0;

    String mac;

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

    // 手动解析 MAC 字符串（格式：AA:BB:CC:DD:EE:FF）
    bool parseMac(String str, byte* outMac) {
        if (str.length() != 17) return false;  // 长度校验
        for (int i = 0; i < 6; i++) {
            // 提取每个字节的两位十六进制字符（跳过冒号）
            String hexByte = str.substring(i*3, i*3 + 2);
            // 转换为字节（16进制转10进制）
            outMac[i] = (byte)strtol(hexByte.c_str(), NULL, 16);
        }
        return true;
    }

    void begin(){
        String macStr = MiniIotUtils::getMacByChipId();
        this->mac = macStr;
        byte mac[6];
        if (!this->parseMac(macStr, mac)) {
            MiniIot_LOG_LN(macStr);
            MiniIot_LOG_LN("MAC格式错误！");
        }
        
        IPAddress ip(192, 168, 0, 29);
        IPAddress myDns(119, 29, 29, 29);

        MiniIot_LOG_LN();
        MiniIot_LOG_LN(F("[Ethernet] 初始化Ethernet模块"));

        this->restart();

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

    void restart(){
        digitalWrite(MiniIot_ETH_RST, LOW);
    #ifdef MiniIot_RTOS
        vTaskDelay(200 / portTICK_RATE_MS);
    #else
        delay(200);
    #endif
        digitalWrite(MiniIot_ETH_RST, HIGH);
    #ifdef MiniIot_RTOS
        vTaskDelay(200 / portTICK_RATE_MS);
    #else
        delay(200);
    #endif
    }

    // 连接网络
    bool connect()
    {
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

};
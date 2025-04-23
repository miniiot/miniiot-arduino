#pragma once
#include <ArduinoJson.h>
#include <LittleFS.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif

class MiniIotWifi
{

private:
    String configName = "/wifiConfig.json";
    #ifdef DEFAULT_WIFI_SSID
        #ifdef DEFAULT_WIFI_PASSWORD
            String jsonData = String("{\"ssid\":\"") + DEFAULT_WIFI_SSID + String("\",\"passwd\":\"") + DEFAULT_WIFI_PASSWORD + String("\"}");
        #else
            String jsonData = "{\"ssid\":\"miniiot\",\"passwd\":\"miniiot\"}";
        #endif
    #else
        String jsonData = "{\"ssid\":\"miniiot\",\"passwd\":\"miniiot\"}";
    #endif

    // wifi信息
    String WifiSsid;
    String WifiPasswd;

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
    bool loadConfig()
    {
        if (!initializeLittleFS())
        {
            return false;
        }

        File config = LittleFS.open(this->configName, "r");
        if (!config)
        {
            MiniIot_LOG(F("[WIFI] 无法打开配置文件："));
            MiniIot_LOG_LN(this->configName);
            MiniIot_LOG(F("[WIFI] 使用默认配置："));
            MiniIot_LOG_LN(this->jsonData);
        }
        else
        {
            this->jsonData = config.readString();
            config.close();
            LittleFS.end();

            MiniIot_LOG(F("[WIFI] 成功读取配置："));
            MiniIot_LOG_LN(this->jsonData);
        }

        DynamicJsonDocument JSON_Buffer(this->jsonData.length() + 20);
        if (deserializeJson(JSON_Buffer, this->jsonData) != DeserializationError::Ok)
        {
            MiniIot_LOG_LN(F("[WIFI] 配置解析JSON错误"));
            return false;
        }

        this->WifiSsid = JSON_Buffer["ssid"].as<String>();
        this->WifiPasswd = JSON_Buffer["passwd"].as<String>();

        MiniIot_LOG(F("[WIFI] 成功解析配置："));
        MiniIot_LOG_LN(this->jsonData);
        return true;
    }

    // 配置写入
    bool write()
    {
        if (!initializeLittleFS())
        {
            return false;
        }

        File config = LittleFS.open(this->configName, "w");
        if (!config)
        {
            MiniIot_LOG(F("[WIFI] 无法打开配置文件："));
            MiniIot_LOG_LN(this->configName);
            LittleFS.end();
            return false;
        }

        String data = "{\"ssid\":\"" + this->WifiSsid + "\",\"passwd\":\"" + this->WifiPasswd + "\"}";
        if (config.print(data) != data.length())
        {
            config.close();
            LittleFS.end();
            MiniIot_LOG(F("[WIFI] 配置写入失败："));
            MiniIot_LOG_LN(this->configName);
            return false;
        }

        MiniIot_LOG(F("[WIFI] 配置写入成功："));
        MiniIot_LOG_LN(data);
        config.close();
        LittleFS.end();

        return true;
    }

public:
    // 连接WIFI（LED常量）
    bool wifiConnect()
    {
        this->loadConfig();

        digitalWrite(MiniIot_STATE_LED, 0);
        int retry_times = 0;

        WiFi.begin(this->WifiSsid.c_str(), this->WifiPasswd.c_str());

        MiniIot_LOG(F("[WIFI] WIFI连接中"));
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            MiniIot_LOG(".");
            retry_times++;
            // 500ms*20=10秒超时
            if (retry_times >= 20)
            {
                MiniIot_LOG_LN();
                MiniIot_LOG_LN(F("[WIFI] WIFI连接超时"));
                digitalWrite(MiniIot_STATE_LED, 1);
                return false;
            }
        }

        MiniIot_LOG_LN();
        MiniIot_LOG(F("[WIFI] WIFI连接成功,IP: "));
        MiniIot_LOG(WiFi.localIP());
        MiniIot_LOG(F(",MAC: "));
        MiniIot_LOG_LN(WiFi.macAddress());

        digitalWrite(MiniIot_STATE_LED, 1);
        return true;
    }

    // 获取MAC地址
    String getWifiMac()
    {
        return WiFi.macAddress();
    }

    // 获取wifi状态
    int getStatus()
    {
        return WiFi.status();
    }

    // 清除WIFI信息
    void clear()
    {
        if (!initializeLittleFS())
        {
            return;
        }

        if (LittleFS.remove(this->configName))
        {
            MiniIot_LOG(F("[WIFI] 配置清除成功："));
        }
        else
        {
            MiniIot_LOG(F("[WIFI] 配置清除失败："));
        }
        MiniIot_LOG_LN(this->configName);

        LittleFS.end();
    }

    // wifi信息写入
    void update(String wifiName, String wifiPassword)
    {
        this->WifiSsid = wifiName;
        this->WifiPasswd = wifiPassword;
        this->write();
    }
};
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <sstream>

#include "Config.h"
#include "Core/MiniIotUtils.h"

// wifi设备
#ifdef __UseWifiClient__
    #ifdef ESP8266
        #include <ESP8266WiFi.h>
    #endif

    #ifdef ESP32
        #include <WiFi.h>
    #endif

    #include "Core/MiniIotWifi.h"
    MiniIotWifi MiniIotWifiObj;

    WiFiClient MiniIotNetworkClient;

    #include "Core/MiniIotOTA.h"
#endif

// 消息处理
#include "MiniIotMessage.h"

// 使用mqtt客户端
#ifdef __UseMqttClient__
    #include "MiniIotMQTT.h"
    MiniIotMQTT MiniIotClient(MiniIotNetworkClient);
#endif

// 使用管理后台
#ifdef __UseAdminService__
    #include "Core/MiniIotAP.h"
    #include "Core/MiniIotAdminWebServer.h"
    MiniIotAdminWebServer MiniIotAdminWebServerClient;
#endif

// 核心类
class MiniIot
{
private:
    int workstate = MINIIOT_WORK_STATE_INIT; // 主进程工作状态

    String ProductId;
    String DeviceId;

    // 文件系统初始化
    void FS_Init()
    {
#ifdef ESP32
        if (!LittleFS.begin(true))
#else
        if (!LittleFS.begin())
#endif
        {
            MiniIot_LOG_LN(F("[SYSTEM] 文件系统初始化失败"));
            return;
        }

        if (!LittleFS.format())
        {
            MiniIot_LOG_LN(F("[SYSTEM] 文件系统格式化失败"));
        }
        else
        {
            MiniIot_LOG_LN(F("[SYSTEM] 文件系统格式化成功"));
        }

        LittleFS.end();
    }

    // 管理系统初始化
    void init_admin_web_server()
    {
#ifdef __UseAdminService__
        MiniIotAP MiniIotAP(this->DeviceId);
        MiniIotAdminWebServerClient.init(MiniIotMessage::SysCallBack);
#endif
    }

    void init()
    {
        // 指示灯初始化（低电平亮）
        pinMode(MiniIot_STATE_LED, OUTPUT);
        digitalWrite(MiniIot_STATE_LED, 1);

        MiniIotMessage::attachSysCallback(SysCallBack);

        this->init_admin_web_server(); // 管理系统初始化
    }

    static void SysCallBack(JsonObject dataObj)
    {
        String serviceName = dataObj["serviceName"].as<String>();

        if (serviceName == "miniiot_wifi_update")
        {
            // 修改wifi
            MiniIot_LOG_LN(F("[SYSTEM] 修改wifi"));
            #ifdef __UseWifiClient__
                MiniIotWifiObj.update(dataObj["serviceParams"]["ssid"].as<String>(), dataObj["serviceParams"]["password"].as<String>());
                MiniIotClient.disconnect();
                ESP.restart();
            #endif
        }
        else if (serviceName == "miniiot_ota_update")
        {
            // OTA升级
            MiniIot_LOG_LN(F("[SYSTEM] OTA升级"));
            #ifdef __UseWifiClient__
                MiniIotOTA MiniIotOtaObj;
                MiniIotOtaObj.updateBin(dataObj["serviceParams"]["url"].as<String>());
            #endif
        }
        else if (serviceName == "miniiot_admin_update")
        {
            // 更新管理后台账号密码
            MiniIot_LOG_LN(F("[ADMIN] 更新账号密码"));
            #ifdef __UseAdminService__
                MiniIotAdminWebServerClient.setUsername(dataObj["serviceParams"]["username"].as<String>());
                MiniIotAdminWebServerClient.setPassword(dataObj["serviceParams"]["password"].as<String>());
            #endif
        }
        else if (serviceName == "miniiot_reboot")
        {
            // 重启
            MiniIot_LOG_LN(F("[SYSTEM] 重启"));
            MiniIotClient.disconnect();
            ESP.restart();
        }
        else
        {
            MiniIot_LOG_LN(F("[SYSTEM] 未知事件"));
        }
    }

public:
    // 初始化(设备秘钥认证,2)
    void begin(String ProductId_, String DeviceId_, String Secret_)
    {
        Serial.println();
        Serial.println();
        Serial.println("[MiniIot] 库版本：" + (String)MiniIot_VERSION);
        Serial.println("[MiniIot] 程序版本：" + (String)APP_VERSION);
        Serial.println("[MiniIot] 产品ID：" + ProductId_);
        Serial.println("[MiniIot] 设备ID：" + DeviceId_);

        this->ProductId = ProductId_;
        this->DeviceId = DeviceId_;

        MiniIotClient.begin(ProductId_, DeviceId_, Secret_, "2");
        this->init();
    }

    // 初始化(产品秘钥认证,1)
    void begin(String ProductId_, String Secret_)
    {
        // 获取芯片唯一ID
        String DeviceId_ = "A" + MiniIotUtils::ESPsha1(MiniIotUtils::ESPchipId()).substring(0, 9);

        Serial.println();
        Serial.println();
        Serial.println("[MiniIot] 库版本：" + (String)MiniIot_VERSION);
        Serial.println("[MiniIot] 程序版本：" + (String)APP_VERSION);
        Serial.println("[MiniIot] 产品ID：" + ProductId_);
        Serial.println("[MiniIot] 设备ID：" + DeviceId_);

        this->ProductId = ProductId_;
        this->DeviceId = DeviceId_;

        MiniIotClient.begin(ProductId_, DeviceId_, Secret_, "1");
        this->init();
    }

    // 绑定业务事件回调
    void attach(JsonObjectCallbackFunction AppCallBack_)
    {
        MiniIotMessage::attachAppCallback(AppCallBack_);
    }

    // 主进程
    void loop();

    // 恢复出厂设置
    void RESET()
    {
        this->FS_Init();
        MiniIot_LOG_LN(F("[SYSTEM] 已恢复出厂设置，开始重启\n"));
        #ifdef ESP8266
            ESP.reset();
        #endif
        #ifdef ESP32
            ESP.restart();
        #endif
    }

    // 获取运行状态
    bool running()
    {
        return this->workstate == MINIIOT_WORK_STATE_WORKING ? true : false;
    }

    // 业务延时
    void delay(unsigned long ms)
    {
        uint32_t start = millis();
        while (millis() - start < ms)
        {
            if (this->workstate == MINIIOT_WORK_STATE_WORKING)
            {
                this->loop();
            }
            yield(); // 保持yield()以允许其他任务执行
        }
    }

    // 业务属性上报
    void propertyPost(String property_name, int property_value)
    {
        this->propertyPost(property_name, (String)property_value);
    }

    void propertyPost(String property_name, float property_value)
    {
        this->propertyPost(property_name, (String)property_value);
    }

    void propertyPost(String property_name, bool property_value)
    {
        this->propertyPost(property_name, (String)property_value);
    }
    
    void propertyPost(String property_name, String property_value)
    {
        static std::stringstream postData;
        postData.str("");
        postData << "{";
        postData << "\"id\" : \"" << MiniIotUtils::randomString(32).c_str() << "\", ";
        postData << "\"version\" : \"1.0\", ";
        postData << "\"method\" : \"property.post\", ";

        postData << "\"sys\" : { \"ack\" : 0, ";
        postData << "\"product\" : \"" << this->ProductId.c_str() << "\", ";
        postData << "\"device\" : \"" << this->DeviceId.c_str() << "\"";
        postData << " }, ";

        postData << "\"params\" : {";
        postData << "\"" << property_name.c_str() << "\" : {\"value\" : \"" << property_value.c_str() << "\"}";
        postData << "}";
        postData << "}";

        MiniIotClient.propertyPost(postData.str().c_str());
    }

    // 业务事件上报
    void eventPost(String event_name, JsonObject event_data)
    {
        // static std::stringstream postData;
        // postData.str("");
        // postData << "{";
        // postData << "\"id\" : \"" << MiniIotUtils::randomString(32).c_str() << "\", ";
        // postData << "\"version\" : \"1.0\", ";
        // postData << "\"method\" : \"event.post\", ";

        // postData << "\"sys\" : { \"ack\" : 0, ";
        // postData << "\"product\" : \"" << this->ProductId.c_str() << "\", ";
        // postData << "\"device\" : \"" << this->DeviceId.c_str() << "\"";
        // postData << " }, ";

        // postData << "\"params\" : {";
        // postData << "\"" << event_name.c_str() << "\" : " << event_data.as<String>();
        // postData << "}";
        // postData << "}";

        // MiniIotClient.eventPost(postData.str().c_str());
    }
};

// 主进程
void MiniIot::loop()
{
    #ifdef __UseAdminService__
        MiniIotAdminWebServerClient.loop();
    #endif

    static int serverErrNum = 0; // 服务器重连次数，使用static变量避免每次进入函数时重置

    switch (this->workstate)
    {
    case MINIIOT_WORK_STATE_INIT:                                // 初始化
        this->workstate = MINIIOT_WORK_STATE_NETWORK_CONNECTING; // 下一步，网络连接
        break;

    case MINIIOT_WORK_STATE_NETWORK_CONNECTING: // 网络连接
        #ifdef __UseWifiClient__
            if (MiniIotWifiObj.wifiConnect() == true)
            {
                this->workstate = MINIIOT_WORK_STATE_SERVER_CONNECTING; // 下一步，服务器连接
            }
            else
            {
                this->workstate = MINIIOT_WORK_STATE_NETWORK_ERROR; // 下一步，网络连接错误或断开
            }
        #endif
        break;

    case MINIIOT_WORK_STATE_SERVER_CONNECTING: // 服务器连接
        if (MiniIotClient.mqttConnect(MiniIotWifiObj.getWifiMac()) == true)
        {
            serverErrNum = 0;
            this->workstate = MINIIOT_WORK_STATE_WORKING; // 下一步，工作中
        }
        else
        {
            serverErrNum++;
            this->workstate = MINIIOT_WORK_STATE_SERVER_ERROR; // 下一步，服务器连接错误或断开
        }
        break;

    case MINIIOT_WORK_STATE_WORKING: // 工作中，随时监测状态
        MiniIotClient.loop();

        if (!MiniIotClient.connected())
        {
            #ifdef __UseWifiClient__
                if (MiniIotWifiObj.getStatus() != WL_CONNECTED)
                {
                    this->workstate = MINIIOT_WORK_STATE_NETWORK_ERROR; // 下一步，网络连接错误或断开
                }
                else
                {
                    this->workstate = MINIIOT_WORK_STATE_SERVER_ERROR; // 下一步，服务器连接错误或断开
                }
            #endif

            #ifdef __UseEthernetClient__
                this->workstate = MINIIOT_WORK_STATE_SERVER_ERROR; // 下一步，服务器连接错误或断开
            #endif
        }
        break;

    case MINIIOT_WORK_STATE_SERVER_ERROR: // 服务器连接错误或断开
        if (serverErrNum > 3)
        {
            delay(30000); // 30秒后重试
        }
        this->workstate = MINIIOT_WORK_STATE_SERVER_CONNECTING; // 下一步，服务器连接
        break;

    case MINIIOT_WORK_STATE_NETWORK_ERROR:                       // 网络连接错误或断开
        this->workstate = MINIIOT_WORK_STATE_NETWORK_CONNECTING; // 下一步，网络连接
        break;

    default:
        break;
    }
}

MiniIot MiniIot;

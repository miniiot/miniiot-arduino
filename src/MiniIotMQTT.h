#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "Config.h"
#include "Core/MiniIotUtils.h"

#ifdef __UseWifiClient__
    #ifdef ESP8266
        #include <ESP8266WiFi.h>
        #include <ESP8266HTTPClient.h>
    #endif

    #ifdef ESP32
        #include <WiFi.h>
        #include <HTTPClient.h>
    #endif
#endif

#ifdef __UseEthernetClient__
    #include <Ethernet.h>
    #include <EthernetClient.h>
    #include <EthernetHttpClient.h>
#endif


// MQTT
class MiniIotMQTT
{
private:
    String Version; // 业务程序版本号
    const String BinInfo = "{\"MiniIot_Version\":\"" + (String)MiniIot_VERSION + "\",\"App_Version\":\"" + (String)APP_VERSION + "\"}"; // 固件信息

    String ProductId;  // 产品ID
    String DeviceId;   // 设备ID
    String Secret;     // 密钥
    String SecretType; // 密钥类型

    String MqttUser;
    String MqttPassword;

    PubSubClient MqttClient;

#ifdef __UseWifiClient__
    WiFiClient MiniIotNetworkClient;
#endif
#ifdef __UseEthernetClient__
    EthernetClient MiniIotNetworkClient;
#endif

    // 获取网络时间
    String getNowDateTime()
    {

#ifdef __UseWifiClient__
        HTTPClient httpClient;

        String url = String("http://") + MiniIot_HTTP_HOST + String(":8880/miniiot/device/common/date_time");
        if (!httpClient.begin(this->MiniIotNetworkClient, url))
        {
            MiniIot_LOG_LN(F("[MQTT] 时间获取失败，HTTP连接失败"));
            return "2022-07-07 00:00:00;9527";
        }

        int httpCode = httpClient.GET();
        if (httpCode != HTTP_CODE_OK)
        {
            httpClient.end();
            MiniIot_LOG_LN("[MQTT] 时间获取失败，HTTP代码: " + String(httpCode));
            return "2022-07-07 00:00:00;9527";
        }

        String html = httpClient.getString();
        httpClient.end();
#endif

#ifdef __UseEthernetClient__
        EthernetHttpClient httpClient(this->MiniIotNetworkClient, MiniIot_HTTP_HOST, 8880);
        httpClient.beginRequest();
        httpClient.get("/miniiot/device/common/date_time");
        httpClient.endRequest();

        int statusCode = httpClient.responseStatusCode();
        if (statusCode != 200)
        {
            MiniIot_LOG_LN("[MQTT] 时间获取失败，HTTP代码: " + String(statusCode));
            return "2022-07-07 00:00:00;9527";
        }
        
        String html = httpClient.responseBody();
#endif


        MiniIot_LOG(F("[MQTT] "));
        MiniIot_LOG_LN(html);

        DynamicJsonDocument JSON_Buffer(html.length() + 20);
        if (deserializeJson(JSON_Buffer, html))
        {
            MiniIot_LOG_LN(F("[MQTT] 时间解析错误"));
            return "2022-07-07 00:00:00;9527";
        }

        JsonObject jsonData = JSON_Buffer.as<JsonObject>();
        if (!jsonData.containsKey("date_time") || !jsonData.containsKey("rand"))
        {
            MiniIot_LOG_LN(F("[MQTT] 时间解析错误，JSON缺少必要字段"));
            return "2022-07-07 00:00:00;9527";
        }

        String str = jsonData["date_time"].as<String>() + ";" + jsonData["rand"].as<String>();
        MiniIot_LOG_LN("[MQTT] 当前时间: " + str);
        return str;
    }

    // 通过错误码获取错误描述
    String getMqttErrCodeMsg(int state)
    {

        //      -4 : MQTT_ CONNECTION_ TIMEOUT - 服务器在保持活动时间内没有响应。
        //      -3 : MQTT_ CONNECTION_ LOST - 网络连接中断。
        //      -2 : MQTT_ CONNECT_ FAILED - 网络连接失败。
        //      -1 : MQTT_ DISCONNECTED - 客户端干净地断开连接。
        //      0 : MQTT_ CONNECTED - 客户端已连接。
        //      1 : MQTT_ CONNECT_ BAD_ PROTOCOL - 服务器不支持请求的MQTT版本。
        //      2 : MQTT_ CONNECT_ BAD_ CLIENT_ ID - 服务器拒绝了客户端标识符。
        //      3 : MQTT_ CONNECT_ UNAVAILABLE - 服务器无法接受连接。
        //      4 : MQTT_ CONNECT_ BAD_ CREDENTIALS - 用户名/密码被拒绝。
        //      5 : MQTT_ CONNECT_ UNAUTHORIZED - 客户端无权连接。

        switch (state)
        {
        case -4:
            return "服务器在保持活动时间内没有响应";
        case -3:
            return "网络连接中断";
        case -2:
            return "网络连接失败";
        case -1:
            return "客户端干净地断开连接";
        case 0:
            return "客户端已连接";
        case 1:
            return "服务器不支持请求的MQTT版本";
        case 2:
            return "服务器拒绝了客户端标识符";
        case 3:
            return "服务器无法接受连接";
        case 4:
            return "用户名/密码被拒绝";
        case 5:
            return "客户端无权连接";
        default:
            return "未知错误[" + String(state) + "]";
        }
    }

public:
#ifdef __UseWifiClient__
    MiniIotMQTT(WiFiClient &MiniIotNetworkClient_) : MqttClient(MiniIotNetworkClient_)
#endif
#ifdef __UseEthernetClient__
    MiniIotMQTT(EthernetClient &MiniIotNetworkClient_) : MqttClient(MiniIotNetworkClient_)
#endif
    {
        this->MiniIotNetworkClient = MiniIotNetworkClient_;
    }

    // 初始化
    void begin(String ProductId_, String DeviceId_, String Secret_, String SecretType_)
    {
        this->ProductId = ProductId_;
        this->DeviceId = DeviceId_;
        this->Secret = Secret_;
        this->SecretType = SecretType_; // 1:产品秘钥，2:设备秘钥
    }

    // 订阅主题
    void mqttSubscribe()
    {
        // 服务调用
        String topic = "sys/" + this->ProductId + "/" + this->DeviceId + "/service";
        if (MqttClient.subscribe(topic.c_str()))
        {
            MiniIot_LOG_LN("[MQTT] 主题订阅成功【" + topic + "】");
        }
        else
        {
            MiniIot_LOG_LN("[MQTT] 主题订阅失败【" + topic + "】");
        }
    }

    // 连接MQTT服务器
    bool mqttConnect(String mac)
    {
        // 判断MiniIot_MQTT_HOST是否是域名还是ip地址
        #ifdef MiniIot_MQTT_HOST_IS_IP
            const String mqttHost = MiniIot_MQTT_HOST; // MQTT服务器地址，直接使用IP地址
            MiniIot_LOG(F("[MQTT] HOST:"));
            MiniIot_LOG_LN(F(MiniIot_MQTT_HOST));
        #else
            const String mqttHost = this->ProductId + "." + MiniIot_MQTT_HOST; // MQTT服务器地址是域名，拼接产品ID
            MiniIot_LOG(F("[MQTT] HOST:"));
            MiniIot_LOG_LN(mqttHost);
        #endif

        const String mqttClientId = this->ProductId + "_" + this->DeviceId; // MQTT客户端ID
        MqttClient.setBufferSize(512);                                     // 设置MQTT缓冲区大小
        MqttClient.setKeepAlive(MiniIot_MQTT_KeepAlive);                                      // 设置MQTT心跳间隔
        
        MqttClient.setServer(mqttHost.c_str(), MiniIot_MQTT_PORT);
        MqttClient.setCallback(MiniIotMessage::handleMessage);

        this->MqttUser = this->ProductId + ";" + this->DeviceId + ";" + mac + ";" + this->SecretType + ";1;" + this->getNowDateTime() + ";" + this->BinInfo;
        this->MqttPassword = MiniIotUtils::ESPsha1(this->MqttUser + ";天才小坑Bi-<admin@dgwht.com>;" + this->Secret);

        MiniIot_LOG_LN(F("[MQTT] MQTT连接中..."));
        if (MqttClient.connect(mqttClientId.c_str(), this->MqttUser.c_str(), this->MqttPassword.c_str()))
        {
            MiniIot_LOG_LN(F("[MQTT] MQTT连接成功"));
            this->mqttSubscribe();// 订阅主题
            
            return true; // 连接成功
        }
        MiniIot_LOG(F("[MQTT] MQTT连接失败："));
        MiniIot_LOG_LN(this->getMqttErrCodeMsg(MqttClient.state()));
        
        return false; // 连接失败
    }

    int state()
    {
        return MqttClient.state();
    }

    bool connected()
    {
        if (!MqttClient.connected())
        {
            MiniIot_LOG_LN("[MQTT] MQTT连接断开：" + this->getMqttErrCodeMsg(MqttClient.state()));
            return false;
        }
        return true;
    }

    void disconnect()
    {
        MqttClient.disconnect();
    }

    // 属性上报
    void propertyPost(String postData)
    {
        MiniIot_LOG_LN(F("[MQTT] 属性上报："));
        MiniIot_LOG_LN(postData);

        String topic = "sys/" + this->ProductId + "/" + this->DeviceId + "/property";
        if (MqttClient.publish(topic.c_str(), postData.c_str()))
        {
            MiniIot_LOG_LN(F("[MQTT] 属性上报成功"));
        }
        else
        {
            MiniIot_LOG_LN(F("[MQTT] 属性上报失败"));
        }
    }

    // 事件上报
    void eventPost(String cmdData)
    {
        MiniIot_LOG_LN(F("[MQTT] 事件上报："));
        MiniIot_LOG_LN(cmdData);

        String topic = "sys/" + this->ProductId + "/" + this->DeviceId + "/event";
        if (MqttClient.publish(topic.c_str(), cmdData.c_str()))
        {
            MiniIot_LOG_LN(F("[MQTT] 事件上报成功"));
        }
        else
        {
            MiniIot_LOG_LN(F("[MQTT] 事件上报失败"));
        }
    }


    void loop()
    {
        MqttClient.loop();
    }

};
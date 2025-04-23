#pragma once

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif

class MiniIotAP
{
    String SSID;
    String PASSWD = "88888888";
    int isHidden = 0;

    void init()
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAPConfig(IPAddress(192, 168, 101, 1), IPAddress(192, 168, 101, 1), IPAddress(255, 255, 255, 0)); // 设置AP网络参数
    }

    void begin(String SSID_, String PASSWD_, int isHidden_)
    {
        // 设置AP账号、密码、信道、是否隐藏、允许连接数
        WiFi.softAP(SSID_, PASSWD_, 1, isHidden_, 3);

        Serial.print(F("[Admin] AP已开启, SSID: "));
        Serial.print(SSID_);
        Serial.println(F(", IP: 192.168.101.1"));
    }
public:
    MiniIotAP(String deviceName)
    {
        SSID = "MiniIot." + deviceName;
        init();
        begin(SSID, PASSWD, isHidden);
    }

};
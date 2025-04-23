#pragma once
#include <Arduino.h>

#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266httpUpdate.h>
#endif

#ifdef ESP32
    #include <WiFi.h>
    #include <HTTPUpdate.h>
#endif

class MiniIotOTA
{
    public:
        // 固件升级
        static void updateBin(String upUrl)
        {
            WiFiClient UpdateClient;
            #ifdef ESP8266
                ESPhttpUpdate.onStart(MiniIotOTA::update_started);     //当升级开始时
                ESPhttpUpdate.onEnd(MiniIotOTA::update_finished);      //当升级结束时
                ESPhttpUpdate.onProgress(MiniIotOTA::update_progress); //当升级中
                ESPhttpUpdate.onError(MiniIotOTA::update_error);       //当升级失败时
                ESPhttpUpdate.update(UpdateClient, upUrl);
            #endif

            #ifdef ESP32
                httpUpdate.onStart(MiniIotOTA::update_started);     //当升级开始时
                httpUpdate.onEnd(MiniIotOTA::update_finished);      //当升级结束时
                httpUpdate.onProgress(MiniIotOTA::update_progress); //当升级中
                httpUpdate.onError(MiniIotOTA::update_error);       //当升级失败时
                httpUpdate.update(UpdateClient, upUrl);
            #endif
        }

    private:
        String url;

        //当升级开始时，打印日志
        static void update_started()
        {
            Serial.println(F("[固件升级]开始升级"));
        }

        //当升级结束时，打印日志
        static void update_finished()
        {
            Serial.println();
            Serial.println(F("[固件升级]升级成功"));
        }

        //当升级中，打印日志
        static void update_progress(int cur, int total)
        {
            Serial.print(F("[固件升级]升级进度："));
            Serial.print((cur * 100) / total);
            Serial.println("%");
        }

        //当升级失败时，打印日志
        static void update_error(int err)
        {
            Serial.println();
            Serial.printf("[固件升级]升级失败：%d\n", err);
        }
};
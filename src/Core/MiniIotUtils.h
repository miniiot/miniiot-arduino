#pragma once
#include <Arduino.h>
#include <cstdlib> // for rand()

#ifdef ESP8266
#include <Hash.h>
#endif

#ifdef ESP32
#include <SHA1Builder.h>
#endif

class MiniIotUtils
{
public:
    // sha1
    static String ESPsha1(String str)
    {
#ifdef ESP8266
        return sha1(str);
#endif

#ifdef ESP32
        SHA1Builder sha;
        sha.begin();
        sha.add(str);
        sha.calculate();
        return sha.toString();
#endif
    }

    // 获取芯片ID
    static String ESPchipId()
    {
#ifdef ESP8266
        return (String)ESP.getChipId();
#endif

#ifdef ESP32
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        return (String)chipId;
#endif
    }

    // 获取指定长度随机字符串
    static String randomString(int length)
    {
        String s;
        for (int i = 0; i < length; i++)
        {
            s += (char)(rand() % 26 + 'a');
        }
        return s;
    }

    // bool isIP(String ip)
    // {
    //     char *str = new char[ip.length() + 1];
    //     strcpy(str, ip.c_str());

    //     int a,b,c,d;
    //     char temp[255];
    //     if((sscanf(str,"%d.%d.%d.%d",&a,&b,&c,&d))!=4){
    //         return false;
    //     }

    //     sprintf(temp,"%d.%d.%d.%d",a,b,c,d);
    //     if(strcmp(temp,str) != 0){
    //         return false;
    //     }

    //     if(!((a <= 255 && a >= 0)&&(b <= 255 && b >= 0)&&(c <= 255 && c >= 0)&&(d <= 255 && d >= 0))){
    //         return false;
    //     }

    //     return true;
    // }
    // private:
};
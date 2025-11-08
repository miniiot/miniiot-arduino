#pragma once

// MiniIot库文件版本
#define MiniIot_VERSION "miniiot_v2.3.1_251107"

#ifndef APP_VERSION
    #define APP_VERSION "0.0.1"
#endif

/*
=======v1.0.1_250423=======
-   发布版本：1.0.1。

=======v1.0.5_250609=======
-   新增：属性批量上报，但所有的key+value之和不能超过40个字符。

=======v1.1.0_250616=======
-   优化：优化wifi与mqtt连接逻辑，避免阻塞主线程。
-   变更：wifi默认账号改为 miniiot.top
-   变更：wifi默认密码改为 88888888

=======v2.0.1_250716=======
-   新增：尝试适配w5500以太网模块。
-   变更：修改mqtt密码计算盐值。v2.0.1以前版本无法上线。

=======v2.1.0_250730=======
-   优化：优化wifi配置，LittleFS 初始化失败时，使用默认配置。
-   新增：ESP8266、ESP32C3、ESP32S3已适配w5500以太网模块。ESP32C3 Flash大小为4MB时，Partition Scheme需要选择【No OTA (2MB APP/2MB SPIFFS)】

=======v2.2.1_250823=======
-   优化：调整以太网初始化位置。
-   修复：调整以太网mac地址生成方式，避免全为数字时，部分路由器无法分配IP地址的问题。

=======v2.3.1_251107=======
-   新增：兼容STM32F1平台。
-   优化：修改以太网模块默认复位控制IO。
-   优化：可自定义打印日志的串口。#define MiniIotDebugSerial Serial
*/

// MiniIot状态指示灯IO
#ifndef MiniIot_STATE_LED
    #ifdef LED_BUILTIN
        #define MiniIot_STATE_LED LED_BUILTIN
    #endif
#endif

// 系统管理服务端口
#ifndef MiniIot_ADMIN_SERVICE_PORT
    #define MiniIot_ADMIN_SERVICE_PORT 10101
#endif

// 主进程工作状态
#define MINIIOT_WORK_STATE_INIT 100
#define MINIIOT_WORK_STATE_NETWORK_CONNECTING 101
#define MINIIOT_WORK_STATE_SERVER_CONNECTING 102
#define MINIIOT_WORK_STATE_WORKING 103
#define MINIIOT_WORK_STATE_SERVER_ERROR 104
#define MINIIOT_WORK_STATE_NETWORK_ERROR 105

// HTTP域名
#ifndef MiniIot_HTTP_HOST
    #define MiniIot_HTTP_HOST "service.miniiot.top"
#endif

// MQTT域名
#ifndef MiniIot_MQTT_HOST
    #define MiniIot_MQTT_HOST "mqtt.miniiot.top"
#endif

// MQTT端口
#ifndef MiniIot_MQTT_PORT
    #define MiniIot_MQTT_PORT 2082
#endif

// 默认使用wifi客户端，优先使用网口以太网模块
#ifdef MiniIot_USE_ETH
    #define __UseEthernetClient__
#else
    #define __UseWifiClient__
#endif

// 以太网模块复位控制IO
#ifndef MiniIot_ETH_RST
    #ifdef ESP8266
        #define MiniIot_ETH_RST 5
    #endif
    #ifdef ESP32
        #define MiniIot_ETH_RST 2
    #endif
    #ifdef STM32F1
        #define MiniIot_ETH_RST PE9
    #endif
#endif

// 默认使用MQTT客户端
#define __UseMqttClient__
// mqtt客户端心跳时间
#ifndef MiniIot_MQTT_KeepAlive
    #ifdef MiniIot_USE_ETH
        #define MiniIot_MQTT_KeepAlive 5
    #else
        #define MiniIot_MQTT_KeepAlive 30
    #endif
#endif

// 开启后台管理服务
#ifdef MiniIot_Admin_Service
    #define __UseAdminService__
#endif

// 日志打印
#ifdef MiniIot_DEBUG_LOG
    #ifndef MiniIotDebugSerial
        #define MiniIotDebugSerial Serial
    #endif
    #define MiniIot_LOG(message) MiniIotDebugSerial.print(message)
    #define MiniIot_LOG_LN(message) MiniIotDebugSerial.println(message)
#else
    #define MiniIot_LOG(message)
    #define MiniIot_LOG_LN(message)
#endif

// 一个JSON对象参数的回调函数
typedef void (*JsonObjectCallbackFunction)(JsonObject dataObj);

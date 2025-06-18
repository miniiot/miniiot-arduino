#pragma once

// MiniIot库文件版本
#define MiniIot_VERSION "miniiot_v1.1.0_250616"

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
*/

// MiniIot状态指示灯IO
#ifndef MiniIot_STATE_LED
    #define MiniIot_STATE_LED LED_BUILTIN
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

// 默认使用wifi客户端、MQTT客户端
#define __UseWifiClient__
#define __UseMqttClient__

// 开启后台管理服务
#ifdef MiniIot_Admin_Service
    #define __UseAdminService__
#endif

// 日志打印
#ifdef MiniIot_DEBUG_LOG
    #define MiniIot_LOG(message) Serial.print(message)
    #define MiniIot_LOG_LN(message) Serial.println(message)
#else
    #define MiniIot_LOG(message)
    #define MiniIot_LOG_LN(message)
#endif

// 一个JSON对象参数的回调函数
typedef void (*JsonObjectCallbackFunction)(JsonObject dataObj);

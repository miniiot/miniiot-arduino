#pragma once

class MiniIotMessage
{

public:
    static JsonObjectCallbackFunction AppCallBack;
    static JsonObjectCallbackFunction SysCallBack;

    static void attachAppCallback(JsonObjectCallbackFunction callback)
    {
        MiniIotMessage::AppCallBack = callback;
    }

    static void attachSysCallback(JsonObjectCallbackFunction callback)
    {
        MiniIotMessage::SysCallBack = callback;
    }

    // 收到服务器消息（回调）
    static void handleMessage(String topicStr, String msg)
    {
        MiniIot_LOG_LN("[MQTT] 收到消息：\n主题: " + topicStr + "\n内容：" + msg);

        DynamicJsonDocument JSON_Buffer(msg.length() + 20);
        DeserializationError error = deserializeJson(JSON_Buffer, msg);
        if (error)
        {
            MiniIot_LOG_LN("[MQTT] JSON解析错误: " + String(error.c_str()));
            return;
        }

        JsonObject msgData = JSON_Buffer.as<JsonObject>();
        if (!msgData.containsKey("id") || !msgData.containsKey("version") || !msgData.containsKey("method") || !msgData.containsKey("params"))
        {
            MiniIot_LOG_LN(F("[MQTT] JSON缺少必要字段"));
            return;
        }

        if (msgData["version"] != "1.0")
        {
            MiniIot_LOG_LN(F("[MQTT] 协议版本不支持"));
            return;
        }

        if (msgData["method"] == "service.control.sys")
        {
            // 服务调用-系统服务
            MiniIotMessage::SysCallBack(msgData["params"]);
        }
        else if (msgData["method"] == "service.control")
        {
            // 服务调用-自定义服务
            MiniIotMessage::AppCallBack(msgData["params"]);
        }
        else
        {
            MiniIot_LOG_LN(F("[MQTT] 未知method"));
        }
    }

    // 收到服务器消息（回调）
    static void handleMqttMessage(char *topic, byte *payload, unsigned int length)
    {
        String topicStr = topic;

        // 创建一个临时的char数组来存储payload内容
        char *payloadStr = new char[length + 1];
        memcpy(payloadStr, payload, length);
        payloadStr[length] = '\0'; // 确保字符串以null结尾

        String msg = payloadStr;
        delete[] payloadStr; // 释放临时数组
        MiniIotMessage::handleMessage(topicStr, msg);
    }
};

JsonObjectCallbackFunction MiniIotMessage::AppCallBack = nullptr;
JsonObjectCallbackFunction MiniIotMessage::SysCallBack = nullptr;
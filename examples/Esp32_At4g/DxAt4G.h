#pragma once

class DxAt4G
{
private:
    Stream *AT4GSerial;
    MiniIotSystemInfo_t *SystemInfo;

    String topic;
    String message;
    int length;// 数据长度
    int remain_length;// 剩余数据长度
    
    // 指令，回显关键字，超时时间
    bool AT_CMD(String data, String keyword, int num)
    {
        MiniIot_LOG("[DxAt4g] 发送：");
        MiniIot_LOG_LN(data);

        // 清空接收缓冲区
        this->AT4GSerial->flush();
        this->AT4GSerial->println(data.c_str());
        
        uint32_t start_time = millis();
        while (millis() - start_time < num)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            if (this->AT4GSerial->available())
            {
                String ret = this->AT4GSerial->readStringUntil('\n');
                if (ret.indexOf(keyword) != -1)
                {
                    this->AT4GSerial->flush();
                    return true;
                }
            }
        }

        return false;
    }

    // 指令
    void AT_CMD(String data)
    {
        MiniIot_LOG("[DxAt4g] 发送：");
        MiniIot_LOG_LN(data);

        this->AT4GSerial->println(data.c_str());
    }

    // http get 请求
    bool httpGet(String url, String port, String *retData){
       
        url = String("AT$HTTPPARA=") + url + String(",") + port;

        if (!this->AT_CMD("AT$HTTPOPEN", "OK", 2000))
        {
            return false;
        }
        if (!this->AT_CMD(url, "OK", 2000))
        {
            return false;
        }
        if (!this->AT_CMD("AT$HTTPACTION=0", "OK", 2000))
        {
            return false;
        }

        // 清空接收缓冲区
        this->AT4GSerial->flush();
        this->AT4GSerial->println("AT$HTTPACTION=0");

        String html;
        uint32_t start_time = millis();
        while (millis() - start_time < 2000)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            int available = this->AT4GSerial->available();
            if (available > 0)
            {
                html = this->AT4GSerial->readString();

                this->AT4GSerial->flush();
                int index1 = html.indexOf("{");
                html = html.substring(index1);
                int index2 = html.indexOf("}");
                html = html.substring(0, index2 + 1);
                if (index1 == -1 || index2 == -1)
                {
                    return false;
                }

                if (!this->AT_CMD("AT$HTTPCLOSE", "OK", 2000))
                {
                    return false;
                }
                *retData = html;
                return true;
            }
        }
        return false;
    }

    // 解析时间
    bool parseDateTime(String html, String *retData){
        // MiniIot_LOG(F("[DxAt4g] "));
        // MiniIot_LOG_LN(html);

        DynamicJsonDocument JSON_Buffer(html.length() + 20);
        if (deserializeJson(JSON_Buffer, html))
        {
            MiniIot_LOG_LN(F("[DxAt4g] 时间解析错误"));
            return false;
        }

        JsonObject jsonData = JSON_Buffer.as<JsonObject>();
        if (!jsonData.containsKey("date_time") || !jsonData.containsKey("rand"))
        {
            MiniIot_LOG_LN(F("[DxAt4g] 时间解析错误，JSON缺少必要字段"));
            return false;
        }

        String str = jsonData["date_time"].as<String>() + ";" + jsonData["rand"].as<String>();
        MiniIot_LOG_LN("[DxAt4g] 当前时间: " + str);
        *retData = str;
        return true;
    }

    // 获取网络时间
    bool getNowDateTime(String *retData)
    {
        String html;
        String url = String("http://") + MiniIot_HTTP_HOST + String("/miniiot/device/common/date_time");
        if (!this->httpGet(url, "8880", &html))
        {
            return false;
        }
        return parseDateTime(html, retData);
    }

    // 获取到完整消息
    void OK(String *topic, String *message){
        // 截取到最后一个}
        int f = this->message.lastIndexOf("}");
        this->message = this->message.substring(0, f+1);

        *topic = this->topic;
        *message = this->message;
    }
public:
    DxAt4G(Stream *AT4GSerial)
    {
        this->AT4GSerial = AT4GSerial;
    }

    void setSystemInfo(MiniIotSystemInfo_t *SystemInfo)
    {
        this->SystemInfo = SystemInfo;
    }

    // 初始化模块
    bool init(){
        // 重启模块
        MiniIot_LOG_LN("[DxAt4g] 重启4G模块");
        this->AT_CMD("AT+RESET\r\n", "OK\r\n", 1500);

        // 取消回显
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        MiniIot_LOG_LN("[DxAt4g] 取消回显");
        this->AT_CMD("ATE0\r\n", "OK\r\n", 1500);

        // 判断网络是否关闭
        MiniIot_LOG_LN("[DxAt4g] 判断网络是否关闭");
        if (this->AT_CMD("AT+NETOPEN?\r\n", "NETOPEN:0", 3000))
        {
            // 打开网络
            MiniIot_LOG_LN("[DxAt4g] 打开网络");
            if(!this->AT_CMD("AT+NETOPEN\r\n", "NETOPEN:SUCCESS", 2000)){
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                if(!this->AT_CMD("AT+NETOPEN\r\n", "NETOPEN:SUCCESS", 2000)){
                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                    if(!this->AT_CMD("AT+NETOPEN\r\n", "902", 2000)){
                        return false;
                    }
                }
            }
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        return true;
    }
    
    // 连接MQTT服务器
    bool connect()
    {
        // 判断MiniIot_MQTT_HOST是否是域名还是ip地址
        #ifdef MiniIot_MQTT_HOST_IS_IP
            const String mqttHost = MiniIot_MQTT_HOST; // MQTT服务器地址，直接使用IP地址
            MiniIot_LOG(F("[DxAt4g] HOST:"));
            MiniIot_LOG_LN(F(MiniIot_MQTT_HOST));
        #else
            const String mqttHost = this->SystemInfo->ProductId + "." + MiniIot_MQTT_HOST; // MQTT服务器地址是域名，拼接产品ID
            MiniIot_LOG(F("[DxAt4g] HOST:"));
            MiniIot_LOG_LN(mqttHost);
        #endif


        // 获取当前时间
        String datetime;
        if (!this->getNowDateTime(&datetime))
        {
            return false;
        }
        const String mqttClientId = this->SystemInfo->ProductId + "_" + this->SystemInfo->DeviceId; // MQTT客户端ID
        const String MqttUser = this->SystemInfo->ProductId + ";" + this->SystemInfo->DeviceId + ";" + this->SystemInfo->Mac + ";" + this->SystemInfo->SecretType + ";1;" + datetime + ";" + this->SystemInfo->BinInfo;
        const String MqttPassword = MiniIotUtils::ESPsha1(MqttUser + ";天才小坑Bi-<admin@dgwht.com>;" + this->SystemInfo->Secret);


        // 配置 MQTT 客户端所需的客户端 ID、用户名、密码、遗嘱开关
        while (!this->AT_CMD("AT+MCONFIG=\""+mqttClientId+"\",\""+MqttUser+"\",\""+MqttPassword+"\",0\r\n", "OK", 3000));

        // 配置MQTT 服务器 端口号 版本号
        String MQTT_SERVICE = String("AT+MIPSTART=\"") + mqttHost + String("\",") + String(MiniIot_MQTT_PORT) + String(",4\r\n");
        while (!this->AT_CMD(MQTT_SERVICE.c_str(), "MIPSTART: SUCCESS", 3000));

        // 开始连接，临时会话，心跳间隔
        this->AT_CMD("AT+MCONNECT=1,30\r\n", "MCONNECT: SUCCESS", 3000);

        String topic = "sys/" + this->SystemInfo->ProductId + "/" + this->SystemInfo->DeviceId + "/service";
        // // 订阅主题, 主题名 QOS服务质量
        this->AT_CMD("AT+MSUB=\""+topic+"\",0\r\n", "MSUB: SUCCESS", 3000);
        
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return true;
    }

    // 关闭MQTT连接
    void disconnect()
    {
        this->AT_CMD("AT+MDISCONNECT\r\n");
    }
    
    // 检测MQTT状态
    void checkMqttStatus(){
        this->AT_CMD("AT+MQTTSTATU\r\n");
    }

    // 发布消息
    void pushData(String topic, String message)
    {
        // 主题名 QOS 是否保留 内容
        String data = "AT+MPUB=\"" + topic + "\",0,0,\"" + message + "\"\r\n";
        this->AT_CMD(data);
    }
    // 发布长消息（或包含特殊字符、逗号等的消息）
    void pushDataLong(String topic, String message)
    {
        // 主题名 QOS 是否保留 内容 长度
        String data = "AT+MPUBEX=\"" + topic + "\",0,0," + message.length() + "\r\n";
        this->AT_CMD(data);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        this->AT_CMD(message);
    }

    // 解析数据
    int ParseData(String data, String *retTopic, String *retMessage)
    {
        if (data.startsWith("+MSUB:"))
        {
            int f1 = data.indexOf('"');         // 第1个引号的位置
            int s1 = data.indexOf('"', f1 + 1); // 第2个引号的位置
            this->topic = data.substring(f1 + 1, s1);

            int f2 = data.indexOf(',', s1 + 1); // 第1个逗号的位置
            int s2 = data.indexOf('b', f2 + 1);
            this->length = data.substring(f2 + 1, s2 - 1).toInt();

            int f3 = data.indexOf(',', f2 + 1); // 第2个逗号的位置

            // 解析数据
            this->message = data.substring(f3 + 2);
            this->message += "\n";
            // 剩余数据长度
            this->remain_length = this->length - this->message.length() + 2;
            if(this->remain_length <= 0){
                this->remain_length = 0;
                this->OK(retTopic, retMessage);
            }

            return this->remain_length;
        }else{
            this->message = this->message + data + "\n";
            this->remain_length = this->length - this->message.length() + 2;
            if(this->remain_length <= 0){
                this->remain_length = 0;
                this->OK(retTopic, retMessage);
            }

            return this->remain_length;
        }
    }

    // 开启GPS
    void openGPS(){
        this->AT_CMD("AT+MGPSC=1\r\n");
    }

    // 关闭GPS
    void closeGPS(){
        this->AT_CMD("AT+MGPSC=0\r\n");
    }



};
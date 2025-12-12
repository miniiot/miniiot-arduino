
// 当前程序版本（仅用于后台展示）
#define APP_VERSION "esp32_at4g_2512121708"


// 打印日志（建议调试完成后注释掉）
#define MiniIot_DEBUG_LOG

HardwareSerial DebugSerial(0); // 串口1
HardwareSerial AT4GSerial(1);  // 串口2
HardwareSerial TjcSerial(2);    // 串口3


// 自定义日志输出串口
#define MiniIotDebugSerial DebugSerial

// 导入miniiot
#include <MiniIot.h>

#include "DxAt4G.h"
#include "GPS_info.h"
#include "GPS_time.h"

bool isUseAt4g = false;
DxAt4G At4gObj(&AT4GSerial);

// GPS数据结构体
struct GPS_DATA_t GPS_INFO;
GpsInfo gpsInfoObj;

// =============================

void addNum(){
    static int test_num = 0;
    // 上报属性
    MiniIot.propertyPost("test", test_num++);
}

// 业务回调函数
void ServiceCallbackFunction(JsonObject dataObj)
{
    // 所有参数默认都是以String类型返回，需要自行转换成目标类型
    // json库使用ArduinoJson
    String serviceName = dataObj["serviceName"].as<String>();
    
    // Ping
    if(serviceName == "miniiot_ping"){
        addNum();
    }

    if(serviceName == "openGPS"){
        At4gObj.openGPS();
    }

    if(serviceName == "closeGPS"){
        At4gObj.closeGPS();
    }

    if(serviceName == "stop4G"){
        isUseAt4g = false;
    }

    if(serviceName == "open4G"){
        isUseAt4g = true;
    }

    MiniIotDebugSerial.printf("剩余内存:%d KB\n", ESP.getFreeHeap() / 1024);
    MiniIotDebugSerial.printf("剩余栈:%d B\n", uxTaskGetStackHighWaterMark(NULL));
}

// =============================

// MiniIot任务
void Task_miniiot(void *pvParameters __attribute__((unused)))
{
  for (;;)
  {
    MiniIot.loop();
	vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// 输出GPS数据到串口屏
void printGpsBuffer(struct GPS_DATA_t *GPS_INFO)
{
	time_t beijingTime = utcToBeijingTime(GPS_INFO->GPS_date + GPS_INFO->GPS_time);

	TjcSerial.print("gps.t0.txt=\"" + formatDate(beijingTime) + "\"\xff\xff\xff");
	TjcSerial.print("gps.t1.txt=\"" + formatTime(beijingTime) + "\"\xff\xff\xff");

	TjcSerial.print("gps.t2.txt=\"" + GPS_INFO->GPS_longitude + "," + GPS_INFO->GPS_longitude_EW + "\"\xff\xff\xff");
	TjcSerial.print("gps.t3.txt=\"" + GPS_INFO->GPS_latitude + "," + GPS_INFO->GPS_latitude_NS + "\"\xff\xff\xff");

	TjcSerial.print("gps.t9.txt=\"" + GPS_INFO->GPS_altitude + " m" + "\"\xff\xff\xff");
	TjcSerial.print("gps.t10.txt=\"" + GPS_INFO->GPS_satellite + "颗" + "\"\xff\xff\xff");

	TjcSerial.print("gps.t4.txt=\"" + GPS_INFO->GPS_course + "°" + "\"\xff\xff\xff");
	TjcSerial.print("gps.t5.txt=\"" + GPS_INFO->GPS_speed + "km/h" + "\"\xff\xff\xff");

    static uint32_t last_up_time = 0;
    if(millis() - last_up_time > 1000 * 10)
    {
        String up_data = GPS_INFO->GPS_longitude + String(",") + GPS_INFO->GPS_longitude_EW + String("|") + GPS_INFO->GPS_latitude + String(",") + GPS_INFO->GPS_latitude_NS;
        MiniIot.propertyPost("GPS_info", up_data);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        addNum();
        last_up_time = millis();
    }
}

// 串口屏任务
void Task_TJC(void *pvParameters __attribute__((unused)))
{
    for (;;)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (gpsInfoObj.isValid(&GPS_INFO))
        {
            printGpsBuffer(&GPS_INFO);
        }
    }
}

// 数据上报回调函数
void At4gPushCallbackFunction(String topic, String data){
    At4gObj.pushDataLong(topic, data);
}

// At4g任务
void Task_DxAt4g(void *pvParameters __attribute__((unused)))
{
    // 设置系统信息
    At4gObj.setSystemInfo(MiniIot.getSystemInfo());

    bool at4gStatus = false;// 4G模块MQTT连接状态
    uint32_t last_at4g_time = millis();// 上次检测MQTT状态时间

    String data;
    int length = 0;
    String mqttTopic;
    String mqttData;

    for (;;)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);

        // 开启4G
        if(isUseAt4g && !at4gStatus){
            at4gStatus = true;
            // 绑定数据上报回调函数
            MiniIot.attachDataPushService(At4gPushCallbackFunction);
            // 断开MQTT连接
            MiniIot.disconnect();
            // 初始化4G模块
            At4gObj.init();
            // 连接MQTT
            At4gObj.connect();
            // 打开GPS
            // At4gObj.openGPS();
            last_at4g_time = millis();
            continue;
        }
        // 关闭4G
        if(!isUseAt4g &&  at4gStatus){
            at4gStatus = false;
            // 注销数据上报回调函数
            MiniIot.deregisterDataPushService();
            // 断开MQTT连接
            At4gObj.disconnect();
            continue;
        }
        // 4G模块MQTT连接状态检测
        if(at4gStatus){
            if(millis() - last_at4g_time > 1000 * 10){
                last_at4g_time = millis();
                At4gObj.checkMqttStatus();
            }
        }

        // 串口数据处理
        if(AT4GSerial.available()){
            data = AT4GSerial.readStringUntil('\n');

            // MQTT消息
            if(data.startsWith("+MSUB:") || length > 0){
                length = At4gObj.ParseData(data, &mqttTopic, &mqttData);
                if(length == 0){
                    MiniIot.dataPull(mqttTopic, mqttData);
                }
                continue;
            }
            // MQTT断开连接
            if(data.indexOf("+MQTTSTATU: 0") != -1){
                at4gStatus = false;
                continue;
            }

            // GPS数据
            if (data.startsWith("$"))
            {
                // MiniIotDebugSerial.println(data);
                gpsInfoObj.parseBuffer(data, &GPS_INFO);
                continue;
            }

            // 其他数据
            // MiniIotDebugSerial.println(data);
        }
    }
}

// =============================

void setup()
{
    MiniIotDebugSerial.begin(115200);
    MiniIotDebugSerial.println();
    MiniIotDebugSerial.println();

    AT4GSerial.begin(115200, SERIAL_8N1, 19, 20);
    TjcSerial.begin(19200, SERIAL_8N1, 40, 41);
    
    MiniIotDebugSerial.println("==================================");
    MiniIotDebugSerial.printf("cpu运行频率:%d MHz\n", ESP.getCpuFreqMHz());
    MiniIotDebugSerial.printf("当前固件大小:%d KB\n", ESP.getSketchSize() / 1024);
    MiniIotDebugSerial.printf("剩余固件空间:%d KB\n", ESP.getFreeSketchSpace() / 1024);
    MiniIotDebugSerial.printf("剩余内存:%d KB\n", ESP.getFreeHeap() / 1024);
    MiniIotDebugSerial.println("==================================");

    // 使用产品密钥初始化MiniIot，（产品ID，产品密钥）
    // 设备不存在会自动注册添加设备
    MiniIot.begin("agvuKw1U", "nPJbUChwlcINFSEP");

    // 绑定业务回调函数
    MiniIot.attach(ServiceCallbackFunction);
    
    // 任务函数，任务名称，栈大小，参数，优先级(0最低)，句柄,核心
    xTaskCreatePinnedToCore(Task_miniiot, "Task_miniiot", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Task_DxAt4g, "Task_DxAt4g", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(Task_TJC, "Task_TJC", 4096, NULL, 1, NULL, 0);
}

void loop()
{
}

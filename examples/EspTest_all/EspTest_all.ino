// 登录控制台与查看文档请访问官网：http://www.miniiot.top

// 当前程序版本（仅用于后台展示）
#define APP_VERSION "all_2506091055"

// 默认WIFI配置（出厂设置）
#define DEFAULT_WIFI_SSID "Tenda_375160章"
#define DEFAULT_WIFI_PASSWORD "87472998"

// 使用ip测试地址（预留私有部署与调试使用，不必理会）
// #define MiniIot_MQTT_HOST_IS_IP
// #define MiniIot_MQTT_HOST "192.168.0.192"
// #define MiniIot_HTTP_HOST "192.168.0.192"

// 后台服务（用于上位机调试，开发测试中，建议关闭）
// #define MiniIot_Admin_Service

// 打印日志（建议调试完成后注释掉）
#define MiniIot_DEBUG_LOG

// 导入miniiot
#include <MiniIot.h>
// 定时任务（与miniiot无关，用于实现长按恢复出厂设置）
#include <Ticker.h>

// 复用Boot按键用于恢复出厂设置
#ifdef ESP8266
    #define SYS_RST_IO 0
#endif

#ifdef ESP32
    #define SYS_RST_IO 9
#endif

// 恢复出厂设置计数(秒)
int sys_rst_count = 0;

int num = 0;

Ticker TickerRST;


// 按5秒开发板上的按钮恢复出厂设置
void SysRstfun()
{
    if (digitalRead(SYS_RST_IO) == LOW)
    {
        sys_rst_count++;
    }
    else
    {
        sys_rst_count = 0;
    }
    if (sys_rst_count >= 5)
    {
        // 格式化存储
        MiniIot.RESET();
    }
}


// 批量上报属性
void test_1(){
    // 所有的key+value之和不要超过40个字符，超过请分批上报
    JSONVar myObject;
    myObject["switch_1"] = "1";
    myObject["switch_2"] = "2";
    myObject["switch_3"] = (String)num;

    MiniIot.propertyPost(myObject);
}

void addNum(){
    // 上报属性
    MiniIot.propertyPost("num_1", num);
    num++;
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

    // 测试服务(解析参数)
    if(serviceName == "test"){
        Serial.print("参数a1：");Serial.println(dataObj["serviceParams"]["a1"].as<String>());
        Serial.print("参数a2：");Serial.println(dataObj["serviceParams"]["a2"].as<String>());
        Serial.print("参数a3：");Serial.println(dataObj["serviceParams"]["a3"].as<String>());
        Serial.print("参数a4：");Serial.println(dataObj["serviceParams"]["a4"].as<String>());
        Serial.print("参数a5：");Serial.println(dataObj["serviceParams"]["a5"].as<String>());
    }

    Serial.printf("剩余内存: %dB\n", ESP.getFreeHeap());
}



void setup()
{
    Serial.begin(115200);
    Serial.print("开发板型号: ");
    Serial.println(ARDUINO_BOARD);

    pinMode(SYS_RST_IO, INPUT_PULLUP);

    // 绑定定时任务回调函数
    TickerRST.attach(1, SysRstfun);

    // 使用产品密钥初始化MiniIot，（产品ID，产品密钥）
    // 设备不存在会自动注册添加设备
    MiniIot.begin("ZnnQmFTH", "h5Tmn1l3m2S9DY2I");

    // 使用设备密钥初始化MiniIot，（产品ID，设备ID，设备密钥）
    // 必须先在平台上添加设备
    // MiniIot.begin("ZnnQmFTH", "Ab79b2a5a3", "h5Tmn1l3m2S9DY2I");

    // 绑定回调函数
    MiniIot.attach(ServiceCallbackFunction);

    Serial.printf("剩余内存: %dB\n", ESP.getFreeHeap());
    Serial.print("cpu运行频率:");Serial.println(ESP.getCpuFreqMHz());
    Serial.print("当前固件大小:");Serial.println(ESP.getSketchSize());
    Serial.print("剩余固件空间:");Serial.println(ESP.getFreeSketchSpace());
    
}

void loop()
{
    MiniIot.loop();
    
    // 服务器连接成功
    if(MiniIot.running()){
        // 不能直接使用默认的delay()延时，会导致设备掉线，需要使用MiniIot.delay()
        MiniIot.delay(3000);

        Serial.printf("剩余内存: %dB\n", ESP.getFreeHeap());
    }
}

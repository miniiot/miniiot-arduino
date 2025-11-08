// 登录控制台与查看文档请访问官网：http://www.miniiot.top

// 当前程序版本（仅用于后台展示）
#define APP_VERSION "w5500_2511071328"

// 使用W5500以太网模块
#define MiniIot_USE_ETH
// W5500以太网模块复位控制IO
// #ifdef ESP8266
//     #define MiniIot_ETH_RST 5
// #endif
// #ifdef ESP32
//     #define MiniIot_ETH_RST 2
// #endif
// #ifdef STM32F1
//     #define MiniIot_ETH_RST PE9
// #endif

// 使用以太网模块时通过心跳判断网络状态
// MQTT心跳间隔（秒）
// #define MiniIot_MQTT_KeepAlive 5

// 导入miniiot
#include <MiniIot.h>


int num = 0;


void addNum(){
    // 上报属性
    MiniIot.propertyPost("test", num);
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

    // 使用产品密钥初始化MiniIot，（产品ID，产品密钥）
    // 设备不存在会自动注册添加设备
    MiniIot.begin("OWO56Olq", "C2EsALWiPYwd4sAd");

    // 使用设备密钥初始化MiniIot，（产品ID，设备ID，设备密钥）
    // 必须先在平台上添加设备
    // MiniIot.begin("ZnnQmFTH", "Ab79b2a5a3", "h5Tmn1l3m2S9DY2I");

    // 绑定回调函数
    MiniIot.attach(ServiceCallbackFunction);

    Serial.printf("cpu运行频率:%d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("当前固件大小:%d KB\n", ESP.getSketchSize() / 1024);
    Serial.printf("剩余固件空间:%d KB\n", ESP.getFreeSketchSpace() / 1024);
    Serial.printf("剩余内存:%d KB\n", ESP.getFreeHeap() / 1024);
    

    // w5500以太网模块接线，无法更改
    Serial.println("w5500以太网模块接线：");
    Serial.print("MO:");
    Serial.println(MOSI);

    Serial.print("MI:");
    Serial.println(MISO);

    Serial.print("SCLK:");
    Serial.println(SCK);

    Serial.print("SS:");
    Serial.println(SS);
}

void loop()
{
    MiniIot.loop();
    
    // 服务器连接成功
    if(MiniIot.running()){
        // 不能直接使用默认的delay()延时，会导致设备掉线，需要使用MiniIot.delay()
        MiniIot.delay(3000);

        Serial.printf("剩余内存:%d KB\n", ESP.getFreeHeap() / 1024);
    }
}

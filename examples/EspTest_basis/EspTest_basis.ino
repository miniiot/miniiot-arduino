// 登录控制台与查看文档请访问官网：http://www.miniiot.top

// 当前程序版本（仅用于后台展示）
#define APP_VERSION "basis_2504231612"

// 默认WIFI配置（出厂设置）
#define DEFAULT_WIFI_SSID "Tenda_375160章"
#define DEFAULT_WIFI_PASSWORD "87472998"

// 打印日志（建议调试完成后注释掉）
#define MiniIot_DEBUG_LOG

// 导入miniiot
#include <MiniIot.h>


int num = 0;

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
}



void setup()
{
    Serial.begin(115200);

    // 使用产品密钥初始化MiniIot，（产品ID，产品密钥）
    // 设备不存在会自动注册添加设备
    MiniIot.begin("ZnnQmFTH", "h5Tmn1l3m2S9DY2I");

    // 使用设备密钥初始化MiniIot，（产品ID，设备ID，设备密钥）
    // 必须现在平台上添加设备
    // MiniIot.begin("ZnnQmFTH", "Ab79b2a5a3", "h5Tmn1l3m2S9DY2I");

    // 绑定回调函数
    MiniIot.attach(ServiceCallbackFunction);
}

void loop()
{
    MiniIot.loop();
    
    // 服务器连接成功
    if(MiniIot.running()){
        // 不能直接使用delay延时
        MiniIot.delay(3000);

        Serial.printf("剩余内存: %dB\n", ESP.getFreeHeap());
    }
}

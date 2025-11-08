#include <STM32FreeRTOS.h>

// 当前程序版本（仅用于后台展示）
#define APP_VERSION "STM32_RTOS_2511081753"

// 使用W5500以太网模块
#define MiniIot_USE_ETH

// W5500以太网模块复位控制IO
#define MiniIot_ETH_RST PE9
#define MiniIot_STATE_LED PA14

// 打印日志（建议调试完成后注释掉）
#define MiniIot_DEBUG_LOG

// 自定义日志输出串口
#define MiniIotDebugSerial Serial2

// 导入miniiot
#include <MiniIot.h>

int num = 0;

// miniiot任务
void Task_miniiot(void *pvParameters __attribute__((unused)))
{
  for (;;)
  {
    MiniIot.loop();
  }
}

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
        MiniIotDebugSerial.print("参数a1：");MiniIotDebugSerial.println(dataObj["serviceParams"]["a1"].as<String>());
        MiniIotDebugSerial.print("参数a2：");MiniIotDebugSerial.println(dataObj["serviceParams"]["a2"].as<String>());
        MiniIotDebugSerial.print("参数a3：");MiniIotDebugSerial.println(dataObj["serviceParams"]["a3"].as<String>());
        MiniIotDebugSerial.print("参数a4：");MiniIotDebugSerial.println(dataObj["serviceParams"]["a4"].as<String>());
        MiniIotDebugSerial.print("参数a5：");MiniIotDebugSerial.println(dataObj["serviceParams"]["a5"].as<String>());
    }

}


void setup()
{
    MiniIotDebugSerial.begin(115200);
    MiniIotDebugSerial.println();
    MiniIotDebugSerial.println();

    // 使用产品密钥初始化MiniIot，（产品ID，产品密钥）
    // 设备不存在会自动注册添加设备
    MiniIot.begin("OWO56Olq", "C2EsALWiPYwd4sAd");

    // 使用设备密钥初始化MiniIot，（产品ID，设备ID，设备密钥）
    // 必须先在平台上添加设备
    // MiniIot.begin("ZnnQmFTH", "Ab79b2a5a3", "h5Tmn1l3m2S9DY2I");

    // 绑定回调函数
    MiniIot.attach(ServiceCallbackFunction);
    
    // 任务函数，任务名称，栈大小，参数，优先级(0最低)，句柄
    xTaskCreate(Task_miniiot, (const portCHAR *)"Task_miniiot", 1024, NULL, 2, NULL);

    // 启动调度程序
    vTaskStartScheduler();
    while (1)
        ;
}

void loop()
{
}
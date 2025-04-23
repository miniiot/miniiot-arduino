#ifdef ESP8266
#include <ESP8266WebServer.h>
ESP8266WebServer adminWebServer(MiniIot_ADMIN_SERVICE_PORT);
#endif

#ifdef ESP32
#include <WebServer.h>
WebServer adminWebServer(MiniIot_ADMIN_SERVICE_PORT);
#endif

class MiniIotAdminWebServer
{
private:

    static bool basicAuth()
    {
        if (!adminWebServer.authenticate(MiniIotAdminWebServer::username.c_str(), MiniIotAdminWebServer::password.c_str()))
        {
            adminWebServer.send(200, "application/json; charset=utf-8", "{\"code\":200,\"data\":[],\"msg\":\"鉴权失败\"}");
            return false;
        }
        return true;
    }

    static void handleInfo()
    {
        MiniIot_LOG_LN(F("[AdminWebServer] Info"));
        if (!MiniIotAdminWebServer::basicAuth())
        {
            return;
        }

        const String postForms = "{\"code\":200,\"data\":{},\"msg\":\"OK\"}";
        adminWebServer.send(200, "application/json; charset=utf-8", postForms);
    }

    static void handleSyscmd()
    {
        String postForms;

        MiniIot_LOG_LN(F("[AdminWebServer] Syscmd"));
        if (!MiniIotAdminWebServer::basicAuth())
        {
            return;
        }

        String msg = adminWebServer.arg("cmdData");

        DynamicJsonDocument JSON_Buffer(msg.length() + 20);
        DeserializationError error = deserializeJson(JSON_Buffer, msg);
        if (error)
        {
            MiniIot_LOG_LN("[Admin] JSON解析错误: " + String(error.c_str()));

            postForms = "{\"code\":400,\"data\":{},\"msg\":\"JSON解析错误\"}";
            adminWebServer.send(200, "application/json; charset=utf-8", postForms);
            return;
        }

        JsonObject msgData = JSON_Buffer.as<JsonObject>();
        if (!msgData.containsKey("id") || !msgData.containsKey("version") || !msgData.containsKey("method") || !msgData.containsKey("params"))
        {
            MiniIot_LOG_LN(F("[Admin] JSON缺少必要字段"));
            
            postForms = "{\"code\":400,\"data\":{},\"msg\":\"JSON缺少必要字段\"}";
            adminWebServer.send(200, "application/json; charset=utf-8", postForms);
            return;
        }

        if (msgData["version"] != "1.0")
        {
            MiniIot_LOG_LN(F("[Admin] 协议版本不支持"));
            
            postForms = "{\"code\":400,\"data\":{},\"msg\":\"协议版本不支持\"}";
            adminWebServer.send(200, "application/json; charset=utf-8", postForms);
            return;
        }

        if (msgData["method"] == "service.control.sys")
        {
            // 服务调用-系统服务
            MiniIotAdminWebServer::SysCallBack(msgData["params"]);

            postForms = "{\"code\":200,\"data\":{},\"msg\":\"OK\"}";
            adminWebServer.send(200, "application/json; charset=utf-8", postForms);
            return;
        }

        MiniIot_LOG_LN(F("[Admin] 不支持的服务调用"));
        
        postForms = "{\"code\":400,\"data\":{},\"msg\":\"不支持的服务调用\"}";
        adminWebServer.send(200, "application/json; charset=utf-8", postForms);
    }

    static void handleNotFound()
    {
        MiniIot_LOG_LN(F("[AdminWebServer] NotFound"));
        adminWebServer.send(200, "application/json; charset=utf-8", "{\"code\":404,\"data\":[],\"msg\":\"NotFound\"}");
    }

public:
    static String username;
    static String password;
    static JsonObjectCallbackFunction SysCallBack;

    void init(JsonObjectCallbackFunction callback)
    {
        MiniIotAdminWebServer::SysCallBack = callback;

        adminWebServer.on("/info", HTTP_POST, MiniIotAdminWebServer::handleInfo);
        adminWebServer.on("/syscmd", HTTP_POST, MiniIotAdminWebServer::handleSyscmd);
        adminWebServer.onNotFound(MiniIotAdminWebServer::handleNotFound);
        adminWebServer.begin();

        MiniIot_LOG_LN("[Admin] 系统管理服务已启动，端口：" + String(MiniIot_ADMIN_SERVICE_PORT));
    }

    void loop()
    {
        adminWebServer.handleClient();
    }

    void setUsername(String username)
    {
        MiniIotAdminWebServer::username = username;
    }

    void setPassword(String password)
    {
        MiniIotAdminWebServer::password = password;
    }
};

String MiniIotAdminWebServer::username = "admin";
String MiniIotAdminWebServer::password = "admin";
JsonObjectCallbackFunction MiniIotAdminWebServer::SysCallBack = nullptr;
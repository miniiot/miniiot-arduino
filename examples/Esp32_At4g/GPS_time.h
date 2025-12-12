#pragma once
// 需要手动安装【Time】库
#include <TimeLib.h>

// 函数：解析 UTC 时间字符串（ddmmyyhhmmss.sss）并转换为北京时间戳
// 参数：utcStr - 输入的 UTC 时间字符串（例如 "151024123045.123"）
// 返回：time_t 类型的北京时间戳（秒级），解析失败返回 0
time_t utcToBeijingTime(const String &utcStr)
{
    // 1. 校验输入格式（最小长度：ddmmyyhhmmss = 12 字符）
    if (utcStr.length() < 12){
        return 0;
    }

    // 2. 提取各时间字段（字符串截取 + 转整数）
    // 变量名添加前缀 "t_"，避免与库函数名冲突
    int t_day = utcStr.substring(0, 2).toInt();         // 日（01-31）
    int t_month = utcStr.substring(2, 4).toInt();       // 月（01-12）
    int t_year = 2000 + utcStr.substring(4, 6).toInt(); // 年（20yy）
    int t_hour = utcStr.substring(6, 8).toInt();        // 时（00-23）
    int t_minute = utcStr.substring(8, 10).toInt();     // 分（00-59）
    int t_second = utcStr.substring(10, 12).toInt();    // 秒（00-59）

    // 3. 校验字段合法性（避免无效时间）
    if (t_month < 1 || t_month > 12 || t_day < 1 || t_day > 31 ||
        t_hour > 23 || t_minute > 59 || t_second > 59)
    {
        return 0;
    }

    // 4. 构造 tmElements_t 结构体（新版 TimeLib 要求）
    tmElements_t utcTimeElements;
    utcTimeElements.Second = t_second;    // 秒（0-59）
    utcTimeElements.Minute = t_minute;    // 分（0-59）
    utcTimeElements.Hour = t_hour;        // 时（0-23）
    utcTimeElements.Day = t_day;          // 日（1-31）
    utcTimeElements.Month = t_month;      // 月（1-12，注意：新版 TimeLib 支持 1-12 直接传入）
    utcTimeElements.Year = t_year - 1970; // 年（自 1970 年起的年份差，例如 2024-1970=54）

    // 5. 转换为 UTC 时间戳（使用结构体作为参数）
    time_t utcTimestamp = makeTime(utcTimeElements);
    if (utcTimestamp == 0){
        return 0; // 解析失败（如无效日期：2月30日）
    }

    // 6. 时区偏移：UTC+8（加 8*3600 秒）
    time_t beijingTimestamp = utcTimestamp + 8 * 3600;

    return beijingTimestamp;
}

// 函数：格式化时间戳为 "yyyy-mm-dd" 字符串
String formatDate(time_t timestamp)
{
    if (timestamp == 0){
        return "0000-00-00";
    }

    // 调用库函数时，变量名避免与函数名冲突（用 y/m/d 代替 year/month/day）
    int y = year(timestamp) % 100; // 取年份后两位（yy）
    int m = month(timestamp);      // 月（1-12）
    int d = day(timestamp);        // 日（1-31）

    // 补前导零（确保格式统一）
    String yStr = (y < 10) ? "0" + String(y) : String(y);
    String mStr = (m < 10) ? "0" + String(m) : String(m);
    String dStr = (d < 10) ? "0" + String(d) : String(d);

    return "20" + yStr + "-" + mStr + "-" + dStr;
}

// 函数：格式化时间戳为 "hh:mm:ss" 字符串
String formatTime(time_t timestamp)
{
    if (timestamp == 0){
        return "00:00:00";
    }

    // 变量名避免与库函数名冲突（用 h/m/s 代替 hour/minute/second）
    int h = hour(timestamp);   // 时（0-23）
    int m = minute(timestamp); // 分（0-59）
    int s = second(timestamp); // 秒（0-59）

    // 补前导零
    String hStr = (h < 10) ? "0" + String(h) : String(h);
    String mStr = (m < 10) ? "0" + String(m) : String(m);
    String sStr = (s < 10) ? "0" + String(s) : String(s);

    return hStr + ":" + mStr + ":" + sStr;
}

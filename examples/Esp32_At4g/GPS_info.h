#pragma once
#define GPS_ITEM_COUNT 25

// GPS数据结构体
struct GPS_DATA_t
{
	bool GPS_valid;			 // GPS是否有效
	bool GPS_datetime_valid; // GPS日期时间是否有效
	String GPS_date;		 // 定位日期
	String GPS_time;		 // 定位时间
	String GPS_latitude;	 // 纬度
	String GPS_latitude_NS;	 // 纬度方向
	String GPS_longitude;	 // 经度
	String GPS_longitude_EW; // 经度方向
	String GPS_altitude;	 // 海拔
	String GPS_satellite;	 // 卫星数量
	String GPS_speed;		 // 速度
	String GPS_course;		 // 航向角
};

class GpsInfo
{
private:

public:

	// 解析GPS数据
	void parseBuffer(String data, struct GPS_DATA_t *GPS_INFO)
	{
		if (!data.startsWith("$GNGGA") && !data.startsWith("$GPGGA") && !data.startsWith("$BDGGA") && !data.startsWith("$GNRMC") && !data.startsWith("$GPRMC") && !data.startsWith("$BDRMC")){
			return;
		}

		String GPS_Data[GPS_ITEM_COUNT];
		int GPS_paramCount = 0;

		data.remove(data.length() - 1); // 去掉末尾的'\r\n'

		int index = 0;
		int commaIndex = 0;
		while ((commaIndex = data.indexOf(',', index)) != -1)
		{
			GPS_Data[GPS_paramCount++] = data.substring(index, commaIndex);
			index = commaIndex + 1;
			if (GPS_paramCount >= GPS_ITEM_COUNT - 1)
			{
				break;
			}
		}
		// 添加最后一个参数
		GPS_Data[GPS_paramCount++] = data.substring(index);

		// for (int i = 0; i < GPS_paramCount; i++)
		// {
		// 	MiniIotDebugSerial.printf("%d:%s\n", i, GPS_Data[i].c_str());
		// }

		// 解析GPS数据
		if (GPS_Data[0] == "$GNGGA" || GPS_Data[0] == "$GPGGA" || GPS_Data[0] == "$BDGGA")
		{
			if (GPS_Data[6].toInt() > 0)
			{
				GPS_INFO->GPS_latitude = GPS_Data[2];
				GPS_INFO->GPS_latitude_NS = GPS_Data[3];
				GPS_INFO->GPS_longitude = GPS_Data[4];
				GPS_INFO->GPS_longitude_EW = GPS_Data[5];
				GPS_INFO->GPS_altitude = GPS_Data[9];
				GPS_INFO->GPS_satellite = GPS_Data[7];
				if (GPS_INFO->GPS_latitude_NS == "N" || GPS_INFO->GPS_latitude_NS == "S")
				{
					if (GPS_INFO->GPS_longitude_EW == "E" || GPS_INFO->GPS_longitude_EW == "W")
					{
						GPS_INFO->GPS_valid = true;
					}
				}
			}
			else
			{
				GPS_INFO->GPS_valid = false;
			}
		}
		if (GPS_Data[0] == "$GNRMC" || GPS_Data[0] == "$GPRMC" || GPS_Data[0] == "$BDRMC")
		{
			if (GPS_Data[2] == "A")
			{
				GPS_INFO->GPS_time = GPS_Data[1];
				double speed = GPS_Data[7].toDouble();
				// 速度节转千米每小时，四舍五入取整数
				GPS_INFO->GPS_speed = String((int)(speed * 1.852));
				GPS_INFO->GPS_course = GPS_Data[8];
				GPS_INFO->GPS_date = GPS_Data[9];
				GPS_INFO->GPS_datetime_valid = true;
			}
		}
	}

	// 获取GPS是否有效
	int isValid(struct GPS_DATA_t *GPS_INFO)
	{
		if (GPS_INFO->GPS_valid)
		{
			GPS_INFO->GPS_valid = false;
			GPS_INFO->GPS_datetime_valid = false;
			return 1;
		}
		return 0;
	}

};

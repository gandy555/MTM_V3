#ifndef __SETUP_DATA_H__
#define __SETUP_DATA_H__

#define SETUP_DATA_MAGIC				0x4D544D02	//MTM{0x02}		2014-05-01

#define SETUP_DATA_PATH					"/app/"
#define SETUP_DATA_FILE					"setup.dat"

#define SETUP_VOLUME_DEF				7
#define SETUP_ADC_VALUE_DEF				820

#define SETUP_TODAY_TEMP_DEF			123
#define SETUP_TODAY_WEATHER_DEF			0			//IMG_ENUM_WEATHER_SERENITY 맑음
#define SETUP_TOMORROW_TEMP_DEF			123
#define SETUP_TOMORROW_WEATHER_DEF		1			//IMG_ENUM_WEATHER_PARTLY_CLOUDY 구름조금

//
//설정 데이타 레코드
//
typedef struct _SETUP_DATA
{
	ULONG			magic;
//-------------------------------------------------------------------------------------------------
//	설정값
	UINT			volume;				//사운드 볼륨 (0~8)
	UINT			adc_value;			//Microwae Adc 감지 설정값
//-------------------------------------------------------------------------------------------------
//	날씨정보
	UCHAR			weather_year;		//날씨정보의 시간
	UCHAR			weather_month;		//
	UCHAR			weather_day;		//
	UCHAR			weather_hour;		//
	UCHAR			weather_minute;		//
	UCHAR			weather_second;		//
	UINT			current_temp;		//현재온도
	UINT			today_temp_high;	//오늘 최고온도 (소수점 1자리, 나누기 10해서 써야함)			!!SNS버전에서 온도는 정수!!
	UINT			today_temp_low;		//오늘 최저온도 (소수점 1자리, 나누기 10해서 써야함)
	UINT			today_weather;		//오늘 날씨 (아이콘 종류)		*IMG_ENUM_WEATHER_XXX 순서와 동일
	UINT			tomorrow_temp_high;	//내일 최고온도 (소수점 1자리, 나누기 10해서 써야함)
	UINT			tomorrow_temp_low;	//내일 최고온도 (소수점 1자리, 나누기 10해서 써야함)
	UINT			tomorrow_weather;	//내일 날씨 (아이콘 종류)
//-------------------------------------------------------------------------------------------------
//	월패드상태
	UCHAR			alloff_stat;		//일괄소등상태	0x00=OFF(등이 켜짐), 0x01=ON(등이 꺼짐)
	UCHAR			gas_stat;			//가스밸브상태	0x00=닫힘, 0x01=열림
	UCHAR			security_stat;		//외출설정상태	0x00=외출해제, 0x01=외출설정
//-------------------------------------------------------------------------------------------------
//	주차정보
	UCHAR			in_out;				//입출차정보
	UCHAR			car_num[4];			//차량번호
	UCHAR			car_floor[2];		//층정보(층명칭,층번호)
	UCHAR			car_zone[6];		//구역정보(구역명칭,구역번호)
} __attribute__ ((packed)) SETUP_DATA;	//size=


//
// Class Definition
//

class CSetupData : public CFile
{
public:
	CSetupData();
	~CSetupData();

	//Member Function
	void Init();
	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	//---------------------------------
	BOOL LoadSetupData();
	BOOL SaveSetupData();
	void SetSetupDataDefault();

	//Member Variable
	UCHAR			m_CriticalSection;
	SETUP_DATA		m_SetupData;
};

#endif //__SETUP_DATA_H__

#ifndef __OPERATION_DATA_H__
#define __OPERATION_DATA_H__

#define OPERATION_DATA_MAGIC				0x4D544F03	//MTO{0x01}		2014-09-14

#define OPERATION_DATA_PATH					"/app/"
#define OPERATION_DATA_FILE					"operation.dat"

// 시스템 파라미터 디폴트값
#define OPERATION_VOLUME_DEF				7
#define OPERATION_ADC_VALUE_DEF				820

// 날씨정보
#define OPERATION_TODAY_TEMP_DEF			123
#define OPERATION_TODAY_WEATHER_DEF			0			//IMG_ENUM_WEATHER_SERENITY 맑음
#define OPERATION_TOMORROW_TEMP_DEF			123
#define OPERATION_TOMORROW_WEATHER_DEF		1			//IMG_ENUM_WEATHER_PARTLY_CLOUDY 구름조금


//
// 운영 데이타 레코드
//
typedef struct _OPERATION_DATA
{
//-------------------------------------------------------------------------------------------------
//	시스템 설정값
	UINT			volume;					//사운드 볼륨 (0~8)
	UINT			adc_value;				//Microwae Adc 감지 설정값
//-------------------------------------------------------------------------------------------------
//	주변장치 상태
	UCHAR			alloff_stat;			//일괄소등상태	0x00=OFF(등이 켜짐), 0x01=ON(등이 꺼짐)
	UCHAR			gas_stat;				//가스밸브상태	0x00=닫힘(사용가능), 0x01=열림(사용차단)
	UCHAR			security_stat;			//외출설정상태	0x00=외출해제, 0x01=외출설정
//-------------------------------------------------------------------------------------------------
//	날씨정보
	UCHAR			weather_year;			//날씨정보의 시간
	UCHAR			weather_month;			//
	UCHAR			weather_day;			//
	UCHAR			weather_hour;			//
	UCHAR			weather_minute;			//
	UCHAR			weather_second;			//
	UINT			current_temp;			//현재온도
	UINT			today_temp_high;		//오늘 최고온도 (기본: 소수점 1자리, 나누기 10해서 써야함)
	UINT			today_temp_low;			//오늘 최저온도 (기본: 소수점 1자리, 나누기 10해서 써야함)
	UINT			today_weather;			//오늘 날씨 (아이콘 종류)		*IMG_ENUM_WEATHER_XXX 순서와 동일
	UINT			tomorrow_temp_high;		//내일 최고온도 (기본: 소수점 1자리, 나누기 10해서 써야함)
	UINT			tomorrow_temp_low;		//내일 최고온도 (기본: 소수점 1자리, 나누기 10해서 써야함)
	UINT			tomorrow_weather;		//내일 날씨 (아이콘 종류)
//-------------------------------------------------------------------------------------------------
//	주차정보
	UCHAR			in_out;					//입출차정보 (0=주차, 1=출차)
	UCHAR			car_num[4];				//차량번호
	UCHAR			car_floor[2];			//층정보(층명칭,층번호)
	UCHAR			car_zone[6];			//구역정보(구역명칭,구역번호)
} __attribute__ ((packed)) OPERATION_DATA;	//size=58


//
// Class Definition
//

class COperationData : public CFile
{
public:
	COperationData();
	~COperationData();

	//Member Function
	void Init();
	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	//---------------------------------
	BOOL LoadOperationData();
	BOOL SaveOperationData();
	void SetOperationDataDefault();

	//Member Variable
	UCHAR			m_CriticalSection;
	OPERATION_DATA	m_OperationData;
};



#endif //__OPERATION_DATA_H__

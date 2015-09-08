#ifndef __SETUP_DATA_H__
#define __SETUP_DATA_H__

#define SETUP_DATA_MAGIC				0x4D544D04	//MTM{0x04}		2014-12-21

#define SETUP_DATA_PATH					"/app/"
#define SETUP_DATA_FILE					"setup.dat"

#define SETUP_VOLUME_DEF				7
//#define SETUP_ADC_VALUE_DEF			820			//기존 Default
#define SETUP_ADC_VALUE_DEF				780			//KCM 구리수택 현장 Default

#define SETUP_TEMP_DEF					123
#define SETUP_WEATHER_DEF				0			//IMG_ENUM_WEATHER_SERENITY 맑음

//
// 월패드 종류
//
enum
{
	WALLPAD_TYPE_DEFAULT,
	WALLPAD_TYPE_HDT,					//현대통신
	WALLPAD_TYPE_SHN,					//삼성SDS
	WALLPAD_TYPE_KCM,					//코콤
	WALLPAD_TYPE_EZV,					//EZ-Ville
	WALLPAD_TYPE_COUNT
};

//
// 화면표시 옵션
//
enum
{
	UI_OPTION_DEFAULT,					//기본
	UI_OPTION_COUNT
};

//
// KEY 옵션
//
#define KEY_OPTION_ALL					0x3f
#define KEY_OPTION_RT					(1<<0)
#define KEY_OPTION_RM					(1<<1)
#define KEY_OPTION_RB					(1<<2)
#define KEY_OPTION_LT					(1<<3)
#define KEY_OPTION_LM					(1<<4)
#define KEY_OPTION_LB					(1<<5)

//
//설정 데이타 레코드
//
typedef struct _SETUP_DATA
{
	ULONG			magic;
//-------------------------------------------------------------------------------------------------
//	설정값
	UCHAR			wallpad_type;		//월패드 종류
	UCHAR			ui_option;			//화면표시 옵션
	UCHAR			key_option;			//전면키 옵션 (비트별 1=Enable, 0=Disable)
	UINT			volume;				//사운드 볼륨 (0~8)
	UINT			adc_value;			//Microwave Adc 감지 설정값
//-------------------------------------------------------------------------------------------------
//	날씨정보
	UCHAR			weather_year;		//날씨정보의 시간
	UCHAR			weather_month;		//
	UCHAR			weather_day;		//
	UCHAR			weather_hour;		//
	UCHAR			weather_minute;		//
	UCHAR			weather_second;		//
	int				current_temp;		//현재온도
	UINT			weather_left;		//왼쪽 날씨 (아이콘 종류)								*IMG_ENUM_WEATHER_XXX 순서와 동일
	int				temp_low_left;		//왼쪽 최저온도 (소수점 1자리, 나누기 10해서 써야함)
	int				temp_high_left;		//왼쪽 최고온도 (소수점 1자리, 나누기 10해서 써야함)	!!SNS버전에서 온도는 정수!!
	UINT			weather_right;		//오른쪽 날씨 (아이콘 종류)
	int				temp_low_right;		//오른쪽 최고온도 (소수점 1자리, 나누기 10해서 써야함)
	int				temp_high_right;	//오른쪽 최고온도 (소수점 1자리, 나누기 10해서 써야함)
//-------------------------------------------------------------------------------------------------
//	기기상태
	UCHAR			light_stat;			//일괄소등상태	0=점등(소등해제), 1=소등
	UCHAR			gas_stat;			//가스밸브상태	0=열림(사용중), 1=차단
	UCHAR			security_stat;		//보안설정상태	0=해제, 1=설정
//-------------------------------------------------------------------------------------------------
//	주차정보
	UCHAR			status;									//상태
	UCHAR			car_floor[MAX_PARKING_FLOOR_NAME];		//층명칭
	UCHAR			car_zone[MAX_PARKING_ZONE_NAME];		//구역명칭
	// gandy 2014_12_26
	UCHAR			car_num[MAX_PARKING_CAR_NUM];			//차량번호
//-------------------------------------------------------------------------------------------------
//	엘리베이터정보
	UCHAR			elevator_status;
	char			elevator_floor;		//양수(1층~127층), 음수(B1층~Bx층), 0=에러
	UCHAR door_opened;
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

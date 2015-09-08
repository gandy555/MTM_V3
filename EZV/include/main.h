#ifndef __MAIN_H__
#define __MAIN_H__


#define CLOCK_TIMER				1	//화면시계표시 타이머	--> time.h:TIMER_IDLE_CLOCK
#define BACK_LIGHT_TIMER		2	//백라이트 타이머
#define ELEVATOR_TIMER			3	//엘레베이터 주기적 상태요청 타이머
#define PARKING_TIMER			4	//주차위치 확인요청 타이머
#define RESPONSE_TIMER			5	//응답 타이머

#define RETRY_TIMEOUT			0xff

#define BACK_LIGHT_TIME			10	//백라이트 켜있는 시간
#define ELEVATOR_REQ_TIME		2	//엘레베이터 상태 모니터링 타임 (응답기준 다음 요청시까지의 시간)
#define PARKING_REQ_TIME		10	//주차위치 확인대기 타임

#define ELEVATOR_RES_TIMEOUT	10	//엘레베이터 응답 타임아웃 (10초간 응답이 없으면 통신이상)

#define PARKING_REQ_SPIN_PERIOD		300

#define MAX_PARKING_LIST		4
#define MAX_CAR_NUM_SIZE		4
#define PARKING_STATUS_IN		0x5A
#define PARKING_STATUS_OUT	1

//
// Global Data
//
extern GR_SCREEN_INFO			g_scr_info;;
extern GR_WINDOW_ID				g_wid;
extern GR_GC_ID					g_gc;
extern GR_FONT_ID				g_font;

extern BOOL						g_isBackLightOn;
extern BOOL						g_isTimeSync;
//extern BOOL						g_isGasValveOn;
//extern BOOL						g_isLampAllOn;
//extern BOOL						g_isSecurityOn;
extern BOOL						g_isContextSetupWallPad;

extern UINT						g_KeyFlag;
extern UINT						g_KeyEvent;

//
// Class Instance
//
extern class CSetupData			g_setup_data;
extern class CState				g_state;
extern class CMessage			g_message;
extern class CTimer				g_timer;
extern class CGpioMtm			g_gpio_mtm;
extern class CMicrowave			g_microwave;
extern class CWatchdog			g_watchdog;

extern class CWallPad*			g_pWallPad;

//
// Exported Function
//
//extern void back_light_timer_handler(void *pParam);

extern void SetKey(UINT flag, UINT event);
extern void GetKey(UINT* pflag, UINT* pevent);

enum {
	PARKING_ITM_ID = 0,
	PARKING_ITM_FLOOR,
	PARKING_ITM_ZONE
};

extern void parking_list_update(MTM_DATA_PARKING* _info);
extern int parking_list_get_item(int _idx, MTM_DATA_PARKING* _dst);
extern int parking_item_to_str(int _id, char* _src, char* _dst);

//
// Common Image Resource ID Emumeration
//

enum
{
	IMG_ENUM_GAS_ON,					//가스On		/app/img/main_icon/main_gas_on.png
	IMG_ENUM_GAS_OFF,					//가스Off		/app/img/main_icon/main_gas_off.png
	IMG_ENUM_GAS_COUNT
};

enum
{
	IMG_ENUM_WEATHER_SERENITY,			//맑음			/app/img/weather/serenity.png
	IMG_ENUM_WEATHER_PARTLY_CLOUDY,		//구름 조금		/app/img/weather/partly_cloudy.png
	IMG_ENUM_WEATHER_CLOUDY,			//흐림			/app/img/weather/cloudy.png
	IMG_ENUM_WEATHER_SHOWER,			//소나기		/app/img/weather/shower.png
	IMG_ENUM_WEATHER_RAINNY,			//비			/app/img/weather/rainny.png
	IMG_ENUM_WEATHER_THUNDERSTORM,		//뇌우			/app/img/weather/thunderstorm.png
	IMG_ENUM_WEATHER_SNOW,				//눈			/app/img/weather/snow.png
	IMG_ENUM_WEATHER_COUNT
};

enum
{
	IMG_ENUM_ARROW_BLANK,				//공백			/app/img/el_png/arrow_back.png
	IMG_ENUM_ARROW_UP_1,				//상승1			/app/img/el_png/arrow_up_1.png
	IMG_ENUM_ARROW_UP_2,				//상승2			/app/img/el_png/arrow_up_2.png
	IMG_ENUM_ARROW_UP_3,				//상승3			/app/img/el_png/arrow_up_3.png
	IMG_ENUM_ARROW_DOWN_1,				//하강1			/app/img/el_png/arrow_down_1.png
	IMG_ENUM_ARROW_DOWN_2,				//하강2			/app/img/el_png/arrow_down_2.png
	IMG_ENUM_ARROW_DOWN_3,				//하강3			/app/img/el_png/arrow_down_3.png
	IMG_ENUM_ARROW_COUNT
};

enum
{
	IMG_ENUM_ICON_ON,					//ON State		/app/img/icon_xxx_on.png
	IMG_ENUM_ICON_OFF,					//OFF State		/app/img/icon_xxx_off.png
	IMG_ENUM_ICON_COUNT
};

enum
{
	IMG_ENUM_SWITCH_ON,					//스위치ON		/app/img/icon_switch_on.png
	IMG_ENUM_SWITCH_OFF,				//	    OFF		/app/img/icon_switch_off.png
	IMG_ENUM_SWITCH_COUNT
};

#endif //__MAIN_H__


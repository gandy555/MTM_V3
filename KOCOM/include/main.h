#ifndef __MAIN_H__
#define __MAIN_H__


#define CLOCK_TIMER				1	//ȭ��ð�ǥ�� Ÿ�̸�	--> time.h:TIMER_IDLE_CLOCK
#define BACK_LIGHT_TIMER		2	//�����Ʈ Ÿ�̸�
#define ELEVATOR_TIMER			3	//���������� �ֱ��� ���¿�û Ÿ�̸�
#define PARKING_TIMER			4	//������ġ Ȯ�ο�û Ÿ�̸�
#define RESPONSE_TIMER			5	//���� Ÿ�̸�

#define RETRY_TIMEOUT			0xff

#define BACK_LIGHT_TIME			10	//�����Ʈ ���ִ� �ð�
#define ELEVATOR_REQ_TIME		2	//���������� ���� ����͸� Ÿ�� (������� ���� ��û�ñ����� �ð�)
#define PARKING_REQ_TIME		10	//������ġ Ȯ�δ�� Ÿ��

#define ELEVATOR_RES_TIMEOUT	10	//���������� ���� Ÿ�Ӿƿ� (10�ʰ� ������ ������ ����̻�)

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
	IMG_ENUM_GAS_ON,					//����On		/app/img/main_icon/main_gas_on.png
	IMG_ENUM_GAS_OFF,					//����Off		/app/img/main_icon/main_gas_off.png
	IMG_ENUM_GAS_COUNT
};

enum
{
	IMG_ENUM_WEATHER_SERENITY,			//����			/app/img/weather/serenity.png
	IMG_ENUM_WEATHER_PARTLY_CLOUDY,		//���� ����		/app/img/weather/partly_cloudy.png
	IMG_ENUM_WEATHER_CLOUDY,			//�帲			/app/img/weather/cloudy.png
	IMG_ENUM_WEATHER_SHOWER,			//�ҳ���		/app/img/weather/shower.png
	IMG_ENUM_WEATHER_RAINNY,			//��			/app/img/weather/rainny.png
	IMG_ENUM_WEATHER_THUNDERSTORM,		//����			/app/img/weather/thunderstorm.png
	IMG_ENUM_WEATHER_SNOW,				//��			/app/img/weather/snow.png
	IMG_ENUM_WEATHER_COUNT
};

enum
{
	IMG_ENUM_ARROW_BLANK,				//����			/app/img/el_png/arrow_back.png
	IMG_ENUM_ARROW_UP_1,				//���1			/app/img/el_png/arrow_up_1.png
	IMG_ENUM_ARROW_UP_2,				//���2			/app/img/el_png/arrow_up_2.png
	IMG_ENUM_ARROW_UP_3,				//���3			/app/img/el_png/arrow_up_3.png
	IMG_ENUM_ARROW_DOWN_1,				//�ϰ�1			/app/img/el_png/arrow_down_1.png
	IMG_ENUM_ARROW_DOWN_2,				//�ϰ�2			/app/img/el_png/arrow_down_2.png
	IMG_ENUM_ARROW_DOWN_3,				//�ϰ�3			/app/img/el_png/arrow_down_3.png
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
	IMG_ENUM_SWITCH_ON,					//����ġON		/app/img/icon_switch_on.png
	IMG_ENUM_SWITCH_OFF,				//	    OFF		/app/img/icon_switch_off.png
	IMG_ENUM_SWITCH_COUNT
};

#endif //__MAIN_H__


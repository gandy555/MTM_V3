#ifndef __GPIO_MTM_H__
#define __GPIO_MTM_H__

/*
	MTM Board GPIO Note:

	(Input)

	GPJ0~5	- 전면키버튼(좌우각 3개식)
			  GPJ5 +-------+ GPJ2
			  GPJ4 |  LCD  | GPJ1
			  GPJ3 +-------+ GPJ0

	GPF0,1	- 후면 볼륨버튼
			  GPF0 - VolUp
			  GPF1 - VolDown

	GPF3	- 출입문 닫힘감지	(High=닫힘)

	(Output)

	GPB0	- LCD BackLight	(Low=Off, High=On)

	GPG7	- LED (Low=Off, High=On)

	GPG2	- RS485 Control (Low=Off, High=On)

	GPG6	- Relay (Low=Off, High=On)

	GPG8	- Amp (Low=On, High=Off)
*/

enum	//MTM에서 사용하는 GPIO에 대한 일련번호
{
	//Input
	GPIO_FRONT_RIGHT_BOTTOM,		//GPJ0, Active Low
	GPIO_FRONT_RIGHT_MIDDLE,		//GPJ1, Active Low
	GPIO_FRONT_RIGHT_TOP,			//GPJ2, Active Low
	GPIO_FRONT_LEFT_BOTTOM,			//GPJ3, Active Low
	GPIO_FRONT_LEFT_MIDDLE,			//GPJ4, Active Low
	GPIO_FRONT_LEFT_TOP,			//GPJ5, Active Low

	GPIO_REAR_VOL_UP,				//GPF0
	GPIO_REAR_VOL_DOWN,				//GPF1
	
	GPIO_DOOR_DETECT,				//GPF3
	
	//Output
	GPIO_LCD_BACKLIGHT,				//GPG0
	GPIO_LED,						//GPG1
	GPIO_RS485_CTRL,				//GPG2
	GPIO_RELAY,						//GPG7
	GPIO_AMP,						//GPG8

	GPIO_KEY_COUNT
};

// GPIO Input Flag (Enum값과 순서일치)
#define GPIO_FLAG_FRONT_RIGHT_BOTTOM	(1<<0)
#define GPIO_FLAG_FRONT_RIGHT_MIDDLE	(1<<1)
#define GPIO_FLAG_FRONT_RIGHT_TOP		(1<<2)
#define GPIO_FLAG_FRONT_LEFT_BOTTOM		(1<<3)
#define GPIO_FLAG_FRONT_LEFT_MIDDLE		(1<<4)
#define GPIO_FLAG_FRONT_LEFT_TOP		(1<<5)
#define GPIO_FLAG_REAR_VOL_UP			(1<<6)
#define GPIO_FLAG_REAR_VOL_DOWN			(1<<7)
#define GPIO_FLAG_DOOR_DETECT			(1<<8)

#define LONG_KEY_DETECT_TICK			2000	//2초

enum
{
	MTM_GPIO_BUTTON_DOWN,
	MTM_GPIO_BUTTON_UP,
	MTM_GPIO_BUTTON_LONG,
	MTM_GPIO_STATE_CHANGE
};

class CGpioMtm : public CGpio
{
public:
	CGpioMtm();
	~CGpioMtm();

	//Member Function
	BOOL Init();
	void DeInit();

	BOOL Run();
	void Stop();

	void CheckGpioState(UINT gpio_enum, GPIO_GROUP group, int num, USHORT* pusGpioFlag);
	void SetGpioOutPin(UINT gpio_enum, LEVEL level);
	LEVEL GetGpioInputPin(UINT gpio_enum);

	BOOL CheckWatchdogReset();

	BOOL CheckGpioKeyEvent(USHORT* pusGpioFlag, USHORT* pusGpioEvent);

	static void* GpioMtmHandler(void *pParam);

	//Member Variable
	pthread_t		m_HandlerThread;
	BOOL			m_fRunning;

	USHORT			m_usGpioFlag;
	USHORT			m_usGpioEvent;
	BOOL			m_isKeyDown;
	BOOL			m_isLongKeyHandlerDone;
	ULONG			m_tickStartPress;

	BOOL			m_isWatchdogReset;
};

#endif //__GPIO_MTM_H__

/*
*/

#include "common.h"
//#include "gpio_mtm.h"
#include "main.h"

//
// Construction/Destruction
//
CGpioMtm::CGpioMtm()
{
	m_HandlerThread = 0;
	m_fRunning = FALSE;

	m_isKeyDown = FALSE;
	m_isLongKeyHandlerDone = FALSE;

	m_usGpioFlag = 0;
	m_usGpioEvent = 0;

	m_isWatchdogReset = FALSE;
}

CGpioMtm::~CGpioMtm()
{
}

//
// Member Function
//
BOOL CGpioMtm::Init()
{
	if(CGpio::Init()==FALSE)
	{
	//	printf("CGpio::Init() Failure\r\n");
		return FALSE;
	}

	//
	// MTM���� GPIO �ʱ�ȭ: Active Low
	//

	// ��ư
	SetDir(GPIOJ, 0, GPIO_IN);	//����	���ϴ�
	SetDir(GPIOJ, 1, GPIO_IN);	//		���߰�
	SetDir(GPIOJ, 2, GPIO_IN);	//		����
	SetDir(GPIOJ, 3, GPIO_IN);	//		���ϴ�
	SetDir(GPIOJ, 4, GPIO_IN);	//		���߰�
	SetDir(GPIOJ, 5, GPIO_IN);	//		�»��

	SetDir(GPIOF, 0, GPIO_IN);	//�ĸ�	������
	SetDir(GPIOF, 1, GPIO_IN);	//		�����ٿ�

	SetDir(GPIOF, 3, GPIO_IN);	//���Թ� ���� ���� (Low=����, High=����)

	// ��� (�ʱ���� Ȯ�ο�!)
	SetDir(GPIOB, 0, GPIO_OUT);	//LCD BackLight	(Low=Off, High=On)
	SetPin(GPIOB, 0, HIGH);

	SetDir(GPIOG, 7, GPIO_OUT);	//LED (Low=Off, High=On)
	SetPin(GPIOG, 7, HIGH);

	SetDir(GPIOG, 2, GPIO_OUT);	//RS485 Control (Low=Enable, High=Disable)
	SetPin(GPIOG, 2, HIGH);

	SetDir(GPIOG, 6, GPIO_OUT);	//Relay (Low=Off:�ϰ��ҵ�, High=On:�ҵ�����)
	SetPin(GPIOG, 6, HIGH);

	SetDir(GPIOG, 8, GPIO_OUT);	//Amp (Low=On, High=Off)
	SetPin(GPIOG, 8, LOW);

	m_isWatchdogReset = (ChkResetState() & 0x04) ? TRUE : FALSE;

	return TRUE;
}

void CGpioMtm::DeInit()
{
	Stop();

	CGpio::DeInit();
}

BOOL CGpioMtm::Run()
{
	int create_error;

	if(m_fRunning)
	{
		DBGMSG(DBG_GPIO, "%s: Warning!! : Thread already running\r\n", __func__);
		return TRUE;
	}

	m_fRunning = TRUE;

	create_error = pthread_create(&m_HandlerThread, NULL, GpioMtmHandler, this);
	if(create_error)
	{
		DBGMSG(DBG_GPIO, "%s: pthread_create failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		Stop();
	}

	return m_fRunning;
}

void CGpioMtm::Stop()
{
	void* thread_result;
	int ret;

	if(m_fRunning)
	{
		m_fRunning = FALSE;

		ret = pthread_join(m_HandlerThread, &thread_result);
		if(ret)
		{
			DBGMSG(DBG_GPIO, "%s: pthread_join failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		}
	}
}

void CGpioMtm::CheckGpioState(UINT gpio_enum, GPIO_GROUP group, int num, USHORT* pusGpioFlag)
{
	LEVEL level;

	if(pusGpioFlag==0) return;

	level = GetPin(group, num);

	if(!level)	//ActiveLow, GPIO KEY�� ������
		SET_FLAG(*pusGpioFlag, BIT_FLAG(gpio_enum));
	else
		CLR_FLAG(*pusGpioFlag, BIT_FLAG(gpio_enum));
}

void CGpioMtm::SetGpioOutPin(UINT gpio_enum, LEVEL level)
{
	switch(gpio_enum)
	{
	case GPIO_LCD_BACKLIGHT:	//GPG0	GPB0
		SetPin(GPIOB, 0, level);
		break;
	case GPIO_LED:				//GPG1	GPG7
		SetPin(GPIOG, 7, level);
		break;
	case GPIO_RS485_CTRL:		//GPG2	GPG2
		SetPin(GPIOG, 2, level);
		break;
	case GPIO_RELAY:			//GPG7	GPG6
		SetPin(GPIOG, 6, level);
		break;
	case GPIO_AMP:				//GPG8	GPG8
		SetPin(GPIOG, 8, level);
		break;
	default:
		DBGMSG(DBG_GPIO, "%s: invalid gpio_enum=%d\r\n", __func__, gpio_enum);
		break;
	}
}

LEVEL CGpioMtm::GetGpioInputPin(UINT gpio_enum)
{
	LEVEL level = OPEN;

	switch(gpio_enum)
	{
	case GPIO_FRONT_RIGHT_BOTTOM:	//GPJ0
		level = GetPin(GPIOJ, 0);
		break;
	case GPIO_FRONT_RIGHT_MIDDLE:	//GPJ1
		level = GetPin(GPIOJ, 1);
		break;
	case GPIO_FRONT_RIGHT_TOP:		//GPJ2
		level = GetPin(GPIOJ, 2);
		break;
	case GPIO_FRONT_LEFT_BOTTOM:	//GPJ3
		level = GetPin(GPIOJ, 3);
		break;
	case GPIO_FRONT_LEFT_MIDDLE:	//GPJ4
		level = GetPin(GPIOJ, 4);
		break;
	case GPIO_FRONT_LEFT_TOP:		//GPJ5
		level = GetPin(GPIOJ, 5);
		break;
	case GPIO_REAR_VOL_UP:			//GPF0
		level = GetPin(GPIOF, 0);
		break;
	case GPIO_REAR_VOL_DOWN:		//GPF1
		level = GetPin(GPIOF, 1);
		break;
	case GPIO_DOOR_DETECT:			//GPF3
		level = GetPin(GPIOF, 3);
		break;
	default:
		DBGMSG(DBG_GPIO, "%s: invalid gpio_enum=%d\r\n", __func__, gpio_enum);
		break;
	}

	return level;
}

BOOL CGpioMtm::CheckWatchdogReset()
{
	BOOL fRet = m_isWatchdogReset;

	if(fRet) m_isWatchdogReset = FALSE;

	return fRet;
}

BOOL CGpioMtm::CheckGpioKeyEvent(USHORT* pusGpioFlag, USHORT* pusGpioEvent)
{
	USHORT usGpioFlag = 0;
	BOOL fRet = FALSE;

	if( (pusGpioFlag==NULL) || (pusGpioEvent==NULL) ) return FALSE;

	//���� GPIO���¸� Flag�� ��ȯ��
	CheckGpioState(GPIO_FRONT_RIGHT_BOTTOM, GPIOJ, 0, &usGpioFlag);
	CheckGpioState(GPIO_FRONT_RIGHT_MIDDLE, GPIOJ, 1, &usGpioFlag);
	CheckGpioState(GPIO_FRONT_RIGHT_TOP,    GPIOJ, 2, &usGpioFlag);

	CheckGpioState(GPIO_FRONT_LEFT_BOTTOM,  GPIOJ, 3, &usGpioFlag);
	CheckGpioState(GPIO_FRONT_LEFT_MIDDLE,  GPIOJ, 4, &usGpioFlag);
	CheckGpioState(GPIO_FRONT_LEFT_TOP,     GPIOJ, 5, &usGpioFlag);

	CheckGpioState(GPIO_REAR_VOL_UP,		GPIOF, 0, &usGpioFlag);
	CheckGpioState(GPIO_REAR_VOL_DOWN,		GPIOF, 1, &usGpioFlag);

//	CheckGpioState(GPIO_DOOR_DETECT,		GPIOF, 3, &usGpioFlag);

	if(usGpioFlag)
	{
		if(m_isKeyDown==FALSE)	//ó�� ����������
		{
			m_isKeyDown = TRUE;
			m_tickStartPress = GetTickCount();		//������ ���� �ð�
		}
		else
		{
			//��� ���������� ���� Ű���տ� ��ȭ�� ������
			if(usGpioFlag != m_usGpioFlag)
			{
				m_tickStartPress = GetTickCount();	//������ ���۽ð� ����
			}
			
			//��� ���� �ð��� LongKey ���� �ð����� ũ��
			if(GetElapsedTick(m_tickStartPress) >= LONG_KEY_DETECT_TICK)
			{
				if(m_isLongKeyHandlerDone==FALSE)
				{
					m_isLongKeyHandlerDone = TRUE;

					*pusGpioFlag  = usGpioFlag;
					*pusGpioEvent = MTM_GPIO_BUTTON_LONG;

					fRet = TRUE;
				}
			}
		}
	}	
	else //�������� ������ (Idle or KeyUp)
	{
		if(m_usGpioFlag)
		{
			if(m_isLongKeyHandlerDone==FALSE)
			{
				*pusGpioFlag  = m_usGpioFlag;
				*pusGpioEvent = MTM_GPIO_BUTTON_UP;
			}
			else
			{
				*pusGpioFlag  = 0;
				*pusGpioEvent = MTM_GPIO_BUTTON_UP;
			}

			fRet = TRUE;
		}

		m_isKeyDown = FALSE;
		m_tickStartPress = 0;
		m_isLongKeyHandlerDone = FALSE;
	}

	m_usGpioFlag = usGpioFlag;

	return fRet;
}

/*
	Normal Action�� KeyUp���� ó���ϰ�
	Long Key Detection�� KeyDown���� ó����
*/
void* CGpioMtm::GpioMtmHandler(void *pParam)
{
	CGpioMtm *pThis = (CGpioMtm *)pParam;

	USHORT usGpioFlag, usGpioEvent;
	LEVEL  lvDoorOld, lvDoorNew;

	DBGMSG(DBG_GPIO, "%s: Start\r\n", __func__);

	lvDoorOld = pThis->GetGpioInputPin(GPIO_DOOR_DETECT);

	while(pThis->m_fRunning)
	{
		//if(pThis->CheckGpioKeyEvent(&usGpioFlag, &usGpioEvent))
		{
			//g_message.SendMessage(MSG_KEY_EVENT, usGpioFlag, usGpioEvent);
		}
		//else
		{
			lvDoorNew = pThis->GetGpioInputPin(GPIO_DOOR_DETECT);
			if(lvDoorNew != lvDoorOld)
			{
				lvDoorOld = lvDoorNew;
				g_message.SendMessage(MSG_DOOR_EVENT, (ULONG)lvDoorNew);
			}
		}


		usleep(50000);	//Ű���� �ֱ⸦ 50ms��������
	}

	DBGMSG(DBG_GPIO, "%s: End\r\n", __func__);

	pthread_exit(NULL); 
}

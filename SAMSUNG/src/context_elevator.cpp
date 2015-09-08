/*
*/
#include "common.h"
#include "context_elevator.h"
#include "main.h"

#define ELEVATOR_STAT_REQ				(1<<0)
#define ELEVATOR_STAT_RES_WAIT			(1<<1)
#define ELEVATOR_STAT_RES_TIMEOUT		(1<<2)
#define ELEVATOR_CALL_REQ				(1<<3)
#define ELEVATOR_CALL_RES_WAIT			(1<<4)
#define ELEVATOR_CALL_RES_TIMEOUT		(1<<5)

UCHAR	g_ElevatorTimerParam = 0;
UCHAR	g_ElevatorStatResTimeout = 0;
UCHAR	g_ElevatorCallResTimeout = 0;

//엘리베이터 타이머 핸들러 : 1초
void elevator_timer_handler(void *pParam)
{
	UCHAR* pucParam = (UCHAR *)pParam;

	if(g_state.GetState() != STATE_ELEVATOR)
	{
		g_timer.KillTimer(ELEVATOR_TIMER);
		return;
	}

	if(*pucParam & ELEVATOR_STAT_REQ)
	{
		if(*pucParam & ELEVATOR_STAT_RES_WAIT)
		{
			*pucParam |= ELEVATOR_STAT_RES_TIMEOUT;
			g_ElevatorStatResTimeout += ELEVATOR_REQ_TIME;
			if(g_ElevatorStatResTimeout >= ELEVATOR_RES_TIMEOUT)
			{
				printf("Elevator Stat Response Timeout!\r\n");
				g_timer.KillTimer(ELEVATOR_TIMER);
			}
		}
		else
		{
			*pucParam |= ELEVATOR_STAT_RES_WAIT;
			g_ElevatorStatResTimeout = 0;
			//g_wallpad_sns.RequestReserve(SNS_CMD_ELEVATOR_STAT_REQ);
		}
	}

	if(*pucParam & ELEVATOR_CALL_RES_WAIT)
	{
		*pucParam |= ELEVATOR_CALL_RES_TIMEOUT;
	}

	//g_wallpad_sns.RequestReserve(SNS_CMD_ELEVATOR_STAT_REQ);
}

//
// Construction/Destruction
//
CContextElevator::CContextElevator(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
	m_isElevatorStatusSuccess = FALSE;
	m_isElevatorCalled = FALSE;
	m_isElevatorCallSuccess = FALSE;
	m_isElevatorArrive = FALSE;

	m_nElevatorDir = 0;
	m_nElevatorFloor = 0;

	m_pObjectIcon = NULL;
}

CContextElevator::~CContextElevator()
{
}

//
// Member Function
//
void CContextElevator::Init()
{
	CObject* pObject;
	CObjectIcon* pObjectIcon;
	UINT id;

	CContext::Init();

	// Blank 배경 이미지
	pObject = new CObjectImage(m_wid_parent, m_gc, 0, 0, g_scr_info.cols, g_scr_info.rows);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/blank_bg.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 아이콘 이미지
	pObject = new CObjectImage(m_wid_parent, m_gc, 58, 50, 158, 158);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/icon_elevator.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 엘레베이터 이미지
	pObject = new CObjectImage(m_wid_parent, m_gc, 510, 175, 207, 258);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/elevator_image.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 엘레베이터 층표시 박스 이미지
	pObject = new CObjectImage(m_wid_parent, m_gc, 339, 240, 143, 127);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/elevator_display.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 엘레베이터 화살표
//	pObject = new CObjectIcon(m_wid_parent, m_gc, 336, 245, 137, 137);
	pObject = new CObjectIcon(m_wid_parent, m_gc, 232, 210, 86, 190);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_ARROW_COUNT);
		pObject->LoadImage(IMG_ENUM_ARROW_BLANK,	"/app/img/el_png/arrow_back.png");
		pObject->LoadImage(IMG_ENUM_ARROW_UP_1,		"/app/img/el_png/arrow_up_1.png");
		pObject->LoadImage(IMG_ENUM_ARROW_UP_2,		"/app/img/el_png/arrow_up_2.png");
		pObject->LoadImage(IMG_ENUM_ARROW_UP_3,		"/app/img/el_png/arrow_up_3.png");
		pObject->LoadImage(IMG_ENUM_ARROW_DOWN_1,	"/app/img/el_png/arrow_down_1.png");
		pObject->LoadImage(IMG_ENUM_ARROW_DOWN_2,	"/app/img/el_png/arrow_down_2.png");
		pObject->LoadImage(IMG_ENUM_ARROW_DOWN_3,	"/app/img/el_png/arrow_down_3.png");

		id = m_ObjectList.AddObject(pObject);
	}
}

void CContextElevator::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

void CContextElevator::Draw(UINT nContextNum)
{
	CObjectIcon* pObjectIcon = NULL;

	if(m_gc==0) return;

	printf("+CContextElevator::Draw(%d)\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(ELEVATOR_OBJ_BG);

	//아이콘
	m_ObjectList.Draw(ELEVATOR_OBJ_ICON);

	//텍스트
//	DrawText("엘레베이터를 호출하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	//엘레베이터 이미지
	m_ObjectList.Draw(ELEVATOR_OBJ_IMAGE);

	//엘레베이터 층표시 박스
	m_ObjectList.Draw(ELEVATOR_OBJ_DISPLAY);

#if 0
	switch(nContextNum)
	{
	case 0:
	#if 0
		DrawText("엘레베이터 호출중..",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	#else
		if(m_nElevatorDir != ELEVATOR_STOP)
			DrawText("엘레베이터가 이동중 입니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("엘레베이터가 정지해 있습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	#endif
		break;
	case 1:
	#if 0
		if(m_isElevatorCallSuccess)
			DrawText("엘레베이터를 호출하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("엘레베이터 호출이 실패하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	#else
	#endif
		break;
	}
#else
	DrawText("엘레베이터 상태 확인중 입니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
#endif

	printf("-CContextElevator::Draw\r\n");
}

void CContextElevator::Proc(UINT nContextNum)
{
	printf("+CContextElevator::Proc(%d)\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
		m_isElevatorStatusSuccess = FALSE;
		m_isElevatorCalled = FALSE;
		m_isElevatorCallSuccess = FALSE;
		m_isElevatorArrive = FALSE;

		m_nElevatorDir = 0;
		m_nElevatorFloor = 0;

	#if 0
		g_wallpad_sns.RequestReserve(SNS_CMD_ELEVATOR_CALL);
	#else
		//g_wallpad_sns.RequestReserve(SNS_CMD_ELEVATOR_STAT_REQ);
		g_ElevatorTimerParam |= ELEVATOR_STAT_REQ|ELEVATOR_STAT_RES_WAIT;
		g_ElevatorStatResTimeout = 0;
		g_timer.SetTimer(ELEVATOR_TIMER, ELEVATOR_REQ_TIME, elevator_timer_handler, "Elevator Timer", &g_ElevatorTimerParam);
	#endif

		m_pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(ELEVATOR_OBJ_ARROW);
		m_nThreadStep = 0;

		if( (g_wallpad_sns.m_isRequestReserve == FALSE) && (m_isElevatorCalled == FALSE) ) {
			g_wallpad_sns.RequestReserve(SNS_CMD_ELEVATOR_CALL);
			g_ElevatorTimerParam |= ELEVATOR_CALL_RES_WAIT;
			g_ElevatorCallResTimeout = 0;
			m_isElevatorCalled = TRUE;
			m_isElevatorCallSuccess = FALSE;
			m_isElevatorArrive = FALSE;
			g_timer.ResetTimer(ELEVATOR_TIMER);
			g_timer.SetTimer(BACK_LIGHT_TIMER, ELEVATOR_CALL_WAIT_TIME, back_light_timer_handler, "LCD BackLight Timer");
			printf("--> Elevator Call\r\n");
		}			

		ChangeContext(1);
		break;
	case 1:
//		PlayWavFile("/app/sound/elevator.wav\0");
		break;
	}

	printf("-CContextElevator::Proc\r\n");
}

void CContextElevator::TimerProc(UINT idTimer)
{
}

void CContextElevator::RecvProc(UCHAR *pPacket)
{
	ELEVATOR_STATUS*	pElevatorStatus;
	ELEVATOR_CALL_RES*	pElevatorCallRes;
	
	if(pPacket==NULL) return;

	printf("+CContextElevator::RecvProc\r\n");

	switch(pPacket[1])	//cmd
	{
	case SNS_CMD_ELEVATOR_STAT_RES:
//		g_timer.KillTimer(ELEVATOR_TIMER);
		g_ElevatorTimerParam &= ~(ELEVATOR_STAT_RES_WAIT|ELEVATOR_STAT_RES_TIMEOUT);
		g_timer.ResetTimer(ELEVATOR_TIMER);

		pElevatorStatus = (ELEVATOR_STATUS*)&(pPacket[3]);
		m_isElevatorStatusSuccess = (pElevatorStatus->status) ? TRUE : FALSE;

		m_nElevatorDir   = pElevatorStatus->dir;
		m_nElevatorFloor = pElevatorStatus->floor;

		//도착이후 움직이면
		if(m_isElevatorArrive && m_isElevatorCallSuccess && pElevatorStatus->dir)
		{
			m_isElevatorCallSuccess = FALSE;
			m_isElevatorArrive = FALSE;
		}

		break;
	case SNS_CMD_ELEVATOR_CALL:
		printf("%s: SNS_CMD_ELEVATOR_CALL\r\n", __func__);
		g_ElevatorTimerParam &= ~(ELEVATOR_CALL_RES_WAIT|ELEVATOR_CALL_RES_TIMEOUT);

		pElevatorCallRes = (ELEVATOR_CALL_RES*)&(pPacket[3]);
		m_isElevatorCallSuccess = (pElevatorCallRes->status) ? TRUE : FALSE;

		RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, ELEVATOR_OBJ_BG);
		if(m_isElevatorCallSuccess) {
			DrawText("엘레베이터를 호출하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
			PlayWavFile("/app/sound/elevator.wav\0");
		} else {
			DrawText("엘레베이터 호출이 실패하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		}
		break;
	case SNS_CMD_ELEVATOR_ARRIVE:
		printf("%s: SNS_CMD_ELEVATOR_ARRIVE\r\n", __func__);
	//	if(m_isElevatorCalled && m_isElevatorCallSuccess)
	//	{
			m_isElevatorCalled = FALSE;
		//	m_isElevatorCallSuccess = FALSE;
			m_isElevatorArrive = TRUE;
	//	}

		RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, ELEVATOR_OBJ_BG);
		DrawText("엘레베이터가 도착 하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
		break;
	}

	printf("-CContextElevator::RecvProc\r\n");
}

void CContextElevator::ThreadProc()
{
	static int nArrowStep;
	static ULONG ulTickDraw;
	char szText[32] = {0,};

	//호출이 성공하고 엘리베이터 상태를 모니터링 하는 동안 동작함
//	if(m_nContextNum==0) return;

	switch(m_nThreadStep)
	{
	case 0:
		nArrowStep = 0;
		m_nThreadStep++;
		break;
	case 1:
		if( (m_isElevatorStatusSuccess||m_isElevatorArrive) && m_pObjectIcon)
		{
			if( (m_isElevatorCallSuccess==FALSE) && (g_ElevatorTimerParam & ELEVATOR_CALL_RES_TIMEOUT) )
			{
				RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, ELEVATOR_OBJ_BG);
				DrawText("엘레베이터 호출이 실패하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
			}
			else if(m_isElevatorArrive)
			{
				RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, ELEVATOR_OBJ_BG);
				DrawText("엘레베이터가 도착 하였습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
			}
			else
			{
				RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, ELEVATOR_OBJ_BG);
				if(m_nElevatorDir != ELEVATOR_STOP)
					DrawText("엘레베이터가 이동중 입니다",    m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
				else
					DrawText("엘레베이터가 정지해 있습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
			}

			//방향표시
			if( (m_nElevatorDir == ELEVATOR_STOP) || (m_isElevatorArrive) )
			{
				m_pObjectIcon->Draw(IMG_ENUM_ARROW_BLANK);
				nArrowStep = 0;
			}
			else if(m_nElevatorDir == ELEVATOR_DOWN)
			{
				switch(nArrowStep)
				{
				case 0:
					m_pObjectIcon->Draw(IMG_ENUM_ARROW_DOWN_1);
					break;
				case 1:
					m_pObjectIcon->Draw(IMG_ENUM_ARROW_DOWN_2);
					break;
				default:
					m_pObjectIcon->Draw(IMG_ENUM_ARROW_DOWN_3);
					break;
				}

				nArrowStep++;
				if(nArrowStep==5) nArrowStep=0;
			}
			else if(m_nElevatorDir == ELEVATOR_UP)
			{
				switch(nArrowStep)
				{
				case 0:
					m_pObjectIcon->Draw(IMG_ENUM_ARROW_UP_1);
					break;
				case 1:
					m_pObjectIcon->Draw(IMG_ENUM_ARROW_UP_2);
					break;
				default:
					m_pObjectIcon->Draw(IMG_ENUM_ARROW_UP_3);
					break;
				}

				nArrowStep++;
				if(nArrowStep==5) nArrowStep=0;
			}

			//층표시
			if(m_nElevatorFloor > 0)
			{
				if(m_nElevatorFloor >= 128)
	//				sprintf(szText, "B%2d\0", 256-m_nElevatorFloor);
					sprintf(szText, "B%d\0", 256-m_nElevatorFloor);
				else
	//					sprintf(szText, "%3d\0", m_nElevatorFloor);
					sprintf(szText, "%d\0", m_nElevatorFloor);
			}
			else if(m_nElevatorFloor < 0)
			{
	//			sprintf(szText, "B%2d\0", m_nElevatorFloor*(-1));
				sprintf(szText, "B%d\0", m_nElevatorFloor*(-1));
			}
			else
			{
				sprintf(szText, "L\0");
			}

		#if 0
			DrawText(szText, m_wid_parent, m_gc,  339+13, 240+13, 143-26, 127-26, g_font, 72, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		#else
			RedrawImage(m_wid_parent, m_gc, 352, 253, 117, 101, 12, 12, 117, 101, ELEVATOR_OBJ_DISPLAY);
	//		DrawText(szText, m_wid_parent, m_gc,  352, 253, 117, 101, g_font, 80, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_TOP);
			DrawText(szText, m_wid_parent, m_gc,  352, 253, 117, 101, g_font, 80, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_TOP);
		#endif

			ulTickDraw = GetTickCount();
			m_nThreadStep++;
		}
		break;
	case 2:
		if(GetElapsedTick(ulTickDraw) >= 500)
		{
			m_nThreadStep--;
		}
		break;
	}
}

void CContextElevator::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextElevator::ButtonDown\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_DOWN)
	{
	}
	else if(usEventEnum == MTM_GPIO_BUTTON_LONG)
	{
	#if 0
		if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)|FLAG(GPIO_FRONT_RIGHT_BOTTOM)) ||
			CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)|FLAG(GPIO_REAR_VOL_DOWN)) )
		{
			g_state.SetNextState(STATE_SETUP);
		}
	#endif
	}
	printf("-CContextElevator::ButtonDown\r\n");
}

void CContextElevator::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextElevator::ButtonUp\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_UP)
	{
		switch(usGpioFlag)
		{
		case GPIO_FLAG_FRONT_LEFT_TOP:		//Weather
			g_state.SetNextState(STATE_WEATHER);
			break;
		case GPIO_FLAG_FRONT_LEFT_MIDDLE:	//Elevator
		//	g_state.SetNextState(STATE_ELEVATOR);
			if( (g_wallpad_sns.m_isRequestReserve == FALSE) && (m_isElevatorCalled == FALSE) )
			{
				g_wallpad_sns.RequestReserve(SNS_CMD_ELEVATOR_CALL);
				g_ElevatorTimerParam |= ELEVATOR_CALL_RES_WAIT;
				g_ElevatorCallResTimeout = 0;
				m_isElevatorCalled = TRUE;
				m_isElevatorCallSuccess = FALSE;
				m_isElevatorArrive = FALSE;
				g_timer.ResetTimer(ELEVATOR_TIMER);
				g_timer.SetTimer(BACK_LIGHT_TIMER, ELEVATOR_CALL_WAIT_TIME, back_light_timer_handler, "LCD BackLight Timer");
				printf("--> Elevator Call\r\n");
			}
			else
			{
				printf("--> Elevator Call Skip\r\n");
			}
			break;
		case GPIO_FLAG_FRONT_LEFT_BOTTOM:	//Parking
			g_state.SetNextState(STATE_PARKING);
			break;
		case GPIO_FLAG_FRONT_RIGHT_TOP:		//Gas
			g_state.SetNextState(STATE_GAS);
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//AllOff - 반전동작
			g_state.SetNextState(STATE_ALLOFF);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.SetNextState(STATE_SECURITY);
			break;
		}
		//백라이트 타이머 원상복귀 : 10초
		if(usGpioFlag != GPIO_FLAG_FRONT_LEFT_MIDDLE)
		{
			g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
		}
	}

	printf("-CContextElevator::ButtonUp\r\n");
}


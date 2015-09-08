/*
*/
#include "common.h"
#include "context_parking.h"
#include "main.h"

//BOOL g_isSearching = FALSE;

//주차위치 확인 타이머
void parking_timer_handler(void *pParam)
{
	CContext* pContext = NULL;

	pContext = g_state.GetCurrContext();
	if(pContext)
	{
		switch(g_state.GetState())
		{
		case STATE_PARKING:
			pContext->TimerProc(PARKING_TIMER);
			break;
		}
	}

}

//
// Construction/Destruction
//
CContextParking::CContextParking(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
	m_isSearching = FALSE;
}

CContextParking::~CContextParking()
{
}

//
// Member Function
//
void CContextParking::Init()
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
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/icon_parking.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 주차 이미지
	pObject = new CObjectImage(m_wid_parent, m_gc, 245, 195, 511, 257);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/parking_image1.png");

		id = m_ObjectList.AddObject(pObject);
	}

}

void CContextParking::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

void CContextParking::Draw(UINT nContextNum)
{
	PARKING_RES parking_info;
	char szText[64] = {0,};
	int i, line_feed = 0, z_offset;

	if(m_gc==0) return;

	printf("+CContextParking::Draw(%d)\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(PARKING_OBJ_BG);

	//아이콘
	m_ObjectList.Draw(PARKING_OBJ_ICON);

	//주차 이미지
	m_ObjectList.Draw(PARKING_OBJ_IMAGE);

	parking_list_get_item(0, &parking_info);
	if (parking_info.in_out == PARKING_STATUS_IN) {
		szText[0] = '[';
		parking_item_to_str(PARKING_ITM_FLOOR, parking_info.car_floor, &szText[1]);
		z_offset = strlen(szText);
		sprintf(&szText[z_offset], "]층 [");
		
		z_offset = strlen(szText);
		parking_item_to_str(PARKING_ITM_ZONE, parking_info.car_zone, &szText[z_offset]);
		z_offset = strlen(szText);
		sprintf(&szText[z_offset], "]에 주차 되었습니다");

		DrawText(szText,  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	} else {
		DrawText("주차위치를 확인할 수 없습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	}

//	37,251 - 209,450 : w=172, h=200
	RedrawImage(g_wid, g_gc, 37, 251, 172, 200, 37, 251, 172, 200, PARKING_OBJ_BG);

	for (i = 1; i < MAX_PARKING_LIST; i++) {
		parking_list_get_item(i, &parking_info);
		if (parking_info.in_out != PARKING_STATUS_IN)
			break;

		memset(szText, 0, 64);
		parking_item_to_str(PARKING_ITM_FLOOR, parking_info.car_floor, szText);
		z_offset = strlen(szText);
		sprintf(&szText[z_offset], "-");

		z_offset = strlen(szText);
		parking_item_to_str(PARKING_ITM_ZONE, parking_info.car_zone, &szText[z_offset]);
		
		DrawText(szText,  m_wid_parent, m_gc, 37, 251+line_feed*36, 172, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		line_feed++;
	}

	printf("-CContextParking::Draw\r\n");
}

void CContextParking::Proc(UINT nContextNum)
{
	printf("+CContextParking::Proc(%d)\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
		g_wallpad_sns.RequestReserve(SNS_CMD_PARKING_REQ);

		m_isSearching = TRUE;

		g_timer.SetTimer(PARKING_TIMER, PARKING_REQ_TIME, parking_timer_handler, "Parking Timer");
		break;
	case 1:
		PlayWavFile("/app/sound/parking_short.wav\0");
		break;
	case 2:
		break;
	}

	printf("-CContextParking::Proc\r\n");
}

void CContextParking::TimerProc(UINT idTimer)
{
	PARKING_RES parking_info;
	
	if(idTimer == PARKING_TIMER)
	{
		g_timer.KillTimer(idTimer);

		parking_list_get_item(0, &parking_info);
		if (parking_info.in_out != PARKING_STATUS_IN) {
			RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, PARKING_OBJ_BG);
			DrawText("주차위치를 확인할 수 없습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		}
		m_isSearching = FALSE;
	}
}

void CContextParking::RecvProc(UCHAR *pPacket)
{
	PARKING_RES*	pParkingRes;
	char szText[64];

	if(pPacket==NULL) return;

	printf("+CContextParking::RecvProc\r\n");

	g_timer.KillTimer(PARKING_TIMER);

	pParkingRes = (PARKING_RES*)&(pPacket[3]);

	RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, PARKING_OBJ_BG);
	if(pParkingRes->in_out)
	{
		DrawText("출차 되었습니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	}
	
	if (g_isBackLightOn) {
		if (pParkingRes->index == 0xFF)
			ChangeContext(1);
		else
			ChangeContext(2);	// draw 호출.
	}
	m_isSearching = FALSE;

	printf("-CContextParking::RecvProc\r\n");
}

void CContextParking::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextParking::ButtonDown\r\n");

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

	printf("-CContextParking::ButtonDown\r\n");
}

void CContextParking::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextParking::ButtonUp\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_UP)
	{
		switch(usGpioFlag)
		{
		case GPIO_FLAG_FRONT_LEFT_TOP:		//Weather
			g_state.SetNextState(STATE_WEATHER);
			break;
		case GPIO_FLAG_FRONT_LEFT_MIDDLE:	//Elevator
			g_state.SetNextState(STATE_ELEVATOR);
			break;
		case GPIO_FLAG_FRONT_LEFT_BOTTOM:	//Parking
		//	g_state.SetNextState(STATE_PARKING);
			if(m_isSearching == FALSE)
			{
				RedrawImage(m_wid_parent, m_gc, 245, 89, 500, 32, 245, 89, 500, 32, PARKING_OBJ_BG);
				DrawText("주차위치를 확인중 입니다",  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

				g_wallpad_sns.RequestReserve(SNS_CMD_PARKING_REQ);

				g_timer.SetTimer(PARKING_TIMER, PARKING_REQ_TIME, parking_timer_handler, "Parking Timer");
			}
			ChangeContext(2);	
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
	}

	printf("-CContextParking::ButtonUp\r\n");
}


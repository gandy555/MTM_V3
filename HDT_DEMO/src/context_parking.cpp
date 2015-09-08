/*
*/
#include "common.h"
#include "context_parking.h"
#include "main.h"


//
// Global Varialble
//

//
// Construction/Destruction
//
CContextParking::CContextParking(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
	int i;

	m_isSearching = FALSE;

	m_isParkingInfoRequest = FALSE;
//	m_isParkingInfo = FALSE;

/*
	for(i=0; i<MAX_PARKING_HISTORY; i++)
	{
		g_ParkingHistory[i].status = MTM_DATA_PARKING_STATUS_NOINFO;
	}
*/
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
	MTM_DATA_PARKING parking_info;
	char szText[64] = {0,};
	int i, line_feed = 0;

	if(m_gc==0) return;

	DBGMSGC(DBG_PARKING, "++ [%d]\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(PARKING_OBJ_BG);

	//아이콘
	m_ObjectList.Draw(PARKING_OBJ_ICON);

	//주차 이미지
	m_ObjectList.Draw(PARKING_OBJ_IMAGE);

	parking_list_get_item(0, &parking_info);
	if (parking_info.status == MTM_DATA_PARKING_STATUS_IN) {
		sprintf(szText, "[%s]층 [%s]에 주차 되었습니다", parking_info.floor, parking_info.zone);
		DrawText(szText,  m_wid_parent, m_gc,  245, 89, 500, 32, g_font, 24,
			WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	} else {
		DrawText("주차위치를 확인할 수 없습니다", m_wid_parent, m_gc,  245, 89, 500, 32,
			g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	}

//	37,251 - 209,450 : w=172, h=200
	RedrawImage(g_wid, g_gc, 37, 251, 172, 200, 37, 251, 172, 200, PARKING_OBJ_BG);

	for (i = 1; i < MAX_PARKING_LIST; i++) {
		parking_list_get_item(i, &parking_info);
		if (parking_info.status != MTM_DATA_PARKING_STATUS_IN)
			break;

		memset(szText, 0, 64);
		sprintf(szText, "%s -%s", parking_info.floor, parking_info.zone);
		DrawText(szText,  m_wid_parent, m_gc, 37, 251+line_feed*36, 172, 32, g_font,
			24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		line_feed++;
	}

	DBGMSGC(DBG_PARKING, "--\r\n");
}

void CContextParking::Proc(UINT nContextNum)
{
	DBGMSGC(DBG_PARKING, "++ [%d]\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
		if (g_pWallPad) {
			g_pWallPad->RequestParkingInfo();
		}
		PlayWavFile("/app/sound/parking_short.wav\0");
		break;
	case 1:
		break;
	}

	DBGMSGC(DBG_PARKING, "--\r\n");
}

void CContextParking::TimerProc(UINT idTimer)
{
	switch(idTimer)
	{
	case RESPONSE_TIMER:
		if (g_pWallPad) {
			g_pWallPad->RequestParkingInfo();
		}
		break;
	case RETRY_TIMEOUT:
	//	ChangeContext(1);
		break;
	}
}

void CContextParking::RecvProc(UCHAR *pPacket)
{
	PMTM_HEADER pHdr = (PMTM_HEADER)pPacket;
	PMTM_DATA_PARKING pParkingInfo;

	int i, index;

	if(pPacket==NULL) return;

	DBGMSGC(DBG_PARKING, "++\r\n");

	switch(pHdr->type)
	{
	case MTM_DATA_TYPE_ACK:
		break;
	case MTM_DATA_TYPE_WEATHER:
		break;
	case MTM_DATA_TYPE_PARKING:
		if (g_isBackLightOn) {
			pParkingInfo = (PMTM_DATA_PARKING)&pPacket[sizeof(MTM_HEADER)];
			ChangeContext(0);
		}
		break;
	case MTM_DATA_TYPE_ELEVATOR:
		break;
	case MTM_DATA_TYPE_GAS:
		break;
	case MTM_DATA_TYPE_LIGHT:
		break;
	case MTM_DATA_TYPE_SECURITY:
		break;
	}

	DBGMSGC(DBG_PARKING, "--\r\n");
}

void CContextParking::ThreadProc()
{
}

void CContextParking::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_PARKING, "++\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_DOWN)
	{
	}
	else if(usEventEnum == MTM_GPIO_BUTTON_LONG)
	{
#if 0	
		if( CHK_FLAG(usGpioFlag, BIT_FLAG(GPIO_FRONT_RIGHT_TOP)|BIT_FLAG(GPIO_FRONT_RIGHT_BOTTOM)) ||
			CHK_FLAG(usGpioFlag, BIT_FLAG(GPIO_REAR_VOL_UP)|BIT_FLAG(GPIO_REAR_VOL_DOWN)) )
		{
			g_state.ChangeState(STATE_SETUP);
		}
#endif		
	}

	DBGMSGC(DBG_PARKING, "--\r\n");
}

void CContextParking::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_PARKING, "++\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_UP)
	{
		switch(usGpioFlag)
		{
		case GPIO_FLAG_FRONT_LEFT_TOP:		//Weather
			g_state.ChangeState(STATE_WEATHER);
			break;
		case GPIO_FLAG_FRONT_LEFT_MIDDLE:	//Elevator
			g_state.ChangeState(STATE_ELEVATOR);
			break;
		case GPIO_FLAG_FRONT_LEFT_BOTTOM:	//Parking
		//	if(g_pWallPad)
		//	{
		//		g_pWallPad->RequestParkingInfo();
		//	}
			ChangeContext(0);
			break;
		case GPIO_FLAG_FRONT_RIGHT_TOP:		//Gas
			g_state.ChangeState(STATE_GAS);
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//Light
			g_state.ChangeState(STATE_LIGHT);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.ChangeState(STATE_SECURITY);
			break;
		}
	}

	DBGMSGC(DBG_PARKING, "--\r\n");
}

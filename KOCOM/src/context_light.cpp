/*
*/
#include "common.h"
#include "context_light.h"
#include "main.h"

//
// Construction/Destruction
//
CContextLight::CContextLight(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
}

CContextLight::~CContextLight()
{
}

//
// Member Function
//
void CContextLight::Init()
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
	pObject = new CObjectIcon(m_wid_parent, m_gc, 58, 50, 158, 158);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_ICON_COUNT);
		pObject->LoadImage(IMG_ENUM_ICON_ON,	"/app/img/icon_light_on.png");
		pObject->LoadImage(IMG_ENUM_ICON_OFF,	"/app/img/icon_light_off.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 스위치 이미지
	pObject = new CObjectIcon(m_wid_parent, m_gc, 312, 287, 299, 116);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_SWITCH_COUNT);
		pObject->LoadImage(IMG_ENUM_SWITCH_ON,	"/app/img/icon_switch_on.png");
		pObject->LoadImage(IMG_ENUM_SWITCH_OFF,	"/app/img/icon_switch_off.png");

		id = m_ObjectList.AddObject(pObject);
	}
}

void CContextLight::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

//램프가 켜져있다는것은 일괄소등 SW가 OFF됬다는 뜻
void CContextLight::Draw(UINT nContextNum)
{
	CObjectIcon* pObjectIcon = NULL;

	if(m_gc==0) return;

	DBGMSGC(DBG_LIGHT, "++ [%d]\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(LIGHT_OBJ_BG);

	//아이콘
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(LIGHT_OBJ_ICON);
	if(pObjectIcon)
	{
		pObjectIcon->Draw(g_setup_data.m_SetupData.light_stat ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
	}

	//텍스트
	DrawText("일괄소등해제", m_wid_parent, m_gc, 144, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	DrawText("일괄소등", m_wid_parent, m_gc, 625, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	switch(nContextNum)
	{
	case 0:
		//텍스트
		if(g_setup_data.m_SetupData.wallpad_type == WALLPAD_TYPE_SHN)
		{
			DrawText("월패드 상태요청중..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		}
		else
		{
			if(g_setup_data.m_SetupData.light_stat==1)
				DrawText("일괄소등되었습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
			else
				DrawText("일괄소등 해제되었습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		}
		break;
	case 1:
		//텍스트
		if(g_setup_data.m_SetupData.light_stat==1)
			DrawText("일괄소등되었습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("일괄소등 해제되었습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		break;
	case 2:
		DrawText("통신에러: 다시 눌러 주세요", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		break;
	}

	//스위치
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(LIGHT_OBJ_SWITCH);
	if(pObjectIcon)
	{
		pObjectIcon->Draw(g_setup_data.m_SetupData.light_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
	}

	DBGMSGC(DBG_LIGHT, "--\r\n");
}

void CContextLight::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "light_on\0", "light_off\0" };	//ON=소등해제, OFF=소등
	char szWaveFilePath[128] = {0,};

	DBGMSGC(DBG_LIGHT, "++ [%d]\r\n", nContextNum);

	m_requested_val = g_setup_data.m_SetupData.light_stat;	/* gandy 2015-03-30 */

	switch(nContextNum)
	{
	case 0:
	#if 0
		if(g_pWallPad)
		{
			g_pWallPad->RequestLightStatus();
		}
	#endif
		break;
	case 1:
		sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.light_stat]);
		PlayWavFile(szWaveFilePath);
	//	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (g_setup_data.m_SetupData.light_stat) ? LOW : HIGH);	//LOW=소등, HIGH=소등해제
		break;
	}

	DBGMSGC(DBG_LIGHT, "--\r\n");
}

void CContextLight::TimerProc(UINT idTimer)
{
	switch(idTimer)
	{
	case RESPONSE_TIMER:
		if(g_pWallPad)
		{
			// light_stat: 0=점등(소등해제) --> TRUE=소등 
			//			   1=소등           --> FALSE=점등(소등해제)
			g_pWallPad->RequestLightSet( (g_setup_data.m_SetupData.light_stat) ? FALSE : TRUE );
		}
		break;
	}
}

void CContextLight::RecvProc(UCHAR *pPacket)
{
	PMTM_HEADER pHdr = (PMTM_HEADER)pPacket;
	PMTM_DATA_LIGHT pLightInfo;

	if(pPacket==NULL) return;

	DBGMSGC(DBG_LIGHT, "++\r\n");

	//DBGDMP(DBG_LIGHT, pPacket, 21, TRUE);

	switch(pHdr->type)
	{
	case MTM_DATA_TYPE_WEATHER:
		break;
	case MTM_DATA_TYPE_PARKING:
		break;
	case MTM_DATA_TYPE_ELEVATOR:
		break;
	case MTM_DATA_TYPE_GAS:
		break;
	case MTM_DATA_TYPE_LIGHT:
		if(g_isBackLightOn)
		{
			pLightInfo = (PMTM_DATA_LIGHT)&pPacket[sizeof(MTM_HEADER)];
			if (m_requested_val != pLightInfo->status) {	/* gandy 2015-03-30 */
				ChangeContext(1);
			}
		}
		break;
	case MTM_DATA_TYPE_SECURITY:
		break;
	}

	DBGMSGC(DBG_LIGHT, "--\r\n");
}

void CContextLight::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_LIGHT, "++\r\n");

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

	DBGMSGC(DBG_LIGHT, "--\r\n");
}

void CContextLight::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_LIGHT, "++\r\n");

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
			g_state.ChangeState(STATE_PARKING);
			break;
		case GPIO_FLAG_FRONT_RIGHT_TOP:		//Gas
			g_state.ChangeState(STATE_GAS);
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//Light
			if(g_pWallPad)
			{
				// light_stat: 0=점등(소등해제) --> TRUE=소등 
				//			   1=소등           --> FALSE=점등(소등해제)
				g_pWallPad->RequestLightSet( (g_setup_data.m_SetupData.light_stat) ? FALSE : TRUE );

				// 조명 키고 끄는것은 MTM이 주체다! - 손정우
				// 현재상태와 반전동작
				g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (g_setup_data.m_SetupData.light_stat) ? HIGH : LOW);	//LOW=소등, HIGH=소등해제

				// 제어가 이루어 졌다고 가정하고 상태를 반전시킴 (월패드의 응답이 없을때 반전동작이 이루어 지지 않는 문제때문)
				// 정상적으로 월패드가 응답한다면 응답상의 결과가 반영됨
				g_setup_data.m_SetupData.light_stat = (g_setup_data.m_SetupData.light_stat==1) ? 0 : 1;
				m_requested_val = g_setup_data.m_SetupData.light_stat;	/* gandy 2015-03-30 */
				// 화면갱신
				ChangeContext(1);
			}
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.ChangeState(STATE_SECURITY);
			break;
		}
	}

	DBGMSGC(DBG_LIGHT, "--\r\n");
}


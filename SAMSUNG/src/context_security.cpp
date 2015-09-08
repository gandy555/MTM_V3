/*
*/
#include "common.h"
#include "context_security.h"
#include "main.h"

//
// Construction/Destruction
//
CContextSecurity::CContextSecurity(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
//	m_isWallPadStatus = FALSE;
	m_isSecurityOn = FALSE;
}

CContextSecurity::~CContextSecurity()
{
}

//
// Member Function
//
void CContextSecurity::Init()
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
		pObject->LoadImage(IMG_ENUM_ICON_ON,	"/app/img/icon_security_on.png");
		pObject->LoadImage(IMG_ENUM_ICON_OFF,	"/app/img/icon_security_off.png");

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

	m_isSecurityOn = (g_setup_data.m_SetupData.security_stat) ? TRUE : FALSE;
}

void CContextSecurity::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

void CContextSecurity::Draw(UINT nContextNum)
{
	CObjectIcon* pObjectIcon = NULL;

	if(m_gc==0) return;

	printf("+CContextSecurity::Draw(%d)\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(SECURITY_OBJ_BG);

	//아이콘
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(SECURITY_OBJ_ICON);
	if(pObjectIcon)
	{
//		pObjectIcon->Draw(m_isSecurityOn ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
		pObjectIcon->Draw(g_setup_data.m_SetupData.security_stat ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
	}

	//텍스트
	DrawText("방범해제", m_wid_parent, m_gc, 144, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	DrawText("방범설정", m_wid_parent, m_gc, 625, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	switch(nContextNum)
	{
	case 0:
		//텍스트
		DrawText("월패드 상태요청중..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//스위치 이미지
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(SECURITY_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.security_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
		}
		break;
	case 1:
		//텍스트
	//	if(m_isSecurityOn)
		if(g_setup_data.m_SetupData.security_stat)
			DrawText("자동방범이 설정되었습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("자동방범이 해제되었습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//스위치 이미지
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(SECURITY_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(m_isSecurityOn ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.security_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
		}
		break;
	}

	printf("-CContextSecurity::Draw\r\n");
}

void CContextSecurity::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "police_off\0", "police_on\0" };
	char szWaveFilePath[128] = {0,};

	printf("+CContextSecurity::Proc(%d)\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
		g_wallpad_sns.RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
		break;
	case 1:
	//	sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[m_isSecurityOn]);
		sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.security_stat]);
		PlayWavFile(szWaveFilePath);
		break;
	}

	printf("-CContextSecurity::Proc\r\n");
}

void CContextSecurity::TimerProc(UINT idTimer)
{
}

void CContextSecurity::RecvProc(UCHAR *pPacket)
{
	if(pPacket==NULL) return;

	printf("+CContextSecurity::RecvProc\r\n");

	switch(pPacket[1])	//cmd
	{
	case SNS_CMD_WALLPAD_STAT_REQ:
	//	m_isWallPadStatus = TRUE;
		m_isSecurityOn = (g_setup_data.m_SetupData.security_stat) ? TRUE : FALSE;
		if (g_isBackLightOn)
			ChangeContext(1);
		break;
//	case SNS_CMD_LAMP_ONOFF_REQ:
//		break;
	}

	printf("-CContextSecurity::RecvProc\r\n");
}

void CContextSecurity::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextSecurity::ButtonDown\r\n");

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

	printf("-CContextSecurity::ButtonDown\r\n");
}

void CContextSecurity::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextSecurity::ButtonUp\r\n");

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
			g_state.SetNextState(STATE_PARKING);
			break;
		case GPIO_FLAG_FRONT_RIGHT_TOP:		//Gas
			g_state.SetNextState(STATE_GAS);
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//AllOff - 반전동작
			g_state.SetNextState(STATE_ALLOFF);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
		//	g_state.SetNextState(STATE_SECURITY);

			//외출설정요청(0x0B)을 보내지만 g_wallpad_sns에서 요청이 성공한 경우 다시 월패드 상태를 요청해서 보냄
			g_wallpad_sns.RequestReserve(SNS_CMD_OUT_ONOFF_REQ, (m_isSecurityOn) ? 0 : 1);
			break;
		}
	}
	printf("-CContextSecurity::ButtonUp\r\n");
}


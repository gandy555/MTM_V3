/*
*/
#include "common.h"
#include "context_gas.h"
#include "main.h"

//
// Construction/Destruction
//
CContextGas::CContextGas(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
//	m_isWallPadStatus = FALSE;
	m_isGasOff = FALSE;
}

CContextGas::~CContextGas()
{
}

//
// Member Function
//
void CContextGas::Init()
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
		pObject->LoadImage(IMG_ENUM_ICON_ON,	"/app/img/icon_gas_on.png");
		pObject->LoadImage(IMG_ENUM_ICON_OFF,	"/app/img/icon_gas_off.png");

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

	m_isGasOff = (g_setup_data.m_SetupData.gas_stat) ? FALSE : TRUE;	//반대
}

void CContextGas::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

void CContextGas::Draw(UINT nContextNum)
{
	CObjectIcon* pObjectIcon;

	if(m_gc==0) return;

	printf("+CContextGas::Draw(%d)\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(GAS_OBJ_BG);

	//아이콘
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_ICON);
	if(pObjectIcon)
	{
	//	pObjectIcon->Draw(m_isGasOff ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
		pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);	//ICON_ON=사용중, ICON_OFF=차단
	}

	//텍스트
	DrawText("가스사용", m_wid_parent, m_gc, 144, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	DrawText("가스차단", m_wid_parent, m_gc, 625, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	switch(nContextNum)
	{
	case 0:
		//텍스트
		DrawText("월패드 상태요청중..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//스위치 이미지
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_OFF : IMG_ENUM_SWITCH_ON);	//스위치는 반대 OFF=사용중, ON=차단
		}
		break;
	case 1:
		//텍스트
	//	if(m_isGasOff)
		if(g_setup_data.m_SetupData.gas_stat==0)
			DrawText("가스를 차단하였습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("가스를 사용중입니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//스위치 이미지
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(m_isGasOff ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_OFF : IMG_ENUM_SWITCH_ON);
		}		
		break;
	}

	printf("-CContextGas::Draw\r\n");
}

void CContextGas::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "gas_off\0", "gas_on\0" };
	char szWaveFilePath[128] = {0,};

	printf("+CContextGas::Proc(%d)\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
		g_wallpad_sns.RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
		break;
	case 1:
	//	sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[m_isGasOff]);
		sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.gas_stat]);
		PlayWavFile(szWaveFilePath);
		break;
	}

	printf("-CContextGas::Proc\r\n");
}

void CContextGas::TimerProc(UINT idTimer)
{
}

void CContextGas::RecvProc(UCHAR *pPacket)
{
	if(pPacket==NULL) return;

	printf("+CContextGas::RecvProc\r\n");

	switch(pPacket[1])	//cmd
	{
	case SNS_CMD_WALLPAD_STAT_REQ:
	case SNS_CMD_GAS_STAT_RES:
//		m_isWallPadStatus = TRUE;
		m_isGasOff = (g_setup_data.m_SetupData.gas_stat) ? FALSE : TRUE;	//반대
		if (g_isBackLightOn)
			ChangeContext(1);
		break;
//	case SNS_CMD_LAMP_ONOFF_REQ:
//		break;
	}

	printf("-CContextGas::RecvProc\r\n");
}

void CContextGas::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextGas::ButtonDown\r\n");

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

	printf("-CContextGas::ButtonDown\r\n");
}

void CContextGas::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextGas::ButtonUp\r\n");

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
		//	g_state.SetNextState(STATE_GAS);

			//가스밸프OFF요청(0x0A)을 보내지만 g_wallpad_sns에서 요청이 성공한 경우 다시 월패드 상태를 요청해서 보냄
			g_wallpad_sns.RequestReserve(SNS_CMD_GAS_OFF_REQ, 1);	//차단Only
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//AllOff - 반전동작
			g_state.SetNextState(STATE_ALLOFF);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.SetNextState(STATE_SECURITY);
			break;

		}
	}

	printf("-CContextGas::ButtonUp\r\n");
}


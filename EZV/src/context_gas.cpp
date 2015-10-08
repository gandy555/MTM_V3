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
	m_isWallPadStatus = FALSE;
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
		pObject->LoadImage(IMG_ENUM_SWITCH_ON,	"/app/img/icon_switch_on.png");		//녹색 ON  차단중
		pObject->LoadImage(IMG_ENUM_SWITCH_OFF,	"/app/img/icon_switch_off.png");	//적색 OFF 사용중

		id = m_ObjectList.AddObject(pObject);
	}

	m_isGasOff = (g_setup_data.m_SetupData.gas_stat) ? FALSE : TRUE;	//반대
}

void CContextGas::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

u8 CContextGas::check_update_cond(UINT _cntxt)
{
	return 1;
}

void CContextGas::update_new_info(void)
{

}

void CContextGas::Draw(UINT nContextNum)
{
	CObjectIcon* pObjectIcon;

	if(m_gc==0) return;

	DBGMSGC(DBG_GAS, "++ [%d]\r\n", nContextNum);

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
		if (pObjectIcon)
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);	//ON=차단, OFF=사용중
		break;
	case 1:
	case 2:
		//텍스트
	//	if(m_isGasOff)
		if(g_setup_data.m_SetupData.gas_stat)
			DrawText("가스를 차단하였습니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("가스를 사용중입니다", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//스위치 이미지
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(m_isGasOff ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
		}		
		break;
	case 3:
		DrawText("월패드 응답대기중..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		//스위치 이미지
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if (pObjectIcon)
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);	//ON=차단, OFF=사용중
		break;
	}

	DBGMSGC(DBG_GAS, "--\r\n");
}

void CContextGas::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "gas_on\0", "gas_off\0" };	//gas_off="가스밸브가 차단되었습니다", gas_on="가스밸브가 개방설정 되었습니다"
	char szWaveFilePath[128] = {0,};

	DBGMSGC(DBG_GAS, "++ [%d]\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
	//	g_wallpad_sns.RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
		if (g_pWallPad) {
			m_isWallPadStatus = TRUE;
			g_pWallPad->RequestGasStatus();
		}
		break;
	case 1:
	case 2:
	//	sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[m_isGasOff]);
		sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.gas_stat]);
		PlayWavFile(szWaveFilePath);
		break;
	}

	DBGMSGC(DBG_GAS, "--\r\n");
}

void CContextGas::TimerProc(UINT idTimer)
{
	switch(idTimer)
	{
	case RESPONSE_TIMER:
		if(g_pWallPad)
		{
			g_pWallPad->RequestGasCut();	//가스차단요청
		}
		break;
	}
}

void CContextGas::RecvProc(UCHAR *pPacket)
{
	PMTM_HEADER pHdr = (PMTM_HEADER)pPacket;

	if(pPacket==NULL) return;

	DBGMSGC(DBG_GAS, "++\r\n");

	switch(pHdr->type)
	{
	case MTM_DATA_TYPE_WEATHER:
		break;
	case MTM_DATA_TYPE_PARKING:
		break;
	case MTM_DATA_TYPE_ELEVATOR:
		break;
	case MTM_DATA_TYPE_GAS:
		if (g_isBackLightOn) {
			if (m_isWallPadStatus)
				ChangeContext(1);
			else
				ChangeContext(2);
		}
		m_isWallPadStatus = FALSE;
		break;
	case MTM_DATA_TYPE_LIGHT:
		break;
	case MTM_DATA_TYPE_SECURITY:
		break;
	}

	DBGMSGC(DBG_GAS, "--\r\n");
}

void CContextGas::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_GAS, "++\r\n");

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

	DBGMSGC(DBG_GAS, "--\r\n");
}

void CContextGas::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_GAS, "++\r\n");

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
			if(g_pWallPad)
			{
				g_pWallPad->RequestGasCut();
				ChangeContext(3);
			#if 0
				if(g_setup_data.m_SetupData.wallpad_type == WALLPAD_TYPE_HDT)
				{
					//	제어가 이루어 졌다고 판단하고 상태전환
					g_setup_data.m_SetupData.gas_stat = (g_setup_data.m_SetupData.gas_stat==1) ? 0 : 1;

					// 화면갱신
					ChangeContext(1);
				}
			#endif
			}
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//Light
			g_state.ChangeState(STATE_LIGHT);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.ChangeState(STATE_SECURITY);
			break;

		}
	}

	DBGMSGC(DBG_GAS, "--\r\n");
}


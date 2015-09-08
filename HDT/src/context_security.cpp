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

	// Blank ��� �̹���
	pObject = new CObjectImage(m_wid_parent, m_gc, 0, 0, g_scr_info.cols, g_scr_info.rows);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/blank_bg.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// ������ �̹���
	pObject = new CObjectIcon(m_wid_parent, m_gc, 58, 50, 158, 158);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_ICON_COUNT);
		pObject->LoadImage(IMG_ENUM_ICON_ON,	"/app/img/icon_security_on.png");
		pObject->LoadImage(IMG_ENUM_ICON_OFF,	"/app/img/icon_security_off.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// ����ġ �̹���
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

	DBGMSGC(DBG_SECURITY, "++ [%d]\r\n", nContextNum);

	//���
	m_ObjectList.Draw(SECURITY_OBJ_BG);

	//������
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(SECURITY_OBJ_ICON);
	if(pObjectIcon)
	{
//		pObjectIcon->Draw(m_isSecurityOn ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
		pObjectIcon->Draw(g_setup_data.m_SetupData.security_stat ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
	}

	//�ؽ�Ʈ
	DrawText("�������", m_wid_parent, m_gc, 144, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	DrawText("�������", m_wid_parent, m_gc, 625, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	switch(nContextNum)
	{
	case 0:
		//�ؽ�Ʈ
		if(g_setup_data.m_SetupData.wallpad_type == WALLPAD_TYPE_SHN)
		{
			DrawText("���е� ���¿�û��..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		}
		else
		{
			if(g_setup_data.m_SetupData.security_stat)
				DrawText("�ڵ������ �����Ǿ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
			else
				DrawText("�ڵ������ �����Ǿ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		}

		//����ġ �̹���
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(SECURITY_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.security_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);	//ON=����, OFF=����
		}
		break;
	case 1:
		//�ؽ�Ʈ
		if(g_setup_data.m_SetupData.security_stat)
			DrawText("�ڵ������ �����Ǿ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("�ڵ������ �����Ǿ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//����ġ �̹���
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(SECURITY_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(m_isSecurityOn ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.security_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
		}
		break;
	}

	DBGMSGC(DBG_SECURITY, "--\r\n");
}

void CContextSecurity::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "police_off\0", "police_on\0" };
	char szWaveFilePath[128] = {0,};

	DBGMSGC(DBG_SECURITY, "++ [%d]\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
	//	g_wallpad_sns.RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
		if(g_pWallPad)
		{
	//		g_pWallPad->RequestSecurityStatus();
		}
		break;
	case 1:
	//	sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[m_isSecurityOn]);
		sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.security_stat]);
		PlayWavFile(szWaveFilePath);
		break;
	}

	DBGMSGC(DBG_SECURITY, "--\r\n");
}

void CContextSecurity::TimerProc(UINT idTimer)
{
	switch(idTimer)
	{
	case RESPONSE_TIMER:
		if(g_pWallPad)
		{
			// security_stat: 0=���� --> TRUE =���� 
			//			      1=���� --> FALSE=����
			g_pWallPad->RequestSecuritySet( (g_setup_data.m_SetupData.security_stat) ? FALSE : TRUE );
		}
		break;
	}
}

void CContextSecurity::RecvProc(UCHAR *pPacket)
{
	PMTM_HEADER pHdr = (PMTM_HEADER)pPacket;
	PMTM_DATA_SECURITY pSecurityInfo;

	if(pPacket==NULL) return;

	DBGMSGC(DBG_SECURITY, "++\r\n");

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
		break;
	case MTM_DATA_TYPE_SECURITY:
		if(g_isBackLightOn)
		{
			pSecurityInfo = (PMTM_DATA_SECURITY)&pPacket[sizeof(MTM_HEADER)];
			ChangeContext(1);
		}
		break;
	}

	DBGMSGC(DBG_SECURITY, "--\r\n");
}

void CContextSecurity::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_SECURITY, "++\r\n");

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

	DBGMSGC(DBG_SECURITY, "--\r\n");
}

void CContextSecurity::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	DBGMSGC(DBG_SECURITY, "++\r\n");

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
			g_state.ChangeState(STATE_LIGHT);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			if(g_pWallPad)
			{
				// security_stat: 0=���� --> TRUE =���� 
				//			      1=���� --> FALSE=����
				g_pWallPad->RequestSecuritySet( (g_setup_data.m_SetupData.security_stat) ? FALSE : TRUE );
			}
			break;
		}
	}

	DBGMSGC(DBG_SECURITY, "--\r\n");
}


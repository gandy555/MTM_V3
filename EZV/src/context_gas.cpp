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
		pObject->LoadImage(IMG_ENUM_ICON_ON,	"/app/img/icon_gas_on.png");
		pObject->LoadImage(IMG_ENUM_ICON_OFF,	"/app/img/icon_gas_off.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// ����ġ �̹���
	pObject = new CObjectIcon(m_wid_parent, m_gc, 312, 287, 299, 116);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_SWITCH_COUNT);
		pObject->LoadImage(IMG_ENUM_SWITCH_ON,	"/app/img/icon_switch_on.png");		//��� ON  ������
		pObject->LoadImage(IMG_ENUM_SWITCH_OFF,	"/app/img/icon_switch_off.png");	//���� OFF �����

		id = m_ObjectList.AddObject(pObject);
	}

	m_isGasOff = (g_setup_data.m_SetupData.gas_stat) ? FALSE : TRUE;	//�ݴ�
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

	//���
	m_ObjectList.Draw(GAS_OBJ_BG);

	//������
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_ICON);
	if(pObjectIcon)
	{
	//	pObjectIcon->Draw(m_isGasOff ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
		pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);	//ICON_ON=�����, ICON_OFF=����
	}

	//�ؽ�Ʈ
	DrawText("�������", m_wid_parent, m_gc, 144, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	DrawText("��������", m_wid_parent, m_gc, 625, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	switch(nContextNum)
	{
	case 0:
		//�ؽ�Ʈ
		DrawText("���е� ���¿�û��..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//����ġ �̹���
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if (pObjectIcon)
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);	//ON=����, OFF=�����
		break;
	case 1:
	case 2:
		//�ؽ�Ʈ
	//	if(m_isGasOff)
		if(g_setup_data.m_SetupData.gas_stat)
			DrawText("������ �����Ͽ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("������ ������Դϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		//����ġ �̹���
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if(pObjectIcon)
		{
		//	pObjectIcon->Draw(m_isGasOff ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
		}		
		break;
	case 3:
		DrawText("���е� ��������..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		//����ġ �̹���
		pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(GAS_OBJ_SWITCH);
		if (pObjectIcon)
			pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);	//ON=����, OFF=�����
		break;
	}

	DBGMSGC(DBG_GAS, "--\r\n");
}

void CContextGas::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "gas_on\0", "gas_off\0" };	//gas_off="������갡 ���ܵǾ����ϴ�", gas_on="������갡 ���漳�� �Ǿ����ϴ�"
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
			g_pWallPad->RequestGasCut();	//�������ܿ�û
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
					//	��� �̷�� ���ٰ� �Ǵ��ϰ� ������ȯ
					g_setup_data.m_SetupData.gas_stat = (g_setup_data.m_SetupData.gas_stat==1) ? 0 : 1;

					// ȭ�鰻��
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


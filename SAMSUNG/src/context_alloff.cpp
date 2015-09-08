/*
*/
#include "common.h"
#include "context_alloff.h"
#include "main.h"

//
// Construction/Destruction
//
CContextAllOff::CContextAllOff(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
//	m_isWallPadStatus = FALSE;
//	m_isAllOff = FALSE;
}

CContextAllOff::~CContextAllOff()
{
}

//
// Member Function
//
void CContextAllOff::Init()
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
		pObject->LoadImage(IMG_ENUM_ICON_ON,	"/app/img/icon_alloff_on.png");
		pObject->LoadImage(IMG_ENUM_ICON_OFF,	"/app/img/icon_alloff_off.png");

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

//	m_isAllOff = (g_setup_data.m_SetupData.alloff_stat) ? TRUE : FALSE;
}

void CContextAllOff::DeInit()
{
	CContext::DeInit();
	
	m_ObjectList.RemoveAll();
}

//������ �����ִٴ°��� �ϰ��ҵ� SW�� OFF��ٴ� ��
void CContextAllOff::Draw(UINT nContextNum)
{
	CObjectIcon* pObjectIcon = NULL;

	if(m_gc==0) return;

	printf("+CContextAllOff::Draw(%d)\r\n", nContextNum);

	//���
	m_ObjectList.Draw(ALLOFF_OBJ_BG);

	//������
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(ALLOFF_OBJ_ICON);
	if(pObjectIcon)
	{
	//	pObjectIcon->Draw(m_isAllOff ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
		pObjectIcon->Draw(g_setup_data.m_SetupData.alloff_stat ? IMG_ENUM_ICON_ON : IMG_ENUM_ICON_OFF);
	}

	//�ؽ�Ʈ
	DrawText("�ϰ��ҵ�����", m_wid_parent, m_gc, 144, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	DrawText("�ϰ��ҵ�", m_wid_parent, m_gc, 625, 329, 150, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	switch(nContextNum)
	{
	case 0:
		//�ؽ�Ʈ
		DrawText("���е� ���¿�û��..", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		break;
	case 1:
		//�ؽ�Ʈ
		if(g_setup_data.m_SetupData.alloff_stat)
			DrawText("�ϰ��ҵ�Ǿ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		else
			DrawText("�ϰ��ҵ� �����Ǿ����ϴ�", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		break;
	case 2:
		DrawText("��ſ���: �ٽ� ���� �ּ���", m_wid_parent, m_gc, 240, 108, 500, 40, g_font, 32, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
		break;
	}

	//����ġ
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(ALLOFF_OBJ_SWITCH);
	if(pObjectIcon)
	{
		pObjectIcon->Draw(g_setup_data.m_SetupData.alloff_stat ? IMG_ENUM_SWITCH_ON : IMG_ENUM_SWITCH_OFF);
	}

	printf("-CContextAllOff::Draw\r\n");
}

void CContextAllOff::Proc(UINT nContextNum)
{
	char szWaveFile[2][14] = { "all_on\0", "all_off\0" };
	char szWaveFilePath[128] = {0,};

	printf("+CContextAllOff::Proc(%d)\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:
		g_wallpad_sns.RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
		break;
	case 1:
	//	sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[m_isAllOff]);
		sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.alloff_stat]);
		PlayWavFile(szWaveFilePath);
	//	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (m_isAllOff) ? HIGH : LOW);
		g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (g_setup_data.m_SetupData.alloff_stat) ? LOW : HIGH);
		break;
	}

	printf("-CContextAllOff::Proc\r\n");
}

void CContextAllOff::TimerProc(UINT idTimer)
{
}

void CContextAllOff::RecvProc(UCHAR *pPacket)
{
	if(pPacket==NULL) return;

	printf("+CContextAllOff::RecvProc\r\n");

	switch(pPacket[1])	//cmd
	{
	case SNS_CMD_WALLPAD_STAT_REQ:
	case SNS_CMD_LAMP_ONOFF_REQ:
		if (g_isBackLightOn)
			ChangeContext(1);
		break;
	}

	printf("-CContextAllOff::RecvProc\r\n");
}

void CContextAllOff::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextAllOff::ButtonDown\r\n");

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

	printf("-CContextAllOff::ButtonDown\r\n");
}

void CContextAllOff::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextAllOff::ButtonUp\r\n");

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
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//AllOff - ��������
		//	g_state.SetNextState(STATE_ALLOFF);

			//�ϰ��ҵ��û(0x09)�� �������� g_wallpad_sns���� ��û�� ������ ��� �ٽ� ���е� ���¸� ��û�ؼ� ����
		//	g_wallpad_sns.RequestReserve(SNS_CMD_LAMP_ONOFF_REQ, (m_isAllOff) ? 0 : 1);		//0=Off, 1=On(�ϰ��ҵ�)
			g_wallpad_sns.RequestReserve(SNS_CMD_LAMP_ONOFF_REQ, (g_setup_data.m_SetupData.alloff_stat) ? 0 : 1);		//0=Off, 1=On(�ϰ��ҵ�)
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.SetNextState(STATE_SECURITY);
			break;
		}
	}

	printf("-CContextAllOff::ButtonUp\r\n");
}


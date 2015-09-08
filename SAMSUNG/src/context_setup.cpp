/*
*/
#include "common.h"
#include "context_setup.h"
#include "main.h"

//
// Construction/Destruction
//
CContextSetup::CContextSetup(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
	m_fMicroWaveMonitorRunning = FALSE;

	m_isAdcValueLongKeyPlus = FALSE;
	m_isAdcValueLongKeyMinus = FALSE;

	m_isWaitForResponse = FALSE;
	m_isRequestReserve = FALSE;

	m_nWallPadKeyPage = 0;

	m_nSetupStep = CONTEXT_SETUP_STEP_SELECT;
}

CContextSetup::~CContextSetup()
{
}

//
// Member Function
//
void CContextSetup::Init()
{
	CContext::Init();

	SetRect(&m_rcTitle, SCR_WIDTH/2-TITLE_WIDTH/2, 20, TITLE_WIDTH, TITLE_HEIGHT);
	SetRect(&m_rcArea, BUTTON_WIDTH+20, TITLE_HEIGHT+40, SCR_WIDTH - (BUTTON_WIDTH+20)*2, BUTTON_VCENTER_3-(TITLE_HEIGHT+40)+BUTTON_HEIGHT/2);

	SetRect(&m_rcButton[0], 0, BUTTON_VCENTER_1-BUTTON_HEIGHT/2, BUTTON_WIDTH, BUTTON_HEIGHT);
	SetRect(&m_rcButton[1], 0, BUTTON_VCENTER_2-BUTTON_HEIGHT/2, BUTTON_WIDTH, BUTTON_HEIGHT);
	SetRect(&m_rcButton[2], 0, BUTTON_VCENTER_3-BUTTON_HEIGHT/2, BUTTON_WIDTH, BUTTON_HEIGHT);
	SetRect(&m_rcButton[3], SCR_WIDTH-1-BUTTON_WIDTH, BUTTON_VCENTER_1-BUTTON_HEIGHT/2, BUTTON_WIDTH, BUTTON_HEIGHT);
	SetRect(&m_rcButton[4], SCR_WIDTH-1-BUTTON_WIDTH, BUTTON_VCENTER_2-BUTTON_HEIGHT/2, BUTTON_WIDTH, BUTTON_HEIGHT);
	SetRect(&m_rcButton[5], SCR_WIDTH-1-BUTTON_WIDTH, BUTTON_VCENTER_3-BUTTON_HEIGHT/2, BUTTON_WIDTH, BUTTON_HEIGHT);

	SetRect(&m_rcGraph, m_rcArea.x+20, m_rcArea.y+20, m_rcArea.w-40, m_rcArea.h-40);

	m_pixmapArea  = GrNewPixmap(m_rcArea.w-2, m_rcArea.h-2, NULL);	//for WallPad Serial Dump
	m_pixmapGraph = GrNewPixmap(m_rcGraph.w, m_rcGraph.h, NULL);	//for Microwave Graph
}

void CContextSetup::DeInit()
{
	GrDestroyWindow(m_pixmapGraph);
	GrDestroyWindow(m_pixmapArea);

	CContext::DeInit();
}

void CContextSetup::Draw(UINT nContextNum)
{
	RECT rcText;

	if(m_gc_parent==0) return;

	printf("+CContextSetup::Draw(%d)\r\n", nContextNum);

//	EnterCS();

	//��üȭ���� ���������� ĥ�Ѵ�.
	DrawRect(0, 0, g_scr_info.cols, g_scr_info.rows, g_wid, g_gc, BLACK, TRUE);

	//Ÿ��Ʋ �ڽ�
	DrawRect2(&m_rcTitle, g_wid, g_gc, WHITE, FALSE);

	//ǥ�ÿ���
	DrawRect2(&m_rcArea, g_wid, g_gc, BLACK, TRUE);		//���������� �����
	DrawRect2(&m_rcArea, g_wid, g_gc, WHITE, FALSE);	//������� �ڽ��׸���

	//���� ��ư �ڽ�
	DrawRect2(&m_rcButton[BUTTON_RT], g_wid, g_gc, WHITE, FALSE);
	DrawRect2(&m_rcButton[BUTTON_RM], g_wid, g_gc, WHITE, FALSE);
	DrawRect2(&m_rcButton[BUTTON_RB], g_wid, g_gc, WHITE, FALSE);

	if(m_nSetupStep==CONTEXT_SETUP_STEP_SELECT)
	{
		//������ư
		DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
			DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
			DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);

		if(nContextNum != CONTEXT_SETUP_DOOR)
		{
			DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RB], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RB].x, m_rcButton[BUTTON_RB].y+m_rcButton[BUTTON_RB].h+2, m_rcButton[BUTTON_RB].w, 16);
				DrawTextRect("VolDn Long\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
	}
	else
	{
		DrawTextRect("�Ϸ�", g_wid, g_gc, &m_rcButton[BUTTON_RB], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			SetRect(&rcText, m_rcButton[BUTTON_RB].x, m_rcButton[BUTTON_RB].y+m_rcButton[BUTTON_RB].h+2, m_rcButton[BUTTON_RB].w, 16);
			DrawTextRect("VolUp Long\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	}

	switch(nContextNum)
	{
	case CONTEXT_SETUP_INIT:
		break;
	case CONTEXT_SETUP_LED:	//LED
		//Title
		DrawTextRect("LED", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//������ư
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("ON", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("OFF", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
		break;
	case CONTEXT_SETUP_BACKLIGHT:
		//Title
		DrawTextRect("BackLight", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//������ư
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("ON", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("OFF", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
		break;
	case CONTEXT_SETUP_RELAY:
		//Title
		DrawTextRect("Relay", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//������ư
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("ON", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("OFF", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
		break;
	case CONTEXT_SETUP_DOOR:
		//Title
		DrawTextRect("Door", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//������ư : ǥ�þ���
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			DrawTextRect("", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		}

		if(g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT)==HIGH)
			DrawTextRect2("���� �������ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		else
			DrawTextRect2("���� ���Ƚ��ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		break;
	case CONTEXT_SETUP_SOUND:
		//Title
		DrawTextRect("Sound", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//������ư
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
		break;
	case CONTEXT_SETUP_MICROWAVE:
		//Title
		DrawTextRect("MicroWave", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//������ư
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp (L=������)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("����", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn (L=������)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}

		//�׷���
		DrawLine(m_rcGraph.x-1, m_rcGraph.y, m_rcGraph.x-1, m_rcGraph.y+m_rcGraph.h+1, g_wid, g_gc, DKGRAY, FALSE);
		DrawLine(m_rcGraph.x-1, m_rcGraph.y+m_rcGraph.h+1, m_rcGraph.x+m_rcGraph.w+1, m_rcGraph.y+m_rcGraph.h+1, g_wid, g_gc, DKGRAY, FALSE);

		SetRect(&rcText, m_rcArea.x, m_rcGraph.y, m_rcGraph.x-m_rcArea.x-2, 12);
		DrawTextRect("1023", g_wid, g_gc, &rcText, g_font, 8, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
		SetRect(&rcText, m_rcArea.x, m_rcGraph.y+m_rcGraph.h-6, m_rcGraph.x-m_rcArea.x-2, 12);
		DrawTextRect("0", g_wid, g_gc, &rcText, g_font, 8, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
		break;
	case CONTEXT_SETUP_WALLPAD:
		//Title
		DrawTextRect("WallPad", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			//������ư
			switch(m_nWallPadKeyPage)
			{
			case CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF:		//�ϰ��ҵ�
				DrawTextRect("�ϰ��ҵ�", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_GAS_CUT:		//��������
				DrawTextRect("��������", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_ON:	//���ȼ���
				DrawTextRect("���ȼ���", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_OFF:	//��������
				DrawTextRect("��������", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_WEATHER_INFO:	//������û
				DrawTextRect("������û", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_PARKING_INFO:	//������û
				DrawTextRect("������û", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_ELV_INFO:		//E/V��û
				DrawTextRect("E/V��û", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_ELV_CALL:		//E/Vȣ��
				DrawTextRect("E/Vȣ��", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			default:
				break;
			}
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);

			DrawTextRect("�����ȯ", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn (L=�������)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}

		GrCopyArea(g_wid, g_gc, m_rcArea.x+1, m_rcArea.y+1, m_rcArea.w-2, m_rcArea.h-2, m_pixmapArea, 0, 0, MWROP_SRCCOPY);
		break;
	}

	//�ϴܾȳ�
	SetRect(&rcText, m_rcArea.x, m_rcArea.y+m_rcArea.h+2, m_rcArea.w, 32);
	DrawTextRect("S:ª�Դ���,  L:��Դ���\0", g_wid, g_gc, &rcText, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	if(nContextNum==CONTEXT_SETUP_WALLPAD)
	{
		SetRect(&rcText, m_rcArea.x+200, m_rcArea.y+m_rcArea.h+2, m_rcArea.w-200, 32);
		DrawTextRect("(FreeData��Ŷ����)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
	}

//	LeaveCS();

	printf("-CContextSetup::Draw\r\n");
}

void CContextSetup::Proc(UINT nContextNum)
{
	char szText[128] = {0,};
	RECT rcText;

	printf("+CContextSetup::Proc(%d)\r\n", nContextNum);

//	EnterCS();

	switch(nContextNum)
	{
	case CONTEXT_SETUP_INIT:
		g_timer.KillTimer(BACK_LIGHT_TIMER);
	//	g_microwave.StopMeasure();				//MicrowaveHandler������ ������ ��ܽ�Ŵ

		ChangeContext(1);
		break;
	case CONTEXT_SETUP_LED:			//LED
		break;
	case CONTEXT_SETUP_BACKLIGHT:	//BackLight
		break;
	case CONTEXT_SETUP_RELAY:		//Relay
		break;
	case CONTEXT_SETUP_DOOR:		//Door
		break;
	case CONTEXT_SETUP_SOUND:		//Sound
		sprintf(szText, "Volume Level: %d\0", g_setup_data.m_SetupData.volume);
		SetRect(&rcText, m_rcArea.x, m_rcArea.y+m_rcArea.h/2-16, m_rcArea.w, 32);
		DrawTextRect(szText, g_wid, g_gc, &rcText, g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		SetRect(&rcText, m_rcArea.x, m_rcArea.y+m_rcArea.h-32, m_rcArea.w, 32);
		DrawTextRect("*������ ����: 0~8\0", g_wid, g_gc, &rcText, g_font, 18, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		break;
	case CONTEXT_SETUP_MICROWAVE:	//MicroWave
		break;
	case CONTEXT_SETUP_WALLPAD:		//WallPad
		g_wallpad_sns.m_isContextSetupWallPad = TRUE;
		break;
	}

//	LeaveCS();

	printf("-CContextSetup::Proc\r\n");
}

void CContextSetup::TimerProc(UINT idTimer)
{
}

void CContextSetup::RecvProc(UCHAR *pPacket)
{
//	UCHAR *pBuffer=NULL, cmd;
	UCHAR cmd;

	if(pPacket==NULL) return;

	if(m_nContextNum != CONTEXT_SETUP_WALLPAD) return;	//WallPad�׽�Ʈ�� �ƴѰ��� ó������ ����

//	printf("+CContextSetup::RecvProc\r\n");

//	pBuffer = (UCHAR *)pParam;

//	PrintPacket(pBuffer);

	cmd = pPacket[1];

	// Command�� ó��
	switch(cmd)
	{
//-------------------------------------------------------------------------------------
	case SNS_CMD_WEATHER_RES:		//0x01, ���е忡�� ��Ȱ������� �������� ����
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
	case SNS_CMD_WEATHER_REQ:		//0x02, ��Ȱ�����⿡�� ���е�� �������� ��û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_PARKING_RES:		//0x03, ���е忡�� ��Ȱ������� �������� ����
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
	case SNS_CMD_PARKING_REQ:		//0x04, ��Ȱ�����⿡�� ���е�� �������� ��û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_ELEVATOR_STAT_REQ:	//0x05, ��Ȱ�����⿡�� ���е�� ���������� ���¿�û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
	case SNS_CMD_ELEVATOR_STAT_RES:	//0x06, ���е忡�� ��Ȱ������� ���������� ��������
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
	case SNS_CMD_ELEVATOR_CALL:		//0x07, ���������⿡�� ���е�� ���������� ȣ��
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_WALLPAD_STAT_REQ:	//0x08, ���������⿡�� ���е�� �ϰ��ҵ�,����,���⼳�� ���¿�û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_LAMP_ONOFF_REQ:	//0x09, ���������⿡�� ���е�� �ϰ��ҵ� On/Off��û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_GAS_OFF_REQ:		//0x0A, ���������⿡�� ���е�� ������� Off��û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_OUT_ONOFF_REQ:		//0x0B, ���������⿡�� ���е�� ���⼳�� On/Off��û
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_OUT_STAT_RES:		//0x0C, ���е忡�� ��Ȱ������� ���⼳�� ��������
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_GAS_STAT_RES:		//0x0D, ���е忡�� ��Ȱ������� ������� ��������
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_ELEVATOR_ARRIVE:	//0x0E, ���е忡�� ��Ȱ������� ���������� ������ ������ �޼��� ����
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_INIT_DATA:			//0x5A, ���е忡�� ��Ȱ������� : ���е尡 ������, ���е尡 Ack Data 10ȸ�̻� �̼����� ���
		g_wallpad_sns.SendAck(cmd);
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_FREE_DATA:			//0x41, ���е忡�� ��Ȱ������� : Event Data ���� ���, Live Check

		//����� ��û�� ������
		if(m_isRequestReserve)
		{
			g_wallpad_sns.SendRequest(m_cmdRequestReserve, m_dataRequestReserve);
			PrintPacket(pPacket);
			PrintSendPacket(m_cmdRequestReserve, m_dataRequestReserve);
			m_isRequestReserve = FALSE;			//����ó����
			m_isWaitForResponse = TRUE;			//������
			m_nWaitCountForResponse = 0;
		}
		//��û�� ���� ������ ������� ���¶�� --> ������ FreeData�� �´ٴ°��� ���û�� �ʿ��� ��Ȳ
		else if(m_isWaitForResponse)
		{
			//��û������ ���� ��� 3ȸ ������
			m_nWaitCountForResponse++;
			if(m_nWaitCountForResponse <= MAX_WAIT_COUNT_FOR_RESPONSE)
			{
			//	printf("%s: Re-RequestReserve(retry=%d) cmd=0x%x, data=0x%x\r\n", __func__, 
			//			m_nWaitCountForResponse, m_cmdRequestReserve, m_dataRequestReserve);
				g_wallpad_sns.SendRequest(m_cmdRequestReserve, m_dataRequestReserve);
				PrintPacket(pPacket);
				PrintSendPacket(m_cmdRequestReserve, m_dataRequestReserve);
			}
			//������ Ÿ�Ӿƿ��̸� �����⸦ �ߴ�
			else
			{
			//	printf("%s: RequestReserve Canceled: retry timeout!\r\n", __func__);
				m_isWaitForResponse = FALSE;
				g_wallpad_sns.SendFreeData();
				PrintPacket(pPacket);
				PrintSendPacket(SNS_CMD_FREE_DATA);
			}
		}
		//������� ��û�� ���� ����
		else
		{
			g_wallpad_sns.SendFreeData();
			PrintPacket(pPacket);
			PrintSendPacket(SNS_CMD_FREE_DATA);
		}
		break; 
//-------------------------------------------------------------------------------------
	default:
		PrintPacket(pPacket);
		break;
	}

//	printf("-CContextSetup::RecvProc\r\n");
}

void CContextSetup::ThreadProc()
{
	static int y1=0, y2=0, ySetupValue;
	UINT nAdcValue;

	char szText[64] = {0,};
	RECT rcText;

	if(m_nContextNum == CONTEXT_SETUP_MICROWAVE)	//MicroWave
	{
		if(m_isAdcValueLongKeyPlus)
		{
			if(g_setup_data.m_SetupData.adc_value < MAX_ADC_VALUE)
			{
				g_setup_data.m_SetupData.adc_value++;
			}
		}
		if(m_isAdcValueLongKeyMinus)
		{
			if(g_setup_data.m_SetupData.adc_value > 0)
			{
				g_setup_data.m_SetupData.adc_value--;
			}
		}

		nAdcValue = g_microwave.GetAdcSamplingValue();
		y2 = (int)(m_rcGraph.h*nAdcValue/1023.0);
		if(y1==0) y1 = y2;

		//�׷��� Shift
		GrCopyArea(m_pixmapGraph, g_gc, 0, 0, m_rcGraph.w-GRAPH_SPEED, m_rcGraph.h, m_pixmapGraph, GRAPH_SPEED, 0, MWROP_SRCCOPY);

		//Shift�ǰ� �κ� ����� (��������)
		DrawRect(m_rcGraph.w-GRAPH_SPEED, 0, GRAPH_SPEED, m_rcGraph.h, m_pixmapGraph, g_gc, BLACK, TRUE);
		DrawRect(m_rcGraph.x+m_rcGraph.w-GRAPH_SPEED, m_rcGraph.y, GRAPH_SPEED, m_rcGraph.h, g_wid, g_gc, BLACK, TRUE);

		//�׷��� �׸���
		DrawLine(m_rcGraph.w-GRAPH_SPEED, m_rcGraph.h-y1, m_rcGraph.w, m_rcGraph.h-y2, m_pixmapGraph, g_gc, CYAN, FALSE);

		//�׷��� �ű��
		GrCopyArea(g_wid, g_gc, m_rcGraph.x, m_rcGraph.y, m_rcGraph.w, m_rcGraph.h, m_pixmapGraph, 0, 0, MWROP_SRCCOPY);

		//������ ǥ��(�������)
		ySetupValue = (int)(m_rcGraph.h*g_setup_data.m_SetupData.adc_value/1023.0);
		DrawLine(m_rcGraph.x, m_rcGraph.y+m_rcGraph.h-ySetupValue, 
				 m_rcGraph.x+m_rcGraph.w, m_rcGraph.y+m_rcGraph.h-ySetupValue, 
				 g_wid, g_gc, YELLOW, FALSE);

		//�ؽ�Ʈ ǥ��
		sprintf(szText, "������: %04d,  ������: %04d\0", nAdcValue, g_setup_data.m_SetupData.adc_value);
		SetRect(&rcText, m_rcGraph.x+1, m_rcArea.y+5, m_rcGraph.w-2, 32);
		DrawTextRect2(szText, g_wid, g_gc, &rcText, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		y1 = y2;
	}
	else if( (y1>0) || (y2>0) )
	{
		DrawRect(0, 0, m_rcGraph.w, m_rcGraph.h, m_pixmapGraph, g_gc, BLACK, TRUE);
		y1 = y2 = 0;
	}
}

void CContextSetup::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
//	printf("+CContextSetup::ButtonDown\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_LONG)
	{
		if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)|FLAG(GPIO_FRONT_RIGHT_BOTTOM)) ||
			CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)|FLAG(GPIO_REAR_VOL_DOWN)) )
		{
			g_setup_data.SaveSetupData();
			g_state.SetNextState(STATE_WEATHER);
			g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
			g_wallpad_sns.m_isContextSetupWallPad = FALSE;
		}
		else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)) )
		{
			if(m_nSetupStep == CONTEXT_SETUP_STEP_PAGE)
			{
				if(m_nContextNum==CONTEXT_SETUP_MICROWAVE)
				{
					m_isAdcValueLongKeyPlus = TRUE;
				}
			}
		}
		else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_MIDDLE)) )
		{
			if(m_nSetupStep == CONTEXT_SETUP_STEP_PAGE)
			{
				if(m_nContextNum==CONTEXT_SETUP_MICROWAVE)
				{
					m_isAdcValueLongKeyMinus = TRUE;
				}
				else if(m_nContextNum==CONTEXT_SETUP_WALLPAD)
				{
					switch(m_nWallPadKeyPage)
					{
					case CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF:
						RequestDirect(SNS_CMD_LAMP_ONOFF_REQ, 1);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_GAS_CUT:
						RequestDirect(SNS_CMD_GAS_OFF_REQ, 1);		//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_ON:
						RequestDirect(SNS_CMD_OUT_ONOFF_REQ, 1);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_OFF:
						RequestDirect(SNS_CMD_OUT_ONOFF_REQ, 0);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_WEATHER_INFO:
						RequestDirect(SNS_CMD_WEATHER_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_PARKING_INFO:
						RequestDirect(SNS_CMD_PARKING_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_ELV_INFO:
						RequestDirect(SNS_CMD_ELEVATOR_STAT_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_ELV_CALL:
						RequestDirect(SNS_CMD_ELEVATOR_CALL);
						break;
					}
				}
			}
		}
//		else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_BOTTOM)) )
//		{
//		}
		else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)) )
		{
			if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
			{
				if(m_nContextNum==CONTEXT_SETUP_MICROWAVE)
				{
					m_isAdcValueLongKeyPlus = TRUE;
				}
				else if(m_nContextNum != CONTEXT_SETUP_DOOR)
				{
					m_nSetupStep = CONTEXT_SETUP_STEP_SELECT;
					ChangeContext(m_nContextNum);
				}
			}
		}
		else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_DOWN)) )
		{
			if(m_nSetupStep==CONTEXT_SETUP_STEP_SELECT)
			{
				if(m_nContextNum != CONTEXT_SETUP_DOOR)
				{
					m_nSetupStep = CONTEXT_SETUP_STEP_PAGE;
					ChangeContext(m_nContextNum);
				}
			}
			else 
			{
				if(m_nContextNum==CONTEXT_SETUP_MICROWAVE)
				{
					m_isAdcValueLongKeyMinus = TRUE;
				}
				else if(m_nContextNum==CONTEXT_SETUP_WALLPAD)
				{
					switch(m_nWallPadKeyPage)
					{
					case CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF:
						RequestDirect(SNS_CMD_LAMP_ONOFF_REQ, 1);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_GAS_CUT:
						RequestDirect(SNS_CMD_GAS_OFF_REQ, 1);		//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_ON:
						RequestDirect(SNS_CMD_OUT_ONOFF_REQ, 1);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_OFF:
						RequestDirect(SNS_CMD_OUT_ONOFF_REQ, 0);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_WEATHER_INFO:
						RequestDirect(SNS_CMD_WEATHER_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_PARKING_INFO:
						RequestDirect(SNS_CMD_PARKING_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_ELV_INFO:
						RequestDirect(SNS_CMD_ELEVATOR_STAT_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_ELV_CALL:
						RequestDirect(SNS_CMD_ELEVATOR_CALL);
						break;
					}
				}
			}
		}
	}

//	printf("-CContextSetup::ButtonDown\r\n");
}

void CContextSetup::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	char szText[128] = {0,};
	RECT rcText;

//	printf("+CContextSetup::ButtonUp\r\n");

	if(usEventEnum == MTM_GPIO_STATE_CHANGE)
	{
		if(m_nContextNum == CONTEXT_SETUP_DOOR)
		{
			ChangeContext(m_nContextNum);
		}
	}
	else if(usEventEnum == MTM_GPIO_BUTTON_UP)
	{
		//�����׸� ���ø��
		if(m_nSetupStep == CONTEXT_SETUP_STEP_SELECT)
		{
			if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)) ||			//����
				CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)) )
			{
				if(m_nContextNum == CONTEXT_SETUP_LED)
				{
					//WallPad�� �ȼ��� Ŭ����
					DrawRect(m_rcArea.x+1, m_rcArea.y+1, m_rcArea.w-2, m_rcArea.h-2, m_pixmapArea, g_gc, BLACK, TRUE);
					g_wallpad_sns.m_isContextSetupWallPad = TRUE;
					ChangeContext(CONTEXT_SETUP_WALLPAD);
				}
				else
				{
					g_wallpad_sns.m_isContextSetupWallPad = FALSE;
					ChangeContext(m_nContextNum-1);
				}
			}
			else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_MIDDLE)) ||		//����
					 CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_DOWN)) )
			{
				if(m_nContextNum == CONTEXT_SETUP_WALLPAD)
				{
					g_wallpad_sns.m_isContextSetupWallPad = FALSE;
					ChangeContext(CONTEXT_SETUP_LED);
				}
				else if(m_nContextNum == CONTEXT_SETUP_MICROWAVE)
				{
					//WallPad�� �ȼ��� Ŭ����
					DrawRect(m_rcArea.x+1, m_rcArea.y+1, m_rcArea.w-2, m_rcArea.h-2, m_pixmapArea, g_gc, BLACK, TRUE);
					g_wallpad_sns.m_isContextSetupWallPad = TRUE;
					ChangeContext(m_nContextNum+1);
				}
				else
				{
					g_wallpad_sns.m_isContextSetupWallPad = FALSE;
					ChangeContext(m_nContextNum+1);
				}
			}
			else if(CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_BOTTOM)))	//����
			{
				if(m_nContextNum != CONTEXT_SETUP_DOOR)
				{
					m_nSetupStep = CONTEXT_SETUP_STEP_PAGE;
					ChangeContext(m_nContextNum);
				}
			}
		}
		else	//�����׸� ������
		{
			if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)) ||
				CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)) )
			{
				switch(m_nContextNum)
				{
				case CONTEXT_SETUP_LED:			// ON
					g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);
					DrawTextRect2("LED�� �׽��ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_BACKLIGHT:	// ON
					g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
					DrawTextRect2("�����Ʈ�� �׽��ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_RELAY:		// ON
					g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, HIGH);
					DrawTextRect2("�����̸� �׽��ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_DOOR:
					break;
				case CONTEXT_SETUP_SOUND:		// ����
					if(g_setup_data.m_SetupData.volume < MAX_VOLUME_LEVEL)
					{
						g_setup_data.m_SetupData.volume++;
						SetVolume(g_setup_data.m_SetupData.volume);

						sprintf(szText, "Volume Level: %d\0", g_setup_data.m_SetupData.volume);
						SetRect(&rcText, m_rcArea.x, m_rcArea.y+m_rcArea.h/2-16, m_rcArea.w, 32);
						DrawTextRect2(szText, g_wid, g_gc, &rcText, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

						PlayWavFile("/app/sound/touch.wav");
					}
					break;
				case CONTEXT_SETUP_MICROWAVE:	// ����
					if(g_setup_data.m_SetupData.adc_value < MAX_ADC_VALUE)
					{
						g_setup_data.m_SetupData.adc_value++;
					}
					break;
				case CONTEXT_SETUP_WALLPAD:		// ��ɿ�û
					switch(m_nWallPadKeyPage)
					{
					case CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF:
						RequestReserve(SNS_CMD_LAMP_ONOFF_REQ, 1);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_GAS_CUT:
						RequestReserve(SNS_CMD_GAS_OFF_REQ, 1);		//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_ON:
						RequestReserve(SNS_CMD_OUT_ONOFF_REQ, 1);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_OFF:
						RequestReserve(SNS_CMD_OUT_ONOFF_REQ, 0);	//0=OFF, 1=ON
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_WEATHER_INFO:
						RequestReserve(SNS_CMD_WEATHER_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_PARKING_INFO:
						RequestReserve(SNS_CMD_PARKING_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_ELV_INFO:
						RequestReserve(SNS_CMD_ELEVATOR_STAT_REQ);
						break;
					case CONTEXT_SETUP_WALLPAD_PAGE_ELV_CALL:
						RequestReserve(SNS_CMD_ELEVATOR_CALL);
						break;
					}
					break;
				}
			}
			else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_MIDDLE)) ||
				     CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_DOWN)) )
			{
				switch(m_nContextNum)
				{
				case CONTEXT_SETUP_LED:			// OFF
					g_gpio_mtm.SetGpioOutPin(GPIO_LED, LOW);
					DrawTextRect2("LED�� �����ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_BACKLIGHT:	// OFF
					g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, LOW);
					DrawTextRect2("�����Ʈ�� �����ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_RELAY:		// OFF
					g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, LOW);
					DrawTextRect2("�����̸� �����ϴ�", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_DOOR:
					break;
				case CONTEXT_SETUP_SOUND:		// ����
					if(g_setup_data.m_SetupData.volume > 0)
					{
						g_setup_data.m_SetupData.volume--;
						SetVolume(g_setup_data.m_SetupData.volume);

						sprintf(szText, "Volume Level: %d\0", g_setup_data.m_SetupData.volume);
						SetRect(&rcText, m_rcArea.x, m_rcArea.y+m_rcArea.h/2-16, m_rcArea.w, 32);
						DrawTextRect2(szText, g_wid, g_gc, &rcText, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

						PlayWavFile("/app/sound/touch.wav");
					}
					break;
				case CONTEXT_SETUP_MICROWAVE:	// ����
					if(g_setup_data.m_SetupData.adc_value > 0)
					{
						g_setup_data.m_SetupData.adc_value--;
					}
					break;
				case CONTEXT_SETUP_WALLPAD:		// �����ȯ
					m_nWallPadKeyPage++;
					if(m_nWallPadKeyPage == CONTEXT_SETUP_WALLPAD_PAGE_COUNT)
					{
						m_nWallPadKeyPage = CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF;
					}
					ChangeContext(m_nContextNum);
					break;
				}
			}
			else if(CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_BOTTOM)))
			{
				if(m_nContextNum != CONTEXT_SETUP_DOOR)
				{
					m_nSetupStep = CONTEXT_SETUP_STEP_SELECT;
					ChangeContext(m_nContextNum);
				}
			}

			if(m_nContextNum==CONTEXT_SETUP_MICROWAVE)	//MicroWave
			{
				if(m_isAdcValueLongKeyPlus) 
				{
					m_isAdcValueLongKeyPlus=FALSE;
				}
				if(m_isAdcValueLongKeyMinus) 
				{
					m_isAdcValueLongKeyMinus=FALSE;
				}
			}
		}
	}

//	printf("-CContextSetup::ButtonUp\r\n");
}

void CContextSetup::RequestReserve(UCHAR cmd, UCHAR data)
{
	m_isRequestReserve = TRUE;
	m_cmdRequestReserve = cmd;
	m_dataRequestReserve = data;
}

void CContextSetup::RequestDirect(UCHAR cmd, UCHAR data)
{
	g_wallpad_sns.SendRequest(cmd, data);
	PrintSendPacket(cmd, data);
//	m_isRequestReserve = FALSE;			//����ó����
//	m_isWaitForResponse = TRUE;			//������
//	m_nWaitCountForResponse = 0;
}

// SNS��Ŷ�� ���� ����� ������������ 22bytes
// ��Ŷ������ "T(R)x: xx xx ..." �����̹Ƿ� �ִ���Ŷ ���� ��Ʈ�� ������� 70bytes
// 70����Ʈ�� 18��Ʈ �������� ȭ�鿡�� 630pixel�� �ʿ���
// ǥ�ÿ����� ȭ������ 518 pixel�̰� 18��Ʈ ũ��������� 57����Ʈ ��Ʈ������ ǥ�ð���

#define MAX_PRINTABLE_STRING	55

void CContextSetup::PrintPacket(UCHAR* pPacket, BOOL isRx)
{
	char szText[128] = {0,}, temp;
	int i, size, idx=0, wScroll, hScroll, sizeFont=18, heighLine=22;
	UCHAR src, cmd, *p = pPacket;
	UINT color;

	if(pPacket==NULL) return;

	src = pPacket[0];
	cmd = pPacket[1];

	if(cmd == SNS_CMD_FREE_DATA) return;

	if(isRx)
		size = (cmd==SNS_CMD_FREE_DATA) ? 4 : (pPacket[2]+4);	//HDR+CMD+(LEN)+DATA+CS, FreeData�� Length�� ����
	else
		size = pPacket[2]+4;									//HDR+CMD+LEN+DATA+CS

	if(isRx)
		idx = sprintf(szText, "Rx: ");
	else
		idx = sprintf(szText, "Tx: ");
	for(i=0; i<size; i++)
	{
		idx += sprintf(&szText[idx], "%02x ", *p++);
	}

	wScroll = m_rcArea.w-2;
	hScroll = m_rcArea.h-2;

	switch(cmd)
	{
	case SNS_CMD_WEATHER_REQ:			//0x02
	case SNS_CMD_PARKING_REQ:			//0x04
	case SNS_CMD_ELEVATOR_STAT_REQ:		//0x05
	case SNS_CMD_ELEVATOR_CALL:			//0x07
	case SNS_CMD_WALLPAD_STAT_REQ:		//0x08
	case SNS_CMD_LAMP_ONOFF_REQ:		//0x09
	case SNS_CMD_GAS_OFF_REQ:			//0x0A
	case SNS_CMD_OUT_ONOFF_REQ:			//0x0B
		color = (src==SNS_HDR_DEV) ? LTCYAN : CYAN;
		break;
	case SNS_CMD_WEATHER_RES:			//0x01
	case SNS_CMD_PARKING_RES:			//0x03
	case SNS_CMD_ELEVATOR_STAT_RES:		//0x06
	case SNS_CMD_OUT_STAT_RES:			//0x0C
	case SNS_CMD_GAS_STAT_RES:			//0x0D
		color = (src==SNS_HDR_WALLPAD) ? LTGREEN : GREEN;
		break;
	case SNS_CMD_INIT_DATA:
		color = RED;
		break;
	case SNS_CMD_FREE_DATA:
		color = WHITE;
		break;
	default:
		color = GRAY;
		break;
	}

	size = strlen(szText);
	if(size <= MAX_PRINTABLE_STRING)
	{
		//���� �Ѷ��� ��ũ��
		GrCopyArea(m_pixmapArea, g_gc, 0, 0, wScroll, hScroll-heighLine, m_pixmapArea, 0, heighLine, MWROP_SRCCOPY);

		//�ϴܿ� ǥ��
		DrawRect(0, hScroll-heighLine, wScroll, heighLine, m_pixmapArea, g_gc, BLACK, TRUE);
	//	DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-heighLine, wScroll, heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-heighLine, wScroll, heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	}
	else
	{
		//���� �ζ��� ��ũ��
		GrCopyArea(m_pixmapArea, g_gc, 0, 0, wScroll, hScroll-(heighLine*2), m_pixmapArea, 0, heighLine*2, MWROP_SRCCOPY);

		//�ϴܿ� ǥ��
		DrawRect(0, hScroll-(heighLine*2), wScroll, (heighLine*2), m_pixmapArea, g_gc, BLACK, TRUE);
		szText[MAX_PRINTABLE_STRING-1] = 0;	//space�� ä���� �κ�
	//	DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-(heighLine*2), wScroll, heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	//	DrawText2("    ", m_pixmapArea, g_gc, 0, hScroll-heighLine, sizeFont/2*4, heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	//	DrawText2(&szText[MAX_PRINTABLE_STRING], m_pixmapArea, g_gc, sizeFont/2*4, hScroll-heighLine, wScroll-(sizeFont/2*4), heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-(heighLine*2), wScroll, heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2("    ", m_pixmapArea, g_gc, 0, hScroll-heighLine, sizeFont/2*4, heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2(&szText[MAX_PRINTABLE_STRING], m_pixmapArea, g_gc, sizeFont/2*4, hScroll-heighLine, wScroll-(sizeFont/2*4), heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	}

	//�׷��� �ű��
	GrCopyArea(g_wid, g_gc, m_rcArea.x+1, m_rcArea.y+1, m_rcArea.w-2, m_rcArea.h-2, m_pixmapArea, 0, 0, MWROP_SRCCOPY);
}

void CContextSetup::PrintSendPacket(UCHAR cmd, UCHAR data)
{
	UCHAR pkt[5];
	int ret = ERROR;

	pkt[0] = SNS_HDR_DEV;
	pkt[1] = cmd;
	pkt[2] = 1;
	pkt[3] = data;
	pkt[4] = g_wallpad_sns.CalcCheckSum(pkt, 4);

	PrintPacket(pkt, FALSE);
}

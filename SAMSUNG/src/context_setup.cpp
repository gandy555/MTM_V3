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

	//전체화면을 검정색으로 칠한다.
	DrawRect(0, 0, g_scr_info.cols, g_scr_info.rows, g_wid, g_gc, BLACK, TRUE);

	//타이틀 박스
	DrawRect2(&m_rcTitle, g_wid, g_gc, WHITE, FALSE);

	//표시영역
	DrawRect2(&m_rcArea, g_wid, g_gc, BLACK, TRUE);		//검정색으로 지우고
	DrawRect2(&m_rcArea, g_wid, g_gc, WHITE, FALSE);	//흰색으로 박스그리고

	//우측 버튼 박스
	DrawRect2(&m_rcButton[BUTTON_RT], g_wid, g_gc, WHITE, FALSE);
	DrawRect2(&m_rcButton[BUTTON_RM], g_wid, g_gc, WHITE, FALSE);
	DrawRect2(&m_rcButton[BUTTON_RB], g_wid, g_gc, WHITE, FALSE);

	if(m_nSetupStep==CONTEXT_SETUP_STEP_SELECT)
	{
		//우측버튼
		DrawTextRect("이전", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
			DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawTextRect("다음", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
			DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);

		if(nContextNum != CONTEXT_SETUP_DOOR)
		{
			DrawTextRect("설정", g_wid, g_gc, &m_rcButton[BUTTON_RB], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RB].x, m_rcButton[BUTTON_RB].y+m_rcButton[BUTTON_RB].h+2, m_rcButton[BUTTON_RB].w, 16);
				DrawTextRect("VolDn Long\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
	}
	else
	{
		DrawTextRect("완료", g_wid, g_gc, &m_rcButton[BUTTON_RB], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
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

		//우측버튼
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

		//우측버튼
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

		//우측버튼
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

		//우측버튼 : 표시없음
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
			DrawTextRect("", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		}

		if(g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT)==HIGH)
			DrawTextRect2("문이 닫혔습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		else
			DrawTextRect2("문이 열렸습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		break;
	case CONTEXT_SETUP_SOUND:
		//Title
		DrawTextRect("Sound", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//우측버튼
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("증가", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("감소", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}
		break;
	case CONTEXT_SETUP_MICROWAVE:
		//Title
		DrawTextRect("MicroWave", g_wid, g_gc, &m_rcTitle, g_font, 36, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		//우측버튼
		if(m_nSetupStep==CONTEXT_SETUP_STEP_PAGE)
		{
			DrawTextRect("증가", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp (L=빠르게)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
			DrawTextRect("감소", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn (L=빠르게)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}

		//그래프
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
			//우측버튼
			switch(m_nWallPadKeyPage)
			{
			case CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF:		//일괄소등
				DrawTextRect("일괄소등", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_GAS_CUT:		//가스차단
				DrawTextRect("가스차단", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_ON:	//보안설정
				DrawTextRect("보안설정", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_OFF:	//보안해제
				DrawTextRect("보안해제", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_WEATHER_INFO:	//날씨요청
				DrawTextRect("날씨요청", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_PARKING_INFO:	//주차요청
				DrawTextRect("주차요청", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_ELV_INFO:		//E/V요청
				DrawTextRect("E/V요청", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			case CONTEXT_SETUP_WALLPAD_PAGE_ELV_CALL:		//E/V호출
				DrawTextRect("E/V호출", g_wid, g_gc, &m_rcButton[BUTTON_RT], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				break;
			default:
				break;
			}
				SetRect(&rcText, m_rcButton[BUTTON_RT].x, m_rcButton[BUTTON_RT].y+m_rcButton[BUTTON_RT].h+2, m_rcButton[BUTTON_RT].w, 16);
				DrawTextRect("VolUp\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);

			DrawTextRect("명령전환", g_wid, g_gc, &m_rcButton[BUTTON_RM], g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
				SetRect(&rcText, m_rcButton[BUTTON_RM].x, m_rcButton[BUTTON_RM].y+m_rcButton[BUTTON_RM].h+2, m_rcButton[BUTTON_RM].w, 16);
				DrawTextRect("VolDn (L=직접명령)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		}

		GrCopyArea(g_wid, g_gc, m_rcArea.x+1, m_rcArea.y+1, m_rcArea.w-2, m_rcArea.h-2, m_pixmapArea, 0, 0, MWROP_SRCCOPY);
		break;
	}

	//하단안내
	SetRect(&rcText, m_rcArea.x, m_rcArea.y+m_rcArea.h+2, m_rcArea.w, 32);
	DrawTextRect("S:짧게누름,  L:길게누름\0", g_wid, g_gc, &rcText, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

	if(nContextNum==CONTEXT_SETUP_WALLPAD)
	{
		SetRect(&rcText, m_rcArea.x+200, m_rcArea.y+m_rcArea.h+2, m_rcArea.w-200, 32);
		DrawTextRect("(FreeData패킷생략)\0", g_wid, g_gc, &rcText, g_font, 12, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);
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
	//	g_microwave.StopMeasure();				//MicrowaveHandler에서의 측정을 충단시킴

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
		DrawTextRect("*가능한 범위: 0~8\0", g_wid, g_gc, &rcText, g_font, 18, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
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

	if(m_nContextNum != CONTEXT_SETUP_WALLPAD) return;	//WallPad테스트가 아닌경우는 처리하지 않음

//	printf("+CContextSetup::RecvProc\r\n");

//	pBuffer = (UCHAR *)pParam;

//	PrintPacket(pBuffer);

	cmd = pPacket[1];

	// Command별 처리
	switch(cmd)
	{
//-------------------------------------------------------------------------------------
	case SNS_CMD_WEATHER_RES:		//0x01, 월패드에서 생활정보기로 날씨정보 전송
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
	case SNS_CMD_WEATHER_REQ:		//0x02, 생활정보기에서 월패드로 날씨정보 요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_PARKING_RES:		//0x03, 월패드에서 생활정보기로 주차정보 전송
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
	case SNS_CMD_PARKING_REQ:		//0x04, 생활정보기에서 월패드로 주차정보 요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_ELEVATOR_STAT_REQ:	//0x05, 생활정보기에서 월패드로 엘레베이터 상태요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
	case SNS_CMD_ELEVATOR_STAT_RES:	//0x06, 월패드에서 생활정보기로 엘레베이터 상태전송
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
	case SNS_CMD_ELEVATOR_CALL:		//0x07, 생할정보기에서 월패드로 엘레베이터 호출
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_WALLPAD_STAT_REQ:	//0x08, 생할정보기에서 월패드로 일괄소등,가스,외출설정 상태요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_LAMP_ONOFF_REQ:	//0x09, 생할정보기에서 월패드로 일괄소등 On/Off요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_GAS_OFF_REQ:		//0x0A, 생할정보기에서 월패드로 가스밸브 Off요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_OUT_ONOFF_REQ:		//0x0B, 생할정보기에서 월패드로 외출설정 On/Off요청
		g_wallpad_sns.SendFreeData();
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(SNS_CMD_FREE_DATA);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_OUT_STAT_RES:		//0x0C, 월패드에서 생활정보기로 외출설정 상태전송
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_GAS_STAT_RES:		//0x0D, 월패드에서 생활정보기로 가스밸브 상태전송
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_ELEVATOR_ARRIVE:	//0x0E, 월패드에서 생활정보기로 엘리베이터 목적층 도착시 메세지 전송
		g_wallpad_sns.SendAck(cmd);
		m_isWaitForResponse = FALSE;
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_INIT_DATA:			//0x5A, 월패드에서 생활정보기로 : 월패드가 켜질때, 월패드가 Ack Data 10회이상 미수신할 경우
		g_wallpad_sns.SendAck(cmd);
		PrintPacket(pPacket);
		PrintSendPacket(cmd);
		break;
//-------------------------------------------------------------------------------------
	case SNS_CMD_FREE_DATA:			//0x41, 월패드에서 생활정보기로 : Event Data 없을 경우, Live Check

		//예약된 요청이 있으면
		if(m_isRequestReserve)
		{
			g_wallpad_sns.SendRequest(m_cmdRequestReserve, m_dataRequestReserve);
			PrintPacket(pPacket);
			PrintSendPacket(m_cmdRequestReserve, m_dataRequestReserve);
			m_isRequestReserve = FALSE;			//예약처리됨
			m_isWaitForResponse = TRUE;			//대기시작
			m_nWaitCountForResponse = 0;
		}
		//요청에 대한 응답을 대기중인 상태라면 --> 응답대신 FreeData가 온다는것은 재요청이 필요한 상황
		else if(m_isWaitForResponse)
		{
			//요청응답이 없는 경우 3회 재전송
			m_nWaitCountForResponse++;
			if(m_nWaitCountForResponse <= MAX_WAIT_COUNT_FOR_RESPONSE)
			{
			//	printf("%s: Re-RequestReserve(retry=%d) cmd=0x%x, data=0x%x\r\n", __func__, 
			//			m_nWaitCountForResponse, m_cmdRequestReserve, m_dataRequestReserve);
				g_wallpad_sns.SendRequest(m_cmdRequestReserve, m_dataRequestReserve);
				PrintPacket(pPacket);
				PrintSendPacket(m_cmdRequestReserve, m_dataRequestReserve);
			}
			//재전송 타임아웃이면 응답대기를 중단
			else
			{
			//	printf("%s: RequestReserve Canceled: retry timeout!\r\n", __func__);
				m_isWaitForResponse = FALSE;
				g_wallpad_sns.SendFreeData();
				PrintPacket(pPacket);
				PrintSendPacket(SNS_CMD_FREE_DATA);
			}
		}
		//대기중인 요청이 없는 상태
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

		//그래프 Shift
		GrCopyArea(m_pixmapGraph, g_gc, 0, 0, m_rcGraph.w-GRAPH_SPEED, m_rcGraph.h, m_pixmapGraph, GRAPH_SPEED, 0, MWROP_SRCCOPY);

		//Shift되고난 부분 지우기 (우측영역)
		DrawRect(m_rcGraph.w-GRAPH_SPEED, 0, GRAPH_SPEED, m_rcGraph.h, m_pixmapGraph, g_gc, BLACK, TRUE);
		DrawRect(m_rcGraph.x+m_rcGraph.w-GRAPH_SPEED, m_rcGraph.y, GRAPH_SPEED, m_rcGraph.h, g_wid, g_gc, BLACK, TRUE);

		//그래프 그리기
		DrawLine(m_rcGraph.w-GRAPH_SPEED, m_rcGraph.h-y1, m_rcGraph.w, m_rcGraph.h-y2, m_pixmapGraph, g_gc, CYAN, FALSE);

		//그래프 옮기기
		GrCopyArea(g_wid, g_gc, m_rcGraph.x, m_rcGraph.y, m_rcGraph.w, m_rcGraph.h, m_pixmapGraph, 0, 0, MWROP_SRCCOPY);

		//설정값 표시(노란라인)
		ySetupValue = (int)(m_rcGraph.h*g_setup_data.m_SetupData.adc_value/1023.0);
		DrawLine(m_rcGraph.x, m_rcGraph.y+m_rcGraph.h-ySetupValue, 
				 m_rcGraph.x+m_rcGraph.w, m_rcGraph.y+m_rcGraph.h-ySetupValue, 
				 g_wid, g_gc, YELLOW, FALSE);

		//텍스트 표시
		sprintf(szText, "측정값: %04d,  설정값: %04d\0", nAdcValue, g_setup_data.m_SetupData.adc_value);
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
		//설정항목 선택모드
		if(m_nSetupStep == CONTEXT_SETUP_STEP_SELECT)
		{
			if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)) ||			//이전
				CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)) )
			{
				if(m_nContextNum == CONTEXT_SETUP_LED)
				{
					//WallPad용 픽셀맵 클리어
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
			else if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_MIDDLE)) ||		//다음
					 CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_DOWN)) )
			{
				if(m_nContextNum == CONTEXT_SETUP_WALLPAD)
				{
					g_wallpad_sns.m_isContextSetupWallPad = FALSE;
					ChangeContext(CONTEXT_SETUP_LED);
				}
				else if(m_nContextNum == CONTEXT_SETUP_MICROWAVE)
				{
					//WallPad용 픽셀맵 클리어
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
			else if(CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_BOTTOM)))	//설정
			{
				if(m_nContextNum != CONTEXT_SETUP_DOOR)
				{
					m_nSetupStep = CONTEXT_SETUP_STEP_PAGE;
					ChangeContext(m_nContextNum);
				}
			}
		}
		else	//설정항목 변경모드
		{
			if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)) ||
				CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)) )
			{
				switch(m_nContextNum)
				{
				case CONTEXT_SETUP_LED:			// ON
					g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);
					DrawTextRect2("LED를 켰습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_BACKLIGHT:	// ON
					g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
					DrawTextRect2("백라이트를 켰습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_RELAY:		// ON
					g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, HIGH);
					DrawTextRect2("릴레이를 켰습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_DOOR:
					break;
				case CONTEXT_SETUP_SOUND:		// 증가
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
				case CONTEXT_SETUP_MICROWAVE:	// 증가
					if(g_setup_data.m_SetupData.adc_value < MAX_ADC_VALUE)
					{
						g_setup_data.m_SetupData.adc_value++;
					}
					break;
				case CONTEXT_SETUP_WALLPAD:		// 명령요청
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
					DrawTextRect2("LED를 껏습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_BACKLIGHT:	// OFF
					g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, LOW);
					DrawTextRect2("백라이트를 껏습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_RELAY:		// OFF
					g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, LOW);
					DrawTextRect2("릴레이를 껏습니다", g_wid, g_gc, &m_rcArea, g_font, 24, WHITE, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
					break;
				case CONTEXT_SETUP_DOOR:
					break;
				case CONTEXT_SETUP_SOUND:		// 감소
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
				case CONTEXT_SETUP_MICROWAVE:	// 감소
					if(g_setup_data.m_SetupData.adc_value > 0)
					{
						g_setup_data.m_SetupData.adc_value--;
					}
					break;
				case CONTEXT_SETUP_WALLPAD:		// 명령전환
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
//	m_isRequestReserve = FALSE;			//예약처리됨
//	m_isWaitForResponse = TRUE;			//대기시작
//	m_nWaitCountForResponse = 0;
}

// SNS패킷중 제일 긴것은 날씨응답으로 22bytes
// 패킷덤프는 "T(R)x: xx xx ..." 형태이므로 최대패킷 덤프 스트링 사이즈는 70bytes
// 70바이트는 18폰트 기준으로 화면에서 630pixel이 필요함
// 표시영역의 화면폭은 518 pixel이고 18폰트 크기기준으로 57바이트 스트링까지 표시가능

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
		size = (cmd==SNS_CMD_FREE_DATA) ? 4 : (pPacket[2]+4);	//HDR+CMD+(LEN)+DATA+CS, FreeData의 Length가 없음
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
		//위로 한라인 스크롤
		GrCopyArea(m_pixmapArea, g_gc, 0, 0, wScroll, hScroll-heighLine, m_pixmapArea, 0, heighLine, MWROP_SRCCOPY);

		//하단에 표시
		DrawRect(0, hScroll-heighLine, wScroll, heighLine, m_pixmapArea, g_gc, BLACK, TRUE);
	//	DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-heighLine, wScroll, heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-heighLine, wScroll, heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	}
	else
	{
		//위로 두라인 스크롤
		GrCopyArea(m_pixmapArea, g_gc, 0, 0, wScroll, hScroll-(heighLine*2), m_pixmapArea, 0, heighLine*2, MWROP_SRCCOPY);

		//하단에 표시
		DrawRect(0, hScroll-(heighLine*2), wScroll, (heighLine*2), m_pixmapArea, g_gc, BLACK, TRUE);
		szText[MAX_PRINTABLE_STRING-1] = 0;	//space로 채워진 부분
	//	DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-(heighLine*2), wScroll, heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	//	DrawText2("    ", m_pixmapArea, g_gc, 0, hScroll-heighLine, sizeFont/2*4, heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	//	DrawText2(&szText[MAX_PRINTABLE_STRING], m_pixmapArea, g_gc, sizeFont/2*4, hScroll-heighLine, wScroll-(sizeFont/2*4), heighLine, g_font, sizeFont, (isRx) ? WHITE : YELLOW, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2(szText, m_pixmapArea, g_gc, 0, hScroll-(heighLine*2), wScroll, heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2("    ", m_pixmapArea, g_gc, 0, hScroll-heighLine, sizeFont/2*4, heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
		DrawText2(&szText[MAX_PRINTABLE_STRING], m_pixmapArea, g_gc, sizeFont/2*4, hScroll-heighLine, wScroll-(sizeFont/2*4), heighLine, g_font, sizeFont, color, BLACK, TXT_HALIGN_LEFT|TXT_VALIGN_TOP);
	}

	//그래프 옮기기
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

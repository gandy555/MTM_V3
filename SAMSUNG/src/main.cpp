/*

*/

#include "common.h"

//#include "serial_wallpad_sns.h"

#include "context_weather.h"
#include "context_elevator.h"
#include "context_parking.h"
#include "context_security.h"
#include "context_alloff.h"
#include "context_gas.h"
#include "context_setup.h"

#include "main.h"

//#define BACKLIGHT_OFF_DISABLE

//
// Global Data
//
GR_SCREEN_INFO			g_scr_info = {0};
GR_WINDOW_ID			g_wid = 0;
GR_GC_ID				g_gc = 0;
GR_FONT_ID				g_font = 0;

struct parking_node{
	PARKING_RES curr;
	struct parking_node* prev;
	struct parking_node* next;
};

struct parking_info_list {
	struct parking_node *head;
	struct parking_node *tail;
	int cnt;
};

struct parking_info_list g_parking_info_list = {
	NULL,
	NULL,
	0,
};

BOOL					g_isMainLoopRunning = TRUE;
BOOL					g_isBackLightOn = TRUE;

//
// Class Instance
//
CSetupData				g_setup_data;
CGpioMtm				g_gpio_mtm;
CTimer					g_timer;
CState					g_state;
CMicrowave				g_microwave;
CSerialWallPad_SNS		g_wallpad_sns;
CWatchdog				g_watchdog;

//
// Prototype
//
BOOL Initialize();
void Deinitialize();
void MainKeyProc(USHORT usKeyFlag, USHORT usKeyEvent);
void MicrowaveProc();

BOOL init_gui();
BOOL init_setup();
BOOL init_state();

//시그날 핸들러
void user_signal_register();
void user_signal_unregister();
static void error_handler(GR_EVENT *ep);

//타이머 핸들러
void clock_timer_handler(void *pParam);
void back_light_timer_handler(void *pParam);


//
// Main Procedure
//
int main(int arg_gc, char *argv[])
{
//	BOOL isContextDraw = TRUE;

	UCHAR MainBuffer[MAX_RECV_BUFFER];

	void* thread_result;

	GR_EVENT	event;

	USHORT		usKeyFlag = 0;
	USHORT		usKeyEvent = 0;
	UINT		uAdcValue = 0;

	LEVEL		lvDoorOld, lvDoorNew;

	CContext* pContext = NULL;

//	RECT rcCapture = {0, 0, 20, 20};

	int x, y, ret;

	printf("Starting Main..\r\n");

	if(Initialize()==FALSE)
	{
		Deinitialize();
		printf("Main Stopped By Initialize Failure\r\n");
		return 0;
	}

	printf("Starting Main Loop..\r\n");

	lvDoorOld = g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT);

	g_watchdog.SetPeriod(2000);
	g_watchdog.Enable();
	printf("Watchdog Enabled\r\n");

	while(g_isMainLoopRunning)
	{
		//키이벤트 체크
		if(g_gpio_mtm.CheckGpioKeyEvent(&usKeyFlag, &usKeyEvent))
		{
			MainKeyProc(usKeyFlag, usKeyEvent);

			if(g_state.isStateChange())	StopWavPlay();
		}

		//Door 체크: 상태를 이벤트로 전환
		if(g_state.GetState() == STATE_SETUP)
		{
			lvDoorNew = g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT);
			if(lvDoorNew != lvDoorOld)
			{
				if(lvDoorNew)
					SET_FLAG(usKeyFlag, GPIO_DOOR_DETECT);
				else
					CLR_FLAG(usKeyFlag, GPIO_DOOR_DETECT);
				lvDoorOld = lvDoorNew;

				pContext = g_state.GetCurrContext();
				if(pContext)
				{
					pContext->ButtonUp(usKeyFlag, MTM_GPIO_STATE_CHANGE);
				}
			}
		}

		//마이크로웨이브 체크
		if(g_microwave.DoAdcSamplingStep())
		{
			uAdcValue = g_microwave.GetAdcSamplingResult();
			if(uAdcValue > g_setup_data.m_SetupData.adc_value)
			{
				MicrowaveProc();
			}
		}

		//상태변환 체크
		if(g_state.isStateChange())
		{
		//	StopWavPlay();
			g_state.ChangeState(g_state.m_nNextState);
		}

		//컨텍스트 루틴 실행
		if(g_state.m_pCurrContext)
		{
			//화면갱신
			if(g_state.m_pCurrContext->m_isContextDraw)
			{
				g_state.m_pCurrContext->m_isContextDraw = FALSE;
				g_state.m_pCurrContext->Draw(g_state.m_pCurrContext->m_nContextNum);
				g_state.m_pCurrContext->Proc(g_state.m_pCurrContext->m_nContextNum);

				if(g_isBackLightOn==FALSE)
				{
					g_isBackLightOn = TRUE;

					//BackLight 다시키고 BACK_LIGHT_TIMER 재시작
					g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
					g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);
					g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
				}
			}

		#if 0
			//수신된 패킷이 큐에 있으면
			else if(g_wallpad_sns.GetPacketCountInQue())
			{
				//큐에서 패킷을 복사해와서
				if(g_wallpad_sns.PopPacketFromQue(MainBuffer))
				{
					//컨텍스트에서 처리하게 한다
					g_state.m_pCurrContext->RecvProc(MainBuffer);
				}
			}
		#endif

			//컨텍스트의 처리루틴 수행 : Threading
			else
			{
				g_state.m_pCurrContext->ThreadProc();
			}
		}

	#if 0
		GrCheckNextEvent(&event);
		if(event.type == GR_EVENT_TYPE_CLOSE_REQ)
		{
			g_isMainLoopRunning = FALSE;
		}
	#endif

		usleep(1000);

		g_watchdog.Refresh();
	}

	printf("Main Loop Terminated\r\n");

	Deinitialize();

	printf("Main Stopped\r\n");
	
	return 0;
}

//
// Implementation
//
BOOL Initialize()
{
	user_signal_register();

	printf("Setup Initialize ");
	if(init_setup())
	printf("[Done]\r\n");

	printf("GUI Initializing..\r\n");
	if(init_gui()==FALSE) return FALSE;
	printf("GUI Initialize [Done]\r\n");

	printf("State Initializing..\r\n");
	if(init_state()==FALSE) return FALSE;
	printf("State Initialize [Done]\r\n");

	printf("GPIO Initialize ");
	if(g_gpio_mtm.Init()==FALSE) return FALSE;
	printf("[Done]\r\n");

	printf("WATCHDOG Initialize ");
	if(g_watchdog.Init()==FALSE) return FALSE;
	printf("[Done]\r\n");

	printf("Microwave Initialize ");
	if(g_microwave.Init()==FALSE) return FALSE;
	printf("[Done]\r\n");

	printf("Serial Initialize ");
	if(g_wallpad_sns.Open(UART1_DEV, B9600)==FALSE) return FALSE;
	if(g_wallpad_sns.Run()==FALSE) return FALSE;
	printf("[Done]\r\n");

	printf("Timer Initialize ");
	if(g_timer.Run()==FALSE) return FALSE;
	printf("[Done]\r\n");

	g_timer.SetTimer(CLOCK_TIMER, 1, clock_timer_handler, "Clock Timer");
#ifndef BACKLIGHT_OFF_DISABLE
	g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
#endif

	g_gpio_mtm.SetGpioOutPin(GPIO_AMP, LOW);	//

	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (g_setup_data.m_SetupData.alloff_stat) ? LOW : HIGH);
	g_state.ChangeState(STATE_WEATHER);

	return TRUE;
}

void Deinitialize()
{
	g_timer.Stop();

	g_wallpad_sns.Stop();
	g_wallpad_sns.Close();

	g_watchdog.Disable();
	g_watchdog.DeInit();

//	g_microwave.Stop();
	g_microwave.DeInit();

//	g_gpio_mtm.Stop();
	g_gpio_mtm.DeInit();

	g_state.DeInit();

#if 0
	DrawRect(0, 0, g_scr_info.cols-1, g_scr_info.rows-1, g_wid, g_gc, BLACK, TRUE);
#else
	DrawRect(200, 140, 400, 200, g_wid, g_gc, BLACK, TRUE);
	DrawText("종료되었습니다.", g_wid, g_gc, 200, 140, 400, 200, g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
#endif

	GrDestroyGC(g_gc);
	GrClose();

	user_signal_unregister();
}

void MainKeyProc(USHORT usKeyFlag, USHORT usKeyEvent)
{
	CContext* pContext = NULL;

	//Todo: BackLight Timer갱신
	//	    Setup상태면 KeyFlag,KeyEvent를 ContextSetup에서 처리하게 하고
	//	    Setup상태가 아니면 각키에 해당하는 상태로 전환

	//백라이트 처리
	if(g_isBackLightOn)
	{
		g_timer.ResetTimer(BACK_LIGHT_TIMER);
	}
	else
	{
		g_timer.KillTimer(PARKING_REQ_SPIN_TIMER);		
		g_state.ChangeState(STATE_WEATHER);

	#if 0
		g_isBackLightOn = TRUE;

		//BackLight 다시키고 BACK_LIGHT_TIMER 재시작
		g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
		g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);
		g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
	#endif
		return;
	}

	//키이벤트 처리
	pContext = g_state.GetCurrContext();
	if(pContext)
	{
		if(usKeyEvent==MTM_GPIO_BUTTON_UP)
		{
			if(g_state.GetState() != STATE_SETUP)
			{
				if(CHK_FLAG(usKeyFlag, FLAG(GPIO_REAR_VOL_UP)))
				{
					if(g_setup_data.m_SetupData.volume < MAX_VOLUME_LEVEL)
					{
						g_setup_data.m_SetupData.volume++;
						SetVolume(g_setup_data.m_SetupData.volume);
						PlayWavFile("/app/sound/touch.wav");
					}
				}
				else if(CHK_FLAG(usKeyFlag, FLAG(GPIO_REAR_VOL_DOWN)))
				{
					if(g_setup_data.m_SetupData.volume > 0)
					{
						g_setup_data.m_SetupData.volume--;
						SetVolume(g_setup_data.m_SetupData.volume);
						PlayWavFile("/app/sound/touch.wav");
					}
				}
				else
				{
					pContext->ButtonUp(usKeyFlag, usKeyEvent);
				}
			}
			else
			{
				pContext->ButtonUp(usKeyFlag, usKeyEvent);
			}
		}
		else //if(usKeyEvent==MTM_GPIO_BUTTON_LONG)
		{
		#if 0
			UCHAR ucV1 = 0;
			UCHAR* p = NULL;

			//나누기0
			if(CHK_FLAG(usKeyFlag, FLAG(GPIO_FRONT_LEFT_TOP)|FLAG(GPIO_FRONT_LEFT_BOTTOM)))
			{
				printf("--> Dividing 0 !!\r\n");
				ucV1 = 10 / ucV1;
			}
			//널포인터
			else if(CHK_FLAG(usKeyFlag, FLAG(GPIO_FRONT_LEFT_MIDDLE)|FLAG(GPIO_FRONT_LEFT_BOTTOM)))
			{
				printf("--> Null Pointer Access !!\r\n");
				*p = 0x12345678;
			}
		#endif
			pContext->ButtonDown(usKeyFlag, usKeyEvent);
		}
	}

}

void MicrowaveProc()
{
	//Todo: 꺼져있는 상태(IDLE)에서는 WEATHER로 상태전환 하고 BackLight On
	//		켜져있는 상태에서는 BackLight Check Timer갱신

	if(g_isBackLightOn)
	{
		g_timer.ResetTimer(BACK_LIGHT_TIMER);
	}
	else
	{
		g_timer.KillTimer(PARKING_REQ_SPIN_TIMER);	
		g_state.ChangeState(STATE_WEATHER);
	#if 0
		g_isBackLightOn = TRUE;

		//BackLight 다시키고 BACK_LIGHT_TIMER 재시작
		g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
		g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);
		g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, back_light_timer_handler, "LCD BackLight Timer");
	#endif
	}
}

BOOL init_gui()
{
	CObject* pObject;
	UINT id;

	// start nano-x service
	if(GrOpen() < 0) 
	{
		printf("[Failure]\r\n--> %s: GrOpen Failed!!\r\n", __func__);
		return FALSE;
	}

	GrSetErrorHandler(error_handler);
	GrGetScreenInfo(&g_scr_info);

	// prepare g_font
	g_font = GrCreateFontEx((GR_CHAR *)FONT_PATH, 18, 18, NULL);
	GrSetFontAttr(g_font, (GR_TFKERNING | GR_TFANTIALIAS), 0);

	g_wid = GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0, g_scr_info.cols, g_scr_info.rows, 0, BLACK, 0);
	if(g_wid == 0)
	{
		printf("[Failure]\r\n--> %s: GrNewWindow(main) failure\r\n", __func__);
		GrClose();
		return FALSE;
	}

	g_gc = GrNewGC();

	GrRaiseWindow(g_wid);
    GrMapWindow(g_wid);

	GrSelectEvents(g_wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE);

	GrSetGCUseBackground(g_gc, FALSE);
//	GrSetGCUseBackground(g_gc, TRUE);
	GrSetGCFont(g_gc, g_font);

//	GrSetGCBackground(g_gc, BLACK);
	GrSetGCForeground(g_gc, WHITE);

//	BuildObject();

	return TRUE;
}

BOOL init_setup()
{
	BOOL fRet;

	fRet = g_setup_data.LoadSetupData();
	if(fRet==FALSE)
	{
		g_setup_data.SetSetupDataDefault();
	} else {
		g_setup_data.m_SetupData.alloff_stat	= 0;	//일괄소등 해제
		g_setup_data.m_SetupData.gas_stat		= 1;	//가스밸브 사용중
		g_setup_data.m_SetupData.security_stat	= 0;	//보안설정 해제	
	}

	SetVolume(g_setup_data.m_SetupData.volume);

	return fRet;
}

BOOL init_state()
{
	CContext* pContext;

	if(g_state.Init()==FALSE) return FALSE;

	DrawRect(200, 140, 400, 200, g_wid, g_gc, BLACK, TRUE);
	DrawText("프로그램 로딩중...", g_wid, g_gc, 200, 140, 400, 200, g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

	//step=20 x 7 = 140
	DrawRect(330, 260, 140, 20, g_wid, g_gc, WHITE, FALSE);

	//메인 (대기화면:날씨)
	printf("Initialize Context: Weather ");
	pContext = new CContextWeather(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_WEATHER, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 20, 20, g_wid, g_gc, WHITE, TRUE);

	//엘리베이터
	printf("Initialize Context: Elevator ");
	pContext = new CContextElevator(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_ELEVATOR, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 40, 20, g_wid, g_gc, WHITE, TRUE);

	//주차
	printf("Initialize Context: Parking ");
	pContext = new CContextParking(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_PARKING, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 60, 20, g_wid, g_gc, WHITE, TRUE);

	//보안(방범)
	printf("Initialize Context: Security ");
	pContext = new CContextSecurity(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_SECURITY, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 80, 20, g_wid, g_gc, WHITE, TRUE);

	//소등
	printf("Initialize Context: AllOff ");
	pContext = new CContextAllOff(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_ALLOFF, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 100, 20, g_wid, g_gc, WHITE, TRUE);

	//가스
	printf("Initialize Context: Gas ");
	pContext = new CContextGas(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_GAS, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 120, 20, g_wid, g_gc, WHITE, TRUE);

	//설정
	printf("Initialize Context: Setup ");
	pContext = new CContextSetup(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_SETUP, pContext);
		printf("[Done]\r\n");
	}
	else
	{
		printf("[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 140, 20, g_wid, g_gc, WHITE, TRUE);

	return TRUE;
}

static void user_signal_handler(int sig)
{
	switch(sig) 
	{
	case SIGINT:
		printf("--> SIGINT Occured\r\n");
	//	post_event(GR_EVENT_TYPE_CLOSE_REQ, 0, 0, 0);
		g_isMainLoopRunning = FALSE;
		g_watchdog.Disable();
		kill(getpid(), SIGKILL);
		break;
	case SIGTERM:
		printf("--> SIGTERM Occured\r\n");
	//	post_event(GR_EVENT_TYPE_CLOSE_REQ, 0, 0, 0);
		g_isMainLoopRunning = FALSE;
	//	g_watchdog.Disable();
		kill(getpid(), SIGKILL);
		break;
	case SIGABRT:
		printf("--> SIGABRT Occured\r\n");
	//	post_event(GR_EVENT_TYPE_CLOSE_REQ, 0, 0, 0);
		g_isMainLoopRunning = FALSE;
		kill(getpid(), SIGKILL);
		break;
	case SIGSEGV:
		printf("--> SIGSEGV Occured\r\n");
	//	post_event(GR_EVENT_TYPE_CLOSE_REQ, 0, 0, 0);
		g_isMainLoopRunning = FALSE;
		kill(getpid(), SIGKILL);
		break;
	default:
		printf("--> Unknown Signal(0x%x) Occured\r\n", sig);
		kill(getpid(), SIGKILL);
		break;
	}
}

void user_signal_register()
{
	if(signal(SIGINT, user_signal_handler) == SIG_IGN) 
	{
		signal(SIGINT, SIG_IGN);
	}
	if(signal(SIGTERM, user_signal_handler) == SIG_IGN) 
	{
		signal(SIGTERM, SIG_IGN);
	}
	if(signal(SIGABRT, user_signal_handler) == SIG_IGN) 
	{
		signal(SIGABRT, SIG_IGN);
	}
	if(signal(SIGSEGV, user_signal_handler) == SIG_IGN) 
	{
		signal(SIGSEGV, SIG_IGN);
	}
}

void user_signal_unregister()
{
	signal (SIGINT, SIG_IGN);
	signal (SIGTERM, SIG_IGN);
	signal (SIGABRT, SIG_IGN);
	signal (SIGSEGV, SIG_IGN);
}

static void error_handler(GR_EVENT *ep)
{
	printf("ERROR: code=%d, name=%s\r\n", ep->error.code, ep->error.name);
}

void clock_timer_handler(void *pParam)
{
	CContext* pContext = NULL;

	pContext = g_state.GetCurrContext();
	if(pContext)
	{
		switch(g_state.GetState())
		{
		case STATE_WEATHER:
			pContext->TimerProc(CLOCK_TIMER);
			break;
		}
	}
}

//---------------------------------------------------------------------------------
// Function Name  : parking_info_req_handler()
// Description    :
//---------------------------------------------------------------------------------
static void parking_info_req_handler(void *_param)
{
	printf("<%s>\r\n", __func__);

	g_wallpad_sns.RequestReserve(SNS_CMD_PARKING_REQ);	
}

void back_light_timer_handler(void *pParam)
{
	//BACK_LIGHT_TIMER 중단
	g_timer.KillTimer(BACK_LIGHT_TIMER);

	//BackLight 끄기
	g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, LOW);
	g_gpio_mtm.SetGpioOutPin(GPIO_LED, LOW);
	g_isBackLightOn = FALSE;

	g_timer.SetTimer(PARKING_REQ_SPIN_TIMER, PARKING_REQ_SPIN_PERIOD, parking_info_req_handler, "Parking info request spinner");
	
	//날씨화면으로 전환
//	g_state.SetNextState(STATE_WEATHER);
	g_state.SetNextState(STATE_IDLE);
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_dump()
// Description    :
//---------------------------------------------------------------------------------
static void parking_list_dump(void)
{
	struct parking_info_list* plist = &g_parking_info_list;
	struct parking_node* node;
	int i;

	printf("<%s>\r\n", __func__);				
	
	if (plist != NULL) {
		node = plist->head;
		printf("---------------------------------------------\r\n");
		for (i = 0; i < plist->cnt; i++) {			
			printf("ID : 0x%02X%02X%02X%02X\r\n", node->curr.car_num[0], 
					node->curr.car_num[1], node->curr.car_num[2], node->curr.car_num[3]);
		
			printf("floor : %c%d, zone : %c%d%d%d%d%d\r\n", node->curr.car_floor[0], node->curr.car_floor[1]
				, node->curr.car_zone[0], node->curr.car_zone[1], node->curr.car_zone[2], node->curr.car_zone[3]
				, node->curr.car_zone[4], node->curr.car_zone[5]);
			
			printf("---------------------------------------------\r\n");			
			node = node->next;
		}
	}
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_find_node()
// Description    :
//---------------------------------------------------------------------------------
static struct parking_node* parking_list_find_node(UCHAR* _card_id)
{
	struct parking_info_list* plist = &g_parking_info_list;
	struct parking_node* node;
	int i;

	if (plist != NULL) {
		node = plist->head;
		for (i = 0; i < plist->cnt; i++) {
			if (memcmp(node->curr.car_num, _card_id, MAX_CAR_NUM_SIZE) == 0) {
				return node;
			}
			node = node->next;
		}
	}

	return (struct parking_node *)NULL;
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_add_tail()
// Description    :
//---------------------------------------------------------------------------------
static void parking_list_add_tail(struct parking_node* _node)
{
	struct parking_info_list* plist = &g_parking_info_list;

	_node->next = NULL;
	_node->prev = NULL;
	if (plist->head == NULL) {
		plist->tail = _node;
		plist->head = _node;
	} else {
		_node->prev = plist->tail;
		plist->tail->next = _node;
		plist->tail = _node;
	}
	plist->cnt += 1;
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_add_head()
// Description    :
//---------------------------------------------------------------------------------
static void parking_list_add_head(struct parking_node* _node)
{
	struct parking_info_list* plist = &g_parking_info_list;

	_node->prev = NULL;
	_node->next = NULL;
	if (plist->head == NULL) {
		plist->head = _node;
		plist->tail = _node;
	} else {
		plist->head->prev = _node;	
		_node->next = plist->head;
		plist->head = _node;
	}

	plist->cnt += 1;
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_delete()
// Description    :
//---------------------------------------------------------------------------------
static int parking_list_delete(struct parking_node* _node)
{
	struct parking_info_list* plist = &g_parking_info_list;
		
	if (_node == NULL) {
		printf("<%s> delete failed(%d)\r\n", __func__, plist->cnt);
		return 1;
	}
	
	if (plist->head == _node) {
		if (_node->next != NULL) {
			plist->head = _node->next;
			plist->head->prev = NULL;
		} else {
			plist->head = NULL;
			plist->tail = NULL;
		}
		plist->cnt -= 1;
		free(_node);			
	} else {		
		if (plist->tail == _node)
			plist->tail = _node->prev;

		if (_node->prev != NULL)
			_node->prev->next = _node->next;
		if (_node->next != NULL)
			_node->next->prev = _node->prev;
		
		plist->cnt -= 1;
		free(_node);
	}
	printf("<%s> delete success(%d)\r\n", __func__, plist->cnt);
	return 0;
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_make_node()
// Description    :
//---------------------------------------------------------------------------------
static struct parking_node* parking_list_make_node(PARKING_RES* _info)
{
	struct parking_node* pNew = NULL;
	
	pNew = (struct parking_node*)malloc(sizeof(struct parking_node));
	if (pNew != NULL) {
		memset(pNew, 0, sizeof(struct parking_node));
		memcpy(&pNew->curr, _info, sizeof(PARKING_RES));
	}
	return pNew;
}

//---------------------------------------------------------------------------------
// Function Name  : byte_2_ascii()
// Description    :
//---------------------------------------------------------------------------------
static char byte_2_ascii(UCHAR _byte)
{
	char res = 0;

	res = _byte + 0x30;
	
	return res;	
}

//---------------------------------------------------------------------------------
// Function Name  : parking_item_to_str()
// Description    :
//---------------------------------------------------------------------------------
int parking_item_to_str(int _id, UCHAR* _src, char* _dst)
{
	int i, res = 0;
	
	switch (_id) {
	case PARKING_ITM_ID:
		memcpy(_dst, _src, 4);
		break;
	case PARKING_ITM_FLOOR:
		sprintf(_dst, "%c%d", _src[0], _src[1]);
		break;
	case PARKING_ITM_ZONE:
		_dst[0] = _src[0];
		for (i = 1; i < 6; i++) {
			if (_src[i] == 0xFF)
				break;
			_dst[i] = byte_2_ascii(_src[i]);
		}
		break;
	default:
		printf("<%s> Invalid ID (%d)", __func__, _id);
		res = 1;
		break;
	}

	return res;	
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_update()
// Description    :
//---------------------------------------------------------------------------------
void parking_list_update(PARKING_RES* _info)
{
	struct parking_info_list* plist = &g_parking_info_list;
	struct parking_node* pNew = NULL;

	printf("<%s>\r\n", __func__);
	
	pNew = parking_list_find_node(_info->car_num);
	if (pNew != NULL) 	// if node is exist then delete it
		parking_list_delete(pNew);

	pNew = parking_list_make_node(_info);
	if (pNew == NULL) {
		printf("**** ERROR OCCURED malloc failed ****\r\n");
		return;
	}
	
	if (plist->cnt >= MAX_PARKING_LIST) {
		printf("discard tail !! ");
		parking_list_delete(plist->tail);
	}

	// new node is must highest 
	parking_list_add_head(pNew);
	
	parking_list_dump();
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_get_item()
// Description    : _idx is 0 ~ 3
//---------------------------------------------------------------------------------
int parking_list_get_item(int _idx, PARKING_RES* _dst)
{
	struct parking_info_list* plist = &g_parking_info_list;
	struct parking_node* pNew = NULL;
	int i;

	memset(_dst, 0, sizeof(PARKING_RES));

	if (plist->head == NULL) 
		goto not_founded;
	
	pNew = plist->head;
	for (i = 0; i < _idx; i++) {
		if ((pNew->next == NULL) || (pNew->next == pNew))
			goto not_founded;
		pNew = pNew->next;
	}
	memcpy(_dst, &pNew->curr, sizeof(PARKING_RES));

	return 0;

not_founded:	
	printf("<%s> not founded !!(%d)\r\n", __func__, _idx);
	_dst->in_out = 0xff;
	return -1;
}


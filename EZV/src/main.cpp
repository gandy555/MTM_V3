/*

*/
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"

//#include "serial_wallpad_sns.h"

#include "context_weather.h"
#include "context_elevator.h"
#include "context_parking.h"
#include "context_security.h"
#include "context_light.h"
#include "context_gas.h"
#include "context_setup.h"

#include "main.h"
#define USR26_MODE	0x00
#define FIQ26_MODE	0x01
#define IRQ26_MODE	0x02
#define SVC26_MODE	0x03
#define USR_MODE	0x10
#define FIQ_MODE	0x11
#define IRQ_MODE	0x12
#define SVC_MODE	0x13
#define ABT_MODE	0x17
#define UND_MODE	0x1b
#define SYSTEM_MODE	0x1f
#define MODE_MASK	0x1f
#define T_BIT		0x20
#define F_BIT		0x40
#define I_BIT		0x80
#define CC_V_BIT	0x10000000
#define CC_C_BIT	0x20000000
#define CC_Z_BIT	0x40000000
#define CC_N_BIT	0x80000000
#define PCMASK		0x0

struct pt_regs {
	u32	uregs[18];
};

#define ARM_cpsr	uregs[16]
#define ARM_pc		uregs[15]
#define ARM_lr		uregs[14]
#define ARM_sp		uregs[13]
#define ARM_ip		uregs[12]
#define ARM_fp		uregs[11]
#define ARM_r10		uregs[10]
#define ARM_r9		uregs[9]
#define ARM_r8		uregs[8]
#define ARM_r7		uregs[7]
#define ARM_r6		uregs[6]
#define ARM_r5		uregs[5]
#define ARM_r4		uregs[4]
#define ARM_r3		uregs[3]
#define ARM_r2		uregs[2]
#define ARM_r1		uregs[1]
#define ARM_r0		uregs[0]
#define ARM_ORIG_r0	uregs[17]

#define pc_pointer(v)				\
        ((v) & ~PCMASK)

#define instruction_pointer(regs)		\
        (pc_pointer((regs)->ARM_pc))

#define user_mode(regs)				\
	(((regs)->ARM_cpsr & 0xf) == 0)

#define thumb_mode(regs) (0)

#define processor_mode(regs)			\
	((regs)->ARM_cpsr & MODE_MASK)

#define interrupts_enabled(regs)		\
	(!((regs)->ARM_cpsr & I_BIT))

#define fast_interrupts_enabled(regs)		\
	(!((regs)->ARM_cpsr & F_BIT))

#define condition_codes(regs) \
	((regs)->ARM_cpsr & (CC_V_BIT|CC_C_BIT|CC_Z_BIT|CC_N_BIT))
	
//#define BACKLIGHT_OFF_DISABLE
#define WATCHDOG_ENABLE

#define SETUP_SAVE_TIME			30	//  볼륨 변경시 30초 후에 저장 한다.

#define COREDUMP_ABORT_STACK_SIZE	0x100

u8 COREDUMP_abort[COREDUMP_ABORT_STACK_SIZE] __attribute__ ((aligned (4)));

//
// Global Data
//
GR_SCREEN_INFO			g_scr_info = {0};
GR_WINDOW_ID			g_wid = 0;
GR_GC_ID				g_gc = 0;
GR_FONT_ID				g_font = 0;

struct parking_node{
	MTM_DATA_PARKING curr;
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
BOOL					g_isTimeSync = FALSE;
BOOL					g_isContextSetupWallPad = FALSE;

//
// Class Instance
//
CSetupData				g_setup_data;
CState					g_state;
CMessage				g_message;
CTimer					g_timer;
CGpioMtm				g_gpio_mtm;
CMicrowave				g_microwave;
CWatchdog				g_watchdog;

CWallPad*				g_pWallPad = NULL;

//
// Prototype
//
pthread_mutex_t g_parking_mutex;

//초기화 루틴
BOOL Initialize();
void Deinitialize();

BOOL init_gui();
BOOL init_setup();
BOOL init_state();
BOOL init_wallpad();

//시그날 처리루틴
void user_signal_register();
void user_signal_unregister();
static void error_handler(GR_EVENT *ep);

//메세지 핸들러
void GpioEventHandler(USHORT usKeyFlag, USHORT usKeyEvent);
void MicrowaveEventHandler();
void TimerEventHandler(USHORT id, void* pParam);


static void backlight_on(void)
{
	if (g_isBackLightOn)
		return;

	g_isBackLightOn = TRUE;
	g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
	g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);
	g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, NULL, "LCD BackLight Timer");
}

//
// Main Procedure
//
int main(int arg_gc, char *argv[])
{
	CContext* pCurrContext = NULL;
	MTM_MSG msg;
	int x, y, ret;
	USHORT key_f, key_evt;
	
	DBGMSG(DBG_MAIN, "Starting Main..\r\n");

	pthread_mutex_init(&g_parking_mutex, NULL);
	
	if (Initialize() == FALSE) {
		Deinitialize();
		DBGMSG(DBG_MAIN, "Main Stopped By Initialize Failure\r\n");
		return 0;
	}

	DBGMSG(DBG_MAIN, "Starting Main Loop..\r\n");

	while (g_isMainLoopRunning) {
		msg.msg_type = MSG_IDLE;
		ret = g_message.RecvMessage(&msg, FALSE);	//non-blocking
		switch (msg.msg_type) {
		case MSG_IDLE:
			break;
		case MSG_STATE_CHANGE:
		//	DBGMSG(1, "--> MSG_STATE_CHANGE\r\n");
			g_state.ChangeState((UINT)msg.param1);
			break;
		case MSG_TIMER_EVENT:
		//	DBGMSG(1, "--> MSG_TIMER_EVENT\r\n");
			TimerEventHandler((USHORT)msg.param1, (void*)msg.param2);
			break;
		case MSG_KEY_EVENT:
		//	DBGMSG(1, "--> MSG_KEY_EVENT\r\n");
			GpioEventHandler((USHORT)msg.param1, (USHORT)msg.param2);
			break;
		case MSG_MICROWAVE_EVENT:
		//	DBGMSG(1, "--> MSG_MICROWAVE_EVENT\r\n");
			MicrowaveEventHandler();
			break;
		case MSG_WALLPAD_DATA:
			break;
		case MSG_WALLPAD_RSP_ERR:
			#if 0
			u8 rq_id, ctrl;
			rq_id = (u8)((msg.param1 >> 8) & 0xff);
			ctrl = (u8)(msg.param1 & 0xff);
			g_state.ChangeState(STATE_WEATHER);
			#endif	
			break;
		case MSG_EXIT:
			DBGMSG(1, "--> MSG_EXIT\r\n");
			g_isMainLoopRunning = FALSE;
			break;
		}

		pCurrContext = g_state.GetCurrContext();
		if (pCurrContext) {
			if (pCurrContext->m_isContextDraw) 	{
				pCurrContext->m_isContextDraw = FALSE;
				pCurrContext->Draw(pCurrContext->m_nContextNum);
				pCurrContext->Proc(pCurrContext->m_nContextNum);

				if (g_isBackLightOn==FALSE) {
					g_isBackLightOn = TRUE;

					//BackLight 다시키고 BACK_LIGHT_TIMER 재시작
					g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, HIGH);
					g_gpio_mtm.SetGpioOutPin(GPIO_LED, HIGH);

					g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, NULL, "LCD BackLight Timer");
				}
			} else if(msg.msg_type == MSG_WALLPAD_DATA) {
				pCurrContext->RecvProc(msg.data);
			} else {
				pCurrContext->ThreadProc();
			}
		}

		if (g_gpio_mtm.CheckGpioKeyEvent(&key_f, &key_evt))
			GpioEventHandler(key_f, key_evt);
		
		usleep(1000);

#ifdef WATCHDOG_ENABLE
		g_watchdog.Refresh();
#endif
	}

	DBGMSG(DBG_MAIN, "Main Loop Terminated\r\n");

	Deinitialize();

	DBGMSG(DBG_MAIN, "Main Stopped\r\n");
	
	return 0;
}

//
// 초기화 루틴
//
BOOL Initialize()
{
	BOOL fRet;

	DBGMSG(DBG_MAIN, "%s: Start\r\n", __func__);

	user_signal_register();

	DBGMSG(DBG_MAIN, "Setup Initializing..\r\n");
	fRet = init_setup();
	DBGMSG(DBG_MAIN, "Setup Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "GUI Initializing..\r\n");
		fRet = init_gui();
		DBGMSG(DBG_MAIN, "GUI Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "State Initializing..\r\n");
		fRet = init_state();
		DBGMSG(DBG_MAIN, "State Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "Message Initializing..\r\n");
		fRet = g_message.Init();
		DBGMSG(DBG_MAIN, "Message Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "GPIO Initializing..\r\n");
		if( (fRet = g_gpio_mtm.Init()) )
		{
			fRet = g_gpio_mtm.Run();
		}
		DBGMSG(DBG_MAIN, "GPIO Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "Microwave Initializing..\r\n");
		if( (fRet = g_microwave.Init()) )
		{
			fRet = g_microwave.Run();
		}
		DBGMSG(DBG_MAIN, "Microwave Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "Timer Initializing..\r\n");
		fRet = g_timer.Run();
		DBGMSG(DBG_MAIN, "Timer Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

	if(fRet)
	{
		DBGMSG(DBG_MAIN, "Wallpad Initializing..\r\n");
		fRet = init_wallpad();
		DBGMSG(DBG_MAIN, "Wallpad Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}

#ifdef WATCHDOG_ENABLE
	if(fRet)
	{
		DBGMSG(DBG_MAIN, "Watchdog Initializing..\r\n");
		fRet = g_watchdog.Init();
		DBGMSG(DBG_MAIN, "Watchdog Initialize [%s]\r\n", (fRet) ? "Done" : "Fail");
	}
#endif

	if(fRet)
	{
		g_timer.SetTimer(CLOCK_TIMER, 1, NULL, "Clock Timer");
	#ifndef BACKLIGHT_OFF_DISABLE
		g_timer.SetTimer(BACK_LIGHT_TIMER, BACK_LIGHT_TIME, NULL, "LCD BackLight Timer");
	#endif
		g_gpio_mtm.SetGpioOutPin(GPIO_AMP, LOW);	//AM OFF

		g_state.ChangeState(STATE_WEATHER);
	//	g_message.SendMessage(MSG_STATE_CHANGE, (ULONG)STATE_WEATHER);

		g_microwave.StartMeasure();

	#ifdef WATCHDOG_ENABLE
//		g_watchdog.SetPeriod(2000);
		g_watchdog.SetPeriod(MAX_WATCHDOG_PERIOD);
		g_watchdog.Enable();
	#endif
	}

	DBGMSG(DBG_MAIN, "%s: End (result=%d)\r\n", __func__, fRet);

	return fRet;
}

void Deinitialize()
{
#if 1
	DrawRect(200, 140, 400, 200, g_wid, g_gc, BLACK, TRUE);
	DrawText("종료되었습니다.", g_wid, g_gc, 200, 140, 400, 200, g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
	usleep(50000);
#endif

#ifdef WATCHDOG_ENABLE
	g_watchdog.Disable();
	g_watchdog.DeInit();
#endif


	g_timer.Stop();

	if(g_pWallPad)
	{
		g_pWallPad->Stop();
	}

	g_microwave.StopMeasure();
//	g_microwave.Stop();
	g_microwave.DeInit();

//	g_gpio_mtm.Stop();
	g_gpio_mtm.DeInit();

	g_state.DeInit();

	if(g_gc)
	{
		GrDestroyGC(g_gc);
		GrClose();
	}

	user_signal_unregister();

	pthread_mutex_destroy(&g_parking_mutex);
}

BOOL init_gui()
{
	CObject* pObject;
	UINT id;

	// start nano-x service
	if(GrOpen() < 0) 
	{
	//	DBGMSG(DBG_MAIN, "[Failure]\r\n--> %s: GrOpen Failed!!\r\n", __func__);
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
	//	DBGMSG(DBG_MAIN, "[Failure]\r\n--> %s: GrNewWindow failure\r\n", __func__);
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
		fRet = TRUE;
	}
	else
	{
		g_setup_data.m_SetupData.light_stat		= 0;	//일괄소등 해제
		g_setup_data.m_SetupData.gas_stat		= 0;	//가스밸브 사용중
		g_setup_data.m_SetupData.security_stat	= 0;	//보안설정 해제
	}

	SetVolume(g_setup_data.m_SetupData.volume);

	if(g_setup_data.m_SetupData.wallpad_type == WALLPAD_TYPE_DEFAULT)
	{
		g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_HDT;
	//	g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_SHN;
	//	g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_KCM;
	}

	switch(g_setup_data.m_SetupData.wallpad_type)
	{
	case WALLPAD_TYPE_DEFAULT:
		DBGMSG(DBG_MAIN, "WallPad Type: DEFAULT\r\n");
		break;
	case WALLPAD_TYPE_HDT:
		DBGMSG(DBG_MAIN, "WallPad Type: HDT\r\n");
		break;
	case WALLPAD_TYPE_SHN:
		DBGMSG(DBG_MAIN, "WallPad Type: SHN\r\n");
		break;
	case WALLPAD_TYPE_KCM:
		DBGMSG(DBG_MAIN, "WallPad Type: KCM\r\n");
		break;
	}

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
	DBGMSG(DBG_MAIN, "Initialize Context: Weather ");
	pContext = new CContextWeather(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_WEATHER, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 20, 20, g_wid, g_gc, WHITE, TRUE);

	//엘리베이터
	DBGMSG(DBG_MAIN, "Initialize Context: Elevator ");
	pContext = new CContextElevator(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_ELEVATOR, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 40, 20, g_wid, g_gc, WHITE, TRUE);

	//주차
	DBGMSG(DBG_MAIN, "Initialize Context: Parking ");
	pContext = new CContextParking(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_PARKING, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 60, 20, g_wid, g_gc, WHITE, TRUE);

	//보안(방범)
	DBGMSG(DBG_MAIN, "Initialize Context: Security ");
	pContext = new CContextSecurity(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_SECURITY, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 80, 20, g_wid, g_gc, WHITE, TRUE);

	//소등
	DBGMSG(DBG_MAIN, "Initialize Context: Light ");
	pContext = new CContextLight(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_LIGHT, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 100, 20, g_wid, g_gc, WHITE, TRUE);

	//가스
	DBGMSG(DBG_MAIN, "Initialize Context: Gas ");
	pContext = new CContextGas(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_GAS, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 120, 20, g_wid, g_gc, WHITE, TRUE);

	//설정
	DBGMSG(DBG_MAIN, "Initialize Context: Setup ");
	pContext = new CContextSetup(g_wid, g_gc);
	if(pContext) 
	{
		pContext->Init();
		g_state.SetContext(STATE_SETUP, pContext);
		DBGMSG(DBG_MAIN, "[Done]\r\n");
	}
	else
	{
		DBGMSG(DBG_MAIN, "[Failure]\r\n");
		return FALSE;
	}

	DrawRect(330, 260, 140, 20, g_wid, g_gc, WHITE, TRUE);

	return TRUE;
}

BOOL init_wallpad()
{
	BOOL fRet;
#if 0
	switch(g_setup_data.m_SetupData.wallpad_type)
	{
	case WALLPAD_TYPE_DEFAULT:
	#if 1	
		g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_HDT;
		g_pWallPad = new CWallPadEzville();
	#elif 0	//for SHN
		g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_SHN;
		g_pWallPad = new CWallPadShn();
	#else	//for KOCOM
		g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_KCM;
		g_pWallPad = new CWallPadKcm();
	#endif
		break;
	case WALLPAD_TYPE_HDT:
		g_pWallPad = new CWallPadHdt();
		break;
	case WALLPAD_TYPE_SHN:
		g_pWallPad = new CWallPadShn();
		break;
	case WALLPAD_TYPE_KCM:
		g_pWallPad = new CWallPadKcm();
		break;
	case WALLPAD_TYPE_EZV:
		g_pWallPad = new CWallPadEzville();
		break;
	default:
		DBGMSG(DBG_MAIN, "Invalid WallPad Type : %d\r\n", g_setup_data.m_SetupData.wallpad_type);
		return FALSE;
	}
#endif
	g_setup_data.m_SetupData.wallpad_type = WALLPAD_TYPE_HDT;
	g_pWallPad = new CWallPadEzville();

	if(g_pWallPad)
	{
		if( (fRet = g_pWallPad->Run()) == FALSE )
		{
			DBGMSG(DBG_MAIN, "Wall Pad Running Failure\r\n");
		}
	}
	else
	{
		DBGMSG(DBG_MAIN, "Wall Pad Instance Create Failure\r\n");
		fRet = FALSE;
	}

	return fRet;
}

/**
 * Dump register contents.
 */
void dump_regs(struct pt_regs *regs)
{
	unsigned long flags;
	static const char *processor_modes[] = {
		"USER_26",	"FIQ_26",	"IRQ_26",	"SVC_26",
		"UK4_26",	"UK5_26",	"UK6_26",	"UK7_26",
		"UK8_26",	"UK9_26",	"UK10_26",	"UK11_26",
		"UK12_26",	"UK13_26",	"UK14_26",	"UK15_26",
		"USER_32",	"FIQ_32",	"IRQ_32",	"SVC_32",
		"UK4_32",	"UK5_32",	"UK6_32",	"ABT_32",
		"UK8_32",	"UK9_32",	"UK10_32",	"UND_32",
		"UK12_32",	"UK13_32",	"UK14_32",	"SYS_32"
	};

        flags = condition_codes(regs);

	printf("Oops: CPU Exception!\r\n");
	printf("pc : [<%08lx>]    lr : [<%08lx>]\r\n",
	       instruction_pointer(regs),
	       regs->ARM_lr);
	printf("sp : %08lx  ip : %08lx  fp : %08lx\r\n",
	       regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf("r10: %08lx  r9 : %08lx  r8 : %08lx\r\n",
	       regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf("r7 : %08lx  r6 : %08lx  r5 : %08lx\r\n",
	       regs->ARM_r7, regs->ARM_r6, regs->ARM_r5);
	printf("r4 : %08lx  r3 : %08lx  r2 : %08lx\r\n",
	       regs->ARM_r4, regs->ARM_r3, regs->ARM_r2);
	printf("r1 : %08lx  r0 : %08lx\r\n",
	       regs->ARM_r1, regs->ARM_r0);
	printf("Flags: %c%c%c%c\r\n",
	       (flags & CC_N_BIT) ? 'N' : 'n',
	       (flags & CC_Z_BIT) ? 'Z' : 'z',
	       (flags & CC_C_BIT) ? 'C' : 'c',
	       (flags & CC_V_BIT) ? 'V' : 'v');
	printf("IRQs %s  FIQs %s  Mode %s%s\r\n",
	       interrupts_enabled (regs) ? "on" : "off",
	       fast_interrupts_enabled (regs) ? "on" : "off",
	       processor_modes[processor_mode (regs)],
	       thumb_mode (regs) ? " (T)" : "");
}

void print_back_trace(int sig)
{
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\r\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
}

//
// 시그날 루틴
//
static void user_signal_handler(int sig)
{
	//dump_regs((struct pt_regs *) COREDUMP_abort);
	print_back_trace(sig);
	switch(sig) 
	{
	case SIGINT:
		DBGMSG(DBG_SIGNAL, "--> SIGINT Occured\r\n");
		g_isMainLoopRunning = FALSE;
	#ifdef WATCHDOG_ENABLE
		g_watchdog.Disable();
	#endif
		kill(getpid(), SIGKILL);
		break;
	case SIGTERM:
		DBGMSG(DBG_SIGNAL, "--> SIGTERM Occured\r\n");
		g_isMainLoopRunning = FALSE;
	//	g_watchdog.Disable();
		kill(getpid(), SIGKILL);
		break;
	case SIGABRT:
		DBGMSG(DBG_SIGNAL, "--> SIGABRT Occured\r\n");
		g_isMainLoopRunning = FALSE;
		kill(getpid(), SIGKILL);
		break;
	case SIGSEGV:
		DBGMSG(DBG_SIGNAL, "--> SIGSEGV Occured\r\n");
		g_isMainLoopRunning = FALSE;
		kill(getpid(), SIGKILL);
		break;
	default:
		DBGMSG(DBG_SIGNAL, "--> Unknown Signal(0x%x) Occured\r\n", sig);
		kill(getpid(), SIGKILL);
		break;
	}
}

static void error_handler(GR_EVENT *ep)
{
	DBGMSG(DBG_ERROR, "ERROR: code=%d, name=%s\r\n", ep->error.code, ep->error.name);
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

//
// Event Handler
//
void GpioEventHandler(USHORT usKeyFlag, USHORT usKeyEvent)
{
	CContext* pContext = NULL;

	//백라이트 처리
	if(g_isBackLightOn)
	{
		g_timer.ResetTimer(BACK_LIGHT_TIMER);
	}
	else
	{
		g_state.ChangeState(STATE_WEATHER);
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
				if(CHK_FLAG(usKeyFlag, BIT_FLAG(GPIO_REAR_VOL_UP)))
				{
					if(g_setup_data.m_SetupData.volume < MAX_VOLUME_LEVEL)
					{
						g_setup_data.m_SetupData.volume++;
						SetVolume(g_setup_data.m_SetupData.volume);
						PlayWavFile("/app/sound/touch.wav");
						g_timer.SetTimer(SETUP_SAVE_TIMER, SETUP_SAVE_TIME, NULL, "Setup Save Timer");
					}
				}
				else if(CHK_FLAG(usKeyFlag, BIT_FLAG(GPIO_REAR_VOL_DOWN)))
				{
					if(g_setup_data.m_SetupData.volume > 0)
					{
						g_setup_data.m_SetupData.volume--;
						SetVolume(g_setup_data.m_SetupData.volume);
						PlayWavFile("/app/sound/touch.wav");
						g_timer.SetTimer(SETUP_SAVE_TIMER, SETUP_SAVE_TIME, NULL, "Setup Save Timer");
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
			pContext->ButtonDown(usKeyFlag, usKeyEvent);
		}
	}
}

void MicrowaveEventHandler()
{
	//Todo: 꺼져있는 상태(IDLE)에서는 WEATHER로 상태전환 하고 BackLight On
	//		켜져있는 상태에서는 BackLight Check Timer갱신

	if(g_isBackLightOn)
	{
		g_timer.ResetTimer(BACK_LIGHT_TIMER);
	}
	else
	{
		//실제 화면이 그려지고 난뒤 켜지도록 처리하기 위해서
		//상태만 전환하고 백라이트는 켜지 않음
		g_state.ChangeState(STATE_WEATHER);
	}
}

void TimerEventHandler(USHORT id, void* pParam)
{
	CContext* pContext = NULL;
	
	pContext = g_state.GetCurrContext();

	if (pContext == NULL) {
		if (id == SETUP_SAVE_TIMER) {
			DBGMSG(1, "--> SETUP_SAVE_TIMER\r\n");
			g_timer.KillTimer(SETUP_SAVE_TIMER);
			g_setup_data.SaveSetupData();
		}
		return;
	}
	
	switch(id)
	{
	case CLOCK_TIMER:			//clock_timer_handler
	//	DBGMSG(1, "--> CLOCK_TIMER [%s]\r\n", g_state.GetStateName(g_state.GetState()));
		if(g_state.GetState() == STATE_WEATHER)
		{
			pContext->TimerProc(id);
		}
		break;
	case BACK_LIGHT_TIMER:		//back_light_timer_handler
		DBGMSG(1, "--> BACK_LIGHT_TIMER\r\n");
		if(g_isBackLightOn)
		{
			//BACK_LIGHT_TIMER 중단
			g_timer.KillTimer(BACK_LIGHT_TIMER);

			//BackLight 끄기
			g_gpio_mtm.SetGpioOutPin(GPIO_LCD_BACKLIGHT, LOW);
			g_gpio_mtm.SetGpioOutPin(GPIO_LED, LOW);
			g_isBackLightOn = FALSE;

			//날씨화면으로 전환
		//	g_state.ChangeState(STATE_WEATHER);
			g_state.ChangeState(STATE_IDLE);
		}
		break;
	case ELEVATOR_TIMER:		//elevator_timer_handler
		DBGMSG(1, "--> ELEVATOR_TIMER\r\n");
		break;
	case PARKING_TIMER:			//parking_timer_handler
	//	DBGMSG(1, "--> PARKING_TIMER\r\n");
		break;
	case RESPONSE_TIMER:
	//	DBGMSG(1, "--> RESPONSE_TIMER\r\n");
		pContext->TimerProc(id);
		break;
	case RETRY_TIMEOUT:
	//	DBGMSG(1, "--> RETRY_TIMEOUT\r\n");
		pContext->TimerProc(id);
		break;
	case SETUP_SAVE_TIMER:
		DBGMSG(1, "--> SETUP_SAVE_TIMER\r\n");
		g_timer.KillTimer(SETUP_SAVE_TIMER);
		g_setup_data.SaveSetupData();
		break;
	default:
	//	DBGMSG(1, "--> DEFAULT_TIMER\r\n");
		pContext->TimerProc(id);
		break;
	}
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
			printf("ID : %02d%02d%02d%02d\r\n", node->curr.car_num[0], 
					node->curr.car_num[1], node->curr.car_num[2], node->curr.car_num[3]);
		
			printf("floor : %s, zone : %s\r\n", node->curr.floor, node->curr.zone);
			printf("---------------------------------------------\r\n");			
			node = node->next;
		}
	}
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_find_node()
// Description    :
//---------------------------------------------------------------------------------
static struct parking_node* parking_list_find_node(char* _card_id)
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
static struct parking_node* parking_list_make_node(MTM_DATA_PARKING* _info)
{
	struct parking_node* pNew = NULL;
	
	pNew = (struct parking_node*)malloc(sizeof(struct parking_node));
	if (pNew != NULL) {
		memset(pNew, 0, sizeof(struct parking_node));
		memcpy(&pNew->curr, _info, sizeof(MTM_DATA_PARKING));
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
int parking_item_to_str(int _id, char* _src, char* _dst)
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
void parking_list_update(MTM_DATA_PARKING* _info)
{
	struct parking_info_list* plist = &g_parking_info_list;
	struct parking_node* pNew = NULL;
	
	printf("<%s>\r\n", __func__);

	pthread_mutex_lock(&g_parking_mutex);
	pNew = parking_list_find_node(_info->car_num);
	if (pNew != NULL) { 	// if node is exist then delete it
		printf("This Car is Aleady existed\r\n");
		parking_list_delete(pNew);
	}
	
	pNew = parking_list_make_node(_info);
	if (pNew == NULL) {
		printf("** ERROR OCCURED malloc failed **\r\n");
		return;
	}
	
	if (plist->cnt >= MAX_PARKING_LIST) {
		printf("discard tail !!\r\n");
		parking_list_delete(plist->tail);
	}

	// new node is must highest 
	parking_list_add_head(pNew);
	
	parking_list_dump();
	pthread_mutex_unlock(&g_parking_mutex);	
}

//---------------------------------------------------------------------------------
// Function Name  : parking_list_get_item()
// Description    : _idx is 0 ~ 3
//---------------------------------------------------------------------------------
int parking_list_get_item(int _idx, MTM_DATA_PARKING* _dst)
{
	struct parking_info_list* plist = &g_parking_info_list;
	struct parking_node* pNew = NULL;
	int i, found = 0;

	memset(_dst, 0, sizeof(MTM_DATA_PARKING));
	
	if (plist->head == NULL) {
		printf("<%s> not founded !!(%d)\r\n", __func__, _idx);
		_dst->status = 0xff;
		return -1;
	}
	
	pthread_mutex_lock(&g_parking_mutex);	
	pNew = plist->head;
	for (i = 0; i < _idx; i++) {
		if ((pNew->next == NULL) || (pNew->next == pNew)) {
			break;
		}
		pNew = pNew->next;
	}
	if (i == _idx)
		memcpy(_dst, &pNew->curr, sizeof(MTM_DATA_PARKING));
	pthread_mutex_unlock(&g_parking_mutex);	

	if (i != _idx) {
		printf("<%s> not founded !!(%d)\r\n", __func__, _idx);
		_dst->status = 0xff;
		return -1;
	}

	printf("<%s> found !!(%d)\r\n", __func__, _idx);
	return 0;
}


#ifndef __CONTEXT_SETUP_H__
#define __CONTEXT_SETUP_H__

//화면을 구성하는 Object들의 ID (m_ObjectList.AddObject 순서와 일치, CObject::m_id와 동일)
enum
{
	SETUP_OBJ_BG = 1,					//배경	0,0					/app/img/setup_bg.png
	SETUP_OBJ_COUNT
};

#define SCR_WIDTH						800
#define SCR_HEIGHT						480

#define TITLE_WIDTH						300
#define TITLE_HEIGHT					80

#define BUTTON_WIDTH					120
#define BUTTON_HEIGHT					80

#define BUTTON_VCENTER_1				85		//g_scr_info.rows/4*1 - hButton/2 + 5;
#define BUTTON_VCENTER_2				240		//g_scr_info.rows/4*2    
#define BUTTON_VCENTER_3				400		//g_scr_info.rows/4*3 + hButton/2;

#define GRAPH_SPEED						2

enum
{
	BUTTON_LT,
	BUTTON_LM,
	BUTTON_LB,
	BUTTON_RT,
	BUTTON_RM,
	BUTTON_RB,
	BUTTON_COUNT
};

enum
{
	CONTEXT_SETUP_INIT,
	CONTEXT_SETUP_LED,
	CONTEXT_SETUP_BACKLIGHT,
	CONTEXT_SETUP_RELAY,
	CONTEXT_SETUP_DOOR,
	CONTEXT_SETUP_SOUND,
	CONTEXT_SETUP_MICROWAVE,
	CONTEXT_SETUP_WALLPAD
};

enum
{
	CONTEXT_SETUP_STEP_SELECT,
	CONTEXT_SETUP_STEP_PAGE
};

enum
{
	CONTEXT_SETUP_WALLPAD_PAGE_LIGHT_OFF,		//일괄소등
	CONTEXT_SETUP_WALLPAD_PAGE_GAS_CUT,			//가스차단
	CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_ON,		//보안설정
	CONTEXT_SETUP_WALLPAD_PAGE_SECURITY_OFF,	//보안해제
	CONTEXT_SETUP_WALLPAD_PAGE_WEATHER_INFO,	//날씨요청
	CONTEXT_SETUP_WALLPAD_PAGE_PARKING_INFO,	//주차요청
	CONTEXT_SETUP_WALLPAD_PAGE_ELV_INFO,		//E/V요청
	CONTEXT_SETUP_WALLPAD_PAGE_ELV_CALL,		//E/V호출
	CONTEXT_SETUP_WALLPAD_PAGE_COUNT
};

class CContextSetup : public CContext
{
public:
	CContextSetup(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContextSetup();

	// Member Function
	virtual void Init();
	virtual void DeInit();

	virtual void Draw(UINT nContextNum=0);
	virtual void Proc(UINT nContextNum=0);

	virtual void TimerProc(UINT idTimer);
	virtual void RecvProc(UCHAR *pPacket);

	virtual void ThreadProc();

	virtual void ButtonDown(UINT usGpioFlag, UINT usEventEnum);
	virtual void ButtonUp(UINT usGpioFlag, UINT usEventEnum);
	virtual u8 check_update_cond(UINT _cntxt);
	virtual void update_new_info(void);

	void RecvProc_HDT(UCHAR *pPacket);
	void RecvProc_SHN(UCHAR *pPacket);
	void RecvProc_KCM(UCHAR *pPacket);
/*
	void RequestReserve(UCHAR cmd, UCHAR data=0);
	void RequestDirect(UCHAR cmd, UCHAR data=0);

	void PrintSendPacket(UCHAR cmd, UCHAR data=0);
*/
	void PrintPacket(UCHAR* pPacket, int size, UINT color, BOOL isRx=TRUE);

	// Member Variable
	RECT			m_rcTitle;
	RECT			m_rcArea;
	RECT			m_rcButton[BUTTON_COUNT];
	RECT			m_rcGraph;

	BOOL			m_fMicroWaveMonitorRunning;

	BOOL			m_isAdcValueLongKeyPlus;
	BOOL			m_isAdcValueLongKeyMinus;

	BOOL			m_isWaitForResponse;
	int				m_nWaitCountForResponse;

	BOOL			m_isRequestReserve;
	UCHAR			m_cmdRequestReserve;
	UCHAR			m_dataRequestReserve;

	UCHAR			m_nSetupStep;

	GR_WINDOW_ID	m_pixmapArea;
	GR_WINDOW_ID	m_pixmapGraph;
	int				m_nWallPadKeyPage;
};

#endif //__CONTEXT_SETUP_H__

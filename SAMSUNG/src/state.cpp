/*
*/
#include "common.h"
#include "main.h"

char g_StateName[STATE_COUNT][10] = 
{
	{ "IDLE\0" }, { "WEATHER\0" }, { "ELEVATOR\0" }, { "PARKING\0" }, 
	{ "SECURITY\0" }, { "ALLOFF\0" }, { "GAS\0" }, { "SETUP\0" }
};

//
// Global Function
//
/*
void ChangeState(UINT state, BOOL isEvent)
{
	printf("%s: %d --> %d\r\n", __func__, g_state.m_nCurrState, state);

//	if(isEvent)
//		post_event(GR_EVENT_TYPE_USER, EVT_STATE_CHANGE, state, 0);
//	else
		g_state.DoChangeState(state);
}

void ChangeContext(UINT nContextNum, BOOL isEvent)
{
	if(g_state.m_pCurrContext)
		printf("%s: %d --> %d\r\n", __func__, g_state.m_pCurrContext->m_nContextNum, nContextNum);
	else
		printf("%s: ? --> %d\r\n", __func__, nContextNum);

//	if(isEvent)
//		post_event(GR_EVENT_TYPE_USER, EVT_CONTEXT_CHANGE, nContextNum, 0);
//	else
		g_state.DoChangeContext(nContextNum);
}
*/

//
// Construction/Destruction
//
CState::CState()
{
	m_nCurrState = STATE_IDLE;
	m_nNextState = STATE_IDLE;

	m_pContextMap = NULL;
	m_pCurrContext = NULL;
}

CState::~CState()
{
}

//
// Member Function
//
BOOL CState::Init()
{
	int i;

	m_pContextMap = (CContext**)malloc(sizeof(CContext*)*STATE_COUNT);
	if(m_pContextMap==NULL)
	{
		printf("[Failure]\r\n--> %s: m_pContextMap allocation failure\r\n", __func__);
		return FALSE;
	}

	memset(m_pContextMap, 0, sizeof(CContext*)*STATE_COUNT);

	return TRUE;
}

void CState::DeInit()
{
	int i;

	if(m_pContextMap)
	{
		for(i=0; i<STATE_COUNT; i++)
		{
			if(m_pContextMap[i])
			{
				m_pContextMap[i]->DeInit();		//Static Loading시 한꺼번에 리소스 해제 해줘야 함
				delete m_pContextMap[i];
				m_pContextMap[i] = NULL;
			}
		}
		free(m_pContextMap);
	}

	m_pContextMap = NULL;
}

void CState::SetContext(UINT state, CContext* pContext)
{
	if(state > STATE_COUNT) return;
	if(pContext==NULL) return;

	if(m_pContextMap==NULL) return;

	m_pContextMap[state] = pContext;
	m_pContextMap[state]->SetState(state);
}

CContext* CState::GetContext(UINT state)
{
	if(state > STATE_COUNT) return NULL;

	return m_pContextMap[state];
}

void CState::SetState(UINT state)	
{ 
	m_nCurrState = state; 
}

UINT CState::GetState()				
{ 
	return m_nCurrState;  
}

char* CState::GetStateName(UINT state)
{
	if(state > STATE_COUNT) return NULL;

	return &(g_StateName[state][0]);
}

void CState::SetNextState(UINT state)
{
	m_nNextState = state;
}

UINT CState::GetNextState()
{
	return m_nNextState;
}

BOOL CState::isStateChange()
{
	return (m_nCurrState != m_nNextState) ? TRUE : FALSE;
}

void CState::SetCurrContext(UINT state)
{
	CContext* pContext = GetContext(state);

	if(pContext)
	{
		m_pCurrContext = pContext;
	}
}

CContext* CState::GetCurrContext()
{
	return m_pCurrContext;
}

void CState::ChangeState(UINT state)
{
	CContext* pContext = NULL;

	pContext = GetContext(state);
	if(pContext)
	{
	//	printf("%s: %d --> %d\r\n", __func__, m_nCurrState, state);
		printf("%s: %s --> %s\r\n", __func__, GetStateName(m_nCurrState), GetStateName(state));
		m_nCurrState = state;
		m_pCurrContext = pContext;
		m_pCurrContext->m_nContextNum = 0;
		m_pCurrContext->m_isContextDraw = TRUE;
	}
}

void CState::ChangeContext(UINT nContextNum)
{
	if(m_pCurrContext)
	{
		m_pCurrContext->ChangeContext(nContextNum);
	}
}

/*
void CState::DoChangeState(UINT state)
{
	int ret;
	void* thread_result;
	UINT tick;

	if(state > STATE_COUNT) return;

//	EnterCS();

	if(m_pCurrContext)
	{
		if(m_pCurrContext->m_fContextProcRunning)
		{
			m_pCurrContext->m_fContextProcRunning = FALSE;

			tick = GetTickCount();
			ret = pthread_join(m_pCurrContext->m_ProcThread, &thread_result);
			printf("%s: State(%d) Thread Terminated : %d tick\r\n", __func__, m_nCurrState, GetElapsedTick(tick));

			StopWavPlay();
		}
	//	m_pCurrContext->DeInit();
	//	SetState(STATE_IDLE);
	}

	SetCurrContext(state);	//상태별 Context Pointer를 맵에서 찾아 할당
//	SetState(state);		//CState의 상태변수를 설정

	if(m_pCurrContext)
	{
		SetState(state);
	//	m_pCurrContext->Init();
		m_pCurrContext->Draw();		//ContextNum=0
		m_pCurrContext->Proc();

		if(m_pCurrContext->m_pContextProc)
		{
			m_pCurrContext->m_fContextProcRunning = TRUE;
			ret = pthread_create(&m_pCurrContext->m_ProcThread, NULL, m_pCurrContext->m_pContextProc, m_pCurrContext);
			if(ret)
			{
				printf("%s: pthread_create failure: error=%d %s\r\n", __func__, errno, strerror(errno));
				m_pCurrContext->m_fContextProcRunning = FALSE;
			}
		}
	}

//	LeaveCS();
}

void CState::DoChangeContext(UINT nContextNum)
{
	if(m_pCurrContext)
	{
		m_pCurrContext->m_nContextNum = nContextNum;

		m_pCurrContext->Draw(nContextNum);
		m_pCurrContext->Proc(nContextNum);
	}
}
*/

void CState::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	CContext* pContext = GetCurrContext();
	UINT nNextState = GetState();

	if(pContext)
	{
		pContext->ButtonDown(usGpioFlag, usEventEnum);

		//롱키일 경우
		if(usEventEnum == MTM_GPIO_BUTTON_LONG)
		{
			//좌상단 롱키면 설정으로 전환되고 설정에서는 날씨화면으로 빠져나옴
			if(usGpioFlag == (GPIO_FLAG_FRONT_LEFT_TOP))
			{
				nNextState = (GetState() != STATE_SETUP) ? STATE_SETUP : STATE_WEATHER;
			}

			if(GetState() != nNextState)
			{
				ChangeState(nNextState);
			}
		}
	}
}

void CState::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	CContext* pContext = GetCurrContext();
	UINT nNextState = GetState();

	if(pContext)
	{
		pContext->ButtonUp(usGpioFlag, usEventEnum);

		if(GetState() != STATE_SETUP)
		{
			switch(usGpioFlag)
			{
			case GPIO_FLAG_FRONT_LEFT_TOP:
				nNextState = STATE_WEATHER;
				break;
			case GPIO_FLAG_FRONT_LEFT_MIDDLE:
				nNextState = STATE_ELEVATOR;
				break;
			case GPIO_FLAG_FRONT_LEFT_BOTTOM:
				nNextState = STATE_PARKING;
				break;
			case GPIO_FLAG_FRONT_RIGHT_TOP:
				nNextState = STATE_GAS;
				break;
			case GPIO_FLAG_FRONT_RIGHT_MIDDLE:
				nNextState = STATE_ALLOFF;
				break;
			case GPIO_FLAG_FRONT_RIGHT_BOTTOM:
				nNextState = STATE_SECURITY;
				break;
			}
			
			if(GetState() != nNextState)
			{
				ChangeState(nNextState);
			}
		}
	}
}

void CState::RecvProc(UCHAR *pPacket)
{
//	EnterCS();

	if(m_pCurrContext)
	{
		m_pCurrContext->RecvProc(pPacket);
	}

//	LeaveCS();
}

BOOL CState::EnterCS(BOOL isBlock)
{
	if(isBlock)
	{
		while(m_CriticalSection) 
		{ 
			usleep(10); 
		}
		m_CriticalSection = 1;
		return TRUE;
	}
	else
	{
		if(m_CriticalSection)
		{
			return FALSE;
		}
		else
		{
			m_CriticalSection = 1;
			return TRUE;
		}
	}
}

void CState::LeaveCS()
{
	m_CriticalSection = 0;
}

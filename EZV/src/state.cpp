/*
*/
#include "common.h"
#include "main.h"

char g_StateName[STATE_COUNT][10] = 
{
	{ "IDLE\0" }, { "WEATHER\0" }, { "ELEVATOR\0" }, { "PARKING\0" }, 
	{ "SECURITY\0" }, { "LIGHT\0" }, { "GAS\0" }, { "SETUP\0" }
};

//
// Global Function
//


//
// Construction/Destruction
//
CState::CState()
{
	m_nCurrState = STATE_IDLE;
	m_nNextState = STATE_IDLE;

	m_pContextMap = NULL;
	m_pCurrContext = NULL;

	m_CriticalSection = 0;
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
		DBGMSGC(DBG_STATE, "m_pContextMap allocation failure\r\n");
		return FALSE;
	}

	memset(m_pContextMap, 0, sizeof(CContext*)*STATE_COUNT);

	return TRUE;
}

void CState::DeInit()
{
	int i;

//	EnterCS();

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

//	LeaveCS();
}

void CState::SetContext(UINT state, CContext* pContext)
{
	if(state > STATE_COUNT) return;
	if(pContext==NULL)		return;
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
	CContext* pContext = NULL;

	pContext = GetContext(state);
	if(pContext)
	{
	//	EnterCS();
		m_pCurrContext = pContext;
	//	LeaveCS();
	}
}

CContext* CState::GetCurrContext()
{
	CContext* pContext = NULL;

//	EnterCS();
	pContext = m_pCurrContext;
//	LeaveCS();

	return pContext;
}

void CState::ChangeState(UINT state)
{
	CContext* pContext = NULL;

	g_pWallPad->delete_all_request_data();
	
	pContext = GetContext(state);
//	if(pContext)
//	{
		DBGMSG(DBG_STATE, "%s: %s --> %s\r\n", __func__, GetStateName(m_nCurrState), GetStateName(state));
		m_nCurrState = state;
	//	EnterCS();
		m_pCurrContext = pContext;
	//	LeaveCS();
		if (m_pCurrContext) {
			m_pCurrContext->m_nContextNum = 0;
			m_pCurrContext->m_isContextDraw = TRUE;
		}
//	}
}

void CState::ChangeContext(UINT nContextNum)
{
	if(m_pCurrContext)
	{
		m_pCurrContext->ChangeContext(nContextNum);
	}
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


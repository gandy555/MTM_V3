/*
*/
#include "common.h"
#include "main.h"

//
// Construction/Destruction
//
CContext::CContext(GR_WINDOW_ID wid, GR_GC_ID gc)
{
	m_nState = STATE_IDLE;
	m_nContextNum = 0;
	m_nThreadStep = 0;

	m_wid_parent = wid;
	m_gc_parent = gc;

	m_gc = 0;
//	m_pObject = NULL;

	m_isContextDraw = TRUE;

	m_fContextProcRunning = FALSE;
	m_pContextProc = NULL;
}

CContext::~CContext()
{
}

//
// Member Function
//
void CContext::SetParent(GR_WINDOW_ID wid, GR_GC_ID gc)
{
	m_wid_parent = wid;
	m_gc_parent = gc;
}

BOOL CContext::EnterCS(BOOL isBlock)
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

void CContext::LeaveCS()
{
	m_CriticalSection = 0;
}

void CContext::Init()
{
#if 1
	m_gc = m_gc_parent;		// Context는 메인의 GC속성을 따르고 만약 특화된 속성이 필요할 경우 개별 GC를 생성해서 사용한다.
#else
	m_gc = GrNewGC();
	if(m_gc)
	{
		GrSetGCUseBackground(m_gc, FALSE);
		GrSetGCFont(m_gc, g_font);

	//	GrSetGCBackground(m_gc, BLACK);
		GrSetGCForeground(m_gc, WHITE);
	}
	else
	{
		DBGMSG(DBG_CONTEXT, "%s: GrNewGC failure\r\n", __func__);
	}
#endif
}

void CContext::DeInit()
{
	m_fContextProcRunning = FALSE;

#if 0
	if(m_gc)
	{
		GrDestroyGC(m_gc);
	}
	m_gc = 0;
#endif
}

u8 CContext::check_update_cond(UINT _cntxt)
{
}

void CContext::update_new_info(void)
{

}

void CContext::Draw(UINT nContextNum)
{
}

void CContext::Proc(UINT nContextNum)
{
}

void CContext::TimerProc(UINT idTimer)
{
}

void CContext::RecvProc(UCHAR *pPacket)
{
}

void CContext::ThreadProc()
{
}

void CContext::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
}

void CContext::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
}

void CContext::ChangeContext(UINT nContextNum)
{
	DBGMSG(DBG_CONTEXT, "%s: %s: %d --> %d\r\n", __func__, g_state.GetStateName(m_nState), m_nContextNum, nContextNum);
	m_nContextNum = nContextNum;
	m_isContextDraw = TRUE;
}

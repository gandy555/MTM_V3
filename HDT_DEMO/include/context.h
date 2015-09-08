#ifndef __CONTEXT_H__
#define __CONTEXT_H__

typedef void* (*PFN_CONTEXT_PROC)(void *pParam);

class CContext
{
public:
	CContext(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContext();

	// Member Function
	void SetState(UINT state)	{ m_nState = state; }
	UINT GetState()				{ return m_nState;	}

	void SetParent(GR_WINDOW_ID wid, GR_GC_ID gc);

	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	virtual void Init();
	virtual void DeInit();

	virtual void Draw(UINT nContextNum=0);
	virtual void Proc(UINT nContextNum=0);

	virtual void TimerProc(UINT idTimer);
	virtual void RecvProc(UCHAR *pPacket);

	virtual void ThreadProc();

	virtual void ButtonDown(UINT usGpioFlag, UINT usEventEnumd);
	virtual void ButtonUp(UINT usGpioFlag, UINT usEventEnum);
//	virtual void ButtonHandler(UINT id);

	void ChangeContext(UINT nContextNum=0);

	// Member Variable
	UINT				m_nState;
	UINT				m_nContextNum;
	UINT				m_nThreadStep;

	GR_WINDOW_ID		m_wid_parent;
	GR_GC_ID			m_gc_parent;

	GR_GC_ID			m_gc;

//	CObject*			m_pObject;
	CObjectList			m_ObjectList;

	BOOL				m_isContextDraw;
	BOOL				m_fContextProcRunning;
	pthread_t			m_ProcThread;
	PFN_CONTEXT_PROC	m_pContextProc;

	UCHAR				m_CriticalSection;
};

#endif //__CONTEXT_H__

#ifndef __STATE_H__
#define __STATE_H__

enum
{
	STATE_IDLE,				//IDLE
	STATE_WEATHER,			//대기화면(날씨)		전면 좌상단
	STATE_ELEVATOR,			//엘리베이터			전면 좌중간
	STATE_PARKING,			//주차					전면 좌하단
	STATE_SECURITY,			//보안(방범)			전면 우하단
	STATE_LIGHT,			//소등					전면 우중간
	STATE_GAS,				//가스					전면 우상단
	STATE_SETUP,			//설정
	STATE_COUNT
};

//void ChangeState(UINT state, BOOL isEvent);
//void ChangeContext(UINT nContextNum, BOOL isEvent);

//class CContext;
class CState
{
public:
	CState();
	~CState();

	// Member Function
	BOOL Init();
	void DeInit();

	void SetContext(UINT state, CContext* pContext);
	CContext* GetContext(UINT state);

	void SetState(UINT state);
	UINT GetState();

	char* GetStateName(UINT state);

	void SetNextState(UINT state);
	UINT GetNextState();

	BOOL isStateChange();

	void SetCurrContext(UINT state);
	CContext* GetCurrContext();

	void ChangeState(UINT state);
	void ChangeContext(UINT nContextNum=0); 

	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	// Member Variable
	UINT		m_nCurrState;
	UINT		m_nNextState;

	CContext**	m_pContextMap;
	CContext*	m_pCurrContext;

	UCHAR		m_CriticalSection;
};

#endif //__STATE_H__

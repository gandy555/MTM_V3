#ifndef __STATE_H__
#define __STATE_H__

enum
{
	STATE_IDLE,				//IDLE
	STATE_WEATHER,			//���ȭ��(����)		���� �»��
	STATE_ELEVATOR,			//����������			���� ���߰�
	STATE_PARKING,			//����					���� ���ϴ�
	STATE_SECURITY,			//����(���)			���� ���ϴ�
	STATE_LIGHT,			//�ҵ�					���� ���߰�
	STATE_GAS,				//����					���� ����
	STATE_SETUP,			//����
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

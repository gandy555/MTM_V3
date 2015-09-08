#ifndef __CONTEXT_ELEVATOR_H__
#define __CONTEXT_ELEVATOR_H__

//ȭ���� �����ϴ� Object���� ID (m_ObjectList.AddObject ������ ��ġ, CObject::m_id�� ����)
enum
{
	ELEVATOR_OBJ_BG = 1,				//Blank ���	0,0			/app/img/blank_bg.png
	ELEVATOR_OBJ_ICON,					//�»�� ������	58,50		/app/img/icon_elevator.png
	ELEVATOR_OBJ_IMAGE,					//���߰� �̹���	510,175		/app/img/elevator_image.png
	ELEVATOR_OBJ_DISPLAY,				//��ǥ��		339,240		/app/img/elevator_display.png
	ELEVATOR_OBJ_ARROW,					//ȭ��ǥ		231,208
	ELEVATOR_OBJ_COUNT
};

enum
{
	ELEVATOR_STOP,
	ELEVATOR_DOWN,
	ELEVATOR_UP
};

#define ELEVATOR_CALL_WAIT_TIME	300

class CContextElevator : public CContext
{
public:
	CContextElevator(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContextElevator();

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

	// Member Variable
	BOOL	m_isElevatorStatusSuccess;
	BOOL	m_isElevatorCalled;
	BOOL	m_isElevatorCallSuccess;
	BOOL	m_isElevatorArrive;
	int		m_nElevatorDir;
	int		m_nElevatorFloor;

	CObjectIcon* m_pObjectIcon;
};

#endif //__CONTEXT_ELEVATOR_H__

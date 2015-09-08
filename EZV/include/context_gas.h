#ifndef __CONTEXT_GAS_H__
#define __CONTEXT_GAS_H__

//ȭ���� �����ϴ� Object���� ID (m_ObjectList.AddObject ������ ��ġ, CObject::m_id�� ����)
enum
{
	GAS_OBJ_BG = 1,						//Blank ���	0,0					/app/img/blank_bg.png
	GAS_OBJ_ICON,						//�»�� ������	58,50	158x158		/app/img/icon_gas_on.png, icon_gas_off.png
	GAS_OBJ_SWITCH,						//�ϴ� ����ġ	312,287 299x116		/app/img/icon_switch_on.png, icon_switch_off.png
	GAS_OBJ_COUNT
};


class CContextGas : public CContext
{
public:
	CContextGas(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContextGas();

	// Member Function
	virtual void Init();
	virtual void DeInit();

	virtual void Draw(UINT nContextNum=0);
	virtual void Proc(UINT nContextNum=0);

	virtual void TimerProc(UINT idTimer);
	virtual void RecvProc(UCHAR *pPacket);

	virtual void ButtonDown(UINT usGpioFlag, UINT usEventEnum);
	virtual void ButtonUp(UINT usGpioFlag, UINT usEventEnum);

	// Member Variable
	BOOL	m_isWallPadStatus;
	BOOL	m_isGasOff;
};

#endif //__CONTEXT_GAS_H__

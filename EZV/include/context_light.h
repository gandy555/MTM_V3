#ifndef __CONTEXT_LIGHT_H__
#define __CONTEXT_LIGHT_H__

//ȭ���� �����ϴ� Object���� ID (m_ObjectList.AddObject ������ ��ġ, CObject::m_id�� ����)
enum
{
	LIGHT_OBJ_BG = 1,					//Blank ���	0,0					/app/img/blank_bg.png
	LIGHT_OBJ_ICON,						//�»�� ������	58,50	158x158		/app/img/icon_light_on.png, icon_light_off.png
	LIGHT_OBJ_SWITCH,					//�ϴ� ����ġ	312,287 299x116		/app/img/icon_switch_on.png, icon_switch_off.png
	LIGHT_OBJ_COUNT
};


class CContextLight : public CContext
{
public:
	CContextLight(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContextLight();

	// Member Function
	virtual void Init();
	virtual void DeInit();

	virtual void Draw(UINT nContextNum=0);
	virtual void Proc(UINT nContextNum=0);

	virtual void TimerProc(UINT idTimer);
	virtual void RecvProc(UCHAR *pPacket);

	virtual void ButtonDown(UINT usGpioFlag, UINT usEventEnum);
	virtual void ButtonUp(UINT usGpioFlag, UINT usEventEnum);
	virtual u8 check_update_cond(UINT _cntxt);
	virtual void update_new_info(void);

	// Member Variable
	BOOL	m_isWallPadStatus;
};

#endif //__CONTEXT_LIGHT_H__

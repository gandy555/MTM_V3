#ifndef __CONTEXT_LIGHT_H__
#define __CONTEXT_LIGHT_H__

//화면을 구성하는 Object들의 ID (m_ObjectList.AddObject 순서와 일치, CObject::m_id와 동일)
enum
{
	LIGHT_OBJ_BG = 1,					//Blank 배경	0,0					/app/img/blank_bg.png
	LIGHT_OBJ_ICON,						//좌상단 아이콘	58,50	158x158		/app/img/icon_light_on.png, icon_light_off.png
	LIGHT_OBJ_SWITCH,					//하단 스위치	312,287 299x116		/app/img/icon_switch_on.png, icon_switch_off.png
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

	// Member Variable
	UCHAR m_requested_val;	/* gandy 2015-03-30 */
};

#endif //__CONTEXT_LIGHT_H__

#ifndef __CONTEXT_WEATHER_H__
#define __CONTEXT_WEATHER_H__

//화면을 구성하는 Object들의 ID (m_ObjectList.AddObject 순서와 일치, CObject::m_id와 동일)
enum
{
	WEATHER_OBJ_BG = 1,						//배경					0,0
	WEATHER_OBJ_WEATHER_LEFT,				//왼쪽 날씨 이미지		1,169
	WEATHER_OBJ_WEATHER_RIGHT,				//오른쪽 날씨 이미지	499,169
	WEATHER_OBJ_GAS,						//가스 On/Off			336,245
	WEATHER_OBJ_PARKING_BAR,
	WEATHER_OBJ_COUNT
};

class CContextWeather : public CContext
{
public:
	CContextWeather(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContextWeather();

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
	BOOL	m_isGasOff;

	GR_WINDOW_ID	m_wid_ment;
	GR_WINDOW_ID	m_pixmap_ment;

	int		m_len_ment;
	int		m_width_ment;
};

#endif //__CONTEXT_WEATHER_H__

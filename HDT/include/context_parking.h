#ifndef __CONTEXT_PARKING_H__
#define __CONTEXT_PARKING_H__

//ȭ���� �����ϴ� Object���� ID (m_ObjectList.AddObject ������ ��ġ, CObject::m_id�� ����)
enum
{
	PARKING_OBJ_BG = 1,					//Blank ���	0,0			/app/img/blank_bg.png
	PARKING_OBJ_ICON,					//�»�� ������	58,50		/app/img/icon_parking.png
	PARKING_OBJ_IMAGE,					//�ϴ� �̹���	510,175		/app/img/parking_image1.png
	PARKING_OBJ_COUNT
};

//#define MAX_PARKING_HISTORY				4

//extern MTM_DATA_PARKING g_ParkingHistory[MAX_PARKING_HISTORY];

class CContextParking : public CContext
{
public:
	CContextParking(GR_WINDOW_ID wid, GR_GC_ID gc);
	~CContextParking();

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
	BOOL		m_isSearching;

	BOOL		m_isParkingInfoRequest;
//	BOOL		m_isParkingInfo;
	ULONG		m_ulTickWaitForData;
};

#endif //__CONTEXT_PARKING_H__

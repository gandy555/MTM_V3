#ifndef __WALLPAD_KCM_H__
#define __WALLPAD_KCM_H__

class CWallPadKcm : public CWallPad
{
public:
	CWallPadKcm();
	~CWallPadKcm();

	//
	// Interface Function
	//
	virtual BOOL Run();
	virtual void Stop();

	virtual void RequestWeatherInfo();		//날씨정보요청

	virtual void RequestGasStatus();		//가스상태요청
	virtual void RequestGasCut();			//가스차단요청

	virtual void RequestLightSet(BOOL isAllOff);		//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)

	virtual void RequestSecuritySet(BOOL isSecurityOn);	//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)

	virtual void RequestElevatorCall();		//엘레베이터 호출
	virtual void RequestElevatorStatus();	//엘레베이터 상태요청

	virtual void RequestParkingInfo();		//주차위치정보요청

	//
	// Member Function
	//
	int Write(UCHAR *pData, int size);
//	UCHAR CalcCheckSum(UCHAR* pBuffer, UINT size);

	//Thread Context
	static void* SerialListener_KCM(void *pParam);
	void RecvDataProc();



	//
	// Member Variable
	//
	CSerial		m_serial;

	BOOL		m_isRecv;					//수신중 여부 Flag

	KCM_PACKET	m_packet;
	UCHAR		m_nRetry;
};

#endif //__WALLPAD_KCM_H__

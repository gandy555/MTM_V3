#ifndef __WALLPAD_HDT_H__
#define __WALLPAD_HDT_H__

class CWallPadHdt : public CWallPad
{
public:
	CWallPadHdt();
	~CWallPadHdt();

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
	UCHAR CalcCheckSum(UCHAR* pBuffer, UINT size);

	//Thread Context
	static void* SerialListener_HDT(void *pParam);
	void RecvDataProc();

	void WeatherInfoProc(UCHAR *pBuffer);
	void ParkingInfoProc(UCHAR *pBuffer);

	void UssPollingProc(UCHAR *pBuffer);
//	void UssControlProc(UCHAR *pBuffer);

	void ElevatorPollingProc(UCHAR *pBuffer);
//	void ElevatorControlProc(UCHAR *pBuffer);

	//
	// Member Variable
	//
	CSerial		m_serial;

	BOOL		m_isRecv;					//수신중 여부 Flag

	UCHAR		m_GasControlReserve;		//가스제어 예약명령
	UCHAR		m_LightControlReserve;		//일괄소등 예약명령
	UCHAR		m_SecurityControlReserve;	//방범제어 예약명령
	UCHAR		m_ElevatorControlReserve;	//엘리베이터오출 예약명령

	BOOL		m_isElevatorCall;
	BOOL		m_isElevatorArrive;
};

#endif //__WALLPAD_HDT_H__

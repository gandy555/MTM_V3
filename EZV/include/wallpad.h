#ifndef __WALLPAD_H__
#define __WALLPAD_H__

#define MAX_RECV_BUFFER					64
//#define MAX_QUE_COUNT					10

#define SELECT_TIMEOUT_SEC				0
#define SELECT_TIMEOUT_USEC				500000		//500ms

//#define RECEIVE_WAIT_TIMEOUT			5			//sec
//#define MAX_SELECT_TIMEOUT_COUNT		10			//500ms x 10 = 5sec

//#define MAX_WAIT_COUNT_FOR_RESPONSE		3

//#define PROTOCOL_SIMULATE				0			//0=Disable, 1=Enable

class CWallPad
{
public:
	CWallPad()	{ };
	~CWallPad() { };

	//
	// Interface Function
	//
	virtual BOOL Run() = 0;
	virtual void Stop() = 0;

	virtual void RequestWeatherInfo() = 0;		//날씨정보요청

	virtual void RequestGasCut() = 0;			//가스차단요청
	virtual void RequestGasStatus() = 0;		//가스상태요청

	virtual void RequestLightStatus() = 0;
	virtual void RequestLightSet(BOOL isAllOff) = 0;		//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)

	virtual void RequestSecurityStatus() = 0;
	virtual void RequestSecuritySet(BOOL isSecurityOn) = 0;	//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)

	virtual void RequestElevatorCall() = 0;		//엘레베이터 호출
	virtual void RequestElevatorStatus() = 0;	//엘레베이터 상태요청

	virtual void RequestParkingInfo() = 0;		//주차위치정보요청
	virtual void delete_all_request_data(void) = 0;
};

#endif //__WALLPAD_H__

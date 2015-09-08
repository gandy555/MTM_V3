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

	virtual void RequestWeatherInfo() = 0;		//����������û

	virtual void RequestGasCut() = 0;			//�������ܿ�û
	virtual void RequestGasStatus() = 0;		//�������¿�û

	virtual void RequestLightStatus() = 0;
	virtual void RequestLightSet(BOOL isAllOff) = 0;		//�ϰ��ҵ��û(isAllOff: TRUE=�ҵ�, FALSE=�ҵ�����)

	virtual void RequestSecurityStatus() = 0;
	virtual void RequestSecuritySet(BOOL isSecurityOn) = 0;	//���������û(isSecurityOn: TRUE=�������, FALSE=�������)

	virtual void RequestElevatorCall() = 0;		//���������� ȣ��
	virtual void RequestElevatorStatus() = 0;	//���������� ���¿�û

	virtual void RequestParkingInfo() = 0;		//������ġ������û
	virtual void delete_all_request_data(void) = 0;
};

#endif //__WALLPAD_H__

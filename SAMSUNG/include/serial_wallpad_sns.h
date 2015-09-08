#ifndef __SERIAL_WALLPAD_SNS_H__
#define __SERIAL_WALLPAD_SNS_H__


#define MAX_RECV_BUFFER					64
#define MAX_QUE_COUNT					10

#define SELECT_TIMEOUT_SEC				0
#define SELECT_TIMEOUT_USEC				500000		//500ms

#define RECEIVE_WAIT_TIMEOUT			5			//sec
#define MAX_SELECT_TIMEOUT_COUNT		10			//500ms x 10 = 5sec

#define MAX_WAIT_COUNT_FOR_RESPONSE		3

#define PROTOCOL_SIMULATE				0			//0=Disable, 1=Enable

#if(PROTOCOL_SIMULATE == 1)
enum
{
	PROTO_SIM_WEATHER_1,				//열대성폭풍(4)-->뇌우, 이슬비(9)-->비
	PROTO_SIM_WEATHER_2,				//소나기(11)-->소나기, 눈(6)-->비, 
	PROTO_SIM_WEATHER_3,				//흐림(22)-->흐림, 곳에따라 구름(낮)(30)-->구름조금
	PROTO_SIM_WEATHER_4,				//화창(32)-->맑음, 번개를 동반한 소나기(47)-->뇌우
	PROTO_SIM_COUNT
};
#endif

class CSerialWallPad_SNS : public CSerial
{
public:
	CSerialWallPad_SNS();
	~CSerialWallPad_SNS();

	//Member Function
	BOOL Open(const char *pszDev, UINT baudRate=B9600);

	BOOL Run();
	void Stop();

	int Write(const UCHAR *pszBuffer, int size);

	int SendRequest(UCHAR cmd, UCHAR data=0);
	int SendAck(UCHAR cmd);
	int SendFreeData();

	UCHAR CalcCheckSum(UCHAR* pBuffer, UINT size);

	int  GetPacketCountInQue();
	BOOL PushPacketIntoQue(UCHAR* pPacket);
	BOOL PopPacketFromQue(UCHAR* pPacket);

	UINT ConvWeatherIcon(UINT nWallPadIcon);

	void CallContextReceiveProc(void *pParam);

	void RequestReserve(UCHAR cmd, UCHAR data=0);

	void ResponseProc_WeatherRes(UCHAR* pBuffer);
	void ResponseProc_ParkingRes(UCHAR* pBuffer);
	void ResponseProc_ElevatorRes(UCHAR* pBuffer);
	void ResponseProc_ElevatorCall(UCHAR* pBuffer);
//	void ResponseProc_ElevatorArrive(UCHAR* pBuffer);
	void ResponseProc_WallPadReq(UCHAR* pBuffer);
	void ResponseProc_AllOffReq(UCHAR* pBuffer);
	void ResponseProc_GasOffReq(UCHAR* pBuffer);
	void ResponseProc_SecurityReq(UCHAR* pBuffer);
	void ResponseProc_SecurityRes(UCHAR* pBuffer);
	void ResponseProc_GasStatRes(UCHAR* pBuffer);

	static void* SerialListener_SNS(void *pParam);

	//Member Variable
	UCHAR		m_BufferQue[MAX_QUE_COUNT][MAX_RECV_BUFFER];
	UCHAR		m_idxQuePush;
	UCHAR		m_idxQuePop;
	BOOL		m_isQueFull;

	BOOL		m_isReceiveWaitTimeout;

	BOOL		m_isRecv;
	BOOL		m_isWaitForResponse;
	int			m_nWaitCountForResponse;

	BOOL		m_isRequestReserve;
	UCHAR		m_cmdRequestReserve;
	UCHAR		m_dataRequestReserve;

	UINT		m_tickRequest;

#if(PROTOCOL_SIMULATE == 1)
	BOOL		m_isProtocolSimulate;
	int			m_nProtocolSimulateNum;
#endif

	//Status Variable

	WEATHER_RES			m_WeathreRes;
	PARKING_RES			m_ParkingRes;
	ELEVATOR_STATUS		m_ElevatorStatus;
	ELEVATOR_CALL_RES	m_ElevatorCallRes;
	WALLPAD_STATUS		m_WallPadStatus;

	BOOL		m_isWeatherInfo;
	BOOL		m_isParkingInfo;
	BOOL		m_isElevatorInfo;
	BOOL		m_isWallPadInfo;

	BOOL		m_isWeatherInfoCancel;
	BOOL		m_isParkingInfoCancel;
	BOOL		m_isElevatorInfoCancel;
	BOOL		m_isWallPadInfoCancel;

	BOOL		m_isContextSetupWallPad;
};

#endif //__SERIAL_WALLPAD_SNS_H__

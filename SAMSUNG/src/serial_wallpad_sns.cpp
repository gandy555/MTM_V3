#include "common.h"
//#include "serial_wallpad_sns.h"
#include "main.h"

#define PACKET_DUMP_ENABLE
#define DEBUG_RESPONSE

#if(PROTOCOL_SIMULATE == 1)

#define	MAX_PROTO_SIZE			32

UCHAR g_ProtoSimTable[PROTO_SIM_COUNT][MAX_PROTO_SIZE] = 
{
//	PROTO_SIM_WEATHER_1,				//열대성폭풍(4)-->뇌우, 이슬비(9)-->비
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x04, 0x0b, 0x05, 
		0x09, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x72	},
//	PROTO_SIM_WEATHER_2,				//소나기(11)-->소나기, 눈(6)-->눈, 
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x0b, 0x0b, 0x05, 
		0x06, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x72	},
//	PROTO_SIM_WEATHER_3,				//흐림(22)-->흐림, 곳에따라 구름(낮)(30)-->구름조금
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x16, 0x0b, 0x05, 
		0x1E, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x77	},
//	PROTO_SIM_WEATHER_4,				//화창(32)-->맑음, 번개를 동반한 소나기(47)-->뇌우
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x20, 0x0b, 0x05, 
		0x2F, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x70	}
};
#endif

//
// Construction/Destruction
//

CSerialWallPad_SNS::CSerialWallPad_SNS()
{
	int i;

	m_isReceiveWaitTimeout = FALSE;

	m_isRecv = FALSE;
	m_isWaitForResponse = FALSE;

	m_isRequestReserve = FALSE;

	m_isWeatherInfo = FALSE;
	m_isParkingInfo = FALSE;
	m_isElevatorInfo = FALSE;
	m_isWallPadInfo = FALSE;

	m_isWeatherInfoCancel = FALSE;
	m_isParkingInfoCancel = FALSE;
	m_isElevatorInfoCancel = FALSE;
	m_isWallPadInfoCancel = FALSE;

	m_isContextSetupWallPad = FALSE;

	for(i=0; i<MAX_QUE_COUNT; i++)
	{
		memset(&m_BufferQue[i][0], 0, MAX_RECV_BUFFER);
	}
	m_idxQuePush = 0;
	m_idxQuePop = 0;
	m_isQueFull = FALSE;

#if(PROTOCOL_SIMULATE == 1)
	m_isProtocolSimulate = FALSE;
	m_nProtocolSimulateNum = -1;
#endif
}

CSerialWallPad_SNS::~CSerialWallPad_SNS()
{

}

//
// Member Function
//
BOOL CSerialWallPad_SNS::Open(const char *pszDev, unsigned int baudRate)
{
	BOOL fRet = FALSE;
	struct termios tio;

	fRet = CSerial::Open(pszDev, baudRate);
	if(fRet == FALSE) 
	{
		printf("[Failure]\r\n--> %s: open failure\r\n", __func__);
		return FALSE;
	}

//< for SNS
    memset(&tio, 0, sizeof(termios));
	tio.c_cflag = CS8 | CLOCAL | CREAD | PARENB;	// 8 data bits/ even parity / NO-rts / cts

	tio.c_cflag |= baudRate;

    tio.c_lflag = 0;

    tio.c_cc[VTIME] = 1;				// timeout 0.1초 단위
    tio.c_cc[VMIN]  = 1;				// 최소 n 문자 받을 때까진 대기

    tcflush  ( m_fdDev, TCIFLUSH );
    tcsetattr( m_fdDev, TCSANOW, &tio );
//>

	ioctl( m_fdDev, 0x54F0, 0x0001 );
	tcflush ( m_fdDev, TCIFLUSH );

	if(tcgetattr(m_fdDev, &tio)==ERROR)
	{
		printf("[Failure]\r\n--> %s: tcgetattr failure\r\n", __func__);
		return FALSE;
	}

//	printf("%s: '%s' device open success\r\n", __func__, pszDev);

	return TRUE;
}

BOOL CSerialWallPad_SNS::Run()
{
	if(StartListener(SerialListener_SNS)==FALSE)
	{
		printf("[Failure]\r\n--> %s: StartListener Failure\r\n", __func__);
		return FALSE;
	}
	return TRUE;
}

void CSerialWallPad_SNS::Stop()
{
	StopListener();
}

int CSerialWallPad_SNS::Write(const UCHAR *pszBuffer, int size)
{
	int ret;

	if(m_fdDev < 0) return ERROR;

	while(m_isRecv) { usleep(1000); }

	// 485 Enable
	g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, LOW);

	// TX Enable후 최초 패킷을 전송하기까지 1.5ms 지연
	usleep(1500);

	ret = write(m_fdDev, pszBuffer, size);

	usleep(size * 834);

	//마지막 패킷 전송후 TX Disable 하기까지 500us 지연
	usleep(500);

	// 485 Disable
	g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, HIGH);

	if(ret == ERROR)
	{
		printf("%s: write failure : errno=%d %s\r\n", __func__, errno, strerror(errno));
	}

	return ret;
}

int CSerialWallPad_SNS::SendRequest(UCHAR cmd, UCHAR data)
{
	UCHAR req_pkt[5];
	int ret = ERROR;

	req_pkt[0] = SNS_HDR_DEV;
	req_pkt[1] = cmd;
	req_pkt[2] = 1;
	req_pkt[3] = data;
	req_pkt[4] = CalcCheckSum(req_pkt, 4);

	ret = Write(req_pkt, 5);
	if(ret != ERROR)
	{
		m_tickRequest = GetTickCount();
#ifdef PACKET_DUMP_ENABLE
//		if(m_isContextSetupWallPad==FALSE)
//		{
		//	printf("%s: ", __func__);	
			printf("Tx: ");	
			DumpData(req_pkt, ret, TRUE);
//		}
#endif
	}

	return ret;
}

int CSerialWallPad_SNS::SendAck(UCHAR cmd)
{
	return SendRequest(cmd, 0);
}

int CSerialWallPad_SNS::SendFreeData()
{
	return SendRequest(SNS_CMD_FREE_DATA, 0);
}

UCHAR CSerialWallPad_SNS::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	UCHAR cs = 0;
	UINT i;

	if(pBuffer==NULL) return 0;

	for(i=0; i<size; i++)
	{
		cs ^= pBuffer[i];
	}

	return (cs & 0x7F);
}

int CSerialWallPad_SNS::GetPacketCountInQue()
{
	int count = 0;

	if(m_isQueFull)
		count = MAX_QUE_COUNT;
	else if(m_idxQuePush != m_idxQuePop)
		count = (m_idxQuePush > m_idxQuePop) ? (m_idxQuePush-m_idxQuePop) : (MAX_QUE_COUNT+m_idxQuePush-m_idxQuePop);

	return count;
}

//1. 패킷은 호출전 검사된 패킷으로 가정
//2. InitData(0x5A), FreeData(0x41), 값이 없는 ACK들은 Que에 넣지 않음
//   --> 어플에서 응답으로 사용될만한 것들만 넣게됨
//3. Que가 Full이면 가장먼저 Push된 패킷자리에 OverWrite됨
BOOL CSerialWallPad_SNS::PushPacketIntoQue(UCHAR* pPacket)
{
	UCHAR len;

	if(pPacket==NULL) return ERROR;

	len = pPacket[2];

	//패킷의 길이가 버퍼크기를 넘어서는 경우 --> 잘못된 패킷이므로 무시
	if( (len==0) || ((len+4) > MAX_RECV_BUFFER) ) return ERROR;

	//m_isQueFull: Pop이 없이 Push만 MAX_QUE_COUNT만큼 이루어지면 더이상 Push할 공감이 없음
	//			   --> Empty상태와 Full상태는 Push Index와 Pop Index가 같아짐
	if(m_isQueFull)
	{
		//Pop Index를 Plus Shift함으로써 Que에서 가장먼저 Push된 데이타를 버리도록 처리함
		m_idxQuePop++;
		if(m_idxQuePop == MAX_QUE_COUNT) m_idxQuePop=0;

	//	m_isQueFull = FALSE;	//어차피 다시 채워질 상황이므로 의미 없음
	}

	memset(&m_BufferQue[m_idxQuePush][0], 0, MAX_RECV_BUFFER);
	memcpy(&m_BufferQue[m_idxQuePush][0], pPacket, len+4);

	//Push Index Plus Shift
	m_idxQuePush++;
	if(m_idxQuePush == MAX_QUE_COUNT) m_idxQuePush=0;

	if(m_idxQuePush == m_idxQuePop) m_isQueFull=TRUE;

	return TRUE;
}

BOOL CSerialWallPad_SNS::PopPacketFromQue(UCHAR* pPacket)
{
	if(pPacket==NULL) return ERROR;

	//Full이 아닌데 Pop Index가 Push Index와 같다면 Que가 비어 있는 상태
	//--> 따라서 Full이거나 Pop Index와 Push Index가 다르면 Que에 패킷이 있는 상태
	if( (m_isQueFull) || (m_idxQuePop!=m_idxQuePush) )
	{
		//포인터만 전달 할 경우 Full일 경우 동기가 맞지 않아 
		//Que의 처음 패킷이 아니라 마지막 패킷을 처리할 수 있음
		//따라서, Que의 패킷을 복사해주는 방식으로 전환
		memcpy(pPacket, &m_BufferQue[m_idxQuePop][0], MAX_RECV_BUFFER);

		//Pop Index Plus Shift
		m_idxQuePop++;
		if(m_idxQuePop == MAX_QUE_COUNT) m_idxQuePop=0;

		m_isQueFull=FALSE;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UINT CSerialWallPad_SNS::ConvWeatherIcon(UINT nWallPadIcon)
{
	UINT nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;

#if 0	//이전기준
	//WallPad의 날씨 Icon은 1~40
	switch(nWallPadIcon)
	{
	case 1:		//맑음(대체로맑음)
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 2:		//구름조금
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 3:		//구름많음
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 4:		//흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 5:		//흐린후 차차갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 6:		//맑음후 차차 흐려짐
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 7:		//소나기
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 8:		//새벽/아침/오전 소나기
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 9:		//낮/오후/밤 소나기
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 10:	//흐리고 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 11:	//새벽/아침/오전 한때 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 12:	//낮/오후/밤 한때 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 13:	//차차 흐려져 한때 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 14:	//차차 흐려져 낮/오후/밤 한때 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 15:	//비온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 16:	//새벽/아침/오전 한때 비온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 17:	//낮/오후/밤 한때 비온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 18:	//흐리고 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 19:	//새벽/아침/오전 한때 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 20:	//낮/오후/밤 한때 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 21:	//차차 흐려져 한때 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 22:	//차차 흐려져 낮/오후/밤 한때 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 23:	//눈온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 24:	//새벽/아침/오전 한때 눈온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 25:	//낮/오후/밤 한때 눈온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 26:	//비 또는 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 27:	//새벽/아침/오전 한때 비 또는 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 28:	//낮/오후/밤 한때 비 또는 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 29:	//차차 흐려져 한때 비 또는 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 30:	//차차 흐려져 낮/오후/밤 한때 비 또는 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 31:	//눈 또는 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 32:	//새벽/아침/오전 한때 눈 또는 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 33:	//낮/오후/밤 한때 눈 또는 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 34:	//차차 흐려져 한때 눈 또는 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 35:	//차차 흐려져 낮/오후/밤 한때 눈 또는 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 36:	//비 또는 눈 온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 37:	//새벽/아침/오전 한때 비 또는 눈 온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 38:	//낮/오후/밤 한때 비 또는 눈 온후 갬
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 39:	//천둥번개
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 40:	//안개
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	default:
		printf("%s: Unknown WallPad Weather Icon(%d)\r\n", __func__, nWallPadIcon);
		break;
	}
#else	//새기준 (20140826)
	//WallPad의 날씨 Icon은 1~47, 생활정보기의 날씨 Icon은 0~6
	switch(nWallPadIcon)
	{
	case 1:		//열대성폭풍			--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 2:		//허리케인				--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 3:		//심한뇌우				--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 4:		//벼락을 동반한 비		--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 5:		//비 또는 진눈깨비		--> 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 6:		//비 또는 진눈깨비		--> 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 7:		//진눈깨비				--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 8:		//진눈깨비				--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 9:		//이슬비				--> 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 10:	//눈을 동반한 비		--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 11:	//소나기				--> 소나기
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 12:	//소나기				--> 소나기
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 13:	//집중폭설				--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 14:	//한시적 눈내림			--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 15:	//싸리눈				--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 16:	//눈					--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 17:	//싸라기눈				--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 18:	//진눈깨비				--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 19:	//황사주의				--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 20:	//짙은 안개				--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 21:	//엷은 안개				--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 22:	//흐림					--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 23:	//강한 바람				--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 24:	//센 바람				--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 25:	//한파					--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 26:	//흐림					--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 27:	//전반적으로 흐림(밤)	--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 28:	//전반적으로 흐림(낮)	--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 29:	//곳에 따라 구름(밤)	--> 구름 조금
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 30:	//곳에 따라 구름(낮)	--> 구름 조금
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 31:	//맑음(밤)				--> 맑음
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 32:	//화창					--> 맑음
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 33:	//맑음(밤)				--> 맑음
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 34:	//맑음(낮)				--> 맑음
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 35:	//우박을 동반한 비		--> 비
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 36:	//폭염					--> 맑음
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 37:	//지역에 따라 벼락		--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 38:	//때때로 벼락			--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 39:	//때때로 벼락			--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 40:	//때때로 소나기			--> 소나기
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 41:	//폭설					--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 42:	//때때로 눈 또는 비		--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 43:	//폭설					--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 44:	//곳에 따라 흐림		--> 흐림
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 45:	//번개를 동반한 소나기	--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 46:	//폭설					--> 눈
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 47:	//지역에 따라 번개를 동반한 소나기	--> 뇌우
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	default:
		printf("%s: Unknown WallPad Weather Icon(%d)\r\n", __func__, nWallPadIcon);
		break;
	}
#endif

	return nMtmWeatherIcon;
}

void CSerialWallPad_SNS::CallContextReceiveProc(void *pParam)
{
#if 1	//이전방식

#if 0
	CContext* pContext = NULL;

	pContext = g_state.GetCurrContext();
	if(pContext && pParam)
	{
		pContext->RecvProc(pParam);
	}
#else
	g_state.RecvProc((UCHAR *)pParam);
#endif

#else	//새방식 - Que사용

	PushPacketIntoQue((UCHAR *)pParam);

#endif
}

void CSerialWallPad_SNS::RequestReserve(UCHAR cmd, UCHAR data)
{
	m_isRequestReserve = TRUE;
	m_cmdRequestReserve = cmd;
	m_dataRequestReserve = data;
}

void CSerialWallPad_SNS::ResponseProc_WeatherRes(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Time			6		년,월,일,시,분,초
	온도			1
	5일간날씨정보	15		(4일전){날씨아이콘}{최고온도}{최저온도}...(오늘){날씨아이콘}{최고온도}{최저온도}

	*날씨아이콘: 1~40
*/
	WEATHER_RES* pRes = (WEATHER_RES*)&(pBuffer[3]);
	int i;

	struct timeval tvNew;
	struct tm tmNew;

	//수신된 날씨 정보 데이타를 멤버변수로 백업
	memcpy(&m_WeathreRes, pRes, sizeof(WEATHER_RES));

	//시스템 시간설정
	memset(&tmNew, 0, sizeof(tmNew));
//	tmNew.tm_year = pRes->year+2000;
	tmNew.tm_year = pRes->year+100;		//1900년부터 시작
	tmNew.tm_mon  = pRes->month-1;		//0~11
	tmNew.tm_mday = pRes->day;
	tmNew.tm_hour = pRes->hour;
	tmNew.tm_min  = pRes->minute;
	tmNew.tm_sec  = pRes->second;

	tvNew.tv_sec  = mktime(&tmNew);
	tvNew.tv_usec = 0;

	if(settimeofday(&tvNew, NULL)==ERROR)
	{
		printf("settimeofday: error : %d %s\r\n", errno, strerror(errno));
	}
	else
	{
		system("hwclock -w");
		g_timer.Refresh();
		printf("settimeofday: done\r\n");
	}

	//날씨정보를 설정공간으로 복사
	g_setup_data.m_SetupData.weather_year		= pRes->year;
	g_setup_data.m_SetupData.weather_month		= pRes->month;
	g_setup_data.m_SetupData.weather_day		= pRes->day;
	g_setup_data.m_SetupData.weather_hour		= pRes->hour;
	g_setup_data.m_SetupData.weather_minute		= pRes->minute;
	g_setup_data.m_SetupData.weather_second		= pRes->second;
	g_setup_data.m_SetupData.current_temp		= pRes->temp_curr;
	g_setup_data.m_SetupData.today_temp_high	= pRes->weather_info[0].temp_high;
	g_setup_data.m_SetupData.today_temp_low		= pRes->weather_info[0].temp_low;
	g_setup_data.m_SetupData.today_weather		= ConvWeatherIcon(pRes->weather_info[0].weather_icon);
	g_setup_data.m_SetupData.tomorrow_temp_high	= pRes->weather_info[1].temp_high;
	g_setup_data.m_SetupData.tomorrow_temp_low	= pRes->weather_info[1].temp_low;
	g_setup_data.m_SetupData.tomorrow_weather	= ConvWeatherIcon(pRes->weather_info[1].weather_icon);

	g_setup_data.SaveSetupData();

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_WEATHER_RES, 0);
	CallContextReceiveProc(pBuffer);

	m_isWeatherInfo = TRUE;
	m_isWaitForResponse = FALSE;

#if 0
	if((g_state.GetState() == STATE_WEATHER) && (g_state.m_pCurrContext))
	{
		g_state.m_pCurrContext->ChangeContext();	//for redraw
	}
#endif

#ifdef DEBUG_RESPONSE
	printf("Time=%02d/%02d/%02d %02d:%02d:%02d, Temp=%d\r\n", 
			pRes->year, pRes->month, pRes->day,
			pRes->hour, pRes->minute, pRes->second,
			pRes->temp_curr);

	for(i=0; i<5; i++)
	{
		printf("%d) weather=%d, temp(high/low)=%d/%d\r\n", i,
				pRes->weather_info[i].weather_icon,
				pRes->weather_info[i].temp_high,
				pRes->weather_info[i].temp_low);
	}
#endif
}

void CSerialWallPad_SNS::ResponseProc_ParkingRes(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Time			6		년,월,일,시,분,초
	입출차정보		1		??
	차량번호		4		
	층정보			2		층명칭, 층번호
	구역정보		6		구역명칭, 구역번호
*/
	PARKING_RES* pRes = (PARKING_RES*)&(pBuffer[3]);
	int i;

	//수신된 주차 정보 데이타를 멤버변수로 백업
	memcpy(&m_ParkingRes, pRes, sizeof(PARKING_RES));

	//주차정보를 설정공간으로 복사
	g_setup_data.m_SetupData.in_out		= pRes->in_out;
	memcpy(g_setup_data.m_SetupData.car_num, pRes->car_num, 4);
	memcpy(g_setup_data.m_SetupData.car_floor, pRes->car_floor, 2);
	memcpy(g_setup_data.m_SetupData.car_zone, pRes->car_zone, 6);

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_PARKING_RES, 0);
	CallContextReceiveProc(pBuffer);
	m_isParkingInfo = TRUE;
	m_isWaitForResponse = FALSE;

#ifdef DEBUG_RESPONSE
	printf("Time=%02d/%02d/%02d %02d:%02d:%02d\r\n", 
			pRes->year, pRes->month, pRes->day,
			pRes->hour, pRes->minute, pRes->second);
#if 0
	printf("In/Out=%d, CarNum=%c%c%c%c, CarFloor=%c%c%, CarZone=%c%c%c%c%c%c\r\n",
			pRes->in_out,
			pRes->car_num[0], pRes->car_num[1], pRes->car_num[2], pRes->car_num[3],
			pRes->car_floor[0], pRes->car_floor[1], 
			pRes->car_zone[0], pRes->car_zone[1], pRes->car_zone[2], pRes->car_zone[3], pRes->car_zone[4], pRes->car_zone[5]);
#else
	printf("In/Out=%d, CarNum=%d%d%d%d, CarFloor=%c%02d, CarZone=%c%d%d%d%d%d\r\n",
			pRes->in_out,
			pRes->car_num[0], pRes->car_num[1], pRes->car_num[2], pRes->car_num[3],
			pRes->car_floor[0], pRes->car_floor[1], 
			pRes->car_zone[0], pRes->car_zone[1], pRes->car_zone[2], pRes->car_zone[3], pRes->car_zone[4], pRes->car_zone[5]);
#endif
#endif
}

void CSerialWallPad_SNS::ResponseProc_ElevatorRes(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x01=Success, 0x00=Fail
	현재층			1		0xF0(B16)..0xFE(B02),0xFF(B01),0x01(01),0x02(02)...0xEF(239)	*0x00(Loby)
	방향			1		0x01=Down, 0x02=Up, 0x00=Stop
*/
	ELEVATOR_STATUS* pRes = (ELEVATOR_STATUS*)&(pBuffer[3]);

	memcpy(&m_ElevatorStatus, pRes, sizeof(ELEVATOR_STATUS));

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_ELEVATOR_STAT_REQ, 0);
//	if(pBuffer[3]==0)	//Success
//	{
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
//	}

#ifdef DEBUG_RESPONSE
	printf("Status=%d, CurrFloor=%d, Dir=%d\r\n", pRes->status, (int)pRes->floor, pRes->dir);
#endif
}

void CSerialWallPad_SNS::ResponseProc_ElevatorCall(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	State			1		0x01=Down, 0x02=Up, 0x00=Stop
	Status			1		0x01=Success, 0x00=Fail
*/
	ELEVATOR_CALL_RES* pRes = (ELEVATOR_CALL_RES*)&(pBuffer[3]);

	memcpy(&m_ElevatorCallRes, pRes, sizeof(ELEVATOR_CALL_RES));

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_ELEVATOR_CALL, 0);
//	if(pBuffer[3]==0)	//Success
//	{
	#if 1
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else
		RequestReserve(SNS_CMD_ELEVATOR_STAT_REQ);
	#endif
//	}

#ifdef DEBUG_RESPONSE
	printf("State=%d, Status=%d\r\n", pRes->state, pRes->status);
#endif
}

/*
void ResponseProc_ElevatorArrive(UCHAR* pBuffer)
{
}
*/

void CSerialWallPad_SNS::ResponseProc_WallPadReq(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	alloff_stat		1		0x00=OFF(등이 켜짐), 0x01=ON(등이 꺼짐)
	gas_stat		1		0x00=닫힘, 0x01=열림                   
	security_stat	1		0x00=외출해제, 0x01=외출설정           
*/
	WALLPAD_STATUS* pRes = (WALLPAD_STATUS*)&(pBuffer[3]);

	//수신된 월패드 정보 데이타를 멤버변수로 백업
	memcpy(&m_WallPadStatus, pRes, sizeof(WALLPAD_STATUS));

	//월패드 정보를 설정공간으로 복사
	g_setup_data.m_SetupData.alloff_stat	= (pRes->alloff_stat) ? (0):(1);
	g_setup_data.m_SetupData.gas_stat		= pRes->gas_stat;
	g_setup_data.m_SetupData.security_stat	= pRes->security_stat;

	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (g_setup_data.m_SetupData.alloff_stat) ? LOW : HIGH);

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_WALLPAD_STAT_REQ, 0);
	CallContextReceiveProc(pBuffer);
	m_isWallPadInfo = TRUE;
	m_isWaitForResponse = FALSE;

#ifdef DEBUG_RESPONSE
	printf("alloff_stat=%d, gas_stat=%d, security_stat=%d\r\n",
			pRes->alloff_stat, pRes->gas_stat, pRes->security_stat);
#endif
}

void CSerialWallPad_SNS::ResponseProc_AllOffReq(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_LAMP_ONOFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 1
		g_setup_data.m_SetupData.alloff_stat = (g_setup_data.m_SetupData.alloff_stat==1) ? 0 : 1;	//반전
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//일괄소등 요청이 성공한 뒤에 월패드 상태를 다시 요청해서 그 결과로 판단함
		RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
	#endif
	}
	else
	{
		m_isWaitForResponse = TRUE;
	}

#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void CSerialWallPad_SNS::ResponseProc_GasOffReq(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_GAS_OFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 0
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//가스차단 요청이 성공한 뒤에 월패드 상태를 다시 요청해서 그 결과로 판단함
		RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
	#endif
	}
#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void CSerialWallPad_SNS::ResponseProc_SecurityReq(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_OUT_ONOFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 0
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//외출설정 요청이 성공한 뒤에 월패드 상태를 다시 요청해서 그 결과로 판단함
		RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
	#endif
	}

#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void CSerialWallPad_SNS::ResponseProc_SecurityRes(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=외출해제, 0x01=외출설정
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_OUT_STAT_RES, 0);
	CallContextReceiveProc(pBuffer);

#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void CSerialWallPad_SNS::ResponseProc_GasStatRes(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=열렴, 0x01=닫힘
*/
	//가스밸브상태를 설정공간으로 복사
#if 0
	g_setup_data.m_SetupData.gas_stat		= pBuffer[3];
#else
	g_setup_data.m_SetupData.gas_stat		= (pBuffer[3]) ? 0 : 1;		//월패드상태 응답과 일치시킴 (1=열림,0=닫힘)
#endif

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_GAS_STAT_RES, 0);
	CallContextReceiveProc(pBuffer);

#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void* CSerialWallPad_SNS::SerialListener_SNS(void *pParam)
{
	CSerialWallPad_SNS *pThis = (CSerialWallPad_SNS *)pParam;

	CContext* pContext = NULL;

	struct timeval timeout;
	fd_set fdRead;

	UCHAR buffer[MAX_RECV_BUFFER], cmd, len, cs_recv, cs_calc;
	int i, ret, nSelectTimeoutCount=0, len_recv, idx;

	ULONG ulTick;
	BOOL isProtocolSimulate = FALSE;

	printf("+%s\r\n", __func__);

	ulTick = GetTickCount();

	while(pThis->m_fListenerRunning)
	{
	#if 0
		if(pThis->IsOpen()==FALSE)
		{
			usleep(1000);
			continue;
		}
	#endif

	#if(PROTOCOL_SIMULATE == 1)
		if(pThis->m_isProtocolSimulate)
		{
			memset(buffer, 0, sizeof(buffer));
			printf("ProtoSimulate: SimulateNum=%d, Len=%d\r\n", pThis->m_nProtocolSimulateNum, g_ProtoSimTable[pThis->m_nProtocolSimulateNum][2]+4);
			memcpy(buffer, &g_ProtoSimTable[pThis->m_nProtocolSimulateNum][0], g_ProtoSimTable[pThis->m_nProtocolSimulateNum][2]+4);

			isProtocolSimulate = TRUE;
		}
		else
		{
			isProtocolSimulate = FALSE;
		}
	#endif

		//select이후 변수가 초기화되므로 Read Set를 다시 해줘야 함
		FD_ZERO(&fdRead);
		FD_SET(pThis->m_fdDev, &fdRead);

		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		//아래에서 continue로 루프반복될 경우 대비
		if(pThis->m_isRecv) pThis->m_isRecv = FALSE;

		ret = select(pThis->m_fdDev+1, &fdRead, NULL, NULL, &timeout);
	#if(PROTOCOL_SIMULATE == 1)
		if(FD_ISSET(pThis->m_fdDev, &fdRead) || isProtocolSimulate)
	#else
		if(FD_ISSET(pThis->m_fdDev, &fdRead))	//Receive
	#endif
		{
		#if(PROTOCOL_SIMULATE == 1)
			if(isProtocolSimulate==FALSE)
			{
		#endif
			//< 패킷 수신부

				pThis->m_isRecv = TRUE;

				// 1 byte를 읽어서 월패드가 보내는 헤더(0xCD)인가 확인
				ret = pThis->Read(&(buffer[0]), 1);
				if( (ret!=1) || (buffer[0]!=SNS_HDR_WALLPAD) ) continue;

			//	usleep(2 * 834);			//2바이트 수신대기 (cmd, len)

				// Command와 SubData Length 2 bytes를 읽음
				ret = pThis->Read(&(buffer[1]), 2);
				if(ret!=2) continue;

				cmd = buffer[1];
				len = buffer[2];

				// 유효한 Command인지 확인 ( 0x1~0xE, 0x5A, 0x41 )
				if( !( ((cmd>0) && (cmd<=SNS_CMD_ELEVATOR_ARRIVE)) || (cmd==SNS_CMD_INIT_DATA) || (cmd==SNS_CMD_FREE_DATA) ) ) continue;

			//	if( len == 0 ) continue;	//freedata포맷이 len이 없이 오는 경우가 있어서 체크 생략

				// SubData Length가 수신 버퍼크기를 오버하는지 확인
				if( (3+len+1) > MAX_RECV_BUFFER ) continue;

			//	usleep((len+1) * 834);		//SubData와 CS 수신대기

				// SubData와 CheckSum을 읽음
			#if 0
				ret = pThis->Read(&(buffer[3]), len+1);
				if(ret != (len+1)) continue;
			#else	// 이어받기
				len_recv = 0;
				while(len_recv < (len+1))
				{
					idx = 3+len_recv;
					ret = pThis->Read(&(buffer[idx]), len+1-len_recv);
					len_recv += ret;
				}
			#endif

				// 수신된 체크섬과 계산된 체크섬 비교
				cs_recv = buffer[3+len];
				cs_calc = pThis->CalcCheckSum(buffer, 3+len);
				if(cs_recv != cs_calc)
				{
					printf("%s: Invalid Checksum: recv=0x%x, calc=0x%x\r\n", __func__, cs_recv, cs_calc);
					continue;
				}

				pThis->m_isRecv = FALSE;

				nSelectTimeoutCount = 0;
				pThis->m_isReceiveWaitTimeout = FALSE;

			//> 패킷 수신부
		#if(PROTOCOL_SIMULATE == 1)
			}
			else
			{
				cmd = buffer[1];
				len = buffer[2];

				pThis->m_isProtocolSimulate = FALSE;
			}
		#endif

	#ifdef PACKET_DUMP_ENABLE
			printf("Rx(%lu): ", GetElapsedTick(ulTick));
			DumpData(buffer, 3+len+1, TRUE);
			ulTick = GetTickCount();
	#endif

		//< 설정 WallPad Test 인 경우 CContextSetup::RecvProc에서 패킷 처리함
		#if 0
			if(g_state.GetState() == STATE_SETUP)
			{
				pContext = g_state.GetCurrContext();
				if(pContext)
				{
					if(pContext->m_nContextNum == 6)	//WallPad
					{
						pThis->CallContextReceiveProc(buffer);
						continue;
					}
				}
			}
		#else
			if(pThis->m_isContextSetupWallPad)
			{
				pThis->CallContextReceiveProc(buffer);
				continue;
			}
		#endif
		//>

		//< 패킷 응답 및 처리부
//			usleep(1500);		//월패드로부터 수신후 응답을 보내기 위해 TX Enable 하기까지 1.5ms 지연

			// Command별 처리
			switch(cmd)
			{
		//-------------------------------------------------------------------------------------
			case SNS_CMD_WEATHER_RES:		//0x01, 월패드에서 생활정보기로 날씨정보 전송
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_WeatherRes(buffer);
				break;
			case SNS_CMD_WEATHER_REQ:		//0x02, 생활정보기에서 월패드로 날씨정보 요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_PARKING_RES:		//0x03, 월패드에서 생활정보기로 주차정보 전송
				parking_list_update((PARKING_RES*)&(buffer[3]));
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_ParkingRes(buffer);
				break;
			case SNS_CMD_PARKING_REQ:		//0x04, 생활정보기에서 월패드로 주차정보 요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_ELEVATOR_STAT_REQ:	//0x05, 생활정보기에서 월패드로 엘레베이터 상태요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				break;
			case SNS_CMD_ELEVATOR_STAT_RES:	//0x06, 월패드에서 생활정보기로 엘레베이터 상태전송
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_ElevatorRes(buffer);
				break;
			case SNS_CMD_ELEVATOR_CALL:		//0x07, 생할정보기에서 월패드로 엘레베이터 호출
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_ElevatorCall(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_WALLPAD_STAT_REQ:	//0x08, 생할정보기에서 월패드로 일괄소등,가스,외출설정 상태요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_WallPadReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_LAMP_ONOFF_REQ:	//0x09, 생할정보기에서 월패드로 일괄소등 On/Off요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_AllOffReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_GAS_OFF_REQ:		//0x0A, 생할정보기에서 월패드로 가스밸브 Off요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_GasOffReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_OUT_ONOFF_REQ:		//0x0B, 생할정보기에서 월패드로 외출설정 On/Off요청
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_SecurityReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_OUT_STAT_RES:		//0x0C, 월패드에서 생활정보기로 외출설정 상태전송
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_SecurityRes(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_GAS_STAT_RES:		//0x0D, 월패드에서 생활정보기로 가스밸브 상태전송
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_GasStatRes(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_ELEVATOR_ARRIVE:	//0x0E, 월패드에서 생활정보기로 엘리베이터 목적층 도착시 메세지 전송
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->CallContextReceiveProc(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_INIT_DATA:			//0x5A, 월패드에서 생활정보기로 : 월패드가 켜질때, 월패드가 Ack Data 10회이상 미수신할 경우
				pThis->SendAck(cmd);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_FREE_DATA:			//0x41, 월패드에서 생활정보기로 : Event Data 없을 경우, Live Check

				//예약된 요청이 있으면
				if(pThis->m_isRequestReserve)
				{
					printf("%s: RequestReserve cmd=0x%x, data=0x%x\r\n", __func__, 
								pThis->m_cmdRequestReserve, pThis->m_dataRequestReserve);
					pThis->SendRequest(pThis->m_cmdRequestReserve, pThis->m_dataRequestReserve);
					pThis->m_isRequestReserve = FALSE;		//예약처리됨
					pThis->m_isWaitForResponse = TRUE;		//대기시작
					pThis->m_nWaitCountForResponse = 0;
				}
				//요청에 대한 응답을 대기중인 상태라면 --> 응답대신 FreeData가 온다는것은 재요청이 필요한 상황
				else if(pThis->m_isWaitForResponse)
				{
					//요청응답이 없는 경우 3회 재전송
					pThis->m_nWaitCountForResponse++;
					if(pThis->m_nWaitCountForResponse <= MAX_WAIT_COUNT_FOR_RESPONSE)
					{
						printf("%s: Re-RequestReserve(retry=%d) cmd=0x%x, data=0x%x\r\n", __func__, 
								pThis->m_nWaitCountForResponse, 
								pThis->m_cmdRequestReserve, 
								pThis->m_dataRequestReserve);
						pThis->SendRequest(pThis->m_cmdRequestReserve, pThis->m_dataRequestReserve);
					}
					//재전송 타임아웃이면 응답대기를 중단
					else
					{
						printf("%s: RequestReserve Canceled: retry timeout!\r\n", __func__);
						pThis->m_isWaitForResponse = FALSE;
						switch(pThis->m_cmdRequestReserve)
						{
						case SNS_CMD_WEATHER_REQ:
							pThis->m_isWeatherInfoCancel = TRUE;
							break;
						case SNS_CMD_PARKING_REQ:
							pThis->m_isParkingInfoCancel = TRUE;
							break;
						case SNS_CMD_WALLPAD_STAT_REQ:
							pThis->m_isWallPadInfoCancel = TRUE;
							break;
						case SNS_CMD_LAMP_ONOFF_REQ:
							if(g_state.GetState()==STATE_ALLOFF)
							{
								g_state.ChangeContext(2);
							}
							break;
						}
						pThis->SendFreeData();
					}
				}
				//대기중인 요청이 없는 상태
				else
				{
			#if 0
				//< 최초 실행시 자동으로 정보요청

					//날씨정보가 수신되지 않았으면 (타임아웃으로 취소된 경우는 생략)
					if( (pThis->m_isWeatherInfo==FALSE) && (pThis->m_isWeatherInfoCancel==FALSE) )
					{
						printf("%s: WeatherInfo Auto Request Reserved\r\n", __func__);
						pThis->RequestReserve(SNS_CMD_WEATHER_REQ);
					}
					//월패드정보가 수신되지 않았으면 (타임아웃으로 취소된 경우는 생략)
					else if( (pThis->m_isWallPadInfo==FALSE) && (pThis->m_isWallPadInfoCancel==FALSE) )
					{
						printf("%s: WallPadInfo Auto Request Reserved\r\n", __func__);
						pThis->RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
					}
					//주차정보가 수신되지 않았으면 (타임아웃으로 취소된 경우는 생략)
					else if( (pThis->m_isParkingInfo==FALSE) && (pThis->m_isParkingInfoCancel==FALSE) )
					{
						printf("%s: ParkingInfo Auto Request Reserved\r\n", __func__);
						pThis->RequestReserve(SNS_CMD_PARKING_REQ);
					}
				//>
			#endif
					
					pThis->SendFreeData();
				}
				break; 
		//-------------------------------------------------------------------------------------
			default:
				break;
			}
		//> 패킷 응답 및 처리부
		}
		else	//select timeout
		{
			nSelectTimeoutCount++;

			if(nSelectTimeoutCount == MAX_SELECT_TIMEOUT_COUNT)
			{
				pThis->m_isReceiveWaitTimeout = TRUE;
			//	printf("%s: Select Timeout! Check Connection!\r\n", __func__);
				nSelectTimeoutCount = 0;
			}
		}
	}

	printf("-%s\r\n", __func__);

	pthread_exit(NULL); 
}


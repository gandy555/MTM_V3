#include "common.h"
//#include "serial_wallpad_sns.h"
#include "main.h"

#define PACKET_DUMP_ENABLE
#define DEBUG_RESPONSE

#if(PROTOCOL_SIMULATE == 1)

#define	MAX_PROTO_SIZE			32

UCHAR g_ProtoSimTable[PROTO_SIM_COUNT][MAX_PROTO_SIZE] = 
{
//	PROTO_SIM_WEATHER_1,				//���뼺��ǳ(4)-->����, �̽���(9)-->��
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x04, 0x0b, 0x05, 
		0x09, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x72	},
//	PROTO_SIM_WEATHER_2,				//�ҳ���(11)-->�ҳ���, ��(6)-->��, 
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x0b, 0x0b, 0x05, 
		0x06, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x72	},
//	PROTO_SIM_WEATHER_3,				//�帲(22)-->�帲, �������� ����(��)(30)-->��������
	{	0xcd, 0x01, 0x16, 
		0x0e, 0x09, 0x16, 0x0b, 0x0c, 0x23, 
		0x14,							// XOR(FB)
		0x16, 0x0b, 0x05, 
		0x1E, 0x0c, 0x0a, 
		0x03, 0x0d, 0x03, 
		0x04, 0x0e, 0x04, 
		0x05, 0x0f, 0x05, 
		0x77	},
//	PROTO_SIM_WEATHER_4,				//ȭâ(32)-->����, ������ ������ �ҳ���(47)-->����
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

    tio.c_cc[VTIME] = 1;				// timeout 0.1�� ����
    tio.c_cc[VMIN]  = 1;				// �ּ� n ���� ���� ������ ���

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

	// TX Enable�� ���� ��Ŷ�� �����ϱ���� 1.5ms ����
	usleep(1500);

	ret = write(m_fdDev, pszBuffer, size);

	usleep(size * 834);

	//������ ��Ŷ ������ TX Disable �ϱ���� 500us ����
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

//1. ��Ŷ�� ȣ���� �˻�� ��Ŷ���� ����
//2. InitData(0x5A), FreeData(0x41), ���� ���� ACK���� Que�� ���� ����
//   --> ���ÿ��� �������� ���ɸ��� �͵鸸 �ְԵ�
//3. Que�� Full�̸� ������� Push�� ��Ŷ�ڸ��� OverWrite��
BOOL CSerialWallPad_SNS::PushPacketIntoQue(UCHAR* pPacket)
{
	UCHAR len;

	if(pPacket==NULL) return ERROR;

	len = pPacket[2];

	//��Ŷ�� ���̰� ����ũ�⸦ �Ѿ�� ��� --> �߸��� ��Ŷ�̹Ƿ� ����
	if( (len==0) || ((len+4) > MAX_RECV_BUFFER) ) return ERROR;

	//m_isQueFull: Pop�� ���� Push�� MAX_QUE_COUNT��ŭ �̷������ ���̻� Push�� ������ ����
	//			   --> Empty���¿� Full���´� Push Index�� Pop Index�� ������
	if(m_isQueFull)
	{
		//Pop Index�� Plus Shift�����ν� Que���� ������� Push�� ����Ÿ�� �������� ó����
		m_idxQuePop++;
		if(m_idxQuePop == MAX_QUE_COUNT) m_idxQuePop=0;

	//	m_isQueFull = FALSE;	//������ �ٽ� ä���� ��Ȳ�̹Ƿ� �ǹ� ����
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

	//Full�� �ƴѵ� Pop Index�� Push Index�� ���ٸ� Que�� ��� �ִ� ����
	//--> ���� Full�̰ų� Pop Index�� Push Index�� �ٸ��� Que�� ��Ŷ�� �ִ� ����
	if( (m_isQueFull) || (m_idxQuePop!=m_idxQuePush) )
	{
		//�����͸� ���� �� ��� Full�� ��� ���Ⱑ ���� �ʾ� 
		//Que�� ó�� ��Ŷ�� �ƴ϶� ������ ��Ŷ�� ó���� �� ����
		//����, Que�� ��Ŷ�� �������ִ� ������� ��ȯ
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

#if 0	//��������
	//WallPad�� ���� Icon�� 1~40
	switch(nWallPadIcon)
	{
	case 1:		//����(��ü�θ���)
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 2:		//��������
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 3:		//��������
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 4:		//�帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 5:		//�帰�� ������
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 6:		//������ ���� �����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 7:		//�ҳ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 8:		//����/��ħ/���� �ҳ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 9:		//��/����/�� �ҳ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 10:	//�帮�� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 11:	//����/��ħ/���� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 12:	//��/����/�� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 13:	//���� ����� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 14:	//���� ����� ��/����/�� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 15:	//����� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 16:	//����/��ħ/���� �Ѷ� ����� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 17:	//��/����/�� �Ѷ� ����� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 18:	//�帮�� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 19:	//����/��ħ/���� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 20:	//��/����/�� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 21:	//���� ����� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 22:	//���� ����� ��/����/�� �Ѷ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 23:	//������ ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 24:	//����/��ħ/���� �Ѷ� ������ ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 25:	//��/����/�� �Ѷ� ������ ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 26:	//�� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 27:	//����/��ħ/���� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 28:	//��/����/�� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 29:	//���� ����� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 30:	//���� ����� ��/����/�� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 31:	//�� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 32:	//����/��ħ/���� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 33:	//��/����/�� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 34:	//���� ����� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 35:	//���� ����� ��/����/�� �Ѷ� �� �Ǵ� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 36:	//�� �Ǵ� �� ���� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 37:	//����/��ħ/���� �Ѷ� �� �Ǵ� �� ���� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 38:	//��/����/�� �Ѷ� �� �Ǵ� �� ���� ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 39:	//õ�չ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 40:	//�Ȱ�
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	default:
		printf("%s: Unknown WallPad Weather Icon(%d)\r\n", __func__, nWallPadIcon);
		break;
	}
#else	//������ (20140826)
	//WallPad�� ���� Icon�� 1~47, ��Ȱ�������� ���� Icon�� 0~6
	switch(nWallPadIcon)
	{
	case 1:		//���뼺��ǳ			--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 2:		//�㸮����				--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 3:		//���ѳ���				--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 4:		//������ ������ ��		--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 5:		//�� �Ǵ� ��������		--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 6:		//�� �Ǵ� ��������		--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 7:		//��������				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 8:		//��������				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 9:		//�̽���				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 10:	//���� ������ ��		--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 11:	//�ҳ���				--> �ҳ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 12:	//�ҳ���				--> �ҳ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 13:	//��������				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 14:	//�ѽ��� ������			--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 15:	//�θ���				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 16:	//��					--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 17:	//�ζ�⴫				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 18:	//��������				--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 19:	//Ȳ������				--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 20:	//£�� �Ȱ�				--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 21:	//���� �Ȱ�				--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 22:	//�帲					--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 23:	//���� �ٶ�				--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 24:	//�� �ٶ�				--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 25:	//����					--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 26:	//�帲					--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 27:	//���������� �帲(��)	--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 28:	//���������� �帲(��)	--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 29:	//���� ���� ����(��)	--> ���� ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 30:	//���� ���� ����(��)	--> ���� ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;
		break;
	case 31:	//����(��)				--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 32:	//ȭâ					--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 33:	//����(��)				--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 34:	//����(��)				--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 35:	//����� ������ ��		--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;
		break;
	case 36:	//����					--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;
		break;
	case 37:	//������ ���� ����		--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 38:	//������ ����			--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 39:	//������ ����			--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 40:	//������ �ҳ���			--> �ҳ���
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;
		break;
	case 41:	//����					--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 42:	//������ �� �Ǵ� ��		--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 43:	//����					--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 44:	//���� ���� �帲		--> �帲
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;
		break;
	case 45:	//������ ������ �ҳ���	--> ����
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;
		break;
	case 46:	//����					--> ��
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;
		break;
	case 47:	//������ ���� ������ ������ �ҳ���	--> ����
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
#if 1	//�������

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

#else	//����� - Que���

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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Time			6		��,��,��,��,��,��
	�µ�			1
	5�ϰ���������	15		(4����){����������}{�ְ�µ�}{�����µ�}...(����){����������}{�ְ�µ�}{�����µ�}

	*����������: 1~40
*/
	WEATHER_RES* pRes = (WEATHER_RES*)&(pBuffer[3]);
	int i;

	struct timeval tvNew;
	struct tm tmNew;

	//���ŵ� ���� ���� ����Ÿ�� ��������� ���
	memcpy(&m_WeathreRes, pRes, sizeof(WEATHER_RES));

	//�ý��� �ð�����
	memset(&tmNew, 0, sizeof(tmNew));
//	tmNew.tm_year = pRes->year+2000;
	tmNew.tm_year = pRes->year+100;		//1900����� ����
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

	//���������� ������������ ����
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Time			6		��,��,��,��,��,��
	����������		1		??
	������ȣ		4		
	������			2		����Ī, ����ȣ
	��������		6		������Ī, ������ȣ
*/
	PARKING_RES* pRes = (PARKING_RES*)&(pBuffer[3]);
	int i;

	//���ŵ� ���� ���� ����Ÿ�� ��������� ���
	memcpy(&m_ParkingRes, pRes, sizeof(PARKING_RES));

	//���������� ������������ ����
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x01=Success, 0x00=Fail
	������			1		0xF0(B16)..0xFE(B02),0xFF(B01),0x01(01),0x02(02)...0xEF(239)	*0x00(Loby)
	����			1		0x01=Down, 0x02=Up, 0x00=Stop
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
	�׸�			����	����
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	alloff_stat		1		0x00=OFF(���� ����), 0x01=ON(���� ����)
	gas_stat		1		0x00=����, 0x01=����                   
	security_stat	1		0x00=��������, 0x01=���⼳��           
*/
	WALLPAD_STATUS* pRes = (WALLPAD_STATUS*)&(pBuffer[3]);

	//���ŵ� ���е� ���� ����Ÿ�� ��������� ���
	memcpy(&m_WallPadStatus, pRes, sizeof(WALLPAD_STATUS));

	//���е� ������ ������������ ����
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_LAMP_ONOFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 1
		g_setup_data.m_SetupData.alloff_stat = (g_setup_data.m_SetupData.alloff_stat==1) ? 0 : 1;	//����
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//�ϰ��ҵ� ��û�� ������ �ڿ� ���е� ���¸� �ٽ� ��û�ؼ� �� ����� �Ǵ���
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_GAS_OFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 0
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//�������� ��û�� ������ �ڿ� ���е� ���¸� �ٽ� ��û�ؼ� �� ����� �Ǵ���
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_OUT_ONOFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 0
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//���⼳�� ��û�� ������ �ڿ� ���е� ���¸� �ٽ� ��û�ؼ� �� ����� �Ǵ���
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=��������, 0x01=���⼳��
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
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=����, 0x01=����
*/
	//���������¸� ������������ ����
#if 0
	g_setup_data.m_SetupData.gas_stat		= pBuffer[3];
#else
	g_setup_data.m_SetupData.gas_stat		= (pBuffer[3]) ? 0 : 1;		//���е���� ����� ��ġ��Ŵ (1=����,0=����)
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

		//select���� ������ �ʱ�ȭ�ǹǷ� Read Set�� �ٽ� ����� ��
		FD_ZERO(&fdRead);
		FD_SET(pThis->m_fdDev, &fdRead);

		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		//�Ʒ����� continue�� �����ݺ��� ��� ���
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
			//< ��Ŷ ���ź�

				pThis->m_isRecv = TRUE;

				// 1 byte�� �о ���е尡 ������ ���(0xCD)�ΰ� Ȯ��
				ret = pThis->Read(&(buffer[0]), 1);
				if( (ret!=1) || (buffer[0]!=SNS_HDR_WALLPAD) ) continue;

			//	usleep(2 * 834);			//2����Ʈ ���Ŵ�� (cmd, len)

				// Command�� SubData Length 2 bytes�� ����
				ret = pThis->Read(&(buffer[1]), 2);
				if(ret!=2) continue;

				cmd = buffer[1];
				len = buffer[2];

				// ��ȿ�� Command���� Ȯ�� ( 0x1~0xE, 0x5A, 0x41 )
				if( !( ((cmd>0) && (cmd<=SNS_CMD_ELEVATOR_ARRIVE)) || (cmd==SNS_CMD_INIT_DATA) || (cmd==SNS_CMD_FREE_DATA) ) ) continue;

			//	if( len == 0 ) continue;	//freedata������ len�� ���� ���� ��찡 �־ üũ ����

				// SubData Length�� ���� ����ũ�⸦ �����ϴ��� Ȯ��
				if( (3+len+1) > MAX_RECV_BUFFER ) continue;

			//	usleep((len+1) * 834);		//SubData�� CS ���Ŵ��

				// SubData�� CheckSum�� ����
			#if 0
				ret = pThis->Read(&(buffer[3]), len+1);
				if(ret != (len+1)) continue;
			#else	// �̾�ޱ�
				len_recv = 0;
				while(len_recv < (len+1))
				{
					idx = 3+len_recv;
					ret = pThis->Read(&(buffer[idx]), len+1-len_recv);
					len_recv += ret;
				}
			#endif

				// ���ŵ� üũ���� ���� üũ�� ��
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

			//> ��Ŷ ���ź�
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

		//< ���� WallPad Test �� ��� CContextSetup::RecvProc���� ��Ŷ ó����
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

		//< ��Ŷ ���� �� ó����
//			usleep(1500);		//���е�κ��� ������ ������ ������ ���� TX Enable �ϱ���� 1.5ms ����

			// Command�� ó��
			switch(cmd)
			{
		//-------------------------------------------------------------------------------------
			case SNS_CMD_WEATHER_RES:		//0x01, ���е忡�� ��Ȱ������� �������� ����
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_WeatherRes(buffer);
				break;
			case SNS_CMD_WEATHER_REQ:		//0x02, ��Ȱ�����⿡�� ���е�� �������� ��û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_PARKING_RES:		//0x03, ���е忡�� ��Ȱ������� �������� ����
				parking_list_update((PARKING_RES*)&(buffer[3]));
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_ParkingRes(buffer);
				break;
			case SNS_CMD_PARKING_REQ:		//0x04, ��Ȱ�����⿡�� ���е�� �������� ��û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_ELEVATOR_STAT_REQ:	//0x05, ��Ȱ�����⿡�� ���е�� ���������� ���¿�û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				break;
			case SNS_CMD_ELEVATOR_STAT_RES:	//0x06, ���е忡�� ��Ȱ������� ���������� ��������
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_ElevatorRes(buffer);
				break;
			case SNS_CMD_ELEVATOR_CALL:		//0x07, ���������⿡�� ���е�� ���������� ȣ��
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_ElevatorCall(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_WALLPAD_STAT_REQ:	//0x08, ���������⿡�� ���е�� �ϰ��ҵ�,����,���⼳�� ���¿�û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_WallPadReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_LAMP_ONOFF_REQ:	//0x09, ���������⿡�� ���е�� �ϰ��ҵ� On/Off��û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_AllOffReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_GAS_OFF_REQ:		//0x0A, ���������⿡�� ���е�� ������� Off��û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_GasOffReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_OUT_ONOFF_REQ:		//0x0B, ���������⿡�� ���е�� ���⼳�� On/Off��û
				pThis->SendFreeData();
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_SecurityReq(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_OUT_STAT_RES:		//0x0C, ���е忡�� ��Ȱ������� ���⼳�� ��������
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_SecurityRes(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_GAS_STAT_RES:		//0x0D, ���е忡�� ��Ȱ������� ������� ��������
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->ResponseProc_GasStatRes(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_ELEVATOR_ARRIVE:	//0x0E, ���е忡�� ��Ȱ������� ���������� ������ ������ �޼��� ����
				pThis->SendAck(cmd);
				pThis->m_isWaitForResponse = FALSE;
				pThis->CallContextReceiveProc(buffer);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_INIT_DATA:			//0x5A, ���е忡�� ��Ȱ������� : ���е尡 ������, ���е尡 Ack Data 10ȸ�̻� �̼����� ���
				pThis->SendAck(cmd);
				break;
		//-------------------------------------------------------------------------------------
			case SNS_CMD_FREE_DATA:			//0x41, ���е忡�� ��Ȱ������� : Event Data ���� ���, Live Check

				//����� ��û�� ������
				if(pThis->m_isRequestReserve)
				{
					printf("%s: RequestReserve cmd=0x%x, data=0x%x\r\n", __func__, 
								pThis->m_cmdRequestReserve, pThis->m_dataRequestReserve);
					pThis->SendRequest(pThis->m_cmdRequestReserve, pThis->m_dataRequestReserve);
					pThis->m_isRequestReserve = FALSE;		//����ó����
					pThis->m_isWaitForResponse = TRUE;		//������
					pThis->m_nWaitCountForResponse = 0;
				}
				//��û�� ���� ������ ������� ���¶�� --> ������ FreeData�� �´ٴ°��� ���û�� �ʿ��� ��Ȳ
				else if(pThis->m_isWaitForResponse)
				{
					//��û������ ���� ��� 3ȸ ������
					pThis->m_nWaitCountForResponse++;
					if(pThis->m_nWaitCountForResponse <= MAX_WAIT_COUNT_FOR_RESPONSE)
					{
						printf("%s: Re-RequestReserve(retry=%d) cmd=0x%x, data=0x%x\r\n", __func__, 
								pThis->m_nWaitCountForResponse, 
								pThis->m_cmdRequestReserve, 
								pThis->m_dataRequestReserve);
						pThis->SendRequest(pThis->m_cmdRequestReserve, pThis->m_dataRequestReserve);
					}
					//������ Ÿ�Ӿƿ��̸� �����⸦ �ߴ�
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
				//������� ��û�� ���� ����
				else
				{
			#if 0
				//< ���� ����� �ڵ����� ������û

					//���������� ���ŵ��� �ʾ����� (Ÿ�Ӿƿ����� ��ҵ� ���� ����)
					if( (pThis->m_isWeatherInfo==FALSE) && (pThis->m_isWeatherInfoCancel==FALSE) )
					{
						printf("%s: WeatherInfo Auto Request Reserved\r\n", __func__);
						pThis->RequestReserve(SNS_CMD_WEATHER_REQ);
					}
					//���е������� ���ŵ��� �ʾ����� (Ÿ�Ӿƿ����� ��ҵ� ���� ����)
					else if( (pThis->m_isWallPadInfo==FALSE) && (pThis->m_isWallPadInfoCancel==FALSE) )
					{
						printf("%s: WallPadInfo Auto Request Reserved\r\n", __func__);
						pThis->RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
					}
					//���������� ���ŵ��� �ʾ����� (Ÿ�Ӿƿ����� ��ҵ� ���� ����)
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
		//> ��Ŷ ���� �� ó����
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


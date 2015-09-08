#include "common.h"
//#include "serial_wallpad_sns.h"
#include "main.h"


//
// Construction/Destruction
//

CSerialWallPad_HDT::CSerialWallPad_HDT()
	: CSerialWallPad()
{
	m_usMaker = WALLPAD_MAKER_HDT;
	m_usModel = WALLPAD_MODEL_HDT;
}

CSerialWallPad_HDT::~CSerialWallPad_HDT()
	: ~CSerialWallPad()
{

}

//
// Member Function
//
BOOL CSerialWallPad_HDT::Open(const char *pszDev, unsigned int baudRate)
{
	struct termios tio;

	if(CSerialWallPad::Open(pszDev, baudRate) == FALSE)
	{
		return FALSE;
	}

    memset(&tio, 0, sizeof(termios));
	tio.c_cflag = CS8 | CLOCAL | CREAD;	// 8 data bits/ no parity / NO-rts / cts

	tio.c_cflag |= baudRate;

    tio.c_lflag = 0;

    tio.c_cc[VTIME] = 1;				// timeout 0.1�� ����
    tio.c_cc[VMIN]  = 1;				// �ּ� n ���� ���� ������ ���

    tcflush  ( m_fdDev, TCIFLUSH );
    tcsetattr( m_fdDev, TCSANOW, &tio );

	ioctl( m_fdDev, 0x54F0, 0x0001 );
	tcflush ( m_fdDev, TCIFLUSH );

	if(tcgetattr(m_fdDev, &tio)==ERROR)
	{
		printf("[Failure]\r\n--> %s: tcgetattr failure\r\n", __func__);
		return FALSE;
	}

	return TRUE;
}

BOOL CSerialWallPad_HDT::Run()
{
	if(StartListener(SerialListener_HDT)==FALSE)
	{
		printf("[Failure]\r\n--> %s: StartListener Failure\r\n", __func__);
		return FALSE;
	}
	return TRUE;
}

void CSerialWallPad_HDT::Stop()
{
	CSerialWallPad::Stop();
}

int CSerialWallPad_HDT::Write(const UCHAR *pszBuffer, int size)
{
	return CSerialWallPad::Write(pszBuffer, size);
}

UCHAR CSerialWallPad_HDT::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	return CSerialWallPad::CalcCheckSum(pBuffer, size);
}

int CSerialWallPad_HDT::GetPacketCountInQue()
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
BOOL CSerialWallPad_HDT::PushPacketIntoQue(UCHAR* pPacket)
{
	if(pPacket)
		return CSerialWallPad::PushPacketIntoQue(pPacket, pPacket[1]);
	else
		return ERROR;
}

BOOL CSerialWallPad_HDT::PopPacketFromQue(UCHAR* pPacket)
{
	return CSerialWallPad::PopPacketFromQue(pPacket);
}

UINT CSerialWallPad_HDT::ConvWeatherIcon(UINT nWallPadIcon)
{
	UINT nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;

	switch(nWallPadIcon)
	{
	case 1:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;			//����
		break;
	case 2:
	case 9:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;		//���� ����
		break;
	case 3:
	case 9:
	case 11:
	case 16:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;				//�帲
		break;
	case 7:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;				//�ҳ���
		break;
	case 4:
	case 6:
	case 13:
	case 15:
	case 18:
	case 20:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;				//��
		break;
	case 10:
	case 12:
	case 17:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;		//����
		break;
	case 5:
	case 8:
	case 14:
	case 19:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;				//��
		break;
	default:
		printf("%s: Unknown WallPad Weather Icon(%d)\r\n", __func__, nWallPadIcon);
		break;
	}
}

void CSerialWallPad_HDT::CallContextReceiveProc(void *pParam)
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

void CSerialWallPad_HDT::RequestReserve(UCHAR cmd, UCHAR data)
{
	m_isRequestReserve = TRUE;
	m_cmdRequestReserve = cmd;
	m_dataRequestReserve = data;
}

void CSerialWallPad_HDT::ResponseProc_WeatherRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_ParkingRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_ElevatorRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_ElevatorCall(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_WallPadReq(UCHAR* pBuffer)
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
	g_setup_data.m_SetupData.alloff_stat	= pRes->alloff_stat;
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

void CSerialWallPad_HDT::ResponseProc_AllOffReq(UCHAR* pBuffer)
{
/*
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_LAMP_ONOFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 0
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//�ϰ��ҵ� ��û�� ������ �ڿ� ���е� ���¸� �ٽ� ��û�ؼ� �� ����� �Ǵ���
		RequestReserve(SNS_CMD_WALLPAD_STAT_REQ);
	#endif
	}

#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void CSerialWallPad_HDT::ResponseProc_GasOffReq(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_SecurityReq(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_SecurityRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_GasStatRes(UCHAR* pBuffer)
{
/*
	�׸�			����	����
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=����, 0x01=����
*/
	//���������¸� ������������ ����
	g_setup_data.m_SetupData.gas_stat		= pBuffer[3];

//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_GAS_STAT_RES, 0);
	CallContextReceiveProc(pBuffer);

#ifdef DEBUG_RESPONSE
	printf("Status=%d\r\n", pBuffer[3]);
#endif
}

void* CSerialWallPad_HDT::SerialListener_SNS(void *pParam)
{
	CSerialWallPad_HDT *pThis = (CSerialWallPad_HDT *)pParam;

	CContext* pContext = NULL;

	struct timeval timeout;
	fd_set fdRead;

	UCHAR buffer[MAX_RECV_BUFFER], cmd, len, cs_recv, cs_calc;
	int i, ret, nSelectTimeoutCount=0, len_recv, idx;

	ULONG ulTick;

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
		//select���� ������ �ʱ�ȭ�ǹǷ� Read Set�� �ٽ� ����� ��
		FD_ZERO(&fdRead);
		FD_SET(pThis->m_fdDev, &fdRead);

		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		//�Ʒ����� continue�� �����ݺ��� ��� ���
		if(pThis->m_isRecv) pThis->m_isRecv = FALSE;

		ret = select(pThis->m_fdDev+1, &fdRead, NULL, NULL, &timeout);
		if(FD_ISSET(pThis->m_fdDev, &fdRead))	//Receive
		{
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

			// ��ȿ�� Command���� Ȯ�� ( 0x1~0xD, 0x5A, 0x41 )
			if( !( ((cmd>0) && (cmd<=0xD)) || (cmd==SNS_CMD_INIT_DATA) || (cmd==SNS_CMD_FREE_DATA) ) ) continue;

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
		//		pThis->ResponseProc_ElevatorRes(buffer);
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
						case SNS_CMD_WALLPAD_STAT_REQ:
							pThis->m_isWallPadInfoCancel = TRUE;
							break;
						case SNS_CMD_PARKING_REQ:
							pThis->m_isParkingInfoCancel = TRUE;
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


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

    tio.c_cc[VTIME] = 1;				// timeout 0.1초 단위
    tio.c_cc[VMIN]  = 1;				// 최소 n 문자 받을 때까진 대기

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

//1. 패킷은 호출전 검사된 패킷으로 가정
//2. InitData(0x5A), FreeData(0x41), 값이 없는 ACK들은 Que에 넣지 않음
//   --> 어플에서 응답으로 사용될만한 것들만 넣게됨
//3. Que가 Full이면 가장먼저 Push된 패킷자리에 OverWrite됨
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
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SERENITY;			//맑음
		break;
	case 2:
	case 9:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_PARTLY_CLOUDY;		//구름 조금
		break;
	case 3:
	case 9:
	case 11:
	case 16:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_CLOUDY;				//흐림
		break;
	case 7:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SHOWER;				//소나기
		break;
	case 4:
	case 6:
	case 13:
	case 15:
	case 18:
	case 20:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_RAINNY;				//비
		break;
	case 10:
	case 12:
	case 17:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_THUNDERSTORM;		//뇌우
		break;
	case 5:
	case 8:
	case 14:
	case 19:
		nMtmWeatherIcon = IMG_ENUM_WEATHER_SNOW;				//눈
		break;
	default:
		printf("%s: Unknown WallPad Weather Icon(%d)\r\n", __func__, nWallPadIcon);
		break;
	}
}

void CSerialWallPad_HDT::CallContextReceiveProc(void *pParam)
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

void CSerialWallPad_HDT::RequestReserve(UCHAR cmd, UCHAR data)
{
	m_isRequestReserve = TRUE;
	m_cmdRequestReserve = cmd;
	m_dataRequestReserve = data;
}

void CSerialWallPad_HDT::ResponseProc_WeatherRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_ParkingRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_ElevatorRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_ElevatorCall(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_WallPadReq(UCHAR* pBuffer)
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
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=Success, 0x01=Fail
*/
//	post_event(GR_EVENT_TYPE_USER, EVT_WALLPAD_DATA_RECEIVE, SNS_CMD_LAMP_ONOFF_REQ, 0);

	if(pBuffer[3]==0)	//Success
	{
	#if 0
		CallContextReceiveProc(pBuffer);
		m_isWaitForResponse = FALSE;
	#else	//일괄소등 요청이 성공한 뒤에 월패드 상태를 다시 요청해서 그 결과로 판단함
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

void CSerialWallPad_HDT::ResponseProc_SecurityReq(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_SecurityRes(UCHAR* pBuffer)
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

void CSerialWallPad_HDT::ResponseProc_GasStatRes(UCHAR* pBuffer)
{
/*
	항목			길이	내용
	--------------- ------- --------------------------------------------------------------------------------
	Status			1		0x00=열렴, 0x01=닫힘
*/
	//가스밸브상태를 설정공간으로 복사
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
		//select이후 변수가 초기화되므로 Read Set를 다시 해줘야 함
		FD_ZERO(&fdRead);
		FD_SET(pThis->m_fdDev, &fdRead);

		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		//아래에서 continue로 루프반복될 경우 대비
		if(pThis->m_isRecv) pThis->m_isRecv = FALSE;

		ret = select(pThis->m_fdDev+1, &fdRead, NULL, NULL, &timeout);
		if(FD_ISSET(pThis->m_fdDev, &fdRead))	//Receive
		{
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

			// 유효한 Command인지 확인 ( 0x1~0xD, 0x5A, 0x41 )
			if( !( ((cmd>0) && (cmd<=0xD)) || (cmd==SNS_CMD_INIT_DATA) || (cmd==SNS_CMD_FREE_DATA) ) ) continue;

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
		//		pThis->ResponseProc_ElevatorRes(buffer);
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


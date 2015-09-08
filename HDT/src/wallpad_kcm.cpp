#include "common.h"
#include "context_setup.h"
#include "main.h"

//
// Construction/Destruction
//
CWallPadKcm::CWallPadKcm()
{
	m_isRecv = FALSE;
	m_nRetry = 0;
}

CWallPadKcm::~CWallPadKcm()
{
}

//
// Interface Function
//
BOOL CWallPadKcm::Run()
{
	BOOL fRet = FALSE;
	struct termios tio;

	// 9600bps, 8bit, 1stopbit, non-parity
	if( (fRet = m_serial.Open(UART1_DEV, B9600)) == FALSE )
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Open Failure\r\n");
		return FALSE;
	}

	//for RS-485
	ioctl( m_serial.m_fdDev, 0x54F0, 0x0001 );
    tcflush  ( m_serial.m_fdDev, TCIFLUSH );

	//Listener Start
	if((fRet = m_serial.StartListener(SerialListener_KCM, this)) == FALSE)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] StartListener Failure\r\n");
	}

	return fRet;
}

void CWallPadKcm::Stop()
{
	m_serial.StopListener();

	m_serial.Close();
}

//날씨정보요청
#define WEATHER_REQ_APPLY

void CWallPadKcm::RequestWeatherInfo()
{
	KCM_PACKET packet = {0,};

#ifndef WEATHER_REQ_APPLY	//이전방식, 일괄요청사용
	if( (m_packet.opcode != KCM_OP_BATCH_REQ) && !CHK_FLAG(m_packet.data[0], KCM_BATCH_REQ_ALL) )
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_WALLPAD;			//0x01
		packet.adl			= 0;
		packet.ash			= KCM_DEV_MTM;				//0x53
		packet.asl			= 0;
		packet.opcode		= KCM_OP_BATCH_REQ;			//0x00
		packet.data[0]		= KCM_BATCH_REQ_ALL;		//날씨,주차,현관문 일괄요청
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
#else
	if( m_packet.opcode != KCM_OP_WEATHER_REQ )
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_WALLPAD;			//0x01
		packet.adl			= 0;
		packet.ash			= KCM_DEV_MTM;				//0x53
		packet.asl			= 0;
		packet.opcode		= KCM_OP_WEATHER_REQ;		//0x31
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
#endif
}

//가스상태요청
void CWallPadKcm::RequestGasStatus()
{
}

//가스차단요청
void CWallPadKcm::RequestGasCut()
{
	KCM_PACKET packet = {0,};

	if(m_packet.opcode != KCM_OP_GAS_CUT_REQ)
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_GAS_VALVE;		//0x2C
		packet.adl			= 0;
		packet.ash			= KCM_DEV_USS;				//0x54
		packet.asl			= 0;
		packet.opcode		= KCM_OP_GAS_CUT_REQ;		//0x02
		packet.data[0]		= 0;
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
}

//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)
void CWallPadKcm::RequestLightSet(BOOL isAllOff)
{
	KCM_PACKET packet = {0,};

	if(m_packet.opcode != KCM_OP_ALL_OFF_REQ)
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_LIGHT;			//0x0E
		packet.adl			= 0xff;						//전등 전체
		packet.ash			= KCM_DEV_USS;				//0x54
		packet.asl			= 0;
		packet.opcode		= KCM_OP_ALL_OFF_REQ;		//0x00
		if(isAllOff)
			memset(packet.data, 0x00, KCM_DATA_SIZE);
		else
			memset(packet.data, 0xff, KCM_DATA_SIZE);
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
}

//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)
void CWallPadKcm::RequestSecuritySet(BOOL isSecurityOn)
{
	KCM_PACKET packet = {0,};

	if( (m_packet.opcode != KCM_OP_SECURITY_ON) && (m_packet.opcode != KCM_OP_SECURITY_OFF) )
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_WALLPAD;			//0x01
		packet.adl			= 0;
		packet.ash			= KCM_DEV_USS;				//0x54
		packet.asl			= 0;
		packet.opcode		= (isSecurityOn) ? KCM_OP_SECURITY_ON : KCM_OP_SECURITY_OFF;	//ON=0x11, OFF=0x12
		packet.data[0]		= (isSecurityOn) ? 2 : 0;	//D0: U보안			0=사용안함, 1=U보안사용대기, 2=사용
		packet.data[1]		= (isSecurityOn) ? 1 : 0;	//D1: 가스차단여부	0=차단안함, 1=차단
		packet.data[2]		= (isSecurityOn) ? 1 : 0;	//D2: 전등차단여부	0=차단안함, 1=차단
		packet.data[3]		= (isSecurityOn) ? 3 : 3;	//D3: 난방차단여부	0=차단안함, 1=난방OFF, 2=외출
		packet.data[4]		= (isSecurityOn) ? 3 : 3;	//D4: 대기전력차단	0=차단안함, 1=차단
		packet.data[5]		= (isSecurityOn) ? 1 : 0;	//D5: 엘리베이터호출	0=호출안함, 1=차단
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
}

//엘레베이터 호출
void CWallPadKcm::RequestElevatorCall()
{
	KCM_PACKET packet = {0,};

	if(m_packet.opcode != KCM_OP_ELEVATOR_CALL)
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_ELEVATOR;			//0x44
		packet.adl			= 0;
		packet.ash			= KCM_DEV_USS;				//0x54
		packet.asl			= 0;
		packet.opcode		= KCM_OP_ELEVATOR_CALL;		//0x01
		packet.data[0]		= 0;
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
}

//엘레베이터 상태요청
void CWallPadKcm::RequestElevatorStatus()
{
}

//주차위치정보요청
void CWallPadKcm::RequestParkingInfo()
{
	KCM_PACKET packet = {0,};

	if( (m_packet.opcode != KCM_OP_BATCH_REQ) && !CHK_FLAG(m_packet.data[0], KCM_BATCH_REQ_PARKING) )
	{
		m_nRetry = 0;
	}

	if(m_nRetry < 3)
	{
		packet.preamble		= KCM_PRE;
		packet.hd			= KCM_HD;
		packet.cc			= KCM_CC_ACK_IS | m_nRetry++;
		packet.pcnt			= 0;
		packet.adh			= KCM_DEV_WALLPAD;			//0x01
		packet.adl			= 0;
		packet.ash			= KCM_DEV_MTM;				//0x53
		packet.asl			= 0;
		packet.opcode		= KCM_OP_BATCH_REQ;			//0x00
		packet.data[0]		= KCM_BATCH_REQ_PARKING;	//0x02
		packet.fcc			= CalcCheckSum((UCHAR *)&packet.hd, 16);
		packet.eot			= KCM_EOT;

		if(Write((UCHAR*)&packet, sizeof(KCM_PACKET)) > 0)
		{
			g_timer.SetTimer(RESPONSE_TIMER, 1, NULL, "Response Timer");
		}
	}
	else
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
		g_message.SendMessage(MSG_TIMER_EVENT, RETRY_TIMEOUT);
	}
}


//
// Member Function
//
int CWallPadKcm::Write(UCHAR *pData, int size)
{
	CContextSetup* pContextSetup = NULL;
	int ret = ERROR;
	ULONG ulTick, delay;

	if(m_serial.IsOpen())
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Try To Write\r\n");
		DBGDMP(DBG_WALLPAD, pData, size, TRUE);

		//수신중일경우 대기
		ulTick = GetTickCount();
		while(m_isRecv) 
		{ 
			usleep(1000); 
			if(GetElapsedTick(ulTick) >= 300)
			{
				DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Wait for read done: Timeout!!\r\n");
				return -1;
			}
		}

		// 485 Enable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, LOW);

		// TX Enable후 최초 패킷을 전송하기까지 1.5[ms] 지연
		usleep(10);

		ret = m_serial.Write(pData, size);
		if(ret == size)
		{
			memcpy((UCHAR *)&m_packet, pData, sizeof(KCM_PACKET));
		}
		else
		{
			DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Write Bytes Error, write=%d, expect=%d\e\n", ret, size);
		}

		//마지막 패킷 전송후 TX Disable 하기까지 지연 (패킷사이즈 전송시간 + 1.5[ms])
		delay = (size * 1042) + 10;	// 9600[bps]에서 1start+8bit+1stop 전송속도는 1.04167[ms] 이므로 21 byte는 21.875[ms]
		usleep(delay);

		// 485 Disable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, HIGH);

	#if 1
		if(g_isContextSetupWallPad)
		{
			pContextSetup = (CContextSetup*)g_state.GetCurrContext();
			if(pContextSetup)
			{
				pContextSetup->PrintPacket(pData, size, CYAN, FALSE);
			}
		}
	#endif
	}

	return ret;
}

/*
UCHAR CWallPadKcm::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	UCHAR cs = 0;

	if(pBuffer)
	{
		while(size--) cs += *pBuffer++;
	}

	return cs;
}
*/

//
// Thread(SerialListener) Context
//
void* CWallPadKcm::SerialListener_KCM(void *pParam)
{
	CWallPadKcm *pThis = (CWallPadKcm *)pParam;
	CSerial *pSerial = &pThis->m_serial;

	struct timeval timeout;
	fd_set fdRead;
//	ULONG ulTick;
	int ret;

	DBGMSG(DBG_WALLPAD, "%s: Start\r\n", __func__);

	while(pSerial->m_fListenerRunning)
	{
		//select이후 변수가 초기화되므로 Read Set를 다시 해줘야 함
		FD_ZERO(&fdRead);
		FD_SET(pSerial->m_fdDev, &fdRead);

		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		ret = select(pSerial->m_fdDev+1, &fdRead, NULL, NULL, &timeout);

		if(FD_ISSET(pSerial->m_fdDev, &fdRead))
		{
			pThis->RecvDataProc();
		}
	}

	DBGMSG(DBG_WALLPAD, "%s: End\r\n", __func__);

	pthread_exit(NULL); 
}

void CWallPadKcm::RecvDataProc()
{
//	UCHAR buffer[KCM_PACKET_SIZE] = {0,};
	UCHAR buffer[MAX_RECV_BUFFER] = {0,};
	int recv_bytes = 0, byte_to_read, ret;
	UCHAR fcc_calc = 0;
	PKCM_PACKET pPacket = (PKCM_PACKET)&buffer[0];

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER			pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_WEATHER	pMtmDataWeather  = (PMTM_DATA_WEATHER)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_PARKING	pMtmDataParking  = (PMTM_DATA_PARKING)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_ELEVATOR	pMtmDataElevator = (PMTM_DATA_ELEVATOR)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_GAS		pMtmDataGas		 = (PMTM_DATA_GAS)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_LIGHT		pMtmDataLight	 = (PMTM_DATA_LIGHT)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_SECURITY	pMtmDataSecurity = (PMTM_DATA_SECURITY)&buffer_mtm[sizeof(MTM_HEADER)];

	CContextSetup* pContextSetup = NULL;

	time_t now;
	struct tm *t;
	struct timeval tvNew;
	ULONG ulTick, color;

	BOOL isMessage = FALSE;
	UCHAR adh, adl;
	int i, temperature, index;

	m_isRecv = TRUE;

#if 0
	ulTick = GetTickCount();
	while(recv_bytes < KCM_PACKET_SIZE)
	{
		recv_bytes += m_serial.Read(&buffer[recv_bytes], KCM_PACKET_SIZE-recv_bytes);

		if(GetElapsedTick(ulTick) >= 500)
		{
			DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Read Timeout\r\n");
			DBGDMP(DBG_WALLPAD, buffer, recv_bytes, TRUE);
			return;
		}
	}
#else
	//Preamble Search
	ret = m_serial.Read(&buffer[0], 2);
	while(pPacket->preamble != KCM_PRE)
	{
		if(ret <= 0)
		{
			m_isRecv = FALSE;
			return;
		}
		buffer[0] = buffer[1];
		ret = m_serial.Read(&buffer[1], 1);
	}
	recv_bytes = 2;
	//Preamble이후의 데이타 수신
	ulTick = GetTickCount();
	while(recv_bytes < KCM_PACKET_SIZE)
	{
		recv_bytes += m_serial.Read(&buffer[recv_bytes], KCM_PACKET_SIZE-recv_bytes);

		if(GetElapsedTick(ulTick) >= 200)
		{
			DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Read Timeout: %d bytes\r\n", recv_bytes);
			DBGDMP(DBG_WALLPAD, buffer, recv_bytes, TRUE);
			m_isRecv = FALSE;
			return;
		}
	}
#endif

	m_isRecv = FALSE;

	DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Read %d bytes\r\n", recv_bytes);
	DBGDMP(DBG_WALLPAD, buffer, recv_bytes, TRUE);

	//패킷형식 검사 (HD필드는 체크 생략)
	if( (pPacket->preamble != KCM_PRE) || (pPacket->eot != KCM_EOT) )
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Invalid Format\r\n");
		return;
	}

	//체크섬 검사
	fcc_calc = CalcCheckSum(&buffer[2], 16);
	if(fcc_calc != pPacket->fcc)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Invalid FCC, calc=0x%x, data=0x%x\r\n", fcc_calc, pPacket->fcc);
		return;
	}

	//목적지 검사 : WallPad 및 USS Device들에서 MTM으로 전송된 패킷만 처리
	if( (pPacket->adh != KCM_DEV_MTM) && (pPacket->adh != KCM_DEV_USS) )
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Out of Destination\r\n");
		return;
	}

	//응답패킷체크
	if( CHK_FLAG(pPacket->cc, KCM_CC_ACK) )
	{
		m_nRetry = 0;
		g_timer.KillTimer(RESPONSE_TIMER);
	}

	pMtmHdr->stx = MTM_PACKET_STX;

	//명령코드 검사
	if( pPacket->adh == KCM_DEV_MTM )	//MTM 프로토콜
	{
		switch(pPacket->opcode)
		{
		// MTM 프로토콜
		case KCM_OP_BATCH_REQ:				//일괄요청(0x00)
			DBGMSG(DBG_WALLPAD, "--> KCM_OP_BATCH_REQ\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_ACK;
			pMtmHdr->len  = 0;
			buffer_mtm[sizeof(MTM_HEADER)] = MTM_PACKET_ETX;
			isMessage = TRUE;
			break;
		case KCM_OP_ALIVE_CHECK:			//Alive(0x3A)
			DBGMSG(DBG_WALLPAD, "--> KCM_OP_ALIVE_CHECK\r\n");
			//D0 시 : 0~23, 0xff=시간정보 알수없음
			//D1 분 : 0~59, 0xff=시간정보 알수없음
			//D2 초 : 0~59, 0xff=시간정보 알수없음
			//D3 현재온도 : 영상(0~100도), 영하(-1~-100도), 0x80=알수없음
			if( (pPacket->data[0] != 0xff) && (pPacket->data[1] != 0xff) && (pPacket->data[2] != 0xff) )
			{
				//시간갱신(?)
				if(g_isTimeSync==FALSE)
				{
					g_isTimeSync=TRUE;

					now = time(NULL);
					t = localtime(&now);

					t->tm_hour = pPacket->data[0];
					t->tm_min  = pPacket->data[1];
					t->tm_sec  = pPacket->data[2];

					tvNew.tv_sec  = mktime(t);
					tvNew.tv_usec = 0;

					if(settimeofday(&tvNew, NULL)==ERROR)
					{
						DBGMSG(DBG_WALLPAD, "[WallPad:KCM] settimeofday: error : %d %s\r\n", errno, strerror(errno));
					}
					else
					{
						system("hwclock -w");
						g_timer.Refresh();
						DBGMSG(DBG_WALLPAD, "[WallPad:KCM] settimeofday: done\r\n");
					}
				}
			}
			if( pPacket->data[3] != 0x80 )
			{
				//온도갱신
			//	g_setup_data.m_SetupData.current_temp = (int)(char)pPacket->data[3];
				if(pPacket->data[3] == 0x80)
					temperature = 0;
				else if(pPacket->data[3] > 0x80)
					temperature = (pPacket->data[3] - 256);
				else
					temperature = (char)pPacket->data[3];
					
				g_setup_data.m_SetupData.temp_low_right  = temperature;
				g_setup_data.m_SetupData.temp_high_right = temperature;
			}
			break;
		case KCM_OP_WEATHER_INFO:			//시간별 날씨(0x01)
			DBGMSG(DBG_WALLPAD, "--> KCM_OP_WEATHER_INFO\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_WEATHER;
			pMtmHdr->len  = sizeof(MTM_DATA_WEATHER);

			now = time(NULL);
			t = localtime(&now);

			//D0 : 현재시간
			pMtmDataWeather->year	= t->tm_year + 1900;
			pMtmDataWeather->month	= t->tm_mon + 1;
			pMtmDataWeather->day	= t->tm_mday;
			if(pPacket->data[0] < 24)
				pMtmDataWeather->hour = pPacket->data[0];	// 0~23, 0xff=시간정보알수없음
			else
				pMtmDataWeather->hour = t->tm_hour;
			pMtmDataWeather->minute	= t->tm_min;
			pMtmDataWeather->second	= t->tm_sec;

			//D1 : 날씨종류
			switch(pPacket->data[1])
			{
			case 0x01:	//맑음
				pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_SERENITY;	//맑음
				break;
			case 0x02:	//구름조금
				pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_PARTLY_CLOUDY;	//구름조금
				break;
			case 0x03:	//구름많음
			case 0x04:	//흐림
				pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_CLOUDY;	//흐림
				break;
			case 0x05:	//비
			case 0x06:	//비 또는 눈
				pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_RAINNY;	//비
				break;
			case 0x07:	//눈
				pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_SNOW;		//맑음
				break;
			}

			//D2 : 온도		0x00~0x64	영상(0~100도)
			//				0xFF~0x9C	영하(-1~-100도)
			//				0x80		알수없음

			//부호 캐스팅이 안됨!! Y!
			if(pPacket->data[2] == 0x80)
				temperature = 0;
			else if(pPacket->data[2] > 0x80)
				temperature = (pPacket->data[2] - 256);
			else
				temperature = (char)pPacket->data[2];
				
			pMtmDataWeather->temp2_low  = temperature;
			pMtmDataWeather->temp2_high = temperature;

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_WEATHER)] = MTM_PACKET_ETX;

			isMessage = TRUE;

		//	월패드가 시간마다 주는 데이타만 쉬프트 시킴
			if(CHK_FLAG(pPacket->cc, KCM_CC_ACK_IS))
			{
				g_setup_data.m_SetupData.weather_left    = g_setup_data.m_SetupData.weather_right;
				g_setup_data.m_SetupData.temp_low_left   = g_setup_data.m_SetupData.temp_low_right;
				g_setup_data.m_SetupData.temp_high_left  = g_setup_data.m_SetupData.temp_high_right;
			}

			g_setup_data.m_SetupData.weather_right   = pMtmDataWeather->weather2;
			g_setup_data.m_SetupData.temp_low_right  = pMtmDataWeather->temp2_low  * 10;
			g_setup_data.m_SetupData.temp_high_right = pMtmDataWeather->temp2_high * 10;

		//	DBGMSG(1, "Temp: 0x%02x --> %d --> %d --> %d\r\n", pPacket->data[2], temperature, pMtmDataWeather->temp2_high, g_setup_data.m_SetupData.temp_high_right);

			break;
		case KCM_OP_WEATHER_REQ:			//시간변 날씨조회(0x31)
			DBGMSG(DBG_WALLPAD, "--> KCM_OP_WEATHER_REQ\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_ACK;
			pMtmHdr->len  = 0;
			buffer_mtm[sizeof(MTM_HEADER)] = MTM_PACKET_ETX;
			isMessage = TRUE;
			break;
		case KCM_OP_PARKING_INFO:			//주차정보(0x08)
			DBGMSG(DBG_WALLPAD, "--> KCM_OP_PARKING_INFO\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_PARKING;
			pMtmHdr->len  = sizeof(MTM_DATA_PARKING);

			//D0 : 차량등록번호
			if(pPacket->data[0]==0)
				pMtmDataParking->status = MTM_DATA_PARKING_STATUS_NOINFO;
			else
				pMtmDataParking->status = MTM_DATA_PARKING_STATUS_IN;

			sprintf(pMtmDataParking->car_num, "%d\0", pPacket->data[0]);	//차량번호

			//D1~D7 : 주차위치정보
			memcpy(pMtmDataParking->floor, &pPacket->data[1], 2);
			memcpy(pMtmDataParking->zone,  &pPacket->data[3], 5);

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_PARKING)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			if (pMtmDataParking->status == MTM_DATA_PARKING_STATUS_IN) {
				g_setup_data.m_SetupData.status = pMtmDataParking->status;
				memcpy(g_setup_data.m_SetupData.car_floor, pMtmDataParking->floor,   MAX_PARKING_FLOOR_NAME);
				memcpy(g_setup_data.m_SetupData.car_zone,  pMtmDataParking->zone,    MAX_PARKING_ZONE_NAME);
				memcpy(g_setup_data.m_SetupData.car_num,   pMtmDataParking->car_num, MAX_PARKING_CAR_NUM);
				parking_list_update(pMtmDataParking);	
			}

			break;
		case KCM_OP_DOOR_INFO:				//현관문 출입정보(0x10)
			DBGMSG(DBG_WALLPAD, "--> KCM_OP_DOOR_INFO\r\n");
			break;
		}
	}
	else	// USS프로토콜
	{
		switch(pPacket->ash)
		{
		case KCM_DEV_WALLPAD:				//0x01, 월패드(보안)
			DBGMSG(DBG_WALLPAD, "--> KCM_DEV_WALLPAD\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_SECURITY;
			pMtmHdr->len  = sizeof(MTM_DATA_SECURITY);

			if(pPacket->opcode == 0x11)		//외출
			{
				pMtmDataSecurity->status = MTM_DATA_SECURITY_ON;
				g_setup_data.m_SetupData.security_stat = 1;
			}
			else if(pPacket->opcode == 0x12)	//외출해제
			{
				pMtmDataSecurity->status = MTM_DATA_SECURITY_OFF;
				g_setup_data.m_SetupData.security_stat = 0;
			}
			else							//에러
			{
				pMtmDataSecurity->status = MTM_DATA_SECURITY_ERROR;
			}

			//D0 : U보안   0=사용안함, 1=사용대기, 2=사용
			// --> 적용여부 확인필요!

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_SECURITY)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_DEV_GAS_VALVE:				//0x2C, 가스밸브
			DBGMSG(DBG_WALLPAD, "--> KCM_DEV_WALLPAD\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_GAS;
			pMtmHdr->len  = sizeof(MTM_DATA_GAS);

			if(pPacket->opcode == 0x01)		//밸브열림
			{
				pMtmDataGas->status = MTM_DATA_GAS_OPEN;
				g_setup_data.m_SetupData.gas_stat = 0;
			}
			else if(pPacket->opcode == 0x02)	//밸브잠김
			{
				pMtmDataGas->status = MTM_DATA_GAS_CUT;
				g_setup_data.m_SetupData.gas_stat = 1;
			}
			else							//에러
				pMtmDataGas->status = MTM_DATA_GAS_ERROR;

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_GAS)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_DEV_LIGHT:					//0x0E, 전등
			DBGMSG(DBG_WALLPAD, "--> KCM_DEV_LIGHT\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_LIGHT;
			pMtmHdr->len  = sizeof(MTM_DATA_LIGHT);

		//	DBGMSG(1, "LIGHT RES DATA: ");
			if(pPacket->opcode == 0x00)		//제어
			{
				pMtmDataLight->status = MTM_DATA_LIGHT_OFF;			//소등
				for(i=0; i<7; i++)
				{
					if(pPacket->data[i])
					{
					//	DBGMSG(1, "[%d]%02x ", i, pPacket->data[i]);
						pMtmDataLight->status = MTM_DATA_LIGHT_ON;	//점등
						break;
					}
				}

				g_setup_data.m_SetupData.light_stat = (pMtmDataLight->status==MTM_DATA_LIGHT_OFF) ? 1 : 0;	//설정: 0=소등해제, 1=소등

			//	DBGMSG(1, "\r\nStatus: 0x%x -> 0x%x\r\n", pMtmDataLight->status, g_setup_data.m_SetupData.light_stat);

			//	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (g_setup_data.m_SetupData.light_stat) ? LOW : HIGH);	//LOW=소등, HIGH=소등해제
			}
			else							//에러
			{
				pMtmDataLight->status = MTM_DATA_LIGHT_ERROR;
			//	DBGMSG(1, "Status: Error\r\n");
			}

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_LIGHT)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_DEV_ELEVATOR:				//0x44, 엘리베이터
			DBGMSG(DBG_WALLPAD, "--> KCM_DEV_ELEVATOR\r\n");
			pMtmHdr->type = MTM_DATA_TYPE_ELEVATOR;
			pMtmHdr->len  = sizeof(MTM_DATA_ELEVATOR);

			//OP: 0x01=호출, 0xff=호출실패 --> 사용안함

			//D0: 방향
			switch(pPacket->data[0])
			{
			case 0x00:	//정지
				pMtmDataElevator->status = MTM_DATA_EV_STATUS_STOP;
				break;
			case 0x01:	//하향
				pMtmDataElevator->status = MTM_DATA_EV_STATUS_DOWN;
				break;
			case 0x02:	//상향
				pMtmDataElevator->status = MTM_DATA_EV_STATUS_UP;
				break;
			case 0x03:	//도착
				pMtmDataElevator->status = MTM_DATA_EV_STATUS_ARRIVE;
				break;
			default:
				pMtmDataElevator->status = MTM_DATA_EV_STATUS_ERROR;
				break;
			}

			//D1: 현재층
			pMtmDataElevator->floor = (char)(pPacket->data[1]);

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_ELEVATOR)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			g_setup_data.m_SetupData.elevator_status = pMtmDataElevator->status;
			g_setup_data.m_SetupData.elevator_floor  = pMtmDataElevator->floor;

			break;
		case KCM_DEV_BOILER:				//0x36, 보일러
			DBGMSG(DBG_WALLPAD, "--> KCM_DEV_BOILER\r\n");
			break;
		case KCM_DEV_OUTLET:				//0x3B, 콘센트
			DBGMSG(DBG_WALLPAD, "--> KCM_DEV_OUTLET\r\n");
			break;
		}
	}

	if(g_isContextSetupWallPad)
	{
		pContextSetup = (CContextSetup*)g_state.GetCurrContext();
		if(pContextSetup)
		{
			switch(pPacket->adh)
			{
			case KCM_DEV_MTM:
			case KCM_DEV_USS:
				color = LTCYAN;
				break;
			default:
				color = LTGREEN;
				break;
			}
			pContextSetup->PrintPacket(buffer, KCM_PACKET_SIZE, color);
		}
	}

	//ACK확인명령
	if( CHK_FLAG(pPacket->cc, KCM_CC_ACK_IS) )
	{
		usleep(10000);	//10[ms] 이후 응답전송

		//목적지/출발지 바꿔고 FCC 다시계산해서 응답
		adh = pPacket->adh;
		adl = pPacket->adl;
		pPacket->cc  = KCM_CC_ACK;
		pPacket->adh = pPacket->ash;
		pPacket->adl = pPacket->asl;
		pPacket->ash = adh;
		pPacket->asl = adl;
		pPacket->fcc = CalcCheckSum(&buffer[2], 16);
		Write((UCHAR*)pPacket, sizeof(KCM_PACKET));
	}

	if(isMessage)
	{
		if(g_isContextSetupWallPad)
			g_message.SendMessageData(MSG_WALLPAD_DATA, buffer, KCM_PACKET_SIZE);
		else
			g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}


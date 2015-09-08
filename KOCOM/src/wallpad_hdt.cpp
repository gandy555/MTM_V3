#include "common.h"
#include "main.h"

//
// Construction/Destruction
//
CWallPadHdt::CWallPadHdt()
{
}

CWallPadHdt::~CWallPadHdt()
{
}

//
// Interface Function
//
BOOL CWallPadHdt::Run()
{
	BOOL fRet = FALSE;
	struct termios tio;

	// 9600bps, 8bit, 1stopbit, non-parity
	if( (fRet = m_serial.Open(UART1_DEV, B9600)) == FALSE )
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Open Failure\r\n");
		return FALSE;
	}

	//for RS-485
	ioctl( m_serial.m_fdDev, 0x54F0, 0x0001 );
    tcflush  ( m_serial.m_fdDev, TCIFLUSH );

	//Listener Start
	if((fRet = m_serial.StartListener(SerialListener_HDT, this)) == FALSE)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] StartListener Failure\r\n");
	}

	return fRet;
}

void CWallPadHdt::Stop()
{
	m_serial.StopListener();

	m_serial.Close();
}

//날씨정보요청
void CWallPadHdt::RequestWeatherInfo()
{
}

//가스차단요청
void CWallPadHdt::RequestGasCut()
{
}

//가스상태요청
void CWallPadHdt::RequestGasStatus()
{
}

//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)
void CWallPadHdt::RequestLightSet(BOOL isAllOff)
{
}

//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)
void CWallPadHdt::RequestSecuritySet(BOOL isSecurityOn)
{
}

//엘레베이터 호출
void CWallPadHdt::RequestElevatorCall()
{
}

//엘레베이터 상태요청
void CWallPadHdt::RequestElevatorStatus()
{
}

//주차위치정보요청
void CWallPadHdt::RequestParkingInfo()
{
}



//
// Member Function
//
int CWallPadHdt::Write(UCHAR *pData, int size)
{
	int ret = ERROR;

	if(m_serial.IsOpen())
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Write\r\n");
		DBGDMP(DBG_WALLPAD, pData, size, TRUE);

		//수신중일경우 대기
		while(m_isRecv) { usleep(1000); }

		// 485 Enable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, LOW);

		// TX Enable후 최초 패킷을 전송하기까지 1.5ms 지연
		usleep(1500);

		ret = m_serial.Write(pData, size);

		//마지막 패킷 전송후 TX Disable 하기까지 500us 지연
		usleep(500);

		// 485 Disable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, HIGH);
	}

	return ret;
}

UCHAR CWallPadHdt::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	UCHAR cs = 0;

	if(pBuffer)
	{
		while(size--) cs += *pBuffer++;
	}

	return cs;
}

//
// Thread(SerialListener) Context
//
void* CWallPadHdt::SerialListener_HDT(void *pParam)
{
	CWallPadHdt *pThis = (CWallPadHdt *)pParam;
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

void CWallPadHdt::RecvDataProc()
{
	UCHAR buffer[KCM_PACKET_SIZE] = {0,};
	UCHAR recv_bytes = 0, fcc_calc = 0;
	PKCM_PACKET pPacket = (PKCM_PACKET)&buffer[0];

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER			pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_WEATHER	pMtmDataWeather  = (PMTM_DATA_WEATHER)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_PARKING	pMtmDataParking  = (PMTM_DATA_PARKING)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_ELEVATOR	pMtmDataElevator = (PMTM_DATA_ELEVATOR)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_GAS		pMtmDataGas		 = (PMTM_DATA_GAS)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_LIGHT		pMtmDataLight	 = (PMTM_DATA_LIGHT)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_SECURITY	pMtmDataSecurity = (PMTM_DATA_SECURITY)&buffer_mtm[sizeof(MTM_HEADER)];

	time_t now;
	struct tm *t;
	BOOL isMessage = FALSE;

	m_isRecv = TRUE;

	while(recv_bytes < KCM_PACKET_SIZE)
	{
		recv_bytes += m_serial.Read(&buffer[recv_bytes], KCM_PACKET_SIZE-recv_bytes);
	}

	m_isRecv = FALSE;

	DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Read\r\n");
	DBGDMP(DBG_WALLPAD, buffer, recv_bytes, TRUE);

#if 0
	//패킷형식 검사
	if( (pPacket->preamble != KCM_PRE) || (pPacket->eot != KCM_EOT) )
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:KCM] Invalid Format\r\n");
		return;
	}

	//체크섬 검사
	fcc_calc = CalcByteCheckSum(&buffer[2], 16);
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

	pMtmHdr->stx = MTM_PACKET_STX;

	//명령코드 검사
	if( pPacket->adh == KCM_DEV_MTM )	//MTM 프로토콜
	{
		switch(pPacket->opcode)
		{
		// MTM 프로토콜
		case KCM_OP_BATCH_REQ:				//일괄요청(0x00)
			break;
		case KCM_OP_ALIVE_CHECK:			//Alive(0x3A)
			break;
		case KCM_OP_WEATHER_INFO:			//시간별 날씨(0x01)
			pMtmHdr->type = MTM_DATA_TYPE_WEATHER;
			pMtmHdr->len  = sizeof(MTM_DATA_WEATHER);

			now = time(NULL);
			t = localtime(&now);

			//D0 : 현재시간
			pMtmDataWeather->year	= t->tm_year + 1900;
			pMtmDataWeather->month	= t->tm_mon + 1;
			pMtmDataWeather->day	= t->tm_mday
			if(pPacket->data[0] < 24)
				pMtmDataWeather->hour = pPacket->data[0];	// 0~23, 0xff=시간정보알수없음
			else
				pMtmDataWeather->hour = t->tm_hour;
			pMtmDataWeather->minute	= t->tm_min;
			pMtmDataWeather->second	= t->tm_sec

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
				pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_SERENITY;	//맑음
				break;
			}

			//D2 : 온도		0x00~0x64	영상(0~100도)
			//				0xFF~0x9C	영하(-1~-100도)
			//				0x80		알수없음
			if(pPacket->data[2] == 0x80)
			{
				pMtmDataWeather->temp2_low  = 0;
				pMtmDataWeather->temp2_high = 0;
			}
			else 
			{
				pMtmDataWeather->temp2_low  = (short)(char)(pPacket->data[2]);
				pMtmDataWeather->temp2_high = (short)(char)(pPacket->data[2]);
			}

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_WEATHER)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_OP_PARKING_INFO:			//주차정보(0x08)
			pMtmHdr->type = MTM_DATA_TYPE_PARKING;
			pMtmHdr->len  = sizeof(MTM_DATA_PARKING);

			//D0 : 차량등록번호
			if(pPacket->data[0]==0)
				pMtmDataParking->status = MTM_DATA_PARKING_STATUS_NOINFO;
			else
				pMtmDataParking->status = MTM_DATA_PARKING_STATUS_IN;

			//D1~D7 : 주차위치정보
			memcpy(pMtmDataParking->zone, &pPacket->data[1], 7);

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_PARKING)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_OP_DOOR_INFO:				//현관문 출입정보(0x10)
			break;
		}
	}
	else	// USS프로토콜
	{
		switch(pPacket->ash)
		{
		case KCM_DEV_WALLPAD:				//월패드(보안)
			pMtmHdr->type = MTM_DATA_TYPE_SECURITY;
			pMtmHdr->len  = sizeof(MTM_DATA_SECURITY);

			if(pPacket->opcode = 0x11)		//외출
				pMtmDataSecurity->status = MTM_DATA_SECURITY_ON;
			else if(pPacket->opcode = 0x12)	//외출해제
				pMtmDataSecurity->status = MTM_DATA_SECURITY_OFF;
			else							//에러
				pMtmDataSecurity->status = MTM_DATA_SECURITY_ERROR;

			//D0 : U보안   0=사용안함, 1=사용대기, 2=사용
			// --> 적용여부 확인필요!

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_SECURITY)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_DEV_GAS_VALVE:				//가스밸브
			pMtmHdr->type = MTM_DATA_TYPE_GAS;
			pMtmHdr->len  = sizeof(MTM_DATA_GAS);

			if(pPacket->opcode = 0x01)		//밸브열림
				pMtmDataGas->status = MTM_DATA_GAS_OPEN;
			else if(pPacket->opcode = 0x02)	//밸브잠김
				pMtmDataGas->status = MTM_DATA_GAS_CUT;
			else							//에러
				pMtmDataGas->status = MTM_DATA_GAS_ERROR;

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_GAS)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_DEV_LIGHT:					//전등
			pMtmHdr->type = MTM_DATA_TYPE_LIGHT;
			pMtmHdr->len  = sizeof(MTM_DATA_LIGHT);

			if(pPacket->opcode = 0x00)		//제어
			{
				pMtmDataLight->status = MTM_DATA_LIGHT_OFF;
				for(i=0; i<7; i++)
				{
					if(pPacket->data[i])
					{
						pMtmDataLight->status = MTM_DATA_LIGHT_ON;
						break;
					}
				}
			}
			else							//에러
			{
				pMtmDataLight->status = MTM_DATA_LIGHT_ERROR;
			}

			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_LIGHT)] = MTM_PACKET_ETX;

			isMessage = TRUE;

			break;
		case KCM_DEV_ELEVATOR:				//엘리베이터
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

			break;
		case KCM_DEV_BOILER:				//보일러
			break;
		case KCM_DEV_OUTLET:				//콘센트
			break;
		}
	}

	if(isMessage)
	{
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
#endif
}


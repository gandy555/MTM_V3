#include "common.h"
#include "context_setup.h"
#include "main.h"

//
// Construction/Destruction
//
CWallPadHdt::CWallPadHdt()
{
	m_GasControlReserve = 0;
	m_LightControlReserve = 0;
	m_SecurityControlReserve = 0;
	m_ElevatorControlReserve = 0;

	m_isElevatorCall = FALSE;
	m_isElevatorArrive = FALSE;
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
	//별도요청없음
	//Event Sender의 주기적(1시간) 데이타를 수신해서 표시
}

//가스차단요청
void CWallPadHdt::RequestGasCut()
{
/*
		STX LEN VEN DEV TYPE SRV LOC CMD ARG1 ARG2 CRC ETX
요청	F7  0C  01  2A  02   43  11  01  1B   00   **  EE   
응답	F7  0C  01  2A  04   43  11  01  1B   01   **  EE   
*/
#if 0	//직접요청없음
	UCHAR buffer_req[HDT_MAX_PACKET+1] = {0,};
	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)&buffer_req[0];
	PHDT_PACKET_USS_ARG pUssArg = (PHDT_PACKET_USS_ARG)&buffer_req[sizeof(HDT_PACKET_HDR)];
	PHDT_PACKET_TAIL pTail = (PHDT_PACKET_TAIL)&buffer_req[sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)];

	pHeader->stx	= HDT_STX;
	pHeader->len	= sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)+sizeof(HDT_PACKET_TAIL);
	pHeader->ven	= HDT_VEN_UNIFY;
	pHeader->dev	= HDT_DEV_USS_SWITCH;
	pHeader->type	= HDT_TYPE_CONTROL;
	pHeader->srv	= HDT_SRV_GAS_CUT;
	pHeader->loc	= HDT_LOC_GROUP(1)|HDT_LOC_DEV(1);
	pHeader->cmd	= HDT_CMD_GAS_VALVE_CLOSE;

	pUssArg->dev	= HDT_DEV_GAS_SWITCH;
	pUssArg->cmd	= 0;

	pTail->crc		= CalcCheckSum(buffer_req, pHeader->len-2);
	pTail->etx		= HDT_ETX;

	if(Write(buffer_req, pHeader->len) <= 0)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Request Write Failure\r\n");
	}
#else
	m_GasControlReserve = HDT_CMD_GAS_VALVE_CLOSE;
#endif
}

//가스상태요청
void CWallPadHdt::RequestGasStatus()
{
	//별도요청없음.
	//USS의 폴링(5초이내)에 포함된 상태를 수신해서 표시하거나
	//개별응답을 수신해서 표시함
}

//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)
void CWallPadHdt::RequestLightSet(BOOL isAllOff)
{
/*
				STX LEN VEN DEV TYPE SRV LOC CMD ARG1 ARG2 CRC ETX
소등	요청	F7  0C  01  2A  02   40  11  01  19   00   **  EE 
		응답	F7  0C  01  2A  04   40  11  01  19   01   **  EE   

해제	요청	F7  0C  01  2A  02   40  11  02  19   00   **  EE
		응답	F7  0C  01  2A  04   40  11  02  19   02   **  EE   
*/
#if 0	//직접요청없음
	UCHAR buffer_req[HDT_MAX_PACKET+1] = {0,};
	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)&buffer_req[0];
	PHDT_PACKET_USS_ARG pUssArg = (PHDT_PACKET_USS_ARG)&buffer_req[sizeof(HDT_PACKET_HDR)];
	PHDT_PACKET_TAIL pTail = (PHDT_PACKET_TAIL)&buffer_req[sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)];

	pHeader->stx	= HDT_STX;
	pHeader->len	= sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)+sizeof(HDT_PACKET_TAIL);
	pHeader->ven	= HDT_VEN_UNIFY;
	pHeader->dev	= HDT_DEV_USS_SWITCH;
	pHeader->type	= HDT_TYPE_CONTROL;
	pHeader->srv	= HDT_SRV_POWER;
	pHeader->loc	= HDT_LOC_GROUP(1)|HDT_LOC_DEV(1);
	pHeader->cmd	= (isAllOff) ? HDT_CMD_LIGHT_OUT_ON : HDT_CMD_LIGHT_OUT_OFF;

	pUssArg->dev	= HDT_DEV_LIGHT_SWITCH;
	pUssArg->cmd	= 0;

	pTail->crc		= CalcCheckSum(buffer_req, pHeader->len-2);
	pTail->etx		= HDT_ETX;

	if(Write(buffer_req, pHeader->len) <= 0)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Request Write Failure\r\n");
	}
#else
	m_LightControlReserve = (isAllOff) ? HDT_CMD_LIGHT_OUT_ON : HDT_CMD_LIGHT_OUT_OFF;
#endif
}

//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)
void CWallPadHdt::RequestSecuritySet(BOOL isSecurityOn)
{
	//USS폴링 시점에 응답으로 제어함. 따라서 제어에 Delay가 있게됨.
	m_SecurityControlReserve = (isSecurityOn) ? HDT_CMD_SECURITY_ON : HDT_CMD_SECURITY_OFF;
//	g_setup_data.m_SetupData.security_stat = (isSecurityOn) ? 1 : 0;
}

//엘레베이터 호출
void CWallPadHdt::RequestElevatorCall()
{
/*
		STX LEN VEN DEV TYPE SRV LOC CMD ARG1 CRC ETX
요청	F7  0B  01  34  02   41  11  06  00   **  EE   
응답	F7  0B  01  34  04   41  11  06  06   **  EE   
*/
#if 0	//직접제어 없음
	UCHAR buffer_req[HDT_MAX_PACKET+1] = {0,};
	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)&buffer_req[0];
	UCHAR *pArg1 = &buffer_req[sizeof(HDT_PACKET_HDR)];
	PHDT_PACKET_TAIL pTail = (PHDT_PACKET_TAIL)&buffer_req[sizeof(HDT_PACKET_HDR)+1];

	pHeader->stx	= HDT_STX;
	pHeader->len	= sizeof(HDT_PACKET_HDR)+1+sizeof(HDT_PACKET_TAIL);
	pHeader->ven	= HDT_VEN_UNIFY;
	pHeader->dev	= HDT_DEV_ELV_SWITCH;
	pHeader->type	= HDT_TYPE_CONTROL;
	pHeader->srv	= HDT_SRV_ELV_CTRL;
	pHeader->loc	= HDT_LOC_GROUP(1)|HDT_LOC_DEV(1);
	pHeader->cmd	= HDT_CMD_ELV_DOWN;

	*pArg1 = 0;

	pTail->crc		= CalcCheckSum(buffer_req, pHeader->len-2);
	pTail->etx		= HDT_ETX;

	if(Write(buffer_req, pHeader->len) <= 0)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Request Write Failure\r\n");
	}
#else
	m_ElevatorControlReserve = HDT_CMD_ELV_DOWN;
#endif
}

//엘레베이터 상태요청
void CWallPadHdt::RequestElevatorStatus()
{
	//별도요청없음.
	//엘리베이터 스위치폴링에 포함된 상태를 표시함
}

//주차위치정보요청
void CWallPadHdt::RequestParkingInfo()
{
	//별도요청없음
	//입/출차시 Event Sender가 보내는 데이타를 수신해서 표시
}



//
// Member Function
//
int CWallPadHdt::Write(UCHAR *pData, int size)
{
	CContextSetup* pContextSetup = NULL;
	int ret = ERROR;
	ULONG ulTick, delay;

	if(m_serial.IsOpen())
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Write\r\n");
		DBGDMP(DBG_WALLPAD, pData, size, TRUE);

		//수신중일경우 대기
		ulTick = GetTickCount();
		while(m_isRecv) 
		{ 
			usleep(1000); 
			if(GetElapsedTick(ulTick) >= 300)
			{
				DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Wait for read done: Timeout!!\r\n");
				return -1;
			}
		}

		// 485 Enable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, LOW);

		// TX Enable후 최초 패킷을 전송하기까지 1.5ms 지연
		usleep(10);

		ret = m_serial.Write(pData, size);
		if(ret != size)
		{
			DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Write Bytes Error, write=%d, expect=%d\e\n", ret, size);
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

UCHAR CWallPadHdt::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	UCHAR cs = 0;

	if(pBuffer)
	{
		while(size--) cs ^= *pBuffer++;
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

		timeout.tv_sec  = SELECT_TIMEOUT_SEC;
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
	CContextSetup* pContextSetup = NULL;

	UCHAR buffer[HDT_MAX_PACKET+1] = {0,};
	UCHAR recv_bytes = 0, crc_calc = 0;

	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)&buffer[0];
	PHDT_PACKET_TAIL pTail = NULL;

	ULONG color;
	int ret;

	m_isRecv = TRUE;

	//헤더 3 bytes 읽기/검색
	ret = m_serial.Read(&buffer[0], 3);
	while( (pHeader->stx != HDT_STX) || ((pHeader->ven != HDT_VEN_UNIFY) && (pHeader->ven != 0)) )
	{
		if(ret <= 0)
		{
			m_isRecv = FALSE;
			return;
		}
		buffer[0] = buffer[1];
		buffer[1] = buffer[2];
		ret = m_serial.Read(&buffer[2], 1);
	}
	recv_bytes = 3;

	//나머지 패킷 읽기
	while(recv_bytes < pHeader->len)
	{
		recv_bytes += m_serial.Read(&buffer[recv_bytes], pHeader->len-recv_bytes);
	}

	m_isRecv = FALSE;

	//장치 검사
	if ((pHeader->dev != HDT_DEV_EVENT_SENDER) &&
		(pHeader->dev != HDT_DEV_USS_SWITCH) &&
		(pHeader->dev != HDT_DEV_ELV_SWITCH)) {
	//	DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Out of Device\r\n");
		return;
	}

	//check location
	if (pHeader->loc != 0x10) {
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Invalid Location\r\n");
		return;
	}

	DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Recv\r\n");
	DBGDMP(DBG_WALLPAD, buffer, recv_bytes, TRUE);

	pTail = (PHDT_PACKET_TAIL)&buffer[pHeader->len-2];

	//체크섬 검사 (STX~ARG[n])
	crc_calc = CalcCheckSum(&buffer[0], pHeader->len-2);
	if(crc_calc != pTail->crc)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Invalid CRC, calc=0x%x, data=0x%x\r\n", crc_calc, pTail->crc);
		return;
	}

	//장치별 처리
	switch(pHeader->dev)
	{
	case HDT_DEV_EVENT_SENDER:					//0x44, EventSender
		//EventSender가 보내는 브로드캐스팅 데이타만 처리
		if(pHeader->type == HDT_TYPE_EVENT)		//0x0C, Broadcating Data
		{
			switch(pHeader->srv)
			{
			case HDT_SRV_WEATHER_INFO:			//0x62, 날씨정보
				WeatherInfoProc(buffer);
				break;
			case HDT_SRV_PARKING_INFO:			//0x63, 주차정보
				ParkingInfoProc(buffer);
				break;
		//	case HDT_SRV_DOOR_SENS_INFO:		//0x68, USS일괄스위치
		//		break;
		//	case HDT_SRV_LIGHT_OUT_INFO:		//0x69, 일괄소등 설정/해제 정보
		//		break;
			}
		}
		else
		{
			DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Unexpected EventSender [TYPE]=0x%x\r\n", pHeader->type);
			return;
		}
		break;
	case HDT_DEV_USS_SWITCH:					//0x2A, USS Switch
		//월패드의 USS폴링 패킷인 경우 수신해서 상태응답
		if ((pHeader->type == HDT_TYPE_QUERY) || 
			(pHeader->type == HDT_TYPE_CONTROL)) {
			UssPollingProc(buffer);
		}
		break;
	case HDT_DEV_ELV_SWITCH:					//0x34, 엘리베이터 호출스위치
		//월패드의 호출스위치 폴링 패킷인 경우 수신해서 상태표시
		if(pHeader->type == HDT_TYPE_QUERY)
		{
			ElevatorPollingProc(buffer);
		}
		break;
	}

	if(g_isContextSetupWallPad)
	{
		pContextSetup = (CContextSetup*)g_state.GetCurrContext();
		if(pContextSetup)
		{
			switch(pHeader->type)
			{
			case HDT_TYPE_QUERY:
			case HDT_TYPE_EVENT:
				color = LTCYAN;
				break;
			default:
				color = LTGREEN;
				break;
			}
			pContextSetup->PrintPacket(buffer, pHeader->len, color);
		}
	}
}

void CWallPadHdt::WeatherInfoProc(UCHAR *pBuffer)
{
	if(pBuffer==NULL) return;

	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)pBuffer;
//	PHDT_PACKET_TAIL pTail = NULL;
	PHDT_PACKET_WEATHER_INFO pWeatherInfo = (PHDT_PACKET_WEATHER_INFO)&pBuffer[sizeof(HDT_PACKET_HDR)];

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_WEATHER	pMtmDataWeather  = (PMTM_DATA_WEATHER)&buffer_mtm[sizeof(MTM_HEADER)];

	struct tm t = {0,};
	struct timeval tvNew = {0,};
	int i, temperature;

	pMtmHdr->type = MTM_DATA_TYPE_WEATHER;
	pMtmHdr->len  = sizeof(MTM_DATA_WEATHER);

	//현재시간(arg1~arg6)
	pMtmDataWeather->year	= pWeatherInfo->year;
	pMtmDataWeather->month	= pWeatherInfo->month;
	pMtmDataWeather->day	= pWeatherInfo->day;
	pMtmDataWeather->hour	= pWeatherInfo->hour;
	pMtmDataWeather->minute	= pWeatherInfo->minute;
	pMtmDataWeather->second	= pWeatherInfo->second;

	t.tm_year = pWeatherInfo->year + 100;
	t.tm_mon  = pWeatherInfo->month - 1;
	t.tm_mday = pWeatherInfo->day;
	t.tm_hour = pWeatherInfo->hour;
	t.tm_min  = pWeatherInfo->minute;
	t.tm_sec  = pWeatherInfo->second;

	tvNew.tv_sec  = mktime(&t);
	tvNew.tv_usec = 0;

	if(settimeofday(&tvNew, NULL)==ERROR)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] settimeofday: error : %d %s\r\n", errno, strerror(errno));
	}
	else
	{
		system("hwclock -w");
		g_timer.Refresh();
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] settimeofday: done\r\n");
	}

	//날씨아이콘(arg7)
	switch(pWeatherInfo->weather)
	{
	case 1:		//맑음
	case 11:
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_SERENITY;	//맑음
		break;
	case 2:		//구름조금
	case 16:
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_PARTLY_CLOUDY;	//구름조금
		break;
	case 3:		//흐림
	case 9:
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_CLOUDY;	//흐림
		break;
	case 4:		//비
	case 6:
	case 13:
	case 15:
	case 18:
	case 20:
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_RAINNY;	//비
		break;
	case 5:		//눈
	case 8:
	case 14:
	case 19:
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_SNOW;		//눈
		break;
	case 7:		//소나기
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_SHOWER;	//소나기
		break;
	case 10:	//뇌우
	case 12:
	case 17:
		pMtmDataWeather->weather2 = IMG_ENUM_WEATHER_THUNDERSTORM;	//뇌우
		break;
	}

	//온도(arg8~arg10)
	if(pWeatherInfo->temp_under >= 10) pWeatherInfo->temp_under = 1;	//소숫점 1자리로 맞춤
	if(pWeatherInfo->temp_range)	//영하
	{
		pMtmDataWeather->temp2_low  = (pWeatherInfo->temp_int*(-10) - pWeatherInfo->temp_under);
	}
	else 
	{
		pMtmDataWeather->temp2_low  = (pWeatherInfo->temp_int*(10) + pWeatherInfo->temp_under);;
	}
	pMtmDataWeather->temp2_high = pMtmDataWeather->temp2_low;

	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_WEATHER)] = MTM_PACKET_ETX;

	//	월패드가 시간마다 주는 데이타만 쉬프트 시킴
	g_setup_data.m_SetupData.weather_left    = g_setup_data.m_SetupData.weather_right;
	g_setup_data.m_SetupData.temp_low_left   = g_setup_data.m_SetupData.temp_low_right;
	g_setup_data.m_SetupData.temp_high_left  = g_setup_data.m_SetupData.temp_high_right;

	g_setup_data.m_SetupData.weather_right   = pMtmDataWeather->weather2;
	g_setup_data.m_SetupData.temp_low_right  = pMtmDataWeather->temp2_low;
	g_setup_data.m_SetupData.temp_high_right = pMtmDataWeather->temp2_high;

	if(g_state.GetState() == STATE_WEATHER)
	{
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}

void CWallPadHdt::ParkingInfoProc(UCHAR *pBuffer)
{
	if(pBuffer==NULL) return;

	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)pBuffer;
//	PHDT_PACKET_TAIL pTail = NULL;
	PHDT_PACKET_PARKING_INFO pParkingInfo = (PHDT_PACKET_PARKING_INFO)&pBuffer[sizeof(HDT_PACKET_HDR)];

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER			pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_PARKING	pMtmDataParking  = (PMTM_DATA_PARKING)&buffer_mtm[sizeof(MTM_HEADER)];

	char szParkingZone[MAX_PARKING_FLOOR_NAME+MAX_PARKING_ZONE_NAME+1] = {0,};
	char szParkingZoneTemp[MAX_PARKING_FLOOR_NAME+MAX_PARKING_ZONE_NAME+1] = {0,};

	char *dump_ptr;
	
	int i, index;

	pMtmHdr->type = MTM_DATA_TYPE_PARKING;
	pMtmHdr->len  = sizeof(MTM_DATA_PARKING);

	//주차상태 : 입차만 확인
	pMtmDataParking->status = MTM_DATA_PARKING_STATUS_IN;

	//소문자->대문자 변환
	if(pParkingInfo->floor_name >= 0x61) pParkingInfo->floor_name -= 0x20;
	if(pParkingInfo->zone_name >= 0x61)  pParkingInfo->zone_name  -= 0x20;

	//층명칭
	if(pParkingInfo->floor_name)
		sprintf(pMtmDataParking->floor, "%c%d", pParkingInfo->floor_name, pParkingInfo->floor_num);
	else
		sprintf(pMtmDataParking->floor, "%d", pParkingInfo->floor_num);

	//구역명칭
	if(pParkingInfo->zone_name)
		sprintf(pMtmDataParking->zone, "%c%d", pParkingInfo->zone_name, pParkingInfo->zone_num);
	else
		sprintf(pMtmDataParking->zone, "%03d", pParkingInfo->zone_name, pParkingInfo->zone_num);

	// gandy 2014_12_26
	memcpy(pMtmDataParking->car_num, pParkingInfo->card_id, MAX_PARKING_CAR_NUM);
	
	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_PARKING)] = MTM_PACKET_ETX;

	if (pMtmDataParking->status == MTM_DATA_PARKING_STATUS_IN) {
		g_setup_data.m_SetupData.status = pMtmDataParking->status;
		memcpy(g_setup_data.m_SetupData.car_floor, pMtmDataParking->floor,   MAX_PARKING_FLOOR_NAME);
		memcpy(g_setup_data.m_SetupData.car_zone,  pMtmDataParking->zone,    MAX_PARKING_ZONE_NAME);
		memset(g_setup_data.m_SetupData.car_num, 0, MAX_PARKING_CAR_NUM);		//HDT는 차량번호 없음

		parking_list_update(pMtmDataParking);	
	}

	if( (g_state.GetState() == STATE_WEATHER) || (g_state.GetState() == STATE_PARKING) )
	{
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}

// 월패드의 USS폴링 요청에 대한 응답전송루틴
void CWallPadHdt::UssPollingProc(UCHAR *pBuffer)
{
	if(pBuffer==NULL) return;

	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)pBuffer;
	PHDT_PACKET_USS_ARG pUssArgRecv;

	UCHAR buffer_res[HDT_MAX_PACKET+1] = {0,};
	PHDT_PACKET_HDR pHeaderRes = (PHDT_PACKET_HDR)&buffer_res[0];
	PHDT_PACKET_TAIL pTailRes = NULL;
	PHDT_PACKET_USS_ARG pUssArgRes;

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER			pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_GAS		pMtmDataGas		 = (PMTM_DATA_GAS)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_LIGHT		pMtmDataLight	 = (PMTM_DATA_LIGHT)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_SECURITY	pMtmDataSecurity = (PMTM_DATA_SECURITY)&buffer_mtm[sizeof(MTM_HEADER)];

	UCHAR i, uss_dev_count = 0;
	UCHAR light_stat	= g_setup_data.m_SetupData.light_stat;
	UCHAR gas_stat		= g_setup_data.m_SetupData.gas_stat;
	UCHAR security_stat = g_setup_data.m_SetupData.security_stat;

	//수신패킷을 응답패킷 버퍼로 복사
	memcpy(buffer_res, pBuffer, pHeader->len);
	pHeaderRes->type = HDT_TYPE_RESPONSE;

	uss_dev_count = (pHeader->len - HDT_HEADER_SIZE - HDT_TAIL_SIZE) / 2;
	for(i=0; i<uss_dev_count; i++)
	{
		pUssArgRecv = (PHDT_PACKET_USS_ARG)&pBuffer[sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)*i];
		pUssArgRes  = (PHDT_PACKET_USS_ARG)&buffer_res[sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)*i];

		switch(pUssArgRecv->dev)
		{
		case HDT_DEV_USS_SWITCH:	//0x2A, USS일괄스위치 : 일괄상태
			//전체상태가 모두 차단상태일 경우만 차단으로 응답
			if(g_setup_data.m_SetupData.gas_stat && g_setup_data.m_SetupData.light_stat && g_setup_data.m_SetupData.security_stat)
				pUssArgRes->cmd = HDT_USS_ARG_VALUE_CUT_OFF;		//0x01=차단
			else
				pUssArgRes->cmd = HDT_USS_ARG_VALUE_CUT_RELEASE;	//0x02=차단해제
			break;
		case HDT_DEV_LIGHT_SWITCH:	//0x19, 일괄소등 스위치
			if(m_LightControlReserve)	//일괄소등제어 예약명령어가 있으면
			{
				pUssArgRes->cmd = m_LightControlReserve;
				m_LightControlReserve = 0;
			}
			else
			{
				// gandy 2015_01_07
				switch (pUssArgRecv->cmd) {
				case HDT_USS_ARG_VALUE_LIGHT_OUT_ON:
					light_stat = 1;	
					break;
				case HDT_USS_ARG_VALUE_LIGHT_OUT_OFF:
					light_stat = 0;
					break;
				default:
					if(light_stat)
						pUssArgRes->cmd = HDT_USS_ARG_VALUE_LIGHT_OUT_ON;
					else
						pUssArgRes->cmd = HDT_USS_ARG_VALUE_LIGHT_OUT_OFF;
					DBGMSG(DBG_WALLPAD, "[WallPad:UssPollingProc] Invalid Argument %d\r\n", pUssArgRecv->cmd);
					break;
				
				}
			}
			break;
		case HDT_DEV_GAS_SWITCH:	//0x1B, 가스차단 스위치
			if(m_GasControlReserve)		//가스제어 예약명령어가 있으면
			{
				pUssArgRes->cmd = m_GasControlReserve;
				m_GasControlReserve = 0;
			}
			else
			{
			#if 0
				if(g_setup_data.m_SetupData.gas_stat)
					pUssArgRes->cmd = HDT_CMD_GAS_VALVE_CLOSE;			//0x03=CLOSE
				else
					pUssArgRes->cmd = HDT_CMD_GAS_VALVE_OPEN;			//0x04=OPEN
			#else
				gas_stat = (pUssArgRecv->cmd == HDT_CMD_GAS_VALVE_CLOSE) ? 1 : 0;
			#endif
			}
			break;
		case HDT_DEV_SECURITY:		//0x16,	보안장비 (U-보안)
			// //0x0A=외출지연, 0x0B=입장지연 의 경우는 그대로 응답
			if(m_SecurityControlReserve)
			{
				pUssArgRes->cmd = m_SecurityControlReserve;
				m_SecurityControlReserve = 0;
			}
			else if( (pUssArgRecv->cmd == HDT_CMD_SECURITY_ON) || (pUssArgRecv->cmd == HDT_CMD_SECURITY_OFF) )
			{
			#if 0
				if(g_setup_data.m_SetupData.security_stat)
					pUssArgRes->cmd = HDT_CMD_SECURITY_ON;			//0x07=설정
				else
					pUssArgRes->cmd = HDT_CMD_SECURITY_OFF;			//0x08=해제
			#else
				if( (pUssArgRecv->cmd == HDT_CMD_SECURITY_ON) || (pUssArgRecv->cmd == HDT_CMD_SECURITY_OFF) )
				{
					security_stat = (pUssArgRecv->cmd == HDT_CMD_SECURITY_ON) ? 1 : 0;
				}
			#endif
			}
			break;
		case HDT_DEV_IDLE_POWER:	//0x2C, 대기전력					//0x01=ON, 0x02=OFF
			break;
		case HDT_DEV_OUTLET:		//0x1F, 콘센트						//0x01=ON, 0x02=OFF
			break;
		case HDT_DEV_BOILER:		//0x18, 난방기						//0x01=ON, 0x04=OFF(외출)
			break;
		case HDT_DEV_OUT_BUTTON:	//0x17, 외출버튼					//0x01=ON, 0x02=OFF
			break;
		}
	}

	pTailRes = (PHDT_PACKET_TAIL)&buffer_res[pHeaderRes->len - 2];
	pTailRes->crc = CalcCheckSum(buffer_res, pHeaderRes->len - 2);

	usleep(30000);	//30[ms] 이후 응답전송

	if(Write(buffer_res, pHeaderRes->len) <= 0)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Response Write Failure\r\n");
	}

	switch(g_state.GetState())
	{
	case STATE_WEATHER:
	case STATE_GAS:
		if(gas_stat != g_setup_data.m_SetupData.gas_stat)
		{
			g_setup_data.m_SetupData.gas_stat = gas_stat;

			pMtmHdr->type = MTM_DATA_TYPE_GAS;
			pMtmHdr->len  = sizeof(MTM_DATA_GAS);
			if(g_setup_data.m_SetupData.gas_stat)
				pMtmDataSecurity->status = MTM_DATA_GAS_CUT;
			else
				pMtmDataSecurity->status = MTM_DATA_GAS_OPEN;
			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_GAS)] = MTM_PACKET_ETX;
			g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
		}
		break;
	case STATE_LIGHT:
	#if 1	//일괄소등 화면전환은 버튼에 의해서
		if(light_stat != g_setup_data.m_SetupData.light_stat)
		{
			g_setup_data.m_SetupData.light_stat = light_stat;

			pMtmHdr->type = MTM_DATA_TYPE_LIGHT;
			pMtmHdr->len  = sizeof(MTM_DATA_LIGHT);
			if(g_setup_data.m_SetupData.light_stat)
				pMtmDataLight->status = MTM_DATA_LIGHT_OFF;
			else
				pMtmDataLight->status = MTM_DATA_LIGHT_ON;
			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_LIGHT)] = MTM_PACKET_ETX;
			g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
		}
	#endif
		break;
	case STATE_SECURITY:
		if(security_stat != g_setup_data.m_SetupData.security_stat)
		{
			g_setup_data.m_SetupData.security_stat = security_stat;

			pMtmHdr->type = MTM_DATA_TYPE_SECURITY;
			pMtmHdr->len  = sizeof(MTM_DATA_SECURITY);
			if(g_setup_data.m_SetupData.security_stat)
				pMtmDataSecurity->status = MTM_DATA_SECURITY_ON;
			else
				pMtmDataSecurity->status = MTM_DATA_SECURITY_OFF;
			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_SECURITY)] = MTM_PACKET_ETX;
			g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
		}
		break;
	case STATE_ELEVATOR:
		break;
	case STATE_PARKING:
		break;
	case STATE_SETUP:
		break;
	}

	g_setup_data.m_SetupData.gas_stat = gas_stat;
	g_setup_data.m_SetupData.light_stat = light_stat;
	g_setup_data.m_SetupData.security_stat = security_stat;

	//전체상태폴링시 제어상태가 바뀌어 올경우 해당 상태로 제어함
	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (light_stat==1) ? LOW : HIGH);	//LOW=소등, HIGH=소등해제

}

#if 0
void CWallPadHdt::UssControlProc(UCHAR *pBuffer)
{
	if(pBuffer==NULL) return;

	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)pBuffer;
//	PHDT_PACKET_TAIL pTail = NULL;
	PHDT_PACKET_USS_ARG pUssArgRecv;

	UCHAR buffer_res[HDT_MAX_PACKET+1] = {0,};
	PHDT_PACKET_HDR pHeaderRes = (PHDT_PACKET_HDR)&buffer_res[0];
	PHDT_PACKET_TAIL pTailRes = NULL;
	PHDT_PACKET_USS_ARG pUssArgRes;

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER			pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_GAS		pMtmDataGas		 = (PMTM_DATA_GAS)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_LIGHT		pMtmDataLight	 = (PMTM_DATA_LIGHT)&buffer_mtm[sizeof(MTM_HEADER)];
	PMTM_DATA_SECURITY	pMtmDataSecurity = (PMTM_DATA_SECURITY)&buffer_mtm[sizeof(MTM_HEADER)];

	UCHAR i, uss_dev_count = 0;
	UCHAR light_stat	= g_setup_data.m_SetupData.light_stat;
	UCHAR gas_stat		= g_setup_data.m_SetupData.gas_stat;
	UCHAR security_stat = g_setup_data.m_SetupData.security_stat;
	BOOL isResponse = FALSE;

	//수신패킷을 응답패킷 버퍼로 복사
	memcpy(buffer_res, pBuffer, pHeader->len);
	pHeaderRes->type = HDT_TYPE_RESPONSE;

	uss_dev_count = (pHeader->len - HDT_HEADER_SIZE - HDT_TAIL_SIZE) / 2;
	for(i=0; i<uss_dev_count; i++)
	{
		pUssArgRecv = (PHDT_PACKET_USS_ARG)&pBuffer[sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)*i];
		pUssArgRes  = (PHDT_PACKET_USS_ARG)&buffer_res[sizeof(HDT_PACKET_HDR)+sizeof(HDT_PACKET_USS_ARG)*i];

		switch(pUssArgRecv->dev)
		{
		case HDT_DEV_USS_SWITCH:	//0x2A, USS일괄스위치 : 일괄상태
			if(pHeader->type == HDT_TYPE_CONTROL)						//0x02, Control(제어명령)
			{
				g_setup_data.m_SetupData.gas_stat      = (pHeader->cmd == HDT_CMD_ALL_OFF) ? 1 : 0;
				g_setup_data.m_SetupData.light_stat    = (pHeader->cmd == HDT_CMD_ALL_OFF) ? 1 : 0;
				g_setup_data.m_SetupData.security_stat = (pHeader->cmd == HDT_CMD_ALL_OFF) ? 1 : 0;

				pUssArgRes->cmd = pHeader->cmd;
				isResponse = TRUE;
			}
			break;
		case HDT_DEV_LIGHT_SWITCH:	//0x19, 일괄소등 스위치
			if(pHeader->type == HDT_TYPE_RESPONSE)						//0x04, Response(응답)
			{
				light_stat = (pUssArgRecv->cmd == HDT_USS_ARG_VALUE_LIGHT_OUT_ON) ? 1 : 0;
			}
			break;
		case HDT_DEV_GAS_SWITCH:	//0x1B, 가스차단 스위치
			if(pHeader->type == HDT_TYPE_RESPONSE)						//0x04, Response(응답)
			{
				gas_stat = (pUssArgRecv->cmd == HDT_CMD_GAS_VALVE_CLOSE) ? 1 : 0;
			}
			break;
		case HDT_DEV_SECURITY:		//0x16,	보안장비 (U-보안)
			if(pHeader->type == HDT_TYPE_RESPONSE)						//0x04, Response(응답)
			{
				if( (pUssArgRecv->cmd == HDT_CMD_SECURITY_ON) || (pUssArgRecv->cmd == HDT_CMD_SECURITY_OFF) )
				{
					security_stat = (pUssArgRecv->cmd == HDT_CMD_SECURITY_ON) ? 1 : 0;
				}
			}
			break;
		case HDT_DEV_IDLE_POWER:	//0x2C, 대기전력					//0x01=ON, 0x02=OFF
			break;
		case HDT_DEV_OUTLET:		//0x1F, 콘센트						//0x01=ON, 0x02=OFF
			break;
		case HDT_DEV_BOILER:		//0x18, 난방기						//0x01=ON, 0x04=OFF(외출)
			break;
		case HDT_DEV_OUT_BUTTON:	//0x17, 외출버튼					//0x01=ON, 0x02=OFF
			break;
		}
	}

	if(isResponse)
	{
		pTailRes = (PHDT_PACKET_TAIL)&buffer_res[pHeaderRes->len - 2];
		pTailRes->crc = CalcCheckSum(buffer_res, pHeaderRes->len - 2);

		usleep(30000);	//30[ms] 이후 응답전송

		if(Write(buffer_res, pHeaderRes->len) <= 0)
		{
			DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Response Write Failure\r\n");
		}
	}
	else
	{
		switch(g_state.GetState())
		{
		case STATE_WEATHER:
		case STATE_GAS:
			if(gas_stat != g_setup_data.m_SetupData.gas_stat)
			{
				g_setup_data.m_SetupData.gas_stat = gas_stat;

				pMtmHdr->type = MTM_DATA_TYPE_GAS;
				pMtmHdr->len  = sizeof(MTM_DATA_GAS);
				if(g_setup_data.m_SetupData.gas_stat)
					pMtmDataSecurity->status = MTM_DATA_GAS_CUT;
				else
					pMtmDataSecurity->status = MTM_DATA_GAS_OPEN;
				buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_GAS)] = MTM_PACKET_ETX;
				g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
			}
			break;
		case STATE_ELEVATOR:
			break;
		case STATE_PARKING:
			break;
		case STATE_SECURITY:
			if(security_stat != g_setup_data.m_SetupData.security_stat)
			{
				g_setup_data.m_SetupData.security_stat = security_stat;

				pMtmHdr->type = MTM_DATA_TYPE_SECURITY;
				pMtmHdr->len  = sizeof(MTM_DATA_SECURITY);
				if(g_setup_data.m_SetupData.security_stat)
					pMtmDataSecurity->status = MTM_DATA_SECURITY_ON;
				else
					pMtmDataSecurity->status = MTM_DATA_SECURITY_OFF;
				buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_SECURITY)] = MTM_PACKET_ETX;
				g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
			}
			break;
		case STATE_LIGHT:
			if(light_stat != g_setup_data.m_SetupData.light_stat)
			{
				g_setup_data.m_SetupData.light_stat = light_stat;

				pMtmHdr->type = MTM_DATA_TYPE_LIGHT;
				pMtmHdr->len  = sizeof(MTM_DATA_LIGHT);
				if(g_setup_data.m_SetupData.light_stat)
					pMtmDataLight->status = MTM_DATA_LIGHT_OFF;
				else
					pMtmDataLight->status = MTM_DATA_LIGHT_ON;
				buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_LIGHT)] = MTM_PACKET_ETX;
				g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
			}
			break;
		case STATE_SETUP:
			break;
		}

		g_setup_data.m_SetupData.gas_stat = gas_stat;
		g_setup_data.m_SetupData.light_stat = light_stat;
		g_setup_data.m_SetupData.security_stat = security_stat;
	}
}
#endif

void CWallPadHdt::ElevatorPollingProc(UCHAR *pBuffer)
{
	if(pBuffer==NULL) return;

	PHDT_PACKET_HDR pHeader = (PHDT_PACKET_HDR)pBuffer;
	PHDT_PACKET_ELEVATOR_INFO pElevatorInfo = (PHDT_PACKET_ELEVATOR_INFO)&pBuffer[sizeof(HDT_PACKET_HDR)];

	UCHAR buffer_res[HDT_MAX_PACKET+1] = {0,};
	PHDT_PACKET_HDR pHeaderRes = (PHDT_PACKET_HDR)&buffer_res[0];
	UCHAR *pArg1 = &buffer_res[sizeof(HDT_PACKET_HDR)];
	PHDT_PACKET_TAIL pTailRes = (PHDT_PACKET_TAIL)&buffer_res[sizeof(HDT_PACKET_HDR)+1];

	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER			pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_ELEVATOR	pMtmDataElevator = (PMTM_DATA_ELEVATOR)&buffer_mtm[sizeof(MTM_HEADER)];

	UCHAR dir, status;
	UCHAR ev_status, ev_floor;

	//수신패킷을 응답패킷 버퍼로 복사
	memcpy(buffer_res, pBuffer, pHeader->len);
	pHeaderRes->type = HDT_TYPE_RESPONSE;
	//pHeaderRes->len -= 2;
	if(m_ElevatorControlReserve)
	{
		*pArg1 = m_ElevatorControlReserve;
		m_ElevatorControlReserve = 0;
	}
	else
	{
		*pArg1 = 0;
	}
	pTailRes = (PHDT_PACKET_TAIL)&buffer_res[pHeaderRes->len - 2];
	pTailRes->crc = CalcCheckSum(buffer_res, pHeaderRes->len - 2);

	usleep(30000);	//30[ms] 이후 응답전송

	if(Write(buffer_res, pHeaderRes->len) <= 0)
	{
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] Response Write Failure\r\n");
	}

	dir	   = (pElevatorInfo->status & 0xf0);			//방향 (0xa0=상향운행, 0xb0=하향운행)
	status = (pElevatorInfo->status & 0x0f);			//상태 (0x00=평상시, 0x01=도착, 0x02=에러, 0x03=호출성공, 0x05=상향호출, 0x06=하향호출)

	if(status == HDT_EV_ARG_VALUE_STATUS_ARRIVE)		//0x01, 도착
		ev_status = MTM_DATA_EV_STATUS_ARRIVE;
	else if(status == HDT_EV_ARG_VALUE_STATUS_ERROR)	//0x02, 에러
		ev_status = MTM_DATA_EV_STATUS_ERROR;
	else if(dir == HDT_EV_ARG_VALUE_STATUS_DIR_UP)		//0xA0, 상향
		ev_status = MTM_DATA_EV_STATUS_UP;
	else if(dir == HDT_EV_ARG_VALUE_STATUS_DIR_DOWN)	//0xB0, 하향
		ev_status = MTM_DATA_EV_STATUS_DOWN;
	else												//0x00, 정지
		ev_status = MTM_DATA_EV_STATUS_STOP;

	if(pElevatorInfo->floor > 0xb0)	//지하
		ev_floor = 0xb0 - pElevatorInfo->floor;			//음수로 변환
	else
		ev_floor = pElevatorInfo->floor;

	if( (ev_status != g_setup_data.m_SetupData.elevator_status) || (ev_floor != g_setup_data.m_SetupData.elevator_floor) )
	{
		g_setup_data.m_SetupData.elevator_status = ev_status;
		g_setup_data.m_SetupData.elevator_floor  = ev_floor;

		if(g_state.GetState() == STATE_ELEVATOR)
		{
			pMtmHdr->type = MTM_DATA_TYPE_ELEVATOR;
			pMtmHdr->len  = sizeof(MTM_DATA_ELEVATOR);
			pMtmDataElevator->status = g_setup_data.m_SetupData.elevator_status;
			pMtmDataElevator->floor  = g_setup_data.m_SetupData.elevator_floor;
			buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_ELEVATOR)] = MTM_PACKET_ETX;
			g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
		}
	}
}

#if 0
void CWallPadHdt::ElevatorControlProc(UCHAR *pBuffer)
{
	//처리생략
}
#endif

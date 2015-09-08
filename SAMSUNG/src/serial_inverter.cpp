#include "common.h"
#include "measure_data.h"
#include "serial_inverter.h"
#include "main.h"

//#define INVERTER_SIMULATION		//인버터 없이 테스트 할때

//
// Construction/Destruction
//

CSerialInverter::CSerialInverter()
{
	m_ivt_type = IVT_TYPE_HEX;
	m_is485 = FALSE;

	memset(&m_req, 0, sizeof(m_req));

	m_isRecv = FALSE;
	m_isMeasureStart = FALSE;
	m_isWaitForResponse = FALSE;
	m_nMeasureStep = 0;

	m_nIvtCount = 0;
	m_pIvtIdList = NULL;
	m_ppResTable = NULL;
}

CSerialInverter::~CSerialInverter()
{
	RemoveResTable();
}

//
// Member Function
//
BOOL CSerialInverter::Open(const char *pszDev, unsigned int baudRate)
{
	BOOL fRet = FALSE;
	struct termios tio;

	fRet = CSerial::Open(pszDev, baudRate);
	if(fRet == FALSE) 
	{
		printf("%s: open failure\r\n", __func__);
		return FALSE;
	}

	ioctl( m_fdDev, 0x54F0, 0x0001 );
	tcflush ( m_fdDev, TCIFLUSH );

	if(tcgetattr(m_fdDev, &tio)==ERROR)
	{
		printf("%s: tcgetattr failure\r\n", __func__);
		return FALSE;
	}

	tio.c_cc[VMIN] = 8;		// 최소 8byte수신할때 까지 대기, 인버터 응답패킷중 데이타 전까지 8바이트
	if(tcsetattr( m_fdDev, TCSANOW, &tio )==ERROR)
	{
		printf("%s: tcsetattr failure\r\n", __func__);
		return FALSE;
	}

	printf("%s: '%s' device open success\r\n", __func__, pszDev);

	return TRUE;
}

void CSerialInverter::Run()
{
	g_gpio.SetDir(GPIOJ, 9,  GPIO_OUT);		//RS_422_ENABLE
	g_gpio.SetDir(GPIOJ, 10, GPIO_OUT);		//RS_485_ENABLE
	g_gpio.SetDir(GPIOG, 3,  GPIO_OUT);		//nRE_485

	Enable485(FALSE);
	Enable422(TRUE);

	CreateResTable();

	if(StartListener(InverterMeasureTask)==FALSE)
	{
		printf("%s: StartListener Failure\r\n", __func__);
	}

}

void CSerialInverter::Stop()
{
	StopListener();
}

void CSerialInverter::Enable485(BOOL isEnable)
{
	if(isEnable)
	{
		g_gpio.SetPin(GPIOJ, 10, HIGH);
		g_gpio.SetPin(GPIOG, 3, LOW);
	}
	else
	{
		g_gpio.SetPin(GPIOJ, 10, LOW);
		g_gpio.SetPin(GPIOG, 3, HIGH);
	}
}

void CSerialInverter::Enable422(BOOL isEnable)
{
	if(isEnable)
	{
		g_gpio.SetPin(GPIOJ, 9, HIGH);
	}
	else
	{
		g_gpio.SetPin(GPIOJ, 9, LOW);
	}
}

int CSerialInverter::SendRequest(UCHAR id, USHORT addr, UCHAR count)
{
	IVT_REQ req;
	UCHAR* p;
	USHORT chksum = 0;
	int i, ret = ERROR;

//	printf("+%s: id=0x%x, addr=0x%x, count=%d\r\n", __func__, id, addr, count);

	if(m_is485) 
	{
		while(m_isRecv) { usleep(10000); }
		Enable485(TRUE);
	}

	p = (UCHAR *)&m_req;

	m_req.enq = IVT_ENQ;
	Conv_Bin2Asc(id, m_req.id, 2);
	m_req.cmd = IVT_CMD_READ;
	Conv_Bin2Asc(addr, m_req.addr, 4);
	Conv_Bin2Asc(count, m_req.count, 2);
	for(i=1; i<10; i++)
	{
		chksum += p[i];
	}
	Conv_Bin2Asc(chksum, m_req.chksum, 4);
	m_req.eot = IVT_EOT;

#ifndef INVERTER_SIMULATION
	ret = Write((UCHAR *)&m_req, sizeof(IVT_REQ));
#else
	ret = sizeof(IVT_REQ);
#endif

	if(m_is485) 
	{
		usleep(40000);		//485전송시간을 보장하기 위해
		Enable485(FALSE);
	}

	printf("Send to Inverter:\r\n");	
	DumpData((UCHAR *)&m_req, sizeof(IVT_REQ), TRUE);

//	printf("-%s: ret=%d\r\n", __func__, ret);

	return ret;
}

USHORT CSerialInverter::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	USHORT usCS = 0;
	UINT i;

	if(pBuffer==NULL) return 0;

	for(i=0; i<size; i++)
	{
		usCS += pBuffer[i];
	}

	return usCS;
}

/*
	인버터 측정 태스크 시나리오:

		측정대상은 설정등록된 인버터들.
		10초의 Idle이후 측정.
		측정은 인버터당 10회, 1회당 4개의 응답을 요청하여 총 40개의 응답을 요청한다.
		1회 요청에 대한 TO는 500ms이므로 인버터당 최대 5sec의 측정시간이 필요함.
		측정된 데이타는 클래스 멤버의 테이블에 기록함.
		이 테이블은 설정등록된 인버터수에 비례하여 동적으로 생성함.
*/


void* CSerialInverter::InverterMeasureTask(void *pParam)
{
	CSerialInverter *pThis = (CSerialInverter *)pParam;
	unsigned char buffer[MAX_IVT_RECV_BUFFER];
	struct timeval timeout = { 1, 0 };			//1sec
//	struct timeval timeout = { 0, 500000 };		//500ms
	fd_set	fdRead;
	int i, ret, nReqCount, nSizeExpectBytes, nSizeRecvBytes, nTimeOutRetry=0;

	IVT_RES_HDR* pResHdr;
	IVT_RES_TAIL* pResTail;

	UCHAR id, *p;
	USHORT addr, index;
//	USHORT pkt_chksum, calc_chksum;
	int id_index, ivt_index=0;

	USHORT req_id, req_addr = MEASURE_START_ADDR;

	IVT_MEASURE_REC ivt_rec;
	time_t now;
	struct tm *t;
	TIME_STAMP ts;

	BOOL isTimeStamping = FALSE;

	printf("+%s\r\n", __func__);

	if(pThis->Open(UART1_DEV, B9600)==FALSE)
	{
		printf("%s: Open Failure\r\n", __func__);
		return (void *)0;
	}


//	pThis->m_isWaitForResponse = FALSE;

	while(pThis->m_fListenerRunning)
	{
		//측정하지 않는 동안은 Idle 상태
		if(pThis->m_isMeasureStart == FALSE)
		{
			usleep(10000);
			continue;
		}

		if(isTimeStamping==FALSE)
		{
			isTimeStamping = TRUE;

			now = time(NULL);
			t = localtime(&now);

			ts.yy = t->tm_year + 1900;
			ts.mm = t->tm_mon + 1;
			ts.dd = t->tm_mday;
			ts.hr = t->tm_hour;
			ts.mi = t->tm_min;
			ts.se = t->tm_sec;
		}

		//수신대기 상태가 아니면(=요청상태)
		if(pThis->m_isWaitForResponse==FALSE)
		{
			//하나의 인버터당 10회의 요청과 40개의 응답을 받게됨
			//1회요청당 TO=500ms이므로 최대 5sec에 하나의 인버터를 측정하게 됨
			if(pThis->m_nMeasureStep < 10)
			{
			//	printf("%s: Request - Step=%d\r\n", __func__, pThis->m_nMeasureStep);
				if(pThis->m_nMeasureStep == 0)
					req_addr = MEASURE_START_ADDR;
				else
					req_addr += 8;	//4워드씩 요청
				
				//요청하면서 응답테이블의 이전값을 지워야 하는가? 아니면 이전값을 유지하는게 좋은가?

				pThis->SendRequest(pThis->m_pIvtIdList[ivt_index], req_addr, 4);
				pThis->m_isWaitForResponse = TRUE;
				nTimeOutRetry = 0;
			}
			else	//하나의 인버터에 대한 요청이 완료된 경우
			{
				//측정데이타 저장
			#if 0
				now = time(NULL);
				t = localtime(&now);

				ivt_rec.time_stamp.yy = t->tm_year + 1900;
				ivt_rec.time_stamp.mm = t->tm_mon + 1;
				ivt_rec.time_stamp.dd = t->tm_mday;
				ivt_rec.time_stamp.hr = t->tm_hour;
				ivt_rec.time_stamp.mi = t->tm_min;
				ivt_rec.time_stamp.se = t->tm_sec;
			#else
				memcpy(&ivt_rec.time_stamp, &ts, sizeof(TIME_STAMP));
			#endif
			//	printf("TimeStamp: %04d/%02d/%02d %02d:%02d:%02d\r\n", 
			//			ivt_rec.time_stamp.yy, ivt_rec.time_stamp.mm, ivt_rec.time_stamp.dd,
			//			ivt_rec.time_stamp.hr, ivt_rec.time_stamp.mi, ivt_rec.time_stamp.se);

				ivt_rec.id = pThis->m_pIvtIdList[ivt_index];
			//	printf("id=0x%x\r\n", ivt_rec.id);

				//id_index는 마직막 수신레코드의 id로 찾은 index
				index = (MEASURE_ADDR_SOLAR_VOLT-MEASURE_START_ADDR)/2;
				ivt_rec.solar_volt = pThis->m_ppResTable[id_index][index];
			//	printf("solar_volt=%d [V]\r\n", ivt_rec.solar_volt);

				index = (MEASURE_ADDR_SOLAR_CURRENT-MEASURE_START_ADDR)/2;
				ivt_rec.solar_current = pThis->m_ppResTable[id_index][index];
			//	printf("solar_current=%.1f [A]\r\n", ivt_rec.solar_current/10.0);

				index = (MEASURE_ADDR_IVT_VOLT-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_volt = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_volt=%d [V]\r\n", ivt_rec.ivt_volt);

				index = (MEASURE_ADDR_IVT_CURRENT-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_current = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_current=%.1f [A]\r\n", ivt_rec.ivt_current/10.0);

				index = (MEASURE_ADDR_IVT_FREQ-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_frequency = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_frequency=%.1f [Hz]\r\n", ivt_rec.ivt_frequency/10.0);

				index = (MEASURE_ADDR_LINE_VOLT-MEASURE_START_ADDR)/2;
				ivt_rec.line_volt = pThis->m_ppResTable[id_index][index];
			//	printf("line_volt=%d [V]\r\n", ivt_rec.line_volt);

				index = (MEASURE_ADDR_LINE_CURRENT-MEASURE_START_ADDR)/2;
				ivt_rec.line_current = pThis->m_ppResTable[id_index][index];
			//	printf("line_current=%.1f [A]\r\n", ivt_rec.line_current/10.0);

				index = (MEASURE_ADDR_LINE_FREQ-MEASURE_START_ADDR)/2;
				ivt_rec.line_frequency = pThis->m_ppResTable[id_index][index];
			//	printf("line_frequency=%.1f [Hz]\r\n", ivt_rec.line_frequency/10.0);

				index = (MEASURE_ADDR_SOLAR_POWER-MEASURE_START_ADDR)/2;
				ivt_rec.solar_power = pThis->m_ppResTable[id_index][index];
			//	printf("solar_power=%d [W]\r\n", ivt_rec.solar_power);

				//적산전력량 = (High * 1000 + Low) / 1000.
				//	ivt_powr_sum을 쓸때 1000으로 나누어서 써야함.
				index = (MEASURE_ADDR_IVT_POWER_HIGH-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_powr_sum = pThis->m_ppResTable[id_index][index] * 1000 + pThis->m_ppResTable[id_index][index+1];
			//	printf("ivt_powr_sum=%ld [W]\r\n", ivt_rec.ivt_powr_sum);

				index = (MEASURE_ADDR_IVT_POWER_CURRENT-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_power_current = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_power_current=%d [A]\r\n", ivt_rec.ivt_power_current);

				index = (MEASURE_ADDR_IVT_POWER_MAX-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_power_max = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_power_max=%d [W]\r\n", ivt_rec.ivt_power_max);

				index = (MEASURE_ADDR_IVT_POWER_FACTOR-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_power_factor = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_power_factor=%.3f\r\n", ivt_rec.ivt_power_factor/1000.0);

				index = (MEASURE_ADDR_IVT_POWER_DAILY-MEASURE_START_ADDR)/2;
				ivt_rec.ivt_power_daily = pThis->m_ppResTable[id_index][index];
			//	printf("ivt_power_daily=%.1f [W]\r\n", ivt_rec.ivt_power_daily/10.0);

				if(g_MeasureData.SaveInverterData(&ivt_rec)==FALSE)
				{
					//Todo: What?
				}

			//	if(g_MeasureData.AddInverterPower(ivt_rec.id, ivt_rec.ivt_powr_sum)==FALSE)			//적산전력량
				if(g_MeasureData.AddInverterPower(ivt_rec.id, ivt_rec.ivt_power_daily)==FALSE)		//하루적산량
				{
					//Todo: What?
				}

				//다음 인버터가 있는 경우
				if(ivt_index < (pThis->m_nIvtCount-1))
				{
					ivt_index++;
					pThis->m_nMeasureStep = 0;
				}
				else	//요청할 인버터가 없는 경우
				{
					pThis->m_isMeasureStart = FALSE;

					pThis->m_nMeasureStep = 0;
					ivt_index = 0;

					pThis->m_isWaitForResponse = FALSE;
					pThis->m_nMeasureStep = 0;

					isTimeStamping = FALSE;
				}
			}
		}
		else	//요청에 대한 응답 수신대기
		{
		#ifndef INVERTER_SIMULATION
			//select이후 변수가 초기화됨
			FD_ZERO(&fdRead);
			FD_SET(pThis->m_fdDev, &fdRead);
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			ret = select(pThis->m_fdDev+1, &fdRead, NULL, NULL, &timeout);
			if(ret == ERROR)	//Error
			{
				printf("%s: select error\r\n", __func__);
				usleep(WAIT_FOR_IVT_PACKET_RECV);
			}
			else if(ret == 0)	//Timeout
			{
				printf("%s: response timeout\r\n", __func__);

				nTimeOutRetry++;
				if(nTimeOutRetry < 3) 
				{
					usleep(WAIT_FOR_IVT_PACKET_RECV);
					continue;
				}
				//요청에 대한 응답이 없을 경우 Retry를 해야 하는지 무시해야 하는지 ??
			}
			else if(FD_ISSET(pThis->m_fdDev, &fdRead))	//Receive
			{
		#else
			if(1)
			{
				ret = 29;
		#endif
				pThis->m_isRecv = TRUE;

				usleep(WAIT_FOR_IVT_PACKET_RECV);		//한 패킷이 수신되기 위한 시간 확보

				memset(buffer, 0, sizeof(MAX_IVT_RECV_BUFFER));
				nReqCount = (int)Conv_Asc2Bin(pThis->m_req.count, 2);
				nSizeExpectBytes = 13 + nReqCount*4;

			//	요청에 따라 수신해야할 바이트수를 계산함 --> nSizeRecvBytes
			//	printf("%s: nReqCount=%d, nSizeExpectBytes=%d\r\n", __func__, nReqCount, nSizeExpectBytes);
				nSizeRecvBytes = 0;

			#ifndef INVERTER_SIMULATION
			//	수신이 다 될때까지 기다렸다 수신
				while(nSizeRecvBytes < nSizeExpectBytes)
				{
					ret = pThis->Read(&buffer[nSizeRecvBytes], nSizeExpectBytes-nSizeRecvBytes);
					if( ret >= 0 )	//ret가 0인경우(EOF)는 어떻게 처리 해야 하나? 수신대기 or 수신취소
					{
						nSizeRecvBytes += ret;
						if(nSizeRecvBytes < nSizeExpectBytes)
						{
							usleep(WAIT_FOR_IVT_PACKET_RECV);
						}
					}
					else
					{
						printf("%s: Read Error\r\n", __func__);
						break;
					}
				}
			#else
				memset(buffer, 0x30, ret);	//29bytes

				//헤더
				pResHdr  = (IVT_RES_HDR*)&buffer[0];
				pResHdr->ack = IVT_ACK;
				memcpy(pResHdr->id, pThis->m_req.id, 2);
				pResHdr->cmd = IVT_CMD_READ;
				memcpy(pResHdr->addr, pThis->m_req.addr, 4);

				//테일(체크섬생략)
				pResTail = (IVT_RES_TAIL*)&buffer[sizeof(IVT_RES_HDR)+(nReqCount*4)];
				pResTail->eot = IVT_EOT;

				nSizeRecvBytes = nSizeExpectBytes;
			#endif
				if(nSizeRecvBytes == nSizeExpectBytes)
				{
					printf("Recv from Inverter:\r\n");
					DumpData(buffer, ret, TRUE);

				//	calc_chksum = 0;
					pResHdr  = (IVT_RES_HDR*)&buffer[0];
					pResTail = (IVT_RES_TAIL*)&buffer[sizeof(IVT_RES_HDR)+(nReqCount*4)];
					if( (pResHdr->ack == IVT_ACK) && (pResHdr->cmd == IVT_CMD_READ) && (pResTail->eot == IVT_EOT) )
					{
						id   = (UCHAR)Conv_Asc2Bin(pResHdr->id, 2);
						addr = (USHORT)Conv_Asc2Bin(pResHdr->addr, 4);

					#if 0	//체크섬 확인
						pkt_chksum = (USHORT)Conv_Asc2Bin(&buffer[sizeof(IVT_RES_HDR)+(nReqCount*4)], 4);
						calc_chksum = CalcCheckSum(&buffer[1], 9+(nReqCount*4));
						if(pkt_chksum != calc_chksum)
						{
							printf("%s: Invalid CheckSum. pkt_chksum=0x%x vs calc_chksum=0x%x\r\n", __func__, pkt_chksum, calc_chksum);
						}
					#endif

						//어드레스 유효성 체크
						if( (addr < MEASURE_START_ADDR) || (addr > MEASURE_END_ADDR) )	//요청하지 않은 어드레스
						{
							printf("%s: Invalid Address Range\r\n", __func__);
						}
						else
						{
							//패킷의 ID(국번)이 유효한(등록된) ID인지 확인
							id_index = -1;
							for(i=0; i<pThis->m_nIvtCount; i++)
							{
								if(pThis->m_pIvtIdList[i]==id)
								{
									id_index = i;
									break;
								}
							}

							//유효한 ID이면
							if(id_index>=0)
							{
								p = &buffer[sizeof(IVT_RES_HDR)];

								index = ((addr-MEASURE_START_ADDR) / 2);

								for(i=0; i<nReqCount; i++)
								{
									pThis->m_ppResTable[id_index][index+i] = (USHORT)Conv_Asc2Bin(&p[i*4], 4);
								}

							#if 0
								//요청에 대한 응답인지 확인
								if(memcmp(pThis->m_req.addr, pResHdr->addr, 4) == 0)
								{
								}
							#endif

							}
							else
							{
								printf("%s: Unknown Inverter ID(0x%x)\r\n", __func__, id);
							}
						}
					}
					else
					{
						printf("%s: Invalid Packet Format\r\n", __func__);
					}
				}

				pThis->m_isRecv = FALSE;

				usleep(WAIT_FOR_IVT_PACKET_RECV);
			}
			else
			{
				printf("%s: Unknown State\r\n", __func__);
			}

			pThis->m_nMeasureStep++;
			pThis->m_isWaitForResponse = FALSE;
		}
	}

	pThis->Close();

	pthread_exit(NULL); 

	printf("-%s\r\n", __func__);

	return (void *)1;
}

void CSerialInverter::CreateResTable()
{
	int i;

//	printf("+%s\r\n", __func__);

	//Todo: SetupData에서 인버터갯수를 가져옴
	m_nIvtCount = 1;

	if(m_pIvtIdList)
	{
		delete[] m_pIvtIdList;
	}

	if(m_ppResTable && m_nIvtCount)
	{
		RemoveResTable();
	}

#if 1
	m_pIvtIdList = new UCHAR[m_nIvtCount];
#else
	m_pIvtIdList = malloc(sizeof(UCHAR)*m_nIvtCount);
#endif
	if(m_pIvtIdList == NULL)
	{
		printf("%s: Memory Allocation Failure: m_pIvtIdList\r\n", __func__);
		return;
	}

	//Todo: SetupData에서 인버터 ID List를 가져옴
	m_pIvtIdList[0] = 0x01;

#if 0
	m_ppResTable = new (USHORT*)[m_nIvtCount];
#else
	m_ppResTable = (USHORT**)malloc(sizeof(USHORT*)*m_nIvtCount);
#endif
	if(m_ppResTable == NULL)
	{
		printf("%s: Memory Allocation Failure: m_ppResTable\r\n", __func__);
		return;
	}

	for(i=0; i<m_nIvtCount; i++)
	{
		//실제 사용되는것은 MEASURE_IDX_COUNT(15)개 이지만 구현성이나 확장성을 고려해서 40개를 확보함
		m_ppResTable[i] = new USHORT[MAX_RES];
		if(m_ppResTable[i]==NULL)
		{
			printf("%s: Memory Allocation Failure: m_ppResTable[%d]\r\n", __func__, i);
		}
	}

//	printf("-%s\r\n", __func__);
}

void CSerialInverter::RemoveResTable()
{
	int i;

//	printf("+%s\r\n", __func__);

	if(m_pIvtIdList && m_nIvtCount)
	{
	#if 1
		delete[] m_pIvtIdList;
	#else
		free(m_pIvtIdList);
	#endif
		m_pIvtIdList = NULL;
	}

	if(m_ppResTable && m_nIvtCount)
	{
		for(i=0; i<m_nIvtCount; i++)
		{
			if(m_ppResTable[i])
			{
				delete[] m_ppResTable[i];
				m_ppResTable[i] = NULL;
			}
		}
	#if 0
		delete[] m_ppResTable;
	#else
		free(m_ppResTable);
	#endif
		m_ppResTable = NULL;
	}

//	printf("-%s\r\n", __func__);
}


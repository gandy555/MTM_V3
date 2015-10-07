/******************************************************************************
 * Filename:
 *   Wallpad_ezville.cpp
 *
 * Description:
 *   Wallpad of ezville with communication interface module
 *
 * Author:
 *   gandy
 *
 * Version : V0.1_15-05-11
 * ---------------------------------------------------------------------------
 * Abbreviation
 ******************************************************************************/
#include "common.h"
#include "context_setup.h"
#include "main.h"
/******************************************************************************
 *
 * Variable Declaration
 *
 ******************************************************************************/
//---------------------------------------------------------------------------------
// Function Name  : PRINT_SPENDING_TIME()
// Description    : 
//---------------------------------------------------------------------------------
#define PRINT_SPENDING_TIME(_func)			\
{											\
	u32 s_tick, c_tick;							\
											\
	s_tick = GetTickCount();					\
	_func;									\
	c_tick = GetTickCount();					\
	printf("[%08d]spending time: %d ms\r\n", c_tick, c_tick - s_tick);	\
}

#define EZV_HEADER			0xF7
#define EZV_DEV_ID			0x53
#define EZV_SUB_ID			0x01

#define MAX_PKT_SIZE	128	//22

#define REQ_ID_IDX		1
#define REQ_CTRL_IDX	2
#define REQ_ONOFF_IDX	3

#define REQ_RETRY_CNT		5
#define ELEV_REQ_RETRY_CNT	20

#define MAX_REQ_LIST_CNT	50

#define PKT_WAIT_TIME		5000	// wait 10ms after receivee packet
#define PKT_WAIT_TIME_1		9000	// wait 10ms after receivee packet

#define ALPHABET_CHAR(c)	((c >= 'A') && (c <= 'Z')) ? 1 : 0 
#define DECIMAL_CHAR(c)	((c >= '0') && (c <= '9')) ? 1 : 0 
#define DECIMAL_BYTE(c)	((c >= 0) && (c <= 9)) ? 1 : 0 

// header | device ID | sub-ID |     cmd type       | data_len | data |xor | add 
enum {
	HEADER_IDX = 0,
	DEV_ID_IDX,
	SUB_ID_IDX,
	CMD_TYPE_IDX,
	DATA_LEN_IDX,
	DATA_0_IDX,
	DATA_1_IDX,
	DATA_2_IDX,
	DATA_3_IDX,
	DATA_4_IDX,
	DATA_5_IDX,	
	DATA_6_IDX,	
	DATA_7_IDX,	
	DATA_8_IDX,	
	DATA_9_IDX,	
	DATA_10_IDX
};

enum request_list {
	RQL_SWITCH = 0,
	RQL_GAS,
	RQL_SECURITY,
	RQL_ELEVATOR,
	RQL_DOOR,
	RQL_WEATHER,
	RQL_PARKING,
	MAX_RQL
};

#define CMD_STATUS_REQ		0x01
#define CMD_WP_STATUS_RES	0x41
#define CMD_WP_CTRL_RES	0x43

#define CMD_STATUS_RSP		0x81
#define CMD_WP_STATUS_ACK	0xC1
#define CMD_WP_CTRL_ACK	0xC3

#define REQ_QUEUE_SIZE		20

enum {
	WCP_SWITCH = 1,
	WCP_GAS,
	WCP_SECURITY,
	WCP_ELEVATOR,
	WCP_DOOR,
	WCP_WEATHER,
	WCP_PARKING
};

enum {
	MCP_SWITCH = 1,
	MCP_GAS,
	MCP_SECURITY,
	MCP_ELEVATOR
};

struct ezv_weather_info {
	u8 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 weather;			//날씨아이콘
	u8 temp_range;			//영상/영하		(0=영상, 1=영하)
	u8 temp_int;			//정수자리
	u8 temp_under;			//소수이하
} __attribute__ ((packed));

struct ezv_parking_info {
	u8 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 floor_name;			//층명칭
	u8 floor_num;			//층번호
	u8 zone_name;			//구역명칭
	u8 zone_num;			//구역번호
	u8 tag_id[2];	
} __attribute__ ((packed));

static ezv_pkt_info_t g_rcv_pkt;
static ezv_pkt_info_t g_send_pkt;
static req_info_list_t g_req_info_list;
static req_info_list_t g_req_ctrl_list;

struct dummy_req_info {
	u8 cnt;
	u8 data[STATUS_REQ_SIZE];
};

static struct dummy_req_info g_dummy_req_list[4];
static u8 g_dummy_req_idx = 0;

#define CHECK_RQL(s, id)		((s) & (1 <<  (id))) ? 1 : 0
#define SET_RQL(s, id)			((s) = (1 <<  (id)))

#define RQ_ID_SWITCH		0x01
#define RQ_ID_GAS			0x02
#define RQ_ID_SECURITY		0x04
#define RQ_ID_ELEVATOR		0x08
#define RQ_ID_DOOR			0x10
#define RQ_ID_WEATHER		0x20
#define RQ_ID_PARKING		0x40

#define READ_BYTE(p) {		\
	if (m_serial.Read(p, 1) < 0)	\
		return 0;			\
}

/******************************************************************************
 *
 * Public Functions Declaration
 *
 ******************************************************************************/
CWallPadEzville::CWallPadEzville()
{
	m_request_id = 0;
	m_isRecv = 0;

	g_dummy_req_idx = 3;	
	memset(&g_dummy_req_list, 0, 4*sizeof(struct dummy_req_info));

	g_dummy_req_list[0].data[1] = 1 <<  RQL_DOOR;
	g_dummy_req_list[0].data[2] = (1 <<  RQL_STATUS);
	g_dummy_req_list[0].data[3] = 0;

	g_dummy_req_list[1].data[1] = 1 <<  RQL_GAS;
	g_dummy_req_list[1].data[2] = (1 <<  RQL_STATUS);
	g_dummy_req_list[1].data[3] = 0;

	g_dummy_req_list[2].data[1] = 1 <<  RQL_PARKING;
	g_dummy_req_list[2].data[2] = (1 <<  RQL_STATUS);
	g_dummy_req_list[2].data[3] = 0;

	g_dummy_req_list[3].data[1] = 1 <<  RQL_WEATHER;
	g_dummy_req_list[3].data[2] = (1 <<  RQL_STATUS);
	g_dummy_req_list[3].data[3] = 0;

	memset(&g_req_info_list, 0, sizeof(req_info_list_t));
	memset(&g_req_ctrl_list, 0, sizeof(req_info_list_t));	

	memset(&g_send_pkt, 0, sizeof(ezv_pkt_info_t));
	memset(&g_rcv_pkt, 0, sizeof(ezv_pkt_info_t));

	pthread_mutex_init(&m_wp_mutex, NULL);
}

CWallPadEzville::~CWallPadEzville()
{
	pthread_mutex_destroy(&m_wp_mutex);
}

BOOL CWallPadEzville::Run()
{
	BOOL fRet = FALSE;
	struct termios tio;

	// 9600bps, 8bit, 1stopbit, non-parity
	if ((fRet = m_serial.Open(UART1_DEV, B9600)) == FALSE) {
		DBGMSG(DBG_WALLPAD, "[WallPad:EZV] Open Failure\r\n");
		return FALSE;
	}

	//for RS-485
	ioctl( m_serial.m_fdDev, 0x54F0, 0x0001 );
    	tcflush  ( m_serial.m_fdDev, TCIFLUSH );

	//Listener Start
	if ((fRet = m_serial.StartListener(SerialListener_EZV, this)) == FALSE) {
		DBGMSG(DBG_WALLPAD, "[WallPad:EZV] StartListener Failure\r\n");
	}

	return fRet;
}

void CWallPadEzville::Stop()
{
	m_serial.StopListener();

	m_serial.Close();
}

void CWallPadEzville::RequestWeatherInfo()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_WEATHER, RQL_STATUS, 0, REQ_RETRY_CNT);
}

//가스차단요청
void CWallPadEzville::RequestGasCut()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_GAS, RQL_CONTROL, 0, REQ_RETRY_CNT);
}

//가스상태요청
void CWallPadEzville::RequestGasStatus()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_GAS, RQL_STATUS, 0, REQ_RETRY_CNT);
}

void CWallPadEzville::RequestLightStatus()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_SWITCH, RQL_STATUS, 0, REQ_RETRY_CNT);
}

//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)
void CWallPadEzville::RequestLightSet(BOOL isAllOff)
{
	printf("<%s> %d\r\n", __func__, isAllOff);

	if (isAllOff)
		register_request_data(RQL_SWITCH, RQL_CONTROL, 1, REQ_RETRY_CNT);
	else
		register_request_data(RQL_SWITCH, RQL_CONTROL, 0, REQ_RETRY_CNT);
}

void CWallPadEzville::RequestSecurityStatus()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_SECURITY, RQL_STATUS, 0, REQ_RETRY_CNT);
}

//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)
void CWallPadEzville::RequestSecuritySet(BOOL isSecurityOn)
{
	printf("<%s> %d\r\n", __func__, isSecurityOn);

	if (isSecurityOn)
		register_request_data(RQL_SECURITY, RQL_CONTROL, 1, REQ_RETRY_CNT);
	else
		register_request_data(RQL_SECURITY, RQL_CONTROL, 0, REQ_RETRY_CNT);
}

//엘레베이터 호출
void CWallPadEzville::RequestElevatorCall()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_ELEVATOR, RQL_CONTROL, 1, ELEV_REQ_RETRY_CNT);
}

//엘레베이터 상태요청
void CWallPadEzville::RequestElevatorStatus()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_ELEVATOR, RQL_STATUS, 0, ELEV_REQ_RETRY_CNT);
}

//주차위치정보요청
void CWallPadEzville::RequestParkingInfo()
{
	printf("<%s>\r\n", __func__);

	register_request_data(RQL_PARKING, RQL_STATUS, 0, REQ_RETRY_CNT);
}

void CWallPadEzville::get_request_dummy_data(u8 *_data)
{
	printf("<%s> %d: %d\r\n", __func__,
		g_dummy_req_idx, g_dummy_req_list[g_dummy_req_idx].cnt);

	m_request_id = 0;
	memcpy(_data, g_dummy_req_list[g_dummy_req_idx].data, STATUS_REQ_SIZE);
	g_dummy_req_list[g_dummy_req_idx].cnt++;
	
	switch (g_dummy_req_idx) {
	case 0:	/* DOOR */
		if (g_dummy_req_list[0].cnt >= 4) {
			g_dummy_req_idx = 1;
			g_dummy_req_list[0].cnt = 0;
		}
		break;
	case 1:	/* GAS */
		if (g_dummy_req_list[1].cnt >= 4) {
			g_dummy_req_idx = 2;
			g_dummy_req_list[1].cnt = 0;
		} else {
			g_dummy_req_idx = 0;
		}
		break;
	case 2:	/* PARKING */
		if (g_dummy_req_list[2].cnt >= 4) {
			g_dummy_req_idx = 3;
			g_dummy_req_list[2].cnt = 0;
		} else {
			g_dummy_req_idx = 0;
		}
		break;
	case 3:	/* WEATHER */
		if (g_dummy_req_list[3].cnt >= 4) {
			g_dummy_req_idx = 0;
			g_dummy_req_list[3].cnt = 0;
		} else {
			g_dummy_req_idx = 0;
		}
		break;
	default:
		g_dummy_req_idx = 0;
		g_dummy_req_list[0].cnt = 0;
		break;
	}
}

void CWallPadEzville::append_request_list(req_info_list_t *_list, req_node_t *_node)
{
	pthread_mutex_lock(&m_wp_mutex);
	_node->next = NULL;
	if (_list->head == NULL) {
		_list->head = _node;
		_list->tail = _node;
	} else {
		_list->tail->next = _node;
		_list->tail = _node;
	}

	_list->cnt++;
	pthread_mutex_unlock(&m_wp_mutex);	
}

void CWallPadEzville::register_request_data(u8 _req_id, u8 _ctrl, u8 _onoff, u8 _retry)
{
	req_info_list_t *list;
	req_node_t *node = NULL;
	
	if (_req_id >= MAX_RQL) {
		printf("<%s> Invalid request ID(%d)\r\n", __func__, _req_id);
		return;
	}

	if (_ctrl == RQL_CONTROL) 
		list = &g_req_ctrl_list;
	else
		list = &g_req_info_list;

	if (list->cnt >= MAX_REQ_LIST_CNT) {
		printf("<%s> Request Queue Full!\r\n", __func__);
		return;
	}
		
	node = (req_node_t *)malloc(sizeof(req_node_t));
	if (node == NULL) {
		printf("<%s> make node failed!\r\n", __func__);
		return;
	}

	memset(node->data, 0, STATUS_REQ_SIZE);
	SET_RQL(node->data[REQ_ID_IDX], _req_id);
	SET_RQL(node->data[REQ_CTRL_IDX], _ctrl);

	if (_ctrl == RQL_CONTROL) {
		if (_onoff)
			node->data[REQ_ONOFF_IDX] = 1;
		else
			node->data[REQ_ONOFF_IDX] = 2;
	} else {
		node->data[REQ_ONOFF_IDX] = 0;
	}
	node->retry_cnt = _retry;
	
	append_request_list(list, node);
}

u8 CWallPadEzville::delete_request_data(u8 _rql)
{
	req_info_list_t *list = &g_req_ctrl_list;
	u8 res = 0;

	if (_rql == RQL_CONTROL)
		list = &g_req_ctrl_list;
	else
		list = &g_req_info_list;
	
	pthread_mutex_lock(&m_wp_mutex);	
	if (list->cnt > 0) {
		req_node_t *node = list->head;
		//if (node->retry_cnt != REQ_RETRY_CNT) {	/* 시도 되지 않은 패킷 삭제는 금지 */
			if (node->next != NULL) {
				list->head = node->next;
			} else {
				list->head = NULL;
				list->tail = NULL;
			}
			list->cnt -= 1;
			free(node);			
			res = 1;
		//}
	}
	pthread_mutex_unlock(&m_wp_mutex);	

	if (res)
		printf("<%s> %d\r\n", __func__, list->cnt);
	
	return res;
}

void CWallPadEzville::report_request_failed(u8 _rql)
{
	req_info_list_t *list = &g_req_ctrl_list;

	if (_rql == RQL_CONTROL)
		list = &g_req_ctrl_list;
	else
		list = &g_req_info_list;
	
	pthread_mutex_lock(&m_wp_mutex);	
	if (list->cnt > 0) {
		req_node_t *node = list->head;
		g_message.SendMessageData(MSG_WALLPAD_RSP_ERR, &node->data[REQ_ID_IDX], 2);
	}
	pthread_mutex_unlock(&m_wp_mutex);	
}

u8 CWallPadEzville::get_request_data(u8 _rql, u8 *_data)
{
	req_info_list_t *list = &g_req_info_list;
	u8 res = 0;

	if (_rql == RQL_CONTROL)
		list = &g_req_ctrl_list;
	else
		list = &g_req_info_list;
	
	pthread_mutex_lock(&m_wp_mutex);	
	if (list->cnt > 0) {
		memcpy(_data, list->head->data, STATUS_REQ_SIZE);
		list->head->retry_cnt--;
		res = 1;
	}
	pthread_mutex_unlock(&m_wp_mutex);
	
	if (res) {
		if (list->head->retry_cnt == 0) {
			report_request_failed(_rql);
			delete_request_data(_rql);
		}
		m_request_id = _data[REQ_ID_IDX];
		printf("<%s> %d\r\n", __func__, m_request_id);
	}
	
	return res;
}

void CWallPadEzville::delete_all_request_data(void)
{
	m_request_id = 0;
	
	while (delete_request_data(RQL_STATUS)) {
	}

	while (delete_request_data(RQL_CONTROL)) {
	}
}

//
// Member Function
//
int CWallPadEzville::Write(UCHAR *pData, int size)
{
	CContextSetup* pContextSetup = NULL;
	int ret = ERROR;
	ULONG ulTick, delay;

	if (m_serial.IsOpen()) {
		DBGMSG(DBG_WALLPAD, "[WallPad:EZV] Write\r\n");
		DBGDMP(DBG_WALLPAD, pData, size, TRUE);

		//수신중일경우 대기
		ulTick = GetTickCount();
		while (m_isRecv) { 
			usleep(1000); 
			if (GetElapsedTick(ulTick) >= 300) {
				DBGMSG(DBG_WALLPAD, "[WallPad:EZV] Wait for read done: Timeout!!\r\n");
				return -1;
			}
		}

		// 485 Enable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, LOW);

		// TX Enable후 최초 패킷을 전송하기까지 1.5ms 지연
		usleep(10);

		ret = m_serial.Write(pData, size);
		if (ret != size) {
			DBGMSG(DBG_WALLPAD, "[WallPad:EZV] Write Bytes Error, write=%d, expect=%d\e\n", ret, size);
		}

		//마지막 패킷 전송후 TX Disable 하기까지 지연 (패킷사이즈 전송시간 + 1.5[ms])
		delay = (size * 1042) + 10;	// 9600[bps]에서 1start+8bit+1stop 전송속도는 1.04167[ms] 이므로 21 byte는 21.875[ms]
		usleep(delay);

		// 485 Disable
		g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, HIGH);

	#if 1
		if (g_isContextSetupWallPad) {
			pContextSetup = (CContextSetup*)g_state.GetCurrContext();
			if (pContextSetup) {
				pContextSetup->PrintPacket(pData, size, CYAN, FALSE);
			}
		}
	#endif
	}

	return ret;
}

u8 CWallPadEzville::calc_xor_sum(u8 *_pkt, UINT _size)
{
	u8 sum = 0;
	UINT i;
	
	for (i = 0; i < _size-2; i++) {
		sum ^= _pkt[i];
	}

	return sum;
}

u8 CWallPadEzville::calc_add_sum(u8 *_pkt, UINT _size)
{
	u8 sum = 0;
	UINT i;
	
	for (i = 0; i < _size-1; i++) {
		sum += _pkt[i];
	}

	return sum;
}

//
// Thread(SerialListener) Context
//
void* CWallPadEzville::SerialListener_EZV(void *pParam)
{
	CWallPadEzville *pThis = (CWallPadEzville *)pParam;
	CSerial *pSerial = &pThis->m_serial;
	struct timeval timeout;
	fd_set fdRead;
	int ret;

	DBGMSG(DBG_WALLPAD, "%s: Start\r\n", __func__);

	while (pSerial->m_fListenerRunning) {
		FD_ZERO(&fdRead);
		FD_SET(pSerial->m_fdDev, &fdRead);

		timeout.tv_sec  = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;
		ret = select(pSerial->m_fdDev+1, &fdRead, NULL, NULL, &timeout);
		if (FD_ISSET(pSerial->m_fdDev, &fdRead)) {
			pThis->ezv_parser();
		}
	}

	DBGMSG(DBG_WALLPAD, "%s: End\r\n", __func__);

	pthread_exit(NULL); 
}

u8 CWallPadEzville::pkt_rcv_n_delimeter(ezv_pkt_info_t *_pkt)
{
	int ret, t_size;

	switch (_pkt->size) {
	case HEADER_IDX:
		while (1) {
			READ_BYTE(&_pkt->buffer[0]);
			if (_pkt->buffer[0] == EZV_HEADER) {
				_pkt->size++;
				break;
			}
		}
	case DEV_ID_IDX:
		READ_BYTE(&_pkt->buffer[_pkt->size]);
		if (_pkt->buffer[_pkt->size] != EZV_DEV_ID) {
			_pkt->size = 0;
			DBGMSG(DBG_WALLPAD, "Invalid Device ID\r\n");
			return 0;
		}
		_pkt->size++;
	case SUB_ID_IDX:
		READ_BYTE(&_pkt->buffer[_pkt->size]);
		if (_pkt->buffer[_pkt->size] != EZV_SUB_ID) {
			_pkt->size = 0;
			DBGMSG(DBG_WALLPAD, "Invalid Sub ID\r\n");
			return 0;
		}
		_pkt->size++;
	case CMD_TYPE_IDX:	
		READ_BYTE(&_pkt->buffer[_pkt->size]);
		if ((_pkt->buffer[_pkt->size] != CMD_STATUS_REQ) &&
			(_pkt->buffer[_pkt->size] != CMD_WP_STATUS_RES) &&
			(_pkt->buffer[_pkt->size] != CMD_WP_CTRL_RES)) {
			_pkt->size = 0;
			DBGMSG(DBG_WALLPAD, "Invalid CMD TYPE\r\n");
			return 0;
		}
		_pkt->size++;
	case DATA_LEN_IDX: 
		READ_BYTE(&_pkt->buffer[_pkt->size]);
		if (_pkt->buffer[_pkt->size] > MAX_PKT_SIZE) {
			DBGMSG(DBG_WALLPAD, "Invalid PACKET SIZE(%d)\r\n",
				_pkt->buffer[_pkt->size]);
			_pkt->size = 0;
			return 0;
		}
		_pkt->size++;
	default:	/* data */
		t_size = _pkt->buffer[DATA_LEN_IDX] + 7;
		if (_pkt->size == t_size) 
			return 1;
		ret = m_serial.Read(&_pkt->buffer[_pkt->size], t_size-_pkt->size);
		if (ret > 0)
			_pkt->size = _pkt->size + ret;		
		if (_pkt->size == t_size) 
			return 1;
		break;
	}
	
	return 0;
}

u8 CWallPadEzville::pkt_verify(ezv_pkt_info_t *_pkt)
{
	u8 xor_sum, add_sum;
	
	xor_sum = calc_xor_sum(_pkt->buffer, _pkt->size);
	if (xor_sum != _pkt->buffer[_pkt->size-2]) {
		DBGMSG(DBG_WALLPAD, "<%s> Invalid xor sum(0x%X / 0x%X)\r\n",
			__func__, xor_sum, _pkt->buffer[_pkt->size-2]);
		return 0;
	}

	add_sum = calc_add_sum(_pkt->buffer, _pkt->size);
	if (add_sum != _pkt->buffer[_pkt->size-1]) {
		DBGMSG(DBG_WALLPAD, "<%s> Invalid add sum(0x%X / 0x%X)\r\n",
			__func__, add_sum, _pkt->buffer[_pkt->size-1]);
		return 0;
	}
	
	return 1;
}

void CWallPadEzville::build_pkt(ezv_pkt_info_t *_pkt, u8 _cmd, u8 *_data, int _size)
{	
	memset(_pkt, 0, sizeof(ezv_pkt_info_t));

	_pkt->size = _size + 7;
	_pkt->buffer[HEADER_IDX] = EZV_HEADER;
	_pkt->buffer[DEV_ID_IDX] = EZV_DEV_ID;
	_pkt->buffer[SUB_ID_IDX] = EZV_SUB_ID;
	_pkt->buffer[CMD_TYPE_IDX] = _cmd;
	_pkt->buffer[DATA_LEN_IDX] = _size;
	if (_size > 0)
		memcpy(&_pkt->buffer[DATA_0_IDX], _data, _size);
	_pkt->buffer[_pkt->size-2] = calc_xor_sum(_pkt->buffer, _pkt->size);
	_pkt->buffer[_pkt->size-1] = calc_add_sum(_pkt->buffer, _pkt->size);
}

u8 CWallPadEzville::conv_weather_icon_idx(u8 _idx)
{
	if (_idx == 1)
		return IMG_ENUM_WEATHER_SERENITY;	//맑음

	if ((_idx >= 2) && (_idx <= 3))
		return IMG_ENUM_WEATHER_PARTLY_CLOUDY;	//구름조금

	if ((_idx >= 5) && (_idx <= 6))
		return IMG_ENUM_WEATHER_PARTLY_CLOUDY;	//구름조금

	if (_idx == 4)
		return IMG_ENUM_WEATHER_CLOUDY;	//흐림

	if ((_idx >= 7) && (_idx <= 9))
		return IMG_ENUM_WEATHER_CLOUDY;	//흐림

	if ((_idx >= 10) && (_idx <= 17))
		return IMG_ENUM_WEATHER_RAINNY;	//비

	if ((_idx >= 18) && (_idx <= 35))
		return IMG_ENUM_WEATHER_SNOW;		//눈

	if ((_idx >= 36) && (_idx <= 38))
		return IMG_ENUM_WEATHER_SHOWER;	//소나기

	return IMG_ENUM_WEATHER_THUNDERSTORM;	//뇌우
}

/* [DATA_1_IDX] -> 년, 		[DATA_2_IDX] -> 월, [DATA_3_IDX] -> 일,
/* [DATA_4_IDX] -> 시, 		[DATA_5_IDX] -> 분, [DATA_6_IDX] -> 초, 
/* [DATA_7_IDX] -> 아이콘,	[DATA_8_IDX] -> 실외온도 영상/영하,
/* [DATA_9_IDX] -> 실외온도 정수자리, [DATA_10_IDX] -> 실외온도 소수점 이하 */
void CWallPadEzville::update_weather_info(u8 *pBuffer)
{
	struct ezv_weather_info *weather_info = 
		(struct ezv_weather_info *)&pBuffer[DATA_1_IDX];
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_WEATHER	pMtmWeather = 
		(PMTM_DATA_WEATHER)&buffer_mtm[sizeof(MTM_HEADER)];
	struct tm t = {0,};
	struct timeval tvNew = {0,};
	int i, temperature;

	printf("<%s>\r\n", __func__);
	
	pMtmHdr->type = MTM_DATA_TYPE_WEATHER;
	pMtmHdr->len  = sizeof(MTM_DATA_WEATHER);

	//현재시간(arg1~arg6)
	pMtmWeather->year	= weather_info->year;
	pMtmWeather->month	= weather_info->month;
	pMtmWeather->day	= weather_info->day;
	pMtmWeather->hour	= weather_info->hour;
	pMtmWeather->minute	= weather_info->minute;
	pMtmWeather->second	= weather_info->second;

	t.tm_year = weather_info->year + 100;
	t.tm_mon  = weather_info->month - 1;
	t.tm_mday = weather_info->day;
	t.tm_hour = weather_info->hour;
	t.tm_min  = weather_info->minute;
	t.tm_sec  = weather_info->second;

	tvNew.tv_sec  = mktime(&t);
	tvNew.tv_usec = 0;

	if (settimeofday(&tvNew, NULL) == ERROR) {
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] settimeofday: error : %d %s\r\n", errno, strerror(errno));
	} else {
		system("hwclock -w");
		g_timer.Refresh();
		DBGMSG(DBG_WALLPAD, "[WallPad:HDT] settimeofday: done\r\n");
	}

	//날씨아이콘(arg7)
	pMtmWeather->weather2 = conv_weather_icon_idx(weather_info->weather);

	//온도(arg8~arg10)
	if (weather_info->temp_under >= 10)
		weather_info->temp_under = 1;	//소숫점 1자리로 맞춤
	if (weather_info->temp_range) {	//영하
		pMtmWeather->temp2_low  = 
			(weather_info->temp_int*(-10) - weather_info->temp_under);
	} else {
		pMtmWeather->temp2_low  = 
			(weather_info->temp_int*(10) + weather_info->temp_under);;
	}
	pMtmWeather->temp2_high = pMtmWeather->temp2_low;
	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_WEATHER)] = MTM_PACKET_ETX;

	//	월패드가 시간마다 주는 데이타만 쉬프트 시킴
	g_setup_data.m_SetupData.weather_left = pMtmWeather->weather2;
	g_setup_data.m_SetupData.temp_low_left = pMtmWeather->temp2_low;
	g_setup_data.m_SetupData.temp_high_left = pMtmWeather->temp2_high;
	g_setup_data.m_SetupData.weather_right = pMtmWeather->weather2;
	g_setup_data.m_SetupData.temp_low_right = pMtmWeather->temp2_low;
	g_setup_data.m_SetupData.temp_high_right = pMtmWeather->temp2_high;

	if (g_state.GetState() == STATE_WEATHER) {
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}

void CWallPadEzville::update_parking_info(u8 *pBuffer)
{
	struct ezv_parking_info *pParkingInfo = 
		(struct ezv_parking_info *)pBuffer;
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_PARKING pMtmParking = 
		(PMTM_DATA_PARKING)&buffer_mtm[sizeof(MTM_HEADER)];
	char *dump_ptr;
	int i, index;

	printf("<%s>\r\n", __func__);
	
	pMtmHdr->type = MTM_DATA_TYPE_PARKING;
	pMtmHdr->len  = sizeof(MTM_DATA_PARKING);

	//주차상태 : 입차만 확인
	pMtmParking->status = MTM_DATA_PARKING_STATUS_IN;

	//소문자->대문자 변환
	if (pParkingInfo->floor_name >= 'a')
		pParkingInfo->floor_name -= 0x20;
	if (pParkingInfo->zone_name >= 'a')
		pParkingInfo->zone_name  -= 0x20;

	// verify the parking info
	if (!ALPHABET_CHAR(pParkingInfo->floor_name)) {
		printf("Invalid floor name: %c\r\n", pParkingInfo->floor_name);
		return;
	}
#if 0	
	if (!ALPHABET_CHAR(pParkingInfo->floor_num) && 
		!DECIMAL_CHAR(pParkingInfo->floor_num) && 
		!DECIMAL_BYTE(pParkingInfo->floor_num)) {
		printf("Invalid floor number: %d\r\n", pParkingInfo->floor_num);
		return;
	}
	
	if (!ALPHABET_CHAR(pParkingInfo->zone_name) && 
		!DECIMAL_CHAR(pParkingInfo->zone_name) &&
		!DECIMAL_BYTE(pParkingInfo->zone_name)) {
		printf("Invalid zone name: %d\r\n", pParkingInfo->zone_name);
		return;
	}
#endif

	//층명칭
	if (DECIMAL_BYTE(pParkingInfo->floor_num))
		sprintf(pMtmParking->floor, "%c%d", pParkingInfo->floor_name, pParkingInfo->floor_num);
	else
		sprintf(pMtmParking->floor, "%c%c", pParkingInfo->floor_name, pParkingInfo->floor_num);
	
	//구역명칭
	if (DECIMAL_BYTE(pParkingInfo->zone_name))
		sprintf(pMtmParking->zone, "%d-%d", pParkingInfo->zone_name, pParkingInfo->zone_num);
	else
		sprintf(pMtmParking->zone, "%c-%d", pParkingInfo->zone_name, pParkingInfo->zone_num);

	memcpy(pMtmParking->car_num, pParkingInfo->tag_id, 2);
	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_PARKING)] = MTM_PACKET_ETX;
	if (pMtmParking->status == MTM_DATA_PARKING_STATUS_IN) {
		g_setup_data.m_SetupData.status = pMtmParking->status;
		memcpy(g_setup_data.m_SetupData.car_floor, pMtmParking->floor, MAX_PARKING_FLOOR_NAME);
		memcpy(g_setup_data.m_SetupData.car_zone,  pMtmParking->zone, MAX_PARKING_ZONE_NAME);
		memset(g_setup_data.m_SetupData.car_num, 0, MAX_PARKING_CAR_NUM);		//HDT는 차량번호 없음
	}

	parking_list_update(pMtmParking);

	if ((g_state.GetState() == STATE_WEATHER) || (g_state.GetState() == STATE_PARKING)) {
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}

/* [DATA_1_IDX]  1호기 / 2호기 */
/* [DATA_2_IDX]  지상 (0), 로비(0x4C), 지하 (0x42) */
/* [DATA_3_IDX]  floor (1 ~ max) */
/* [DATA_4_IDX]  direction (상향 : 1, 하향 : 0) */
void CWallPadEzville::update_elevator_info(u8 *_pkt)
{
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_ELEVATOR	pMtmElv =
		(PMTM_DATA_ELEVATOR)&buffer_mtm[sizeof(MTM_HEADER)];
	char ev_floor;
	u8 ev_status;

	printf("<%s>\r\n", __func__);

	if (_pkt[DATA_1_IDX] == 1) {	// Arrive
		ev_status = MTM_DATA_EV_STATUS_ARRIVE;
	} else {
		if (_pkt[DATA_4_IDX] == 1)
			ev_status = MTM_DATA_EV_STATUS_UP;
		else
			ev_status = MTM_DATA_EV_STATUS_DOWN;
	}

	if (_pkt[DATA_2_IDX] == 0x42)
		ev_floor = -(_pkt[DATA_3_IDX]);
	else
		ev_floor = _pkt[DATA_3_IDX];

	if (m_request_id != RQ_ID_ELEVATOR) {
		if ((ev_status == g_setup_data.m_SetupData.elevator_status) &&
			(ev_floor == g_setup_data.m_SetupData.elevator_floor)) {
			printf("Aleady Seted \r\n", __func__);
			return;
		}
	}
	
	g_setup_data.m_SetupData.elevator_status = ev_status;
	g_setup_data.m_SetupData.elevator_floor = ev_floor;
	if (g_state.GetState() == STATE_ELEVATOR) {
		pMtmHdr->type = MTM_DATA_TYPE_ELEVATOR;
		pMtmHdr->len  = sizeof(MTM_DATA_ELEVATOR);
		pMtmElv->status = g_setup_data.m_SetupData.elevator_status;
		pMtmElv->floor  = g_setup_data.m_SetupData.elevator_floor;
		buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_ELEVATOR)] = MTM_PACKET_ETX;
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}

void CWallPadEzville::report_elevator_called(u8 *_pkt)
{
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_ELEVATOR	pMtmElv =
		(PMTM_DATA_ELEVATOR)&buffer_mtm[sizeof(MTM_HEADER)];

	printf("<%s>\r\n", __func__);

	if (g_state.GetState() == STATE_ELEVATOR) {
		pMtmHdr->type = MTM_DATA_TYPE_ELEVATOR;
		pMtmHdr->len  = sizeof(MTM_DATA_ELEVATOR);
		pMtmElv->status = MTM_DATA_EV_STATUS_CALLED;
		pMtmElv->floor = 0;
		buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_ELEVATOR)] = MTM_PACKET_ETX;
		g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	}
}

void CWallPadEzville::update_security_info(u8 _new_stat)
{
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_SECURITY	pMtmSecurity = 
		(PMTM_DATA_SECURITY)&buffer_mtm[sizeof(MTM_HEADER)];

	printf("<%s>\r\n", __func__);

	if (m_request_id != RQ_ID_SECURITY) {
		if (_new_stat == g_setup_data.m_SetupData.security_stat) {
			printf("Aleady Seted \r\n");
			return;
		}
	}

	g_setup_data.m_SetupData.security_stat = _new_stat;
	pMtmHdr->type = MTM_DATA_TYPE_SECURITY;
	pMtmHdr->len  = sizeof(MTM_DATA_SECURITY);
	if(g_setup_data.m_SetupData.security_stat)
		pMtmSecurity->status = MTM_DATA_SECURITY_ON;
	else
		pMtmSecurity->status = MTM_DATA_SECURITY_OFF;
	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_SECURITY)] = MTM_PACKET_ETX;
	g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
}

void CWallPadEzville::update_gas_info(u8 _new_stat)
{
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_SECURITY	pMtmSecurity = 
		(PMTM_DATA_SECURITY)&buffer_mtm[sizeof(MTM_HEADER)];
	u8 gas_cut;
	
	printf("<%s>\r\n", __func__);

	if (_new_stat)
		gas_cut = 0;
	else
		gas_cut = 1;

	if (m_request_id != RQ_ID_GAS) {
		if (gas_cut == g_setup_data.m_SetupData.gas_stat) {
			printf("Aleady Seted(%d)\r\n", _new_stat);
			return;
		}
	}

	g_setup_data.m_SetupData.gas_stat = gas_cut;
	pMtmHdr->type = MTM_DATA_TYPE_GAS;
	pMtmHdr->len  = sizeof(MTM_DATA_GAS);
	if(g_setup_data.m_SetupData.gas_stat)
		pMtmSecurity->status = MTM_DATA_GAS_CUT;
	else
		pMtmSecurity->status = MTM_DATA_GAS_OPEN;
	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_GAS)] = MTM_PACKET_ETX;
	g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
}

void CWallPadEzville::update_light_info(u8 _new_stat)
{
	UCHAR buffer_mtm[MTM_PACKET_MAX] = {0,};
	PMTM_HEADER pMtmHdr = (PMTM_HEADER)&buffer_mtm[0];
	PMTM_DATA_LIGHT pMtmDataLight = 
		(PMTM_DATA_LIGHT)&buffer_mtm[sizeof(MTM_HEADER)];

	printf("<%s>\r\n", __func__);

	if (m_request_id != RQ_ID_SWITCH) {
		if (_new_stat == g_setup_data.m_SetupData.light_stat) {
			printf("Aleady Seted \r\n");
			return;
		}
	}

	g_setup_data.m_SetupData.light_stat = _new_stat;
	pMtmHdr->type = MTM_DATA_TYPE_LIGHT;
	pMtmHdr->len  = sizeof(MTM_DATA_LIGHT);
	if(g_setup_data.m_SetupData.light_stat)
		pMtmDataLight->status = MTM_DATA_LIGHT_OFF;
	else
		pMtmDataLight->status = MTM_DATA_LIGHT_ON;
	buffer_mtm[sizeof(MTM_HEADER)+sizeof(MTM_DATA_LIGHT)] = MTM_PACKET_ETX;
	g_message.SendMessageData(MSG_WALLPAD_DATA, buffer_mtm, pMtmHdr->len+5);
	g_gpio_mtm.SetGpioOutPin(GPIO_RELAY, (_new_stat==1) ? LOW : HIGH);			
}

u8 CWallPadEzville::wp_polling_pkt(u8 *_pkt)
{
	ezv_pkt_info_t *pkt = &g_send_pkt;
	u8 data[5] = {0,};
	
	if (!get_request_data(RQL_CONTROL, data)) {
		if (!get_request_data(RQL_STATUS, data)) 
			get_request_dummy_data(data);
	}
	
	build_pkt(pkt, CMD_STATUS_RSP, data, STATUS_REQ_SIZE);

	usleep(PKT_WAIT_TIME_1);

	if (Write(pkt->buffer, pkt->size) <= 0) {
		DBGMSG(DBG_WALLPAD, "<%s> Write Faile\r\n", __func__);
	}	
}

u8 CWallPadEzville::wp_status_pkt(u8 *_pkt, int _size)
{
	ezv_pkt_info_t *pkt = &g_send_pkt;
	u8 rsp_data[64] = {0,};
	
	switch (_pkt[DATA_0_IDX]) {
	case WCP_SWITCH:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_SWITCH;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;		
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 4);		
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_SWITCH)
			delete_request_data(RQL_STATUS);
		update_light_info(_pkt[DATA_1_IDX]);
		break;
	case WCP_GAS:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_GAS;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 4);			
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_GAS)
			delete_request_data(RQL_STATUS);
		update_gas_info(_pkt[DATA_1_IDX]);
		break;
	case WCP_SECURITY:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_SECURITY;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;		
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 4);	
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_SECURITY)
			delete_request_data(RQL_STATUS);
		update_security_info(_pkt[DATA_1_IDX]);
		break;
	case WCP_ELEVATOR:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_ELEVATOR;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = _pkt[DATA_2_IDX];
		rsp_data[4] = _pkt[DATA_3_IDX];		
		rsp_data[5] = _pkt[DATA_4_IDX];		
		rsp_data[6] = 0;		
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 7);
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_ELEVATOR)
			delete_request_data(RQL_STATUS);
		update_elevator_info(_pkt);
		break;
	case WCP_WEATHER:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_WEATHER;
		rsp_data[2] = 0;
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 4);	
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_WEATHER)
			delete_request_data(RQL_STATUS);		
		update_weather_info(_pkt);
		break;
	case WCP_PARKING:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_PARKING;
		rsp_data[2] = 0;
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 4);	
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_PARKING)
			delete_request_data(RQL_STATUS);		
		update_parking_info(&_pkt[DATA_2_IDX]);
		if (_size > 22)
			update_parking_info(&_pkt[21]);
		break;
	case WCP_DOOR:
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_DOOR;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_STATUS_ACK, rsp_data, 4);
		usleep(PKT_WAIT_TIME);	
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if (m_request_id == RQ_ID_DOOR)
			delete_request_data(RQL_STATUS);		
		g_setup_data.m_SetupData.door_opened = _pkt[DATA_1_IDX];
		break;
	default:
		//delete_request_data(RQL_STATUS);		
		break;
	}
}

u8 CWallPadEzville::verify_ctrl_pkt(u8 *_pkt, u8 _id, u8 _data)
{
	u8 ctrl_data;
	
	if (!CHECK_RQL(_pkt[REQ_ID_IDX], _id)) {
		printf("invalid id_idx\r\n");
		return 0;
	}
	
	if (!CHECK_RQL(_pkt[REQ_CTRL_IDX], RQL_CONTROL)) {
		printf("invalid ctrl_idx\r\n");
		return 0;
	}		

	if (_pkt[REQ_ONOFF_IDX] == 2) // OFF
		ctrl_data = 0;
	else
		ctrl_data = 1;
	if (ctrl_data != _data) {
		printf("invalid data!(%d / %d)\r\n", _data, ctrl_data);
		return 0;
	} 

	return 1;
}

u8 CWallPadEzville::wp_ctrl_pkt(u8 *_pkt)
{
	ezv_pkt_info_t *pkt = &g_send_pkt;
	u8 rsp_data[64] = {0,};
	u8 ctrl_data[5] = {0,}, res;

	res = get_request_data(RQL_CONTROL, ctrl_data);	
	switch (_pkt[DATA_0_IDX]) {
	case MCP_SWITCH:
		if (res) 
			res = verify_ctrl_pkt(ctrl_data, RQL_SWITCH, _pkt[DATA_1_IDX]);		
		if (res) {
			rsp_data[0] = 0;	// OK
		} else {
			rsp_data[0] = 1;	// ERROR
		}
		rsp_data[1] = WCP_SWITCH;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_CTRL_ACK, rsp_data, 4);		
		usleep(PKT_WAIT_TIME);
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}	
		if (res)
			delete_request_data(RQL_CONTROL);
		break;
	case MCP_GAS:
		if (res) 
			res = verify_ctrl_pkt(ctrl_data, RQL_GAS, _pkt[DATA_1_IDX]);		
		if (res) 
			rsp_data[0] = 0;	// OK
		else
			rsp_data[0] = 1;	// ERROR
		rsp_data[1] = WCP_GAS;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_CTRL_ACK, rsp_data, 4);	
		usleep(PKT_WAIT_TIME);
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}	
		if (res) {
			delete_request_data(RQL_CONTROL);
			update_gas_info(_pkt[DATA_1_IDX]);
		}
		break;
	case MCP_SECURITY:
		if (res) 
			res = verify_ctrl_pkt(ctrl_data, RQL_SECURITY, _pkt[DATA_1_IDX]);		
		if (res) 
			rsp_data[0] = 0;	// OK
		else 
			rsp_data[0] = 1;	// ERROR
		rsp_data[1] = WCP_SECURITY;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_CTRL_ACK, rsp_data, 4);	
		usleep(PKT_WAIT_TIME);
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}			
		if (res) {
			delete_request_data(RQL_CONTROL);
			update_security_info(_pkt[DATA_1_IDX]);
		}
		break;
	case MCP_ELEVATOR:
		/* [DATA_1_IDX]  호출 (0x01) 도착 (0x02) */
		rsp_data[0] = 0;	// OK
		rsp_data[1] = WCP_ELEVATOR;
		rsp_data[2] = _pkt[DATA_1_IDX];
		rsp_data[3] = 0;
		build_pkt(pkt, CMD_WP_CTRL_ACK, rsp_data, 4);			
		usleep(PKT_WAIT_TIME);
		if (Write(pkt->buffer, pkt->size) <= 0) {
			DBGMSG(DBG_WALLPAD, "<%s> Send ACK Packet Fail\r\n");
		}
		if ((res) && (CHECK_RQL(ctrl_data[REQ_ID_IDX], RQL_ELEVATOR)))  {
			delete_request_data(RQL_CONTROL);		
			report_elevator_called(_pkt);
		}
		break;
	}
}

//------------------------------------------------------------------------------
//  0xF7       0x53           0x01      0x01,0x41,0x43         xx         xx    xx    xx
//------------------------------------------------------------------------------
// header | device ID | sub-ID |     cmd type       | data_len | data |xor | add 
//------------------------------------------------------------------------------
void CWallPadEzville::ezv_parser(void)
{
	CContextSetup* pContextSetup = NULL;
	ezv_pkt_info_t *pkt = &g_rcv_pkt;
	ULONG color;	
	int ret;

	m_isRecv = TRUE;

	if (!pkt_rcv_n_delimeter(pkt)) {
		//printf("<%s> Data size is not enough(%d/%d)\r\n",
			//__func__, pkt->size, pkt->buffer[4]+7);
		return;
	}

	if (!pkt_verify(pkt)) {
		memset(pkt, 0, sizeof(ezv_pkt_info_t));
		return;
	}
	
	m_isRecv = FALSE;

	switch (pkt->buffer[CMD_TYPE_IDX]) {
	case CMD_STATUS_REQ:
		wp_polling_pkt(pkt->buffer);
		break;
	case CMD_WP_STATUS_RES:
		wp_status_pkt(pkt->buffer, pkt->size);
		break;
	case CMD_WP_CTRL_RES:
		wp_ctrl_pkt(pkt->buffer);
		break;
	default:
		DBGMSG(DBG_WALLPAD, "<%s> Invalid CMD TYPE: d\r\n", 
			__func__, pkt->buffer[CMD_TYPE_IDX]);
		break;
	}
	
	if (g_isContextSetupWallPad) {
		pContextSetup = (CContextSetup*)g_state.GetCurrContext();
		if (pContextSetup) {
			switch (pkt->buffer[CMD_TYPE_IDX]) {
			case CMD_WP_STATUS_RES:
			case CMD_WP_CTRL_RES:
				color = LTCYAN;
				break;
			default:
				color = LTGREEN;
				break;
			}
			pContextSetup->PrintPacket(pkt->buffer, pkt->size, color);
		}
	}

	memset(pkt, 0, sizeof(ezv_pkt_info_t));
}


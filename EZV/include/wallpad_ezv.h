#ifndef __WALLPAD_EZVILLE_H__
#define __WALLPAD_EZVILLE_H__

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;

#define STATUS_REQ_SIZE	5
enum control_bit {
	RQL_STATUS = 0,
	RQL_CONTROL,
};

enum onoff_bit {
	RQL_ON = 0,
	RQL_OFF,
};

#define EZV_MAX_PKT_SIZE	512

typedef struct {
	int size;
	u8 buffer[EZV_MAX_PKT_SIZE];
} ezv_pkt_info_t;

typedef struct req_info {
	u8 data[STATUS_REQ_SIZE];
	u8 retry_cnt;
	struct req_info *next;
} req_node_t;

typedef struct {
	req_node_t *head;
	req_node_t *tail;
	int cnt;
} req_info_list_t;

class CWallPadEzville : public CWallPad
{
public:
	CWallPadEzville();
	~CWallPadEzville();

	//
	// Interface Function
	//
	virtual BOOL Run();
	virtual void Stop();

	virtual void RequestWeatherInfo();		//날씨정보요청

	virtual void RequestGasStatus();		//가스상태요청
	virtual void RequestGasCut();			//가스차단요청

	virtual void RequestLightStatus();
	virtual void RequestLightSet(BOOL isAllOff);		//일괄소등요청(isAllOff: TRUE=소등, FALSE=소등해제)

	virtual void RequestSecurityStatus();
	virtual void RequestSecuritySet(BOOL isSecurityOn);	//방범설정요청(isSecurityOn: TRUE=방범설정, FALSE=방범해제)

	virtual void RequestElevatorCall();		//엘레베이터 호출
	virtual void RequestElevatorStatus();	//엘레베이터 상태요청

	virtual void RequestParkingInfo();		//주차위치정보요청
	virtual void delete_all_request_data(void);
	
	//
	// Member Function
	//
	int Write(UCHAR *pData, int size);
	u8 calc_xor_sum(u8 *_data, UINT _size);
	u8 calc_add_sum(u8 *_data, UINT _size);
	void get_request_dummy_data(u8 *_data);
	void append_request_list(req_info_list_t *_list, req_node_t *_node);
	void register_request_data(u8 _req_id, u8 _ctrl, u8 _onoff, u8 _retry);
	u8 get_request_data(u8 _rql, u8 *_data);
	u8 delete_request_data(u8 _rql);
	void ezv_parser();
	u8 conv_weather_icon_idx(u8 _idx);
	u8 pkt_rcv_n_delimeter(ezv_pkt_info_t *_pkt);
	u8 pkt_verify(ezv_pkt_info_t *_pkt);
	void build_pkt(ezv_pkt_info_t *_pkt, u8 _cmd, u8 *_data, int _size);
	u8 wp_polling_pkt(u8 *_pkt);	
	u8 wp_status_pkt(u8 *_pkt, int _size);
	u8 wp_ctrl_pkt(u8 *_pkt);
	u8 verify_ctrl_pkt(u8 *_pkt, u8 _id, u8 _data);

	void report_request_failed(u8 _rql);
	void report_elevator_called(u8 *_pkt);
	void update_light_info(u8 _new_stat);
	void update_gas_info(u8 _new_stat);
	void update_security_info(u8 _new_stat);
	void update_elevator_info(u8 *_pkt);
	void update_weather_info(u8 *pBuffer);
	void update_parking_info(u8 *pBuffer);
	
	static void* SerialListener_EZV(void *pParam);	
	//
	// Member Variable
	//
	CSerial		m_serial;
	BOOL		m_isRecv;					//수신중 여부 Flag
	u8 m_request_id;
	pthread_mutex_t m_wp_mutex;
};

#endif //__WALLPAD_EZVILLE_H__


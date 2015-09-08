#ifndef __SERIAL_INVERTER_H__
#define __SERIAL_INVERTER_H__

enum
{
	IVT_TYPE_HEX,
	IVT_TYPE_HEX2,
	IVT_TYPE_LS,
	IVT_TYPE_COUNT
};

//
// 인버터 어드레스 매핑 : 0x0020 ~ 0x006F 까지의 Word 갯수
//
/*
		   0	   1	   2	   3	   4	   5	   6	   7
	0	0x0020	0x0022	0x0024	0x0026	0x0028	0x002A	0x002C	0x002E
	8	0x0030	0x0032	0x0034	0x0036	0x0038	0x003A	0x003C	0x003E
	16	0x0040	0x0042	0x0044	0x0046	0x0048	0x004A	0x004C	0x004E	(unused)
	24	0x0050	0x0052	0x0054	0x0056	0x0058	0x005A	0x005C	0x005E
	32	0x0060	0x0062	0x0064	0x0066	0x0068	0x006A	0x006C	0x006E
*/
#define MAX_RES							40

#define MEASURE_START_ADDR				0x0020
#define MEASURE_END_ADDR				0x006E

//
// Measure Item Address
//

//2. 태양전지 계측정보 명령 (0x002x)
#define MEASURE_ADDR_SOLAR_VOLT			0x0020	//Data0~Data3 - Solar volt rms address - 태양전지 전압
#define MEASURE_ADDR_SOLAR_CURRENT		0x0022	//Data4~Data7 - Solar current rms address - 태양전지 전류

//3. 인버터 계측 정보 명령 (0x003x)
#define MEASURE_ADDR_IVT_VOLT			0x0030	//Data00~Data03 - Inverter voltage rms address - 인버터 전압
#define MEASURE_ADDR_IVT_CURRENT		0x0036	//Data12~Data15 - Inverter current rms address - 인버터 전류
#define MEASURE_ADDR_IVT_FREQ			0x003C	//Data24~Data27 - Inverter frequency address - 인버터 주파수

//4. 계통 계측 정보 명령 (0x005x)
#define MEASURE_ADDR_LINE_VOLT			0x0050	//Data00~Data03 - Line voltage rms address - 계통 전압
#define MEASURE_ADDR_LINE_CURRENT		0x0056	//Data12~Data15 - Line current rms address - 계통 전류
#define MEASURE_ADDR_LINE_FREQ			0x005C	//Data24~Data27 - Line frequency address - 계통 주파수

//5. 전력량 계측 정보 명령 (0x006x)
#define MEASURE_ADDR_SOLAR_POWER		0x0060	//Data00~Data03 - Solar kW address - 태양전지 현재 전력
#define MEASURE_ADDR_IVT_POWER_HIGH		0x0062	//Data04~Data07 - Inv kWh High address - 인버터 적산 전력량 High
#define MEASURE_ADDR_IVT_POWER_LOW		0x0064	//Data08~Data11 - Inv kWh Low address - 인버터 적산 전력량 Low
#define MEASURE_ADDR_IVT_POWER_CURRENT	0x0066	//Data12~Data15 - Inverter kVA address - 인버터 현재 전력
#define MEASURE_ADDR_IVT_POWER_MAX		0x0068	//Data16~Data19 - Inverter Max Power address - 인버터 최대 출력
#define MEASURE_ADDR_IVT_POWER_FACTOR	0x006A	//Data20~Data23 - Inverter PF address - 인버터 역률
#define MEASURE_ADDR_IVT_POWER_DAILY	0x006C	//Data24~Data27 - Inverter daily kWh address - 인버터 하루 적산 전력량

//
// Measure Item Index
//
enum
{
	//2. 태양전지 계측정보 명령 (0x002x)
	MEASURE_IDX_SOLAR_VOLT,
	MEASURE_IDX_SOLAR_CURRENT,

	//3. 인버터 계측 정보 명령 (0x003x)
	MEASURE_IDX_IVT_VOLT,
	MEASURE_IDX_IVT_CURRENT,
	MEASURE_IDX_IVT_FREQ,

	//4. 계통 계측 정보 명령 (0x005x)
	MEASURE_IDX_LINE_VOLT,
	MEASURE_IDX_LINE_CURRENT,
	MEASURE_IDX_LINE_FREQ,

	//5. 전력량 계측 정보 명령 (0x006x)
	MEASURE_IDX_SOLAR_POWER,
	MEASURE_IDX_IVT_POWER_HIGH,
	MEASURE_IDX_IVT_POWER_LOW,
	MEASURE_IDX_IVT_POWER_CURRENT,
	MEASURE_IDX_IVT_POWER_MAX,
	MEASURE_IDX_IVT_POWER_FACTOR,
	MEASURE_IDX_IVT_POWER_DAILY,
	MEASURE_IDX_COUNT
};

#define MAX_IVT_RECV_BUFFER				64

//요청갯수를 고정으로 4개씩하므로 수신패킷도 29바이트로 균일, 9600bsp에서 24.2ms
#define WAIT_FOR_IVT_PACKET_RECV		30000

class CSerialInverter : public CSerial
{
public:
	CSerialInverter();
	~CSerialInverter();

	//Member Function
	BOOL Open(const char *pszDev, unsigned int baudRate=B9600);

	void Run();
	void Stop();

	void Enable485(BOOL isEnable);
	void Enable422(BOOL isEnable);

	void StartMeasure()		{ m_isMeasureStart = TRUE; }
//	void StopMeasure()		{ m_isMeasureStart = FALSE; }

	int SendRequest(UCHAR id, USHORT addr, UCHAR count);

	USHORT CalcCheckSum(UCHAR* pBuffer, UINT size);

	static void* InverterMeasureTask(void *pParam);

	void CreateResTable();
	void RemoveResTable();

	//Member Variable
	int			m_ivt_type;		//Inverter Type
	BOOL		m_is485;		//TRUE=485, FALSE=422

	IVT_REQ		m_req;

	BOOL		m_isRecv;
	BOOL		m_isMeasureStart;
	BOOL		m_isWaitForResponse;
	UINT		m_nMeasureStep;

//	IVT_RES_NODE*	m_pRootNode;
	UCHAR		m_nIvtCount;	//인버터갯수=국번의갯수
	UCHAR*		m_pIvtIdList;	//인버터ID List
	USHORT**	m_ppResTable;	//인버터 응답Table
};

#endif //__SERIAL_INVERTER_H__

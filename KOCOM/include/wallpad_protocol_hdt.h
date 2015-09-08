/*
	현대통신 월패드 프로토콜

	기반문서: 
		MTM LCD 프로토콜(현대통신연동) 버전1.1, 2012-11-19

	패킷구조:
		+------------------------+---------------------+---------+
		|          HEAD          |         BODY        |   TAIL  |	패킷구분
		+---+---+---+---+----+---+---+---+-------------+----+----+
		| 1 | 2 | 3 | 4 |  5 | 6 | 7 | 8 |    9(1~n)   | 10 | 11 |	필드순서
		+---+---+---+---+----+---+---+---+-------------+----+----+
		|STX|LEN|VEN|DEV|TYPE|SRV|LOC|CMD|ARG[1]~ARG[n]|CRC |ETX |	필드명칭
		+---+---+---+---+----+---+---+---+-------------+----+----+
		| 1 | 1 | 1 | 1 |  1 | 1 | 1 | 1 |      n      |  1 |  1 |	필드크기
		+---+---+---+---+----+---+---+---+-------------+----+----+

		STX		: 0xF7
		LEN		: STX ~ ETX
		VEN		: 0x00을 제외한 Vendor ID, v2.10부터 통합ID(0x01)을 사용함 (이전 고유ID 호환성은 유지)
		DEV		: 0x00을 제외한 Device ID
				  EventSender=0x44, USS일괄스위치=0x2A, 엘리베이터 호출스위치=0x34
		TYPE	: 0x00을 제외한 Packet Type
				  현대통신 - Event(Information)=0x0C, Query(상태요구)=0x01, Control(제어명령)=0x02, 
				  기기업체 - Response Success=0x04
				  *기기는 Event에 대한 응답을 보내지 않음
		SRV		: 0x00을 제외한 Service ID
				  EventSender	- 날씨정보(0x62), 주차정보(0x63), 현관 마그네틱 센서정보(0x68), 일괄소등 설정/해제 정보(0x69)
				  USS일괄스위치	- 전원(0x40), 개폐(0x43), 방범설정(U-보안)(0x60)
				  E/V호출스위치	- 단계조절(Up/Down)(0x41)
		LOC		: Location (동종기기의 그룹/개별 식별자)
				  상위니블=그룹, 하위니블=그룹내 ID (유효값:1~f,  0=전체)
				  ex) 전체기기(0x00), 그룹1-기기1(0x11), 그룹2전체(0x20)
		CMD		: 0x00을 제외한 Command
				  EventSender	- 사용안함
				  USS일괄스위치 - 전원 - ON(0x01), OFF(0x02)
								  개폐 - CLOSE(0x03), OPEN(0x04)
								  방범설정 - 외출방범설정(0x07), 외출방범해제(0x08), 외출지연(0x0A), 입장지연(0x0B)
				  E/V호출스위치 - 단계조절 - UP(0x05), DOWN(0x06)
		ARG		: Argument (n bytes)
				  EventSender	- 날씨정보(10),주차정보(15),현관마그네틱센서정보(7),일괄소등설정/해제정보(7)
				  USS일괄스위치 - 전원/일괄소등(2), 개폐/일괄가스(2), 방범설정/보안(2)
				  E/V호출스위치 - 현재상태(1), 현재층(1), 호기(1)
		CRC		: STX~ARG[n]까지의 XOR
		ETX		: 0xEE

	통신사양:
		9600bps, 8bit, 1stopbit, none parity

		- 30[ms] <= 응답시간 < 300[ms]
		- 통신바이트 간격 <= 5[ms]
*/

#ifndef __WALLPAD_PROTOCOL_HDT_H__
#define __WALLPAD_PROTOCOL_HDT_H__


//
// 패킷 관련 정의
//
#define HDT_STX							0xF7
#define HDT_ETX							0xEE

//필드 Offset : ARG까지 (ARG가 가변이라 CRC,ETX는 ARG크기에 따라 Offset이 달라짐)
enum
{
	HDT_OFFSET_STX,
	HDT_OFFSET_LEN,
	HDT_OFFSET_VEN,
	HDT_OFFSET_DEV,
	HDT_OFFSET_TYPE,
	HDT_OFFSET_SRV,
	HDT_OFFSET_LOC,
	HDT_OFFSET_CMD,
	HDT_OFFSET_ARG
};

//
// 타이밍 관련 정의
//
#define HDT_RES_TIME_NORMAL				30		// [ms]
#define HDT_RES_TIME_MAX				300		// [ms]
#define HDT_BYTE_INTERVAL_MAX			5		// [ms]

//
// 패킷 필드별 정의
//

// VEN (Vendor ID)
#define HDT_VEN_UNIFY					0x01	// 통합ID (v2.10부터)

// DEV (Device ID)
#define HDT_DEV_EVENT_SENDER			0x44	// EventSender
#define HDT_DEV_USS_SWITCH				0x2A	// USS일괄스위치
#define HDT_DEV_ELV_SWITCH				0x34	// 엘리베이터 호출스위치

#define HDT_DEV_LIGHT_SWITCH			0x19	// 일괄소등 스위치
#define HDT_DEV_GAS_SWITCH				0x1B	// 가스차단 스위치
#define HDT_DEV_SECURITY				0x16	// 보안장비

// TYPE (Packet Type)
#define HDT_TYPE_EVENT					0x0C	// HDT-->기기, Event(Information), 응답필요없음
#define HDT_TYPE_QUERY					0x01	// HDT-->기기, Query(상태요구)
#define HDT_TYPE_CONTROL				0x02	// HDT-->기기, Control(제어명령)
#define HDT_TYPE_RESPONSE				0x04	// HDT<->기기, Response

// SRV (Service ID)
#define HDT_SRV_WEATHER_INFO			0x62	// EventSender		날씨정보
#define HDT_SRV_PARKING_INFO			0x63	//					주차정보
#define HDT_SRV_DOOR_SENS_INFO			0x68	//					현관 마그네틱 센서정보
#define HDT_SRV_LIGHT_OUT_INFO			0x69	//					일괄소등 설정/해제 정보

#define HDT_SRV_LIGHT_OUT				0x40	// USS일괄스위치	전원(일괄소등)
#define HDT_SRV_GAS_CUT					0x43	//					개폐(가스차단)
#define HDT_SRV_SECURITY				0x60	//					방범설정(U-보안)

#define HDT_SRV_ELV_CTRL				0x41	// E/V호출스위치	단계조절(Up/Down)

// LOC (Location) : 동종기기의 그룹/개별 식별자	( 상위니블=그룹, 하위니블=그룹내 ID (유효값:1~f,  0=전체) )
#define HDT_LOC_GROUP(l)				((l>>4)&0xf)
#define HDT_LOC_DEV(l)					((l>>0)&0xf)

#define HDT_LOC_ALL						0x00

// CMD (Command)
#define HDT_CMD_EVENT					0x00	// EventSender		월패드->세대기(MTM)

#define HDT_CMD_QUERY					0x00	// USS일괄스위치, E/V호출스위치: 세대기(MTM)->월패드

#define HDT_CMD_LIGHT_OUT_ON			0x01	// USS일괄스위치	전원(일괄소등) ON
#define HDT_CMD_LIGHT_OUT_OFF			0x02	//					전원(일괄소등) OFF

#define HDT_CMD_GAS_VALVE_CLOSE			0x03	//					개폐(가스차단) CLOSE  - 사용중
#define HDT_CMD_GAS_VALVE_OPEN			0x04	//					개폐(가스차단) OPEN   - 사용차단

#define HDT_CMD_SECURITY_ON				0x07	//					방범설정 - 외출방범설정
#define HDT_CMD_SECURITY_OFF			0x08	//					방범설정 - 외출방범해제
#define HDT_CMD_SECURITY_DELAY_OUT		0x0A	//					방범설정 - 외출지연
#define HDT_CMD_SECURITY_DELAY_IN		0x0B	//					방범설정 - 입장지연

#define HDT_CMD_ELV_UP					0x05	// E/V호출스위치	단계조절 - UP
#define HDT_CMD_ELV_DOWN				0x06	//					단계조절 - DOWN

//
// 패킷 유형별 데이타 정의 (ARGUMENT)
//

/*
	EventSender - 날씨정보 : 10 bytes

	+-----------------------------+------+---------------------------------+
	|             TIME            | 날씨 |             실외온도            |
	+----+----+----+----+----+----+------+-----------+----------+----------+
	|  1 |  2 |  3 |  4 |  5 |  6 |   7  |     8     |     9    |    10    |
	+----+----+----+----+----+----+------+-----------+----------+----------+
	| 년 | 월 | 일 | 시 | 분 | 초 | 날씨 | 영상/영하 | 정수자리 | 소수이하 |
	+----+----+----+----+----+----+------+-----------+----------+----------+
*/

// 날씨정보 데이타 필드별 오프셋
enum
{
	HDT_EVS_ARG_OFFSET_WEATHER_OFFSET_YEAR,
	HDT_EVS_ARG_OFFSET_WEATHER_MONTH,
	HDT_EVS_ARG_OFFSET_WEATHER_DAY,
	HDT_EVS_ARG_OFFSET_WEATHER_HOUR,
	HDT_EVS_ARG_OFFSET_WEATHER_MIN,
	HDT_EVS_ARG_OFFSET_WEATHER_SEC,
	HDT_EVS_ARG_OFFSET_WEATHER_ICON,
	HDT_EVS_ARG_OFFSET_WEATHER_TEMP_PM,
	HDT_EVS_ARG_OFFSET_WEATHER_TEMP_INT,
	HDT_EVS_ARG_OFFSET_WEATHER_TEMP_DEC
};

/*
	EventSender - 주차정보 : 15 bytes

	+-----------------------------+------------------------+-------------------------------+
	|             TIME            |         카드ID         |            위치정보           |
	+----+----+----+----+----+----+----+----+----+----+----+------+------+--------+--------+
	|  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 |  12  |  13  |   14   |   15   |
	+----+----+----+----+----+----+----+----+----+----+----+------+------+--------+--------+
	| 년 | 월 | 일 | 시 | 분 | 초 |    |    |    |    |    |층명칭|층번호|구역명칭|구역번호|
	+----+----+----+----+----+----+----+----+----+----+----+------+------+--------+--------+

	카드ID	: 주차카드ID
	층명칭	: 주차층 명칭, [A-Z], 표시없을경우 0x00
	층번호	: 주차층 번호, [0x00~0x09] / [A-Z]
	구역명칭: 주차구역 명칭, [A-Z] / [0-9], 표시없을경우 0x00
	구역번호: 주차구역 번호, [0x00~0xFF]

	ex) B2-C12 : 0x62 0x02 0x63 0x0C
*/

//주차정보 데이타 필드별 오프셋
enum
{
	HDT_EVS_ARG_OFFSET_PARKING_YEAR,
	HDT_EVS_ARG_OFFSET_PARKING_MONTH,
	HDT_EVS_ARG_OFFSET_PARKING_DAY,
	HDT_EVS_ARG_OFFSET_PARKING_HOUR,
	HDT_EVS_ARG_OFFSET_PARKING_MIN,
	HDT_EVS_ARG_OFFSET_PARKING_SEC,
	HDT_EVS_ARG_OFFSET_PARKING_CARD_ID1,
	HDT_EVS_ARG_OFFSET_PARKING_CARD_ID2,
	HDT_EVS_ARG_OFFSET_PARKING_CARD_ID3,
	HDT_EVS_ARG_OFFSET_PARKING_CARD_ID4,
	HDT_EVS_ARG_OFFSET_PARKING_CARD_ID5,
	HDT_EVS_ARG_OFFSET_PARKING_FLOOR_ID,
	HDT_EVS_ARG_OFFSET_PARKING_FLOOR_NUM,
	HDT_EVS_ARG_OFFSET_PARKING_ZONE_ID,
	HDT_EVS_ARG_OFFSET_PARKING_ZONE_NUM
};

/*
	EventSender - 현관 마그네틱 센서정보 : 7 bytes

	+-----------------------------+--------------+
	|             TIME            | 현관센서정보 |
	+----+----+----+----+----+----+--------------+
	|  1 |  2 |  3 |  4 |  5 |  6 |       7      |
	+----+----+----+----+----+----+--------------+
	| 년 | 월 | 일 | 시 | 분 | 초 |              |
	+----+----+----+----+----+----+--------------+
*/

//현관 마그네틱 센서정보 데이타 필드별 오프셋
enum
{
	HDT_EVS_ARG_OFFSET_DOOR_SENS_YEAR,
	HDT_EVS_ARG_OFFSET_DOOR_SENS_MONTH,
	HDT_EVS_ARG_OFFSET_DOOR_SENS_DAY,
	HDT_EVS_ARG_OFFSET_DOOR_SENS_HOUR,
	HDT_EVS_ARG_OFFSET_DOOR_SENS_MIN,
	HDT_EVS_ARG_OFFSET_DOOR_SENS_SEC,
	HDT_EVS_ARG_OFFSET_DOOR_SENS_VALUE
};

#define HDT_EVS_ARG_VALUE_DOOR_SENS_CLOSE		0x03		//닫힘
#define HDT_EVS_ARG_VALUE_DOOR_SENS_OPEN		0x04		//열림

/*
	EventSender - 일괄소등 설정/해제 정보 : 7 bytes

	+-----------------------------+--------------+
	|             TIME            | 일골소등정보 |
	+----+----+----+----+----+----+--------------+
	|  1 |  2 |  3 |  4 |  5 |  6 |       7      |
	+----+----+----+----+----+----+--------------+
	| 년 | 월 | 일 | 시 | 분 | 초 |              |
	+----+----+----+----+----+----+--------------+
*/

//일괄소등 설정/해제 정보 데이타 필드별 오프셋
enum
{
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_YEAR,
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_MONTH,
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_DAY,
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_HOUR,
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_MIN,
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_SEC,
	HDT_EVS_ARG_OFFSET_LIGHT_OUT_VALUE
};

#define HDT_EVS_ARG_VALUE_LIGHT_OUT_ON			0x01		//일괄소등 ON
#define HDT_EVS_ARG_VALUE_LIGHT_OUT_OFF			0x02		//일괄소등 OFF

/*
	USS일괄스위치 - 일괄소등, 가스차단,보안 : 2 bytes

	+--------+-----+
	| DEV ID | CMD |
	+--------+-----+
	|    1   |  1  |
	+--------+-----+
*/

// USS일괄스위치 데이타 필드별 오프셋
enum
{
	HDT_USS_ARG_OFFSET_DEVID,
	HDT_USS_ARG_OFFSET_CMD
};

#define HDT_USS_ARG_VALUE_LIGHT_OUT_ON			0x01		// 일괄소등 ON
#define HDT_USS_ARG_VALUE_LIGHT_OUT_OFF			0x02		// 일괄소등 OFF

#define HDT_USS_ARG_VALUE_GAS_CUT_CLOSE			0x03		// 가스차단 CLOSE (접점연결 = 사용가능)
#define HDT_USS_ARG_VALUE_GAS_CUT_OPEN			0x04		// 가스차단 OPEN  (접점개방 = 사용차단)

#define HDT_USS_ARG_VALUE_SECURITY_ON			0x07		// 보안 - 외출방범설정
#define HDT_USS_ARG_VALUE_SECURITY_OFF			0x08		// 보안 - 외출방범해제
#define HDT_USS_ARG_VALUE_SECURITY_DELAY_OUT	0x0A		// 보안 - 외출지연
#define HDT_USS_ARG_VALUE_SECURITY_DELAY_IN		0x0B		// 보안 - 입장지연

/*
	E/V호출스위치 - 현재상태, 현재층, 호기 : 3 byte

	+----------+--------+------+
	| 현재상태 | 현재층 | 호기 |
	+----------+--------+------+
	|     1    |    1   |   1  |
	+----------+--------+------+

	---------------------------------------------------------------------------------------------
	현재상태		평상시						0x00
					도착						0x01
					에러(고장,수리,이사 등)		0x02
					호출성공, 호출중			0x03 (사용안함)
					호출 및 운행방향			상위니블=운행방향, 하위니블=호출방향
												상향운행=0xa0,  하향운행=0xb0
												상향호출=0x05,  하향호출=0x06
	---------------------------------------------------------------------------------------------
	현재층										지상: 0x01~0x99,	지하: 0xB1~0xBF
												평상시 또는 에러: 0x00
	---------------------------------------------------------------------------------------------
	호기										0x01~0xFF  (평상시 또는 에러: 0x00)
	---------------------------------------------------------------------------------------------
*/

#define HDT_EV_ARG_VALUE_STATUS_IDLE			0x00
#define HDT_EV_ARG_VALUE_STATUS_ARRIVE			0x01
#define HDT_EV_ARG_VALUE_STATUS_ERROR			0x02
#define HDT_EV_ARG_VALUE_STATUS_CALLED			0x03
#define HDT_EV_ARG_VALUE_STATUS_CALL(d,c)		(c|c)
	#define HDT_EV_ARG_VALUE_STATUS_DIR_UP		0xA0
	#define HDT_EV_ARG_VALUE_STATUS_DIR_DOWN	0xB0
	#define HDT_EV_ARG_VALUE_STATUS_CALL_UP		0x05
	#define HDT_EV_ARG_VALUE_STATUS_CALL_DOWN	0x06
#define HDT_EV_ARG_VALUE_CURR_FLOOR_IDLE		0x00
#define HDT_EV_ARG_VALUE_NUMBER_IDLE			0x00

#endif //__WALLPAD_PROTOCOL_HDT_H__

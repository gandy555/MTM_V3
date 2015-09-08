/*
	표준 월패드 프로토콜 for MTM Standard

	목적:
		다양한 월패드에 대응하기 위하여 MTM이 표출하는 데이타들을 기준으로
		가상의 표준 월패드를 상정하여 MTM은 표준 월패드로 부터 수신되는
		동일한 데이타 포맷에 대응하여 해석하고 표현하도록 하기위함.

	구성:
		MTM에서 월패드로의 출력은 실제적으로 각기 다른 월패드이 수신되므로
		프로토콜 자체를 표준화할 수 없기 때문에 기능적으로 표준화하여 이를 함수화 하고
		월패드에서 MTM으로의 입력은 본 프로토콜에 따라 변환하여 MTM으로 전달한다.

	패킷구조:
		+-----+------+-----+------+-----+
		| STX | TYPE | LEN | DATA | ETX |
		+-----+------+-----+------+-----+
		|  1  |   1  |  2  |   n  |  1  |
		+-----+------+-----+------+-----+
		*MTM내부에서만 사용되는 프로토콜이므로 변조의 가능성은 배제함 (CS생략)

		STX	: 0x02
		TYPE: DATA의 성격
		LEN : DATA의 길이
		DATA: 프로토콜상 65536 bytes까지 가능하나 실제적으로는 Max 256 bytes로 정의한다
		ETX	: 0x03

	DATA성격:

		1. 날씨정보

			현재시간(년월일시분초),날씨1,온도1,날씨2,온도2
			날씨1,온도1은 좌측에 표시될 정보
			날씨2,온도2는 우측에 표시될 정보
			*월패드에 따라 날씨/온도의 성격이 다르므로 변환시점에서 처리하도록 함

		2. 주차정보

			주차위치(층,구역)	;완성된 스트링

		3. 엘리베이터정보

			상태(정지/상향/하향/도착),층번호

		4. 가스밸브상태

			상태(열림/차단)
			
		5. 조명기기상태

			상태(사용/소등)

		6. 보안기기상태

			상태(해제/설정)
*/
#ifndef __WALLPAD_PROTOCOL_MTM_H__
#define __WALLPAD_PROTOCOL_MTM_H__

//
// Packet Header
//
typedef struct _MTM_HEADER
{
	UCHAR			stx;
	UCHAR			type;
	USHORT			len;
} __attribute__ ((packed)) MTM_HEADER, *PMTM_HEADER;	//size=4

#define MTM_PACKET_STX					0x02
#define MTM_PACKET_ETX					0x03

#define MTM_PACKET_MAX					32				//실제는 날씨정보일때 21 bytes

//
// Data Type
//
enum
{
	MTM_DATA_TYPE_ACK,					//ACK
	MTM_DATA_TYPE_WEATHER,				//날씨정보
	MTM_DATA_TYPE_PARKING,				//주차정보
	MTM_DATA_TYPE_ELEVATOR,				//엘리베이터정보
	MTM_DATA_TYPE_GAS,					//가스밸브상태
	MTM_DATA_TYPE_LIGHT,				//조명기기상태
	MTM_DATA_TYPE_SECURITY,				//보안기기상태
	MTM_DATA_TYPE_COUNT
};

//
// Data Format
//

/*
	날씨정보:

	+-----------------------------+--------------------+--------------------+ 
	|          현재시간           |        날씨1       |        날씨2       | 
	+----+----+----+----+----+----+------+------+------+------+------+------+ 
	| 년 | 월 | 일 | 시 | 분 | 초 | 날씨 | 최저 | 최저 | 날씨 | 최저 | 최저 | 
	+----+----+----+----+----+----+------+------+------+------+------+------+ 
	|  1 |  1 |  1 |  1 |  1 |  1 |   1  |   2  |   2  |   1  |   2  |   2  | 
	+----+----+----+----+----+----+------+------+------+------+------+------+ 
*/
typedef struct _MTM_DATA_WEATHER
{
	UCHAR			year;
	UCHAR			month;
	UCHAR			day;
	UCHAR			hour;
	UCHAR			minute;
	UCHAR			second;
	UCHAR			weather1;
	short			temp1_low;
	short			temp1_high;
	UCHAR			weather2;
	short			temp2_low;
	short			temp2_high;
} __attribute__ ((packed)) MTM_DATA_WEATHER, *PMTM_DATA_WEATHER;	//size=16

/*
	주차정보:

	+--------+----------+----------+
	| 층명칭 | 구역명칭 | 차량번호 |
	+--------+----------+----------+
	|    4   |     7    |     4    |		: Ascii String
	+--------+----------+----------+
*/
#define MAX_PARKING_FLOOR_NAME			4
#define MAX_PARKING_ZONE_NAME			7
#define MAX_PARKING_CAR_NUM				5

typedef struct _MTM_DATA_PARKING
{
	UCHAR			status;
	char			floor[MAX_PARKING_FLOOR_NAME];
	char			zone[MAX_PARKING_ZONE_NAME];
	char			car_num[MAX_PARKING_CAR_NUM];
} __attribute__ ((packed)) MTM_DATA_PARKING, *PMTM_DATA_PARKING;	//size=16

enum
{
	MTM_DATA_PARKING_STATUS_NOINFO,
	MTM_DATA_PARKING_STATUS_IN,
	MTM_DATA_PARKING_STATUS_OUT,
	MTM_DATA_PARKING_STATUS_COUNT
};

/*
	엘리베이터정보:

	+------+--------+
	| 상태 | 층번호 |
	+------+--------+
	|   1  |    1   |
	+------+--------+
*/
typedef struct _MTM_DATA_ELEVATOR
{
	UCHAR			status;
	char			floor;			//양수(1층~127층), 음수(B1층~Bx층), 0=에러
} __attribute__ ((packed)) MTM_DATA_ELEVATOR, *PMTM_DATA_ELEVATOR;	//size=2

enum
{
	MTM_DATA_EV_STATUS_STOP,
	MTM_DATA_EV_STATUS_UP,
	MTM_DATA_EV_STATUS_DOWN,
	MTM_DATA_EV_STATUS_ARRIVE,
	MTM_DATA_EV_STATUS_ERROR,
	MTM_DATA_EV_STATUS_COUNT
};

/*
	가스밸브상태:

	+------+
	| 상태 |
	+------+
	|   1  |
	+------+
*/
typedef struct _MTM_DATA_GAS
{
	UCHAR			status;			//0=열림(사용중), 1=차단, 2=에러
} __attribute__ ((packed)) MTM_DATA_GAS, *PMTM_DATA_GAS;			//size=1

enum
{
	MTM_DATA_GAS_OPEN,
	MTM_DATA_GAS_CUT,
	MTM_DATA_GAS_ERROR,
	MTM_DATA_GAS_COUNT
};

/*
	조명기기상태:

	+------+
	| 상태 |
	+------+
	|   1  |
	+------+
*/
typedef struct _MTM_DATA_LIGHT
{
	UCHAR			status;			//0=점등(소등해제), 1=소등, 2=에러
} __attribute__ ((packed)) MTM_DATA_LIGHT, *PMTM_DATA_LIGHT;		//size=1

enum
{
	MTM_DATA_LIGHT_ON,			//점등
	MTM_DATA_LIGHT_OFF,			//소등
	MTM_DATA_LIGHT_ERROR,
	MTM_DATA_LIGHT_COUNT
};

/*
	보안기기상태:

	+------+
	| 상태 |
	+------+
	|   1  |
	+------+
*/
typedef struct _MTM_DATA_SECURITY
{
	UCHAR			status;			//0=해제, 1=설정, 2=에러
} __attribute__ ((packed)) MTM_DATA_SECURITY, *PMTM_DATA_SECURITY;	//size=1

enum
{
	MTM_DATA_SECURITY_OFF,
	MTM_DATA_SECURITY_ON,
	MTM_DATA_SECURITY_ERROR,
	MTM_DATA_SECURITY_COUNT
};

#endif //__WALLPAD_PROTOCOL_MTM_H__


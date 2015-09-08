/*
	코콤 월패드 프로토콜

	기반문서: 
		코콤프로토콜(0.21)-MagicTransformMirror(110930).pdf
		코콤프로토콜(0.21)-USS(110506).pdf

	통신환경:
		9600bps, 8bit, 1stopbit, non-parity

	패킷구조:
		+--------+--+--+----+---+---+---+---+------+------+---+---+
		|Preamble|HD|CC|PCNT|ADH|ADL|ASH|ASL|OPCODE|D0..D7|FCC|EOT|
		+--------+--+--+----+---+---+---+---+------+------+---+---+
		|    2   | 1| 1| 1  | 1 | 1 | 1 | 1 |   1  |   8  | 1 | 2 |
		+--------+--+--+----+---+---+---+---+------+------+---+---+

		Preamble: 첫번째(0xAA), 두번째(0x55) 패킷의 시작을 알리는 Start Code
		HD		: 프로토콜의 용도 구분
		CC		: Control Command 재전송 카운트 및 명령 또는 ACK에 대한 구분
		PCNT	: 연속데이타 전송시, 남아있는 패킷 수
		ADH/ADL	: 목적지 상/하 바이트
		ASH/ASL	: 출발지 상/하 바이트
		OPCODE	: Operation Code 명령코드
		D0..D7	: Data Bytes
		FCC		: CheckSum (HD ~ D7 바이트 덧셈)
		EOT		: 마지막 2바이트(0x0D0D)

	ControlCode:
		+----+----+----+----+----+----+----+----+
		| B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
		+----+----+----+----+----+----+----+----+
		|  1 |   TYPE  |  1    1    1 |  COUNT  |
		+----+---------+--------------+---------+

		TYPE	: 00=ACK없는명령	0x9C, 0x9D, 0x9E
				  01=ACK확인명령	0xBC, 0xBD, 0xBE
				  10=ACK			0xDC, 0xDD, 0xDE
		COUNT	: 재전송횟수 (0~2)

	DeviceCode(ADx,ASx):

		AxH : Device Code (0x01=세대기, 0x0E=조명, 0x2C=가스밸브, 0x36=보일러, 0x3B=콘센트, 0x53=MTM, 0x54=USS)
		AxL : 장치일련번호 (0~ )

	통산사양:
		- 패킷응답은 최소 10[ms], 최대 250[ms](=timeout)의 타이밍을 갖는다
		- 응답시 ADx와 ASx 위치를 바꾸고 OPCODE,DATA는 그대로 전송
		- 응답에러시 3회 재전송 (CC필드 1증가), 3회 재전송에 미응답시 에러처리
*/

#ifndef __WALLPAD_PROTOCOL_KCM_H__
#define __WALLPAD_PROTOCOL_KCM_H__


//
// 패킷 관련 정의
//
#define KCM_PRE							0x55AA	// 패킷순서: {AA55} ... 

#define KCM_HD							0x30

#define KCM_CC_TYPE_MASK				0x60
#define KCM_CC_ACK_LESS					0x9C
#define KCM_CC_ACK_IS					0xBC
#define KCM_CC_ACK						0xDC

#define KCM_DEV_WALLPAD					0x01	//MTM/USS

#define KCM_DEV_MTM						0x53	//MTM

#define KCM_DEV_GAS_VALVE				0x2C	//USS
#define KCM_DEV_LIGHT					0x0E	//
#define KCM_DEV_BOILER					0x36	//
#define KCM_DEV_OUTLET					0x3B	//
#define KCM_DEV_ELEVATOR				0x44	//

#define KCM_DEV_USS						0x54	//

#define KCM_EOT							0x0D0D

#define KCM_PACKET_SIZE					21		// 21 bytes fixed
#define KCM_DATA_SIZE					8

typedef struct _KCM_PACKET
{
	USHORT		preamble;
	UCHAR		hd;
	UCHAR		cc;
	UCHAR		pcnt;
	UCHAR		adh;
	UCHAR		adl;
	UCHAR		ash;
	UCHAR		asl;
	UCHAR		opcode;
	UCHAR		data[KCM_DATA_SIZE];
	UCHAR		fcc;
	USHORT		eot;
} __attribute__ ((packed)) KCM_PACKET, *PKCM_PACKET;


//
// MTM 프로토콜
//

// 1) 일괄요청 (MTM->WP)
/*
	D0	정보종류		0xff	정보전체
						0x01	시간별 날씨
						0x02	주차정보
						0x04	현관문 열림정보

	*OR연산 또는 0xFF(전체)
*/
#define KCM_OP_BATCH_REQ				0x00

#define KCM_BATCH_REQ_ALL				0xff
#define KCM_BATCH_REQ_WEATHER			(1<<0)
#define KCM_BATCH_REQ_PARKING			(1<<1)
#define KCM_BATCH_REQ_DOOR				(1<<2)

// 2) Alive (WP->MTM)
/*
	D0	시				0~23 / 0xFF=시간정보 알수없음
	D1	분				0~59 / 0xFF
	D2	초				0~59 / 0xFF
	D3	현재온도		영상(0~100), 영하(-1(0xFF)~-100(0x9C)), 0x80=알수없음
*/
#define KCM_OP_ALIVE_CHECK				0x3A

// 3) 시간별 날씨 (WP->MTM)
/*
	D0	현재시간		0~23 / 0xFF=시간정보 알수없음
	D1	날씨종류		1~7	 (1=맑음, 2=구름조금, 3=구름많음, 4=흐림, 5=비, 6=비 또는 눈, 7=눈)
	D2	온도			영상(0~100), 영하(-1(0xFF)~-100(0x9C)), 0x80=알수없음
*/
#define KCM_OP_WEATHER_INFO				0x01

// 시간변 날씨조회 (WP<-MTM)
/*
	D0~D7	미정
*/
#define KCM_OP_WEATHER_REQ				0x31

// 4) 주차정보 (WP->MTM)
/*
	D0		차량등록번호	0=주차정보없음, 1~10=등록된 차량번호
	D1~D7	주차위치정보	Ascii (여백은 0x00)
*/
#define KCM_OP_PARKING_INFO				0x08

// 5) 현관문 출입정보 (WP->MTM)
/*
	D0	열림/닫힘		0=닫힘, 0xff=열림
*/
#define KCM_OP_DOOR_INFO				0x10


//
// USS 프로토콜
//

// 1) 가스차단 (USS->WP)
/*
	ADH		가스밸브	0x2C
	ASH		USS			0x54
*/
#define KCM_OP_GAS_CUT_REQ				0x02

/*
	가스밸브정보 (WP->USS)

	ADH		USS			0x54
	ASH		가스밸브	0x2C
*/
#define KCM_OP_GAS_CUT_RES_OPEN			0x01
#define KCM_OP_GAS_CUT_RES_CLOSE		0x02
#define KCM_OP_GAS_CUT_RES_ERROR		0xFF


// 2) 일괄소등 (USS->WP)
/*
	ADH		전동		0x0E
	ADL		선택		0xff	(전등전체)
	ASH		USS			0x54
	D0..D7	개별조명	0=일괄소등, 0xFF=일괄소등해제
*/
#define KCM_OP_ALL_OFF_REQ				0x00

/*
	전등정보 (WP->USS)

	ADH		USS			0x54
	ASH		전등		0x0E
	ASL		전등번호	0~7		(개별전등(스위치))
	D0..D7	개별조명	0=소등, 0x01~0xFF=점등 (디밍)
*/
#define KCM_OP_ALL_OFF_RES_OK			0x00
#define KCM_OP_ALL_OFF_RES_ERROR		0xFF


// 3) 난방외출 (USS->WP)
/*
	ADH		난방		0x36
	ADL		선택		0xff	(전체)
	ASH		USS			0x54
	D0		외출		0=외출해제, 1=외출
*/
#define KCM_OP_BOILER_REQ				0x00

/*
	난방정보 (WP->USS)

	ADH		USS			0x54
	ASH		난방		0x36
	ASL		방번호		0~7		개별방번호
	D0		외출		0=외출해제, 1=외출
*/
#define KCM_OP_BOILER_RES_OK			0x00
#define KCM_OP_BOILER_RES_ERROR			0xFF


// 4) 대기전력-콘센트 (WP->USS)
/*
	대기전력제어 (USS->WP)

	ADH		콘센트		0x3B
	ASH		USS			0x54
	ASL		콘센트번호	0xff	(전체)
	D0..D7	개별구		0=전력차단, 0xFF=전력사용, 0xAA=차단불가
*/
#define KCM_OP_OUTLET_RES_OK			0x00
#define KCM_OP_OUTLET_RES_ERROR			0xFF

/*
	콘센트 룸콘에서 대기전력 제어시 (WP->USS)

	ADH		USS			0x54
	ADL		선택		0~7=개별, 0xFF=전체
	ASH		콘센트		0x3B
	D0..D7	개별구		0=전력차단, 0xFF=전력사용
*/

// 5) 엘리베이터 호출 (USS->WP)
/*
	ADH		엘리베이터	0x44
	ASH		USS			0x54
*/
#define KCM_OP_ELEVATOR_CALL			0x01

/*
	엘리베이터 응답 (WP->USS)

	ADH		USS			0x54
	ASH		엘리베이터	0x44
	D0		방향		0=정지, 1=하향, 2=상향, 3=도착
	D1		현재층		0x01~0xff		0=알수없음
*/
#define KCM_OP_ELEVATOR_CALL_OK			0x00
#define KCM_OP_ELEVATOR_CALL_NG			0xFF

// 6) 외출/U보안 (USS->WP)
/*
	ADH		WP			0x01
	ASH		USS			0x54
	D0		U보안		0=사용안함,ㅣ출 1=U보안 사용대기, 2=사용
	D1		가스차단	0=차단안함, 1=차단
	D2		전등차단	0=차단안함, 1=차단
	D3		난방차단	0=차단안함, 1=난방OFF, 2=외출
	D4		대기전력	0=차단안함, 1=차단
	D5		엘리베이터	0=호출안함, 1=?
*/
#define KCM_OP_SECURITY_ON				0x11	//외출
#define KCM_OP_SECURITY_OFF				0x12	//외출해제

/*
	외출 응답 (WP->USS)

	ADH		USS			0x54
	ASH		WP			0x01
	D0		U보안		0=사용안함,ㅣ출 1=U보안 사용대기, 2=사용
*/

// 7) 정보요청 (USS->W) : 미사용??
/*
	ADH		WP			0x01
	ASH		USS			0x54
	D0		U보안		0=요청안함, 1=정보요청
	D1		가스차단	0=요청안함, 1=정보요청
	D2		전등차단	0=요청안함, 1=정보요청
	D3		난방차단	0=요청안함, 1=정보요청
	D4		대기전력	0=요청안함, 1=정보요청
	D5		외출		0=요청안함, 1=정보요청
*/
#define KCM_OP_PARKING_INFO_REQ			0x38
#define KCM_OP_INFO_REQ					0x3E

// 8) Alive (WP->USS)
/*
	ADH		USS			0x54
	ASH		WP			0x01
*/
#define KCM_OP_USS_ALIVE_CHECK			0x3A


#endif //__WALLPAD_PROTOCOL_KCM_H__

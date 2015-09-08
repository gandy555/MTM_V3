#ifndef _SNS_WALLPAD_PROTO_H_
#define _SNS_WALLPAD_PROTO_H_

/*
	RS-485 / 9600bps / Even Parity / 8 Data Bits / 1 Stop Bit

	Format:

	+--------+---------------+--------+---------+----------+
	| Header | Data(Command) | Length | SubData | CheckSum |
	+--------+---------------+--------+---------+----------+
	|   1    |       1       |    1   |    n    |     1    |
	+--------+---------------+--------+---------+----------+

	*Length: SubData의 갯수
	*CheckSum: Header ~ SubData, XOR, bit7=0 (masking)

	적용 프로토콜 기준문서: 2014/08/26
*/

//
// Header
//
#define SNS_HDR_WALLPAD					0xCD	//월패드 --> 생활정보기
#define SNS_HDR_DEV						0xB0	//생활정보기 --> 월패드

//
// Data(Command)
//
#define SNS_CMD_WEATHER_RES				0x01	//월패드에서 생활정보기로 날씨정보 전송
#define SNS_CMD_WEATHER_REQ				0x02	//생활정보기에서 월패드로 날씨정보 요청

#define SNS_CMD_PARKING_RES				0x03	//월패드에서 생활정보기로 주차정보 전송
#define SNS_CMD_PARKING_REQ				0x04	//생할정보기에서 월패드로 주차정보 요청

#define SNS_CMD_ELEVATOR_STAT_REQ		0x05	//생할정보기에서 월패드로 엘레베이터 상태요청
#define SNS_CMD_ELEVATOR_STAT_RES		0x06	//월패드에서 생활정보기로 엘레베이터 상태전송
#define SNS_CMD_ELEVATOR_CALL			0x07	//생할정보기에서 월패드로 엘레베이터 호출

#define SNS_CMD_WALLPAD_STAT_REQ		0x08	//생할정보기에서 월패드로 일괄소등,가스,외출설정 상태요청

#define SNS_CMD_LAMP_ONOFF_REQ			0x09	//생할정보기에서 월패드로 일괄소등 On/Off요청

#define SNS_CMD_GAS_OFF_REQ				0x0A	//생할정보기에서 월패드로 가스밸브 Off요청

#define SNS_CMD_OUT_ONOFF_REQ			0x0B	//생할정보기에서 월패드로 외출설정 On/Off요청
#define SNS_CMD_OUT_STAT_RES			0x0C	//월패드에서 생활정보기로 외출설정 상태전송

#define SNS_CMD_GAS_STAT_RES			0x0D	//월패드에서 생활정보기로 가스밸브 상태전송

#define SNS_CMD_ELEVATOR_ARRIVE			0x0E	//월패드에서 생활정보기로 엘리베이터 목적층 도착시 메세지 전송

#define SNS_CMD_INIT_DATA				0x5A	//월패드에서 생활정보기로 : 월패드가 켜질때, 월패드가 Ack Data 10회이상 미수신할 경우

#define SNS_CMD_FREE_DATA				0x41	//월패드에서 생활정보기로 : Event Data 없을 경우, Live Check
												// *응답까지 지연이 발생하는 경우 임시로 FreeData로 응답함
#define MAX_CAR_NUM_SIZE		4
//
// ACK
//

/*
	- 모든 메세지에 대해 ACK처리가 기본이며 ACK가 없을 경우 10회 Retry
	- 월패드에서 10회동안 ACK가 없으면(수신못하면) 초기화 데이타(0x5A)를 생활정보기로 송신함
	- 생활정보기에서 0x41 데이타 수신시에는 항상 0x41로 응답하고 나머지는 ACK응답함

	* LJK: ACK는 수신된 Command를 그대로 쓰고 SubData는 1byte 널(0x00)을 보낸다
*/

//
// Timing
//

/*
	- 월패드로부터 수신후 응답을 보내기 위해 TX Enable 하기까지 1.5ms 지연
	- TX Enable후 최초 패킷을 전송하기까지 1.5ms 지연
	- 연속적인 패킷을 보낼 경우 패킷간에 500us 지연
	- 마지막 패킷 전송후 TX Disable 하기까지 500us 지연
	- 모든 전송은 지연시간을 포함해서 50ms 이내
*/

//
// 상세프로토콜
//

/*
	날씨정보 응답 (0x01) : 기기<--월패드, 2시간에 한번씩 갱신(야후서버)

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Time			6		년,월,일,시,분,초
		현재온도		1
		5일간날씨정보	15		(오늘){날씨아이콘}{최고온도}{최저온도}...(4일후){날씨아이콘}{최고온도}{최저온도}

		*날씨아이콘: 1~40

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 01 16 xx ... xx CS		
									B0 01 01 00 CS

	날씨정보 요청 (0x02) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00		

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 02 01 00 CS
		CD 02 01 00 CS		
*/
typedef struct _WEATHER_INFO
{
	UCHAR		weather_icon;			//날씨 아이콘 (1~40)
	UCHAR		temp_high;				//최고기온
	UCHAR		temp_low;				//최저기온
} __attribute__ ((packed)) WEATHER_INFO;

typedef struct _WEATHER_RES
{
	UCHAR			year;				//년
	UCHAR			month;				//월
	UCHAR			day;				//일
	UCHAR			hour;				//시
	UCHAR			minute;				//분
	UCHAR			second;				//초
	UCHAR			temp_curr;			//현재온도
	WEATHER_INFO	weather_info[5];	//오늘~4일후
} __attribute__ ((packed)) WEATHER_RES;	//size=22

/*

	주차정보 응답 (0x03) : 기기<--월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Time			6		년,월,일,시,분,초
		입출차정보		1		??
		차량번호		4		
		층정보			2		층명칭, 층번호
		구역정보		6		구역명칭, 구역번호

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 03 0F xx ... xx CS
									B0 03 01 00 CS

	주차정보 요청 (0x04) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00

			월패드						생활정보기
		--------------------------- ---------------------------
									B0 04 01 00 CS
		CD 04 01 00 CS
*/

typedef struct _PARKING_RES
{
	UCHAR 			index;
	UCHAR			year;				//년
	UCHAR			month;				//월
	UCHAR			day;				//일
	UCHAR			hour;				//시
	UCHAR			minute;				//분
	UCHAR			second;				//초
	UCHAR			in_out;				//입출차정보
	UCHAR			car_num[4];			//차량번호
	UCHAR			car_floor[2];		//층정보(층명칭,층번호)
	UCHAR			car_zone[6];		//구역정보(구역명칭,구역번호)
} __attribute__ ((packed)) PARKING_RES;	//size=15

/*
	엘레베이터 상태요청 (0x05) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 05 01 00 CS
		CD 41 01 00 CS
		(delay)
		CD 05 01 00 CS
									B0 41 01 00 CS

	엘레베이터 상태응답 (0x06) : 기기<--월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Status			1		0x01=Success, 0x00=Fail
		현재층			1		0xF0(B16)..0xFE(B02),0xFF(B01),0x01(01),0x02(02)...0xEF(239)	*0x00(Loby)
		방향			1		0x01=Down, 0x02=Up, 0x00=Stop

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 06 03 xx xx xx CS
									B0 06 01 00 CS
*/
typedef struct _ELEVATOR_STATUS
{
	UCHAR			status;				//Status
	char			floor;				//현재층
	UCHAR			dir;				//방향
} __attribute__ ((packed)) ELEVATOR_STATUS;	//size=3

/*
	엘레베이터 호출 (0x07) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00

		(호출에 대한 월패드의 응답)
		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		State			1		0x01=Down, 0x02=Up, 0x00=Stop
		Status			1		0x01=Success, 0x00=Fail

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 07 01 xx CS					; xx=목적층 ??
		CD 41 01 00 CS
		(delay)
		CD 07 02 xx xx CS
									B0 41 01 00 CS
*/
typedef struct _ELEVATOR_CALL_RES
{
	UCHAR			state;				//방향(상태)
	UCHAR			status;				//Status
} __attribute__ ((packed)) ELEVATOR_CALL_RES;	//size=2

/*
	월패드 상태요청 (0x08) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00

		(요청에 대한 월패드의 응답)
		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		일괄소등상태	1		0x00=OFF(등이 켜짐), 0x01=ON(등이 꺼짐)
		가스밸브상태	1		0x00=닫힘, 0x01=열림
		외출설정상태	1		0x00=외출해제, 0x01=외출설정

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 08 01 00 CS
		CD 41 01 00 CS
		(delay)
		CD 08 03 xx xx xx CS
									B0 41 01 00 CS
*/
typedef struct _WALLPAD_STATUS
{
	UCHAR			alloff_stat;		//일괄소등상태
	UCHAR			gas_stat;			//가스밸브상태
	UCHAR			security_stat;		//외출설정상태
} __attribute__ ((packed)) WALLPAD_STATUS;	//size=3

/*
	일괄소등 On/Off요청 (0x09) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		On/Off			1		0x00=OFF(등이 켜짐), 0x01=ON(등이 꺼짐)

		(요청에 대한 월패드의 응답)
		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Status			1		0x00=Success, 0x01=Fail

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 09 01 xx CS
		CD 41 01 00 CS
		(delay)
		CD 09 01 xx CS
									B0 41 01 00 CS
		

	가스밸브 Off요청 (0x0A) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Off				1		0x01=닫기

		(요청에 대한 월패드의 응답)
		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Status			1		0x00=Success, 0x01=Fail

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 0A 01 xx CS
		CD 41 01 00 CS
		(delay)
		CD 0A 01 xx CS
									B0 41 01 00 CS

	외출설정 On/Off요청 (0x0B) : 기기-->월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		On/Off			1		0x00=외출해제, 0x01=외출설정

		(요청에 대한 월패드의 응답)
		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Status			1		0x00=Success, 0x01=Fail
								*외출해제시 비밀번호가 필요한 경우 Fail

		월패드						생활정보기
		--------------------------- ---------------------------
									B0 0B 01 xx CS
		CD 41 01 00 CS
		(delay)
		CD 0B 01 xx CS
									B0 41 01 00 CS
*/

/*
	외출설정 상태응답 (0x0C) : 기기<--월패드, 1)월패드에서 설정할 경우, 2)외출설정 대기시간 초과

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Status			1		0x00=외출해제, 0x01=외출설정

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 0C 01 xx CS
									B0 0C 01 CS
*/

/*
	가스밸브 상태전송 (0x0D) : 기기<--월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Status			1		0x00=열림, 0x01=닫힘

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 0D 01 xx CS
									B0 0D 01 CS
*/

/*
	엘리베이터 도착알림 (0x0E) : 기기<--월패드

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		Reserved		1		0x00

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 0E 01 00 CS
									B0 0E 01 00 CS
*/

/*
	초기 데이타 (0x5A) : 월패드가 켜질때, 월패드가 Ack Data 10회이상 미수신할 경우

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 5A 01 00 CS
									B0 5A 01 00 CS
*/

/*
	Free Data (0x41) : 송신할 Event Data 없을 경우, Live Check

		항목			길이	내용
		--------------- ------- --------------------------------------------------------------------------------
		널				1		0x00

		월패드						생활정보기
		--------------------------- ---------------------------
		CD 41 01 00 CS
									B0 41 01 00 CS
*/


#endif //_SNS_WALLPAD_PROTO_H_

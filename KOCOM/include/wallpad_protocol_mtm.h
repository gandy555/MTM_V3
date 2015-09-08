/*
	ǥ�� ���е� �������� for MTM Standard

	����:
		�پ��� ���е忡 �����ϱ� ���Ͽ� MTM�� ǥ���ϴ� ����Ÿ���� ��������
		������ ǥ�� ���е带 �����Ͽ� MTM�� ǥ�� ���е�� ���� ���ŵǴ�
		������ ����Ÿ ���˿� �����Ͽ� �ؼ��ϰ� ǥ���ϵ��� �ϱ�����.

	����:
		MTM���� ���е���� ����� ���������� ���� �ٸ� ���е��� ���ŵǹǷ�
		�������� ��ü�� ǥ��ȭ�� �� ���� ������ ��������� ǥ��ȭ�Ͽ� �̸� �Լ�ȭ �ϰ�
		���е忡�� MTM������ �Է��� �� �������ݿ� ���� ��ȯ�Ͽ� MTM���� �����Ѵ�.

	��Ŷ����:
		+-----+------+-----+------+-----+
		| STX | TYPE | LEN | DATA | ETX |
		+-----+------+-----+------+-----+
		|  1  |   1  |  2  |   n  |  1  |
		+-----+------+-----+------+-----+
		*MTM���ο����� ���Ǵ� ���������̹Ƿ� ������ ���ɼ��� ������ (CS����)

		STX	: 0x02
		TYPE: DATA�� ����
		LEN : DATA�� ����
		DATA: �������ݻ� 65536 bytes���� �����ϳ� ���������δ� Max 256 bytes�� �����Ѵ�
		ETX	: 0x03

	DATA����:

		1. ��������

			����ð�(����Ͻú���),����1,�µ�1,����2,�µ�2
			����1,�µ�1�� ������ ǥ�õ� ����
			����2,�µ�2�� ������ ǥ�õ� ����
			*���е忡 ���� ����/�µ��� ������ �ٸ��Ƿ� ��ȯ�������� ó���ϵ��� ��

		2. ��������

			������ġ(��,����)	;�ϼ��� ��Ʈ��

		3. ��������������

			����(����/����/����/����),����ȣ

		4. ����������

			����(����/����)
			
		5. ���������

			����(���/�ҵ�)

		6. ���ȱ�����

			����(����/����)
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

#define MTM_PACKET_MAX					32				//������ ���������϶� 21 bytes

//
// Data Type
//
enum
{
	MTM_DATA_TYPE_ACK,					//ACK
	MTM_DATA_TYPE_WEATHER,				//��������
	MTM_DATA_TYPE_PARKING,				//��������
	MTM_DATA_TYPE_ELEVATOR,				//��������������
	MTM_DATA_TYPE_GAS,					//����������
	MTM_DATA_TYPE_LIGHT,				//���������
	MTM_DATA_TYPE_SECURITY,				//���ȱ�����
	MTM_DATA_TYPE_COUNT
};

//
// Data Format
//

/*
	��������:

	+-----------------------------+--------------------+--------------------+ 
	|          ����ð�           |        ����1       |        ����2       | 
	+----+----+----+----+----+----+------+------+------+------+------+------+ 
	| �� | �� | �� | �� | �� | �� | ���� | ���� | ���� | ���� | ���� | ���� | 
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
	��������:

	+--------+----------+----------+
	| ����Ī | ������Ī | ������ȣ |
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
	��������������:

	+------+--------+
	| ���� | ����ȣ |
	+------+--------+
	|   1  |    1   |
	+------+--------+
*/
typedef struct _MTM_DATA_ELEVATOR
{
	UCHAR			status;
	char			floor;			//���(1��~127��), ����(B1��~Bx��), 0=����
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
	����������:

	+------+
	| ���� |
	+------+
	|   1  |
	+------+
*/
typedef struct _MTM_DATA_GAS
{
	UCHAR			status;			//0=����(�����), 1=����, 2=����
} __attribute__ ((packed)) MTM_DATA_GAS, *PMTM_DATA_GAS;			//size=1

enum
{
	MTM_DATA_GAS_OPEN,
	MTM_DATA_GAS_CUT,
	MTM_DATA_GAS_ERROR,
	MTM_DATA_GAS_COUNT
};

/*
	���������:

	+------+
	| ���� |
	+------+
	|   1  |
	+------+
*/
typedef struct _MTM_DATA_LIGHT
{
	UCHAR			status;			//0=����(�ҵ�����), 1=�ҵ�, 2=����
} __attribute__ ((packed)) MTM_DATA_LIGHT, *PMTM_DATA_LIGHT;		//size=1

enum
{
	MTM_DATA_LIGHT_ON,			//����
	MTM_DATA_LIGHT_OFF,			//�ҵ�
	MTM_DATA_LIGHT_ERROR,
	MTM_DATA_LIGHT_COUNT
};

/*
	���ȱ�����:

	+------+
	| ���� |
	+------+
	|   1  |
	+------+
*/
typedef struct _MTM_DATA_SECURITY
{
	UCHAR			status;			//0=����, 1=����, 2=����
} __attribute__ ((packed)) MTM_DATA_SECURITY, *PMTM_DATA_SECURITY;	//size=1

enum
{
	MTM_DATA_SECURITY_OFF,
	MTM_DATA_SECURITY_ON,
	MTM_DATA_SECURITY_ERROR,
	MTM_DATA_SECURITY_COUNT
};

#endif //__WALLPAD_PROTOCOL_MTM_H__


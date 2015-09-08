/*
	���� ���е� ��������

	��ݹ���: 
		������������(0.21)-MagicTransformMirror(110930).pdf
		������������(0.21)-USS(110506).pdf

	���ȯ��:
		9600bps, 8bit, 1stopbit, non-parity

	��Ŷ����:
		+--------+--+--+----+---+---+---+---+------+------+---+---+
		|Preamble|HD|CC|PCNT|ADH|ADL|ASH|ASL|OPCODE|D0..D7|FCC|EOT|
		+--------+--+--+----+---+---+---+---+------+------+---+---+
		|    2   | 1| 1| 1  | 1 | 1 | 1 | 1 |   1  |   8  | 1 | 2 |
		+--------+--+--+----+---+---+---+---+------+------+---+---+

		Preamble: ù��°(0xAA), �ι�°(0x55) ��Ŷ�� ������ �˸��� Start Code
		HD		: ���������� �뵵 ����
		CC		: Control Command ������ ī��Ʈ �� ��� �Ǵ� ACK�� ���� ����
		PCNT	: ���ӵ���Ÿ ���۽�, �����ִ� ��Ŷ ��
		ADH/ADL	: ������ ��/�� ����Ʈ
		ASH/ASL	: ����� ��/�� ����Ʈ
		OPCODE	: Operation Code ����ڵ�
		D0..D7	: Data Bytes
		FCC		: CheckSum (HD ~ D7 ����Ʈ ����)
		EOT		: ������ 2����Ʈ(0x0D0D)

	ControlCode:
		+----+----+----+----+----+----+----+----+
		| B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
		+----+----+----+----+----+----+----+----+
		|  1 |   TYPE  |  1    1    1 |  COUNT  |
		+----+---------+--------------+---------+

		TYPE	: 00=ACK���¸��	0x9C, 0x9D, 0x9E
				  01=ACKȮ�θ��	0xBC, 0xBD, 0xBE
				  10=ACK			0xDC, 0xDD, 0xDE
		COUNT	: ������Ƚ�� (0~2)

	DeviceCode(ADx,ASx):

		AxH : Device Code (0x01=�����, 0x0E=����, 0x2C=�������, 0x36=���Ϸ�, 0x3B=�ܼ�Ʈ, 0x53=MTM, 0x54=USS)
		AxL : ��ġ�Ϸù�ȣ (0~ )

	�����:
		- ��Ŷ������ �ּ� 10[ms], �ִ� 250[ms](=timeout)�� Ÿ�̹��� ���´�
		- ����� ADx�� ASx ��ġ�� �ٲٰ� OPCODE,DATA�� �״�� ����
		- ���信���� 3ȸ ������ (CC�ʵ� 1����), 3ȸ �����ۿ� ������� ����ó��
*/

#ifndef __WALLPAD_PROTOCOL_KCM_H__
#define __WALLPAD_PROTOCOL_KCM_H__


//
// ��Ŷ ���� ����
//
#define KCM_PRE							0x55AA	// ��Ŷ����: {AA55} ... 

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
// MTM ��������
//

// 1) �ϰ���û (MTM->WP)
/*
	D0	��������		0xff	������ü
						0x01	�ð��� ����
						0x02	��������
						0x04	������ ��������

	*OR���� �Ǵ� 0xFF(��ü)
*/
#define KCM_OP_BATCH_REQ				0x00

#define KCM_BATCH_REQ_ALL				0xff
#define KCM_BATCH_REQ_WEATHER			(1<<0)
#define KCM_BATCH_REQ_PARKING			(1<<1)
#define KCM_BATCH_REQ_DOOR				(1<<2)

// 2) Alive (WP->MTM)
/*
	D0	��				0~23 / 0xFF=�ð����� �˼�����
	D1	��				0~59 / 0xFF
	D2	��				0~59 / 0xFF
	D3	����µ�		����(0~100), ����(-1(0xFF)~-100(0x9C)), 0x80=�˼�����
*/
#define KCM_OP_ALIVE_CHECK				0x3A

// 3) �ð��� ���� (WP->MTM)
/*
	D0	����ð�		0~23 / 0xFF=�ð����� �˼�����
	D1	��������		1~7	 (1=����, 2=��������, 3=��������, 4=�帲, 5=��, 6=�� �Ǵ� ��, 7=��)
	D2	�µ�			����(0~100), ����(-1(0xFF)~-100(0x9C)), 0x80=�˼�����
*/
#define KCM_OP_WEATHER_INFO				0x01

// �ð��� ������ȸ (WP<-MTM)
/*
	D0~D7	����
*/
#define KCM_OP_WEATHER_REQ				0x31

// 4) �������� (WP->MTM)
/*
	D0		������Ϲ�ȣ	0=������������, 1~10=��ϵ� ������ȣ
	D1~D7	������ġ����	Ascii (������ 0x00)
*/
#define KCM_OP_PARKING_INFO				0x08

// 5) ������ �������� (WP->MTM)
/*
	D0	����/����		0=����, 0xff=����
*/
#define KCM_OP_DOOR_INFO				0x10


//
// USS ��������
//

// 1) �������� (USS->WP)
/*
	ADH		�������	0x2C
	ASH		USS			0x54
*/
#define KCM_OP_GAS_CUT_REQ				0x02

/*
	����������� (WP->USS)

	ADH		USS			0x54
	ASH		�������	0x2C
*/
#define KCM_OP_GAS_CUT_RES_OPEN			0x01
#define KCM_OP_GAS_CUT_RES_CLOSE		0x02
#define KCM_OP_GAS_CUT_RES_ERROR		0xFF


// 2) �ϰ��ҵ� (USS->WP)
/*
	ADH		����		0x0E
	ADL		����		0xff	(������ü)
	ASH		USS			0x54
	D0..D7	��������	0=�ϰ��ҵ�, 0xFF=�ϰ��ҵ�����
*/
#define KCM_OP_ALL_OFF_REQ				0x00

/*
	�������� (WP->USS)

	ADH		USS			0x54
	ASH		����		0x0E
	ASL		�����ȣ	0~7		(��������(����ġ))
	D0..D7	��������	0=�ҵ�, 0x01~0xFF=���� (���)
*/
#define KCM_OP_ALL_OFF_RES_OK			0x00
#define KCM_OP_ALL_OFF_RES_ERROR		0xFF


// 3) ������� (USS->WP)
/*
	ADH		����		0x36
	ADL		����		0xff	(��ü)
	ASH		USS			0x54
	D0		����		0=��������, 1=����
*/
#define KCM_OP_BOILER_REQ				0x00

/*
	�������� (WP->USS)

	ADH		USS			0x54
	ASH		����		0x36
	ASL		���ȣ		0~7		�������ȣ
	D0		����		0=��������, 1=����
*/
#define KCM_OP_BOILER_RES_OK			0x00
#define KCM_OP_BOILER_RES_ERROR			0xFF


// 4) �������-�ܼ�Ʈ (WP->USS)
/*
	����������� (USS->WP)

	ADH		�ܼ�Ʈ		0x3B
	ASH		USS			0x54
	ASL		�ܼ�Ʈ��ȣ	0xff	(��ü)
	D0..D7	������		0=��������, 0xFF=���»��, 0xAA=���ܺҰ�
*/
#define KCM_OP_OUTLET_RES_OK			0x00
#define KCM_OP_OUTLET_RES_ERROR			0xFF

/*
	�ܼ�Ʈ ���ܿ��� ������� ����� (WP->USS)

	ADH		USS			0x54
	ADL		����		0~7=����, 0xFF=��ü
	ASH		�ܼ�Ʈ		0x3B
	D0..D7	������		0=��������, 0xFF=���»��
*/

// 5) ���������� ȣ�� (USS->WP)
/*
	ADH		����������	0x44
	ASH		USS			0x54
*/
#define KCM_OP_ELEVATOR_CALL			0x01

/*
	���������� ���� (WP->USS)

	ADH		USS			0x54
	ASH		����������	0x44
	D0		����		0=����, 1=����, 2=����, 3=����
	D1		������		0x01~0xff		0=�˼�����
*/
#define KCM_OP_ELEVATOR_CALL_OK			0x00
#define KCM_OP_ELEVATOR_CALL_NG			0xFF

// 6) ����/U���� (USS->WP)
/*
	ADH		WP			0x01
	ASH		USS			0x54
	D0		U����		0=������,���� 1=U���� �����, 2=���
	D1		��������	0=���ܾ���, 1=����
	D2		��������	0=���ܾ���, 1=����
	D3		��������	0=���ܾ���, 1=����OFF, 2=����
	D4		�������	0=���ܾ���, 1=����
	D5		����������	0=ȣ�����, 1=?
*/
#define KCM_OP_SECURITY_ON				0x11	//����
#define KCM_OP_SECURITY_OFF				0x12	//��������

/*
	���� ���� (WP->USS)

	ADH		USS			0x54
	ASH		WP			0x01
	D0		U����		0=������,���� 1=U���� �����, 2=���
*/

// 7) ������û (USS->W) : �̻��??
/*
	ADH		WP			0x01
	ASH		USS			0x54
	D0		U����		0=��û����, 1=������û
	D1		��������	0=��û����, 1=������û
	D2		��������	0=��û����, 1=������û
	D3		��������	0=��û����, 1=������û
	D4		�������	0=��û����, 1=������û
	D5		����		0=��û����, 1=������û
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

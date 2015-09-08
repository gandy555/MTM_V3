/*
	������� ���е� ��������

	��ݹ���: 
		MTM LCD ��������(������ſ���) ����1.1, 2012-11-19

	��Ŷ����:
		+------------------------+---------------------+---------+
		|          HEAD          |         BODY        |   TAIL  |	��Ŷ����
		+---+---+---+---+----+---+---+---+-------------+----+----+
		| 1 | 2 | 3 | 4 |  5 | 6 | 7 | 8 |    9(1~n)   | 10 | 11 |	�ʵ����
		+---+---+---+---+----+---+---+---+-------------+----+----+
		|STX|LEN|VEN|DEV|TYPE|SRV|LOC|CMD|ARG[1]~ARG[n]|CRC |ETX |	�ʵ��Ī
		+---+---+---+---+----+---+---+---+-------------+----+----+
		| 1 | 1 | 1 | 1 |  1 | 1 | 1 | 1 |      n      |  1 |  1 |	�ʵ�ũ��
		+---+---+---+---+----+---+---+---+-------------+----+----+

		STX		: 0xF7
		LEN		: STX ~ ETX
		VEN		: 0x00�� ������ Vendor ID, v2.10���� ����ID(0x01)�� ����� (���� ����ID ȣȯ���� ����)
		DEV		: 0x00�� ������ Device ID
				  EventSender=0x44, USS�ϰ�����ġ=0x2A, ���������� ȣ�⽺��ġ=0x34
		TYPE	: 0x00�� ������ Packet Type
				  ������� - Event(Information)=0x0C, Query(���¿䱸)=0x01, Control(������)=0x02, 
				  ����ü - Response Success=0x04
				  *���� Event�� ���� ������ ������ ����
		SRV		: 0x00�� ������ Service ID
				  EventSender	- ��������(0x62), ��������(0x63), ���� ���׳�ƽ ��������(0x68), �ϰ��ҵ� ����/���� ����(0x69)
				  USS�ϰ�����ġ	- ����(0x40), ����(0x43), �������(U-����)(0x60)
				  E/Vȣ�⽺��ġ	- �ܰ�����(Up/Down)(0x41)
		LOC		: Location (��������� �׷�/���� �ĺ���)
				  �����Ϻ�=�׷�, �����Ϻ�=�׷쳻 ID (��ȿ��:1~f,  0=��ü)
				  ex) ��ü���(0x00), �׷�1-���1(0x11), �׷�2��ü(0x20)
		CMD		: 0x00�� ������ Command
				  EventSender	- ������
				  USS�ϰ�����ġ - ���� - ON(0x01), OFF(0x02)
								  ���� - CLOSE(0x03), OPEN(0x04)
								  ������� - ����������(0x07), ����������(0x08), ��������(0x0A), ��������(0x0B)
				  E/Vȣ�⽺��ġ - �ܰ����� - UP(0x05), DOWN(0x06)
		ARG		: Argument (n bytes)
				  EventSender	- ��������(10),��������(15),�������׳�ƽ��������(7),�ϰ��ҵ��/��������(7)
				  USS�ϰ�����ġ - ����/�ϰ��ҵ�(2), ����/�ϰ�����(2), �������/����(2)
				  E/Vȣ�⽺��ġ - �������(1), ������(1), ȣ��(1)
		CRC		: STX~ARG[n]������ XOR
		ETX		: 0xEE

	��Ż��:
		9600bps, 8bit, 1stopbit, none parity

		- 30[ms] <= ����ð� < 300[ms]
		- ��Ź���Ʈ ���� <= 5[ms]
*/

#ifndef __WALLPAD_PROTOCOL_HDT_H__
#define __WALLPAD_PROTOCOL_HDT_H__


//
// ��Ŷ ���� ����
//
#define HDT_STX							0xF7
#define HDT_ETX							0xEE

//�ʵ� Offset : ARG���� (ARG�� �����̶� CRC,ETX�� ARGũ�⿡ ���� Offset�� �޶���)
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
// Ÿ�̹� ���� ����
//
#define HDT_RES_TIME_NORMAL				30		// [ms]
#define HDT_RES_TIME_MAX				300		// [ms]
#define HDT_BYTE_INTERVAL_MAX			5		// [ms]

//
// ��Ŷ �ʵ庰 ����
//

// VEN (Vendor ID)
#define HDT_VEN_UNIFY					0x01	// ����ID (v2.10����)

// DEV (Device ID)
#define HDT_DEV_EVENT_SENDER			0x44	// EventSender
#define HDT_DEV_USS_SWITCH				0x2A	// USS�ϰ�����ġ
#define HDT_DEV_ELV_SWITCH				0x34	// ���������� ȣ�⽺��ġ

#define HDT_DEV_LIGHT_SWITCH			0x19	// �ϰ��ҵ� ����ġ
#define HDT_DEV_GAS_SWITCH				0x1B	// �������� ����ġ
#define HDT_DEV_SECURITY				0x16	// �������

// TYPE (Packet Type)
#define HDT_TYPE_EVENT					0x0C	// HDT-->���, Event(Information), �����ʿ����
#define HDT_TYPE_QUERY					0x01	// HDT-->���, Query(���¿䱸)
#define HDT_TYPE_CONTROL				0x02	// HDT-->���, Control(������)
#define HDT_TYPE_RESPONSE				0x04	// HDT<->���, Response

// SRV (Service ID)
#define HDT_SRV_WEATHER_INFO			0x62	// EventSender		��������
#define HDT_SRV_PARKING_INFO			0x63	//					��������
#define HDT_SRV_DOOR_SENS_INFO			0x68	//					���� ���׳�ƽ ��������
#define HDT_SRV_LIGHT_OUT_INFO			0x69	//					�ϰ��ҵ� ����/���� ����

#define HDT_SRV_LIGHT_OUT				0x40	// USS�ϰ�����ġ	����(�ϰ��ҵ�)
#define HDT_SRV_GAS_CUT					0x43	//					����(��������)
#define HDT_SRV_SECURITY				0x60	//					�������(U-����)

#define HDT_SRV_ELV_CTRL				0x41	// E/Vȣ�⽺��ġ	�ܰ�����(Up/Down)

// LOC (Location) : ��������� �׷�/���� �ĺ���	( �����Ϻ�=�׷�, �����Ϻ�=�׷쳻 ID (��ȿ��:1~f,  0=��ü) )
#define HDT_LOC_GROUP(l)				((l>>4)&0xf)
#define HDT_LOC_DEV(l)					((l>>0)&0xf)

#define HDT_LOC_ALL						0x00

// CMD (Command)
#define HDT_CMD_EVENT					0x00	// EventSender		���е�->�����(MTM)

#define HDT_CMD_QUERY					0x00	// USS�ϰ�����ġ, E/Vȣ�⽺��ġ: �����(MTM)->���е�

#define HDT_CMD_LIGHT_OUT_ON			0x01	// USS�ϰ�����ġ	����(�ϰ��ҵ�) ON
#define HDT_CMD_LIGHT_OUT_OFF			0x02	//					����(�ϰ��ҵ�) OFF

#define HDT_CMD_GAS_VALVE_CLOSE			0x03	//					����(��������) CLOSE  - �����
#define HDT_CMD_GAS_VALVE_OPEN			0x04	//					����(��������) OPEN   - �������

#define HDT_CMD_SECURITY_ON				0x07	//					������� - ����������
#define HDT_CMD_SECURITY_OFF			0x08	//					������� - ����������
#define HDT_CMD_SECURITY_DELAY_OUT		0x0A	//					������� - ��������
#define HDT_CMD_SECURITY_DELAY_IN		0x0B	//					������� - ��������

#define HDT_CMD_ELV_UP					0x05	// E/Vȣ�⽺��ġ	�ܰ����� - UP
#define HDT_CMD_ELV_DOWN				0x06	//					�ܰ����� - DOWN

//
// ��Ŷ ������ ����Ÿ ���� (ARGUMENT)
//

/*
	EventSender - �������� : 10 bytes

	+-----------------------------+------+---------------------------------+
	|             TIME            | ���� |             �ǿܿµ�            |
	+----+----+----+----+----+----+------+-----------+----------+----------+
	|  1 |  2 |  3 |  4 |  5 |  6 |   7  |     8     |     9    |    10    |
	+----+----+----+----+----+----+------+-----------+----------+----------+
	| �� | �� | �� | �� | �� | �� | ���� | ����/���� | �����ڸ� | �Ҽ����� |
	+----+----+----+----+----+----+------+-----------+----------+----------+
*/

// �������� ����Ÿ �ʵ庰 ������
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
	EventSender - �������� : 15 bytes

	+-----------------------------+------------------------+-------------------------------+
	|             TIME            |         ī��ID         |            ��ġ����           |
	+----+----+----+----+----+----+----+----+----+----+----+------+------+--------+--------+
	|  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 |  12  |  13  |   14   |   15   |
	+----+----+----+----+----+----+----+----+----+----+----+------+------+--------+--------+
	| �� | �� | �� | �� | �� | �� |    |    |    |    |    |����Ī|����ȣ|������Ī|������ȣ|
	+----+----+----+----+----+----+----+----+----+----+----+------+------+--------+--------+

	ī��ID	: ����ī��ID
	����Ī	: ������ ��Ī, [A-Z], ǥ�þ������ 0x00
	����ȣ	: ������ ��ȣ, [0x00~0x09] / [A-Z]
	������Ī: �������� ��Ī, [A-Z] / [0-9], ǥ�þ������ 0x00
	������ȣ: �������� ��ȣ, [0x00~0xFF]

	ex) B2-C12 : 0x62 0x02 0x63 0x0C
*/

//�������� ����Ÿ �ʵ庰 ������
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
	EventSender - ���� ���׳�ƽ �������� : 7 bytes

	+-----------------------------+--------------+
	|             TIME            | ������������ |
	+----+----+----+----+----+----+--------------+
	|  1 |  2 |  3 |  4 |  5 |  6 |       7      |
	+----+----+----+----+----+----+--------------+
	| �� | �� | �� | �� | �� | �� |              |
	+----+----+----+----+----+----+--------------+
*/

//���� ���׳�ƽ �������� ����Ÿ �ʵ庰 ������
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

#define HDT_EVS_ARG_VALUE_DOOR_SENS_CLOSE		0x03		//����
#define HDT_EVS_ARG_VALUE_DOOR_SENS_OPEN		0x04		//����

/*
	EventSender - �ϰ��ҵ� ����/���� ���� : 7 bytes

	+-----------------------------+--------------+
	|             TIME            | �ϰ�ҵ����� |
	+----+----+----+----+----+----+--------------+
	|  1 |  2 |  3 |  4 |  5 |  6 |       7      |
	+----+----+----+----+----+----+--------------+
	| �� | �� | �� | �� | �� | �� |              |
	+----+----+----+----+----+----+--------------+
*/

//�ϰ��ҵ� ����/���� ���� ����Ÿ �ʵ庰 ������
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

#define HDT_EVS_ARG_VALUE_LIGHT_OUT_ON			0x01		//�ϰ��ҵ� ON
#define HDT_EVS_ARG_VALUE_LIGHT_OUT_OFF			0x02		//�ϰ��ҵ� OFF

/*
	USS�ϰ�����ġ - �ϰ��ҵ�, ��������,���� : 2 bytes

	+--------+-----+
	| DEV ID | CMD |
	+--------+-----+
	|    1   |  1  |
	+--------+-----+
*/

// USS�ϰ�����ġ ����Ÿ �ʵ庰 ������
enum
{
	HDT_USS_ARG_OFFSET_DEVID,
	HDT_USS_ARG_OFFSET_CMD
};

#define HDT_USS_ARG_VALUE_LIGHT_OUT_ON			0x01		// �ϰ��ҵ� ON
#define HDT_USS_ARG_VALUE_LIGHT_OUT_OFF			0x02		// �ϰ��ҵ� OFF

#define HDT_USS_ARG_VALUE_GAS_CUT_CLOSE			0x03		// �������� CLOSE (�������� = ��밡��)
#define HDT_USS_ARG_VALUE_GAS_CUT_OPEN			0x04		// �������� OPEN  (�������� = �������)

#define HDT_USS_ARG_VALUE_SECURITY_ON			0x07		// ���� - ����������
#define HDT_USS_ARG_VALUE_SECURITY_OFF			0x08		// ���� - ����������
#define HDT_USS_ARG_VALUE_SECURITY_DELAY_OUT	0x0A		// ���� - ��������
#define HDT_USS_ARG_VALUE_SECURITY_DELAY_IN		0x0B		// ���� - ��������

/*
	E/Vȣ�⽺��ġ - �������, ������, ȣ�� : 3 byte

	+----------+--------+------+
	| ������� | ������ | ȣ�� |
	+----------+--------+------+
	|     1    |    1   |   1  |
	+----------+--------+------+

	---------------------------------------------------------------------------------------------
	�������		����						0x00
					����						0x01
					����(����,����,�̻� ��)		0x02
					ȣ�⼺��, ȣ����			0x03 (������)
					ȣ�� �� �������			�����Ϻ�=�������, �����Ϻ�=ȣ�����
												�������=0xa0,  �������=0xb0
												����ȣ��=0x05,  ����ȣ��=0x06
	---------------------------------------------------------------------------------------------
	������										����: 0x01~0x99,	����: 0xB1~0xBF
												���� �Ǵ� ����: 0x00
	---------------------------------------------------------------------------------------------
	ȣ��										0x01~0xFF  (���� �Ǵ� ����: 0x00)
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

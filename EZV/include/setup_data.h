#ifndef __SETUP_DATA_H__
#define __SETUP_DATA_H__

#define SETUP_DATA_MAGIC				0x4D544D04	//MTM{0x04}		2014-12-21

#define SETUP_DATA_PATH					"/app/"
#define SETUP_DATA_FILE					"setup.dat"

#define SETUP_VOLUME_DEF				7
//#define SETUP_ADC_VALUE_DEF			820			//���� Default
#define SETUP_ADC_VALUE_DEF				780			//KCM �������� ���� Default

#define SETUP_TEMP_DEF					123
#define SETUP_WEATHER_DEF				0			//IMG_ENUM_WEATHER_SERENITY ����

//
// ���е� ����
//
enum
{
	WALLPAD_TYPE_DEFAULT,
	WALLPAD_TYPE_HDT,					//�������
	WALLPAD_TYPE_SHN,					//�ＺSDS
	WALLPAD_TYPE_KCM,					//����
	WALLPAD_TYPE_EZV,					//EZ-Ville
	WALLPAD_TYPE_COUNT
};

//
// ȭ��ǥ�� �ɼ�
//
enum
{
	UI_OPTION_DEFAULT,					//�⺻
	UI_OPTION_COUNT
};

//
// KEY �ɼ�
//
#define KEY_OPTION_ALL					0x3f
#define KEY_OPTION_RT					(1<<0)
#define KEY_OPTION_RM					(1<<1)
#define KEY_OPTION_RB					(1<<2)
#define KEY_OPTION_LT					(1<<3)
#define KEY_OPTION_LM					(1<<4)
#define KEY_OPTION_LB					(1<<5)

//
//���� ����Ÿ ���ڵ�
//
typedef struct _SETUP_DATA
{
	ULONG			magic;
//-------------------------------------------------------------------------------------------------
//	������
	UCHAR			wallpad_type;		//���е� ����
	UCHAR			ui_option;			//ȭ��ǥ�� �ɼ�
	UCHAR			key_option;			//����Ű �ɼ� (��Ʈ�� 1=Enable, 0=Disable)
	UINT			volume;				//���� ���� (0~8)
	UINT			adc_value;			//Microwave Adc ���� ������
//-------------------------------------------------------------------------------------------------
//	��������
	UCHAR			weather_year;		//���������� �ð�
	UCHAR			weather_month;		//
	UCHAR			weather_day;		//
	UCHAR			weather_hour;		//
	UCHAR			weather_minute;		//
	UCHAR			weather_second;		//
	int				current_temp;		//����µ�
	UINT			weather_left;		//���� ���� (������ ����)								*IMG_ENUM_WEATHER_XXX ������ ����
	int				temp_low_left;		//���� �����µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	int				temp_high_left;		//���� �ְ�µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)	!!SNS�������� �µ��� ����!!
	UINT			weather_right;		//������ ���� (������ ����)
	int				temp_low_right;		//������ �ְ�µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	int				temp_high_right;	//������ �ְ�µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
//-------------------------------------------------------------------------------------------------
//	������
	UCHAR			light_stat;			//�ϰ��ҵ����	0=����(�ҵ�����), 1=�ҵ�
	UCHAR			gas_stat;			//����������	0=����(�����), 1=����
	UCHAR			security_stat;		//���ȼ�������	0=����, 1=����
//-------------------------------------------------------------------------------------------------
//	��������
	UCHAR			status;									//����
	UCHAR			car_floor[MAX_PARKING_FLOOR_NAME];		//����Ī
	UCHAR			car_zone[MAX_PARKING_ZONE_NAME];		//������Ī
	// gandy 2014_12_26
	UCHAR			car_num[MAX_PARKING_CAR_NUM];			//������ȣ
//-------------------------------------------------------------------------------------------------
//	��������������
	UCHAR			elevator_status;
	char			elevator_floor;		//���(1��~127��), ����(B1��~Bx��), 0=����
	UCHAR door_opened;
} __attribute__ ((packed)) SETUP_DATA;	//size=


//
// Class Definition
//

class CSetupData : public CFile
{
public:
	CSetupData();
	~CSetupData();

	//Member Function
	void Init();
	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	//---------------------------------
	BOOL LoadSetupData();
	BOOL SaveSetupData();
	void SetSetupDataDefault();

	//Member Variable
	UCHAR			m_CriticalSection;
	SETUP_DATA		m_SetupData;
};

#endif //__SETUP_DATA_H__

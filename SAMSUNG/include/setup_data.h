#ifndef __SETUP_DATA_H__
#define __SETUP_DATA_H__

#define SETUP_DATA_MAGIC				0x4D544D02	//MTM{0x02}		2014-05-01

#define SETUP_DATA_PATH					"/app/"
#define SETUP_DATA_FILE					"setup.dat"

#define SETUP_VOLUME_DEF				7
#define SETUP_ADC_VALUE_DEF				820

#define SETUP_TODAY_TEMP_DEF			123
#define SETUP_TODAY_WEATHER_DEF			0			//IMG_ENUM_WEATHER_SERENITY ����
#define SETUP_TOMORROW_TEMP_DEF			123
#define SETUP_TOMORROW_WEATHER_DEF		1			//IMG_ENUM_WEATHER_PARTLY_CLOUDY ��������

//
//���� ����Ÿ ���ڵ�
//
typedef struct _SETUP_DATA
{
	ULONG			magic;
//-------------------------------------------------------------------------------------------------
//	������
	UINT			volume;				//���� ���� (0~8)
	UINT			adc_value;			//Microwae Adc ���� ������
//-------------------------------------------------------------------------------------------------
//	��������
	UCHAR			weather_year;		//���������� �ð�
	UCHAR			weather_month;		//
	UCHAR			weather_day;		//
	UCHAR			weather_hour;		//
	UCHAR			weather_minute;		//
	UCHAR			weather_second;		//
	UINT			current_temp;		//����µ�
	UINT			today_temp_high;	//���� �ְ�µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)			!!SNS�������� �µ��� ����!!
	UINT			today_temp_low;		//���� �����µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			today_weather;		//���� ���� (������ ����)		*IMG_ENUM_WEATHER_XXX ������ ����
	UINT			tomorrow_temp_high;	//���� �ְ�µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			tomorrow_temp_low;	//���� �ְ�µ� (�Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			tomorrow_weather;	//���� ���� (������ ����)
//-------------------------------------------------------------------------------------------------
//	���е����
	UCHAR			alloff_stat;		//�ϰ��ҵ����	0x00=OFF(���� ����), 0x01=ON(���� ����)
	UCHAR			gas_stat;			//����������	0x00=����, 0x01=����
	UCHAR			security_stat;		//���⼳������	0x00=��������, 0x01=���⼳��
//-------------------------------------------------------------------------------------------------
//	��������
	UCHAR			in_out;				//����������
	UCHAR			car_num[4];			//������ȣ
	UCHAR			car_floor[2];		//������(����Ī,����ȣ)
	UCHAR			car_zone[6];		//��������(������Ī,������ȣ)
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

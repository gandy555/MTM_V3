#ifndef __OPERATION_DATA_H__
#define __OPERATION_DATA_H__

#define OPERATION_DATA_MAGIC				0x4D544F03	//MTO{0x01}		2014-09-14

#define OPERATION_DATA_PATH					"/app/"
#define OPERATION_DATA_FILE					"operation.dat"

// �ý��� �Ķ���� ����Ʈ��
#define OPERATION_VOLUME_DEF				7
#define OPERATION_ADC_VALUE_DEF				820

// ��������
#define OPERATION_TODAY_TEMP_DEF			123
#define OPERATION_TODAY_WEATHER_DEF			0			//IMG_ENUM_WEATHER_SERENITY ����
#define OPERATION_TOMORROW_TEMP_DEF			123
#define OPERATION_TOMORROW_WEATHER_DEF		1			//IMG_ENUM_WEATHER_PARTLY_CLOUDY ��������


//
// � ����Ÿ ���ڵ�
//
typedef struct _OPERATION_DATA
{
//-------------------------------------------------------------------------------------------------
//	�ý��� ������
	UINT			volume;					//���� ���� (0~8)
	UINT			adc_value;				//Microwae Adc ���� ������
//-------------------------------------------------------------------------------------------------
//	�ֺ���ġ ����
	UCHAR			alloff_stat;			//�ϰ��ҵ����	0x00=OFF(���� ����), 0x01=ON(���� ����)
	UCHAR			gas_stat;				//����������	0x00=����(��밡��), 0x01=����(�������)
	UCHAR			security_stat;			//���⼳������	0x00=��������, 0x01=���⼳��
//-------------------------------------------------------------------------------------------------
//	��������
	UCHAR			weather_year;			//���������� �ð�
	UCHAR			weather_month;			//
	UCHAR			weather_day;			//
	UCHAR			weather_hour;			//
	UCHAR			weather_minute;			//
	UCHAR			weather_second;			//
	UINT			current_temp;			//����µ�
	UINT			today_temp_high;		//���� �ְ�µ� (�⺻: �Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			today_temp_low;			//���� �����µ� (�⺻: �Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			today_weather;			//���� ���� (������ ����)		*IMG_ENUM_WEATHER_XXX ������ ����
	UINT			tomorrow_temp_high;		//���� �ְ�µ� (�⺻: �Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			tomorrow_temp_low;		//���� �ְ�µ� (�⺻: �Ҽ��� 1�ڸ�, ������ 10�ؼ� �����)
	UINT			tomorrow_weather;		//���� ���� (������ ����)
//-------------------------------------------------------------------------------------------------
//	��������
	UCHAR			in_out;					//���������� (0=����, 1=����)
	UCHAR			car_num[4];				//������ȣ
	UCHAR			car_floor[2];			//������(����Ī,����ȣ)
	UCHAR			car_zone[6];			//��������(������Ī,������ȣ)
} __attribute__ ((packed)) OPERATION_DATA;	//size=58


//
// Class Definition
//

class COperationData : public CFile
{
public:
	COperationData();
	~COperationData();

	//Member Function
	void Init();
	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	//---------------------------------
	BOOL LoadOperationData();
	BOOL SaveOperationData();
	void SetOperationDataDefault();

	//Member Variable
	UCHAR			m_CriticalSection;
	OPERATION_DATA	m_OperationData;
};



#endif //__OPERATION_DATA_H__

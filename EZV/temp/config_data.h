#ifndef __CONFIG_DATA_H__
#define __CONFIG_DATA_H__

#define CONFIG_DATA_MAGIC				0x4D544303	//MTC{0x01}		2014-09-14

#define CONFIG_DATA_PATH				"/app/"
#define CONFIG_DATA_FILE				"config.dat"

//
//���� ����Ÿ ���ڵ�
//
typedef struct _CONFIG_DATA
{
	ULONG			magic;				//�������� �ĺ���
//-------------------------------------------------------------------------------------------------
//	����Ʈ ������
	USHORT			site_maker;			//����Ʈ �Ǽ���/�귣��			��������,IS����,�븲,AMCO ��
	USHORT			site_region;		//����Ʈ ����					��ź,��õ,�λ� ��
	USHORT			site_option;		//����Ʈ�� ��ɱ�����			���Ͽ콺��,Ű���� ��
//-------------------------------------------------------------------------------------------------
//	���е� ������
	USHORT			wallpad_maker;		//���е� ������
	USHORT			wallpad_model;		//���е� �� (�����纰 �ĺ���)
//-------------------------------------------------------------------------------------------------
	UCHAR			reserved[18];
} __attribute__ ((packed)) CONFIG_DATA;	//size=32


//
// Class Definition
//

class CConfigData : public CFile
{
public:
	CConfigData();
	~CConfigData();

	//Member Function
	void Init();
	BOOL EnterCS(BOOL isBlock=TRUE);
	void LeaveCS();

	//---------------------------------
	BOOL LoadConfigData();
	BOOL SaveConfigData();
	void SetConfigDataDefault();

	//Member Variable
	UCHAR			m_CriticalSection;
	CONFIG_DATA		m_ConfigData;
};

#endif //__CONFIG_DATA_H__

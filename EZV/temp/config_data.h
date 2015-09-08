#ifndef __CONFIG_DATA_H__
#define __CONFIG_DATA_H__

#define CONFIG_DATA_MAGIC				0x4D544303	//MTC{0x01}		2014-09-14

#define CONFIG_DATA_PATH				"/app/"
#define CONFIG_DATA_FILE				"config.dat"

//
//설정 데이타 레코드
//
typedef struct _CONFIG_DATA
{
	ULONG			magic;				//설정파일 식별자
//-------------------------------------------------------------------------------------------------
//	사이트 설정값
	USHORT			site_maker;			//아파트 건설사/브랜드			프루지오,IS동서,대림,AMCO 등
	USHORT			site_region;		//아파트 지역					동탄,사천,부산 등
	USHORT			site_option;		//아파트별 기능구분자			모델하우스용,키변동 등
//-------------------------------------------------------------------------------------------------
//	월패드 설정값
	USHORT			wallpad_maker;		//월패드 제조사
	USHORT			wallpad_model;		//월패드 모델 (제조사별 식별자)
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

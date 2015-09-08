#include "common.h"
#include "operation_data.h"
#include "main.h"

//
// Construction/Destruction
//
COperationData::COperationData()
{
	m_CriticalSection = 0;

	memset(&m_OperationData, 0, sizeof(m_OperationData));
}

COperationData::~COperationData()
{
}

//
// Member Function
//

void COperationData::Init()
{
}

BOOL COperationData::EnterCS(BOOL isBlock)
{
	if(isBlock)
	{
		while(m_CriticalSection) 
		{ 
			usleep(10); 
		}
		m_CriticalSection = 1;
		return TRUE;
	}
	else
	{
		if(m_CriticalSection)
		{
			return FALSE;
		}
		else
		{
			m_CriticalSection = 1;
			return TRUE;
		}
	}
}

void COperationData::LeaveCS()
{
	m_CriticalSection = 0;
}

//-----------------------------------------------------------------------------

BOOL COperationData::LoadSetupData()
{
	char szFilePath[128] = {0,};
	UCHAR* pSetupData;
	int ret, BytesToRead;
	BOOL fRet = TRUE;

	EnterCS();

	sprintf(szFilePath, "%s%s\0", OPERATION_DATA_PATH, OPERATION_DATA_FILE);
	if(Open(szFilePath, "rb")==FALSE)
	{
		printf("[Failure]\r\n%s: '%s' Loading Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	pSetupData = (UCHAR *)&m_OperationData;

	BytesToRead = sizeof(ULONG);	//for read magic
	ret = Read(pSetupData, BytesToRead);
	if(ret != BytesToRead)
	{
		printf("%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
		fRet = FALSE;
	}

	if(fRet)
	{
		if(m_OperationData.magic == OPERATION_DATA_MAGIC)
		{
			BytesToRead = sizeof(m_OperationData)-sizeof(ULONG);
			ret = Read(&pSetupData[sizeof(ULONG)], BytesToRead);
			if(ret != BytesToRead)
			{
				printf("%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
				fRet = FALSE;
			}
		}
		else
		{
			printf("%s: Setup Data Magic Mis-Match: read=0x%x, expect=0x%x\r\n", __func__, m_OperationData.magic, OPERATION_DATA_MAGIC);
			fRet = FALSE;
		}
	}

	Close();

	printf("%s: Load %s\r\n", __func__, (fRet) ? "Success" : "Failure");
	LeaveCS();

	return fRet;
}

BOOL COperationData::SaveSetupData()
{
	char szFilePath[128] = {0,};
	int ret;
	BOOL fRet = TRUE;

	EnterCS();

	sprintf(szFilePath, "%s%s\0", OPERATION_DATA_PATH, OPERATION_DATA_FILE);
	if(Open(szFilePath, "wb")==FALSE)
	{
		printf("%s: '%s' Saving Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	ret = Write(&m_OperationData, sizeof(m_OperationData));
	if(ret != sizeof(m_OperationData))
	{
		printf("%s: Invalid Write Size: write=%d, expect=%d\r\n", __func__, ret, sizeof(m_OperationData));
		fRet = FALSE;
	}

	Close();

	printf("%s: Save %s\r\n", __func__, (fRet) ? "Success" : "Failure");
	LeaveCS();

	return fRet;
}

void COperationData::SetSetupDataDefault()
{
	time_t now;
	struct tm *t;

	now = time(NULL);
	t = localtime(&now);

	memset(&m_OperationData, 0, sizeof(OPERATION_DATA));

	m_OperationData.magic				= OPERATION_DATA_MAGIC;

//	시스템 설정값
	m_OperationData.volume				= OPERATION_VOLUME_DEF;
	m_OperationData.adc_value			= OPERATION_ADC_VALUE_DEF;

//	주변장치 상태
	m_OperationData.alloff_stat			= 0;	//일괄소등 해제
	m_OperationData.gas_stat			= 1;	//가스밸브 열림
	m_OperationData.security_stat		= 0;	//외출설정 해제

//	날씨정보
	m_OperationData.weather_year		= t->tm_year;
	m_OperationData.weather_month		= t->tm_mon + 1;
	m_OperationData.weather_day			= t->tm_mday;
	m_OperationData.weather_hour		= t->tm_hour;
	m_OperationData.weather_minute		= t->tm_min;
	m_OperationData.weather_second		= t->tm_sec;

	m_OperationData.today_temp_high		= OPERATION_TODAY_TEMP_DEF;
	m_OperationData.today_temp_low		= OPERATION_TODAY_TEMP_DEF;
	m_OperationData.today_weather		= OPERATION_TODAY_WEATHER_DEF;

	m_OperationData.tomorrow_temp_high	= OPERATION_TOMORROW_TEMP_DEF;
	m_OperationData.tomorrow_temp_low	= OPERATION_TOMORROW_TEMP_DEF;
	m_OperationData.tomorrow_weather	= OPERATION_TOMORROW_WEATHER_DEF;

//	주차정보
	m_OperationData.in_out				= 0;										//입출차정보 (0=주차, 1=출차)
#if 0
	memset(&m_OperationData.car_num,	' ', sizeof(m_OperationData.car_num));		//차량번호
	memset(&m_OperationData.car_floor,	' ', sizeof(m_OperationData.car_floor));	//층명칭, 층번호
	memset(&m_OperationData.car_zone,	' ', sizeof(m_OperationData.car_zone));		//구역명칭, 구역번호
#endif
}


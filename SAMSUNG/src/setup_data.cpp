#include "common.h"
#include "setup_data.h"
#include "main.h"

//
// Construction/Destruction
//
CSetupData::CSetupData()
{
	m_CriticalSection = 0;

	memset(&m_SetupData, 0, sizeof(m_SetupData));
}

CSetupData::~CSetupData()
{
}

//
// Member Function
//

void CSetupData::Init()
{
}

BOOL CSetupData::EnterCS(BOOL isBlock)
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

void CSetupData::LeaveCS()
{
	m_CriticalSection = 0;
}

//-----------------------------------------------------------------------------

BOOL CSetupData::LoadSetupData()
{
	char szFilePath[128] = {0,};
	UCHAR* pSetupData;
	int ret, BytesToRead;
	BOOL fRet = TRUE;

	EnterCS();

	sprintf(szFilePath, "%s%s\0", SETUP_DATA_PATH, SETUP_DATA_FILE);
	if(Open(szFilePath, "rb")==FALSE)
	{
		printf("[Failure]\r\n%s: '%s' Loading Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	pSetupData = (UCHAR *)&m_SetupData;

	BytesToRead = sizeof(ULONG);	//for read magic
	ret = Read(pSetupData, BytesToRead);
	if(ret != BytesToRead)
	{
		printf("%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
		fRet = FALSE;
	}

	if(fRet)
	{
		if(m_SetupData.magic == SETUP_DATA_MAGIC)
		{
			BytesToRead = sizeof(m_SetupData)-sizeof(ULONG);
			ret = Read(&pSetupData[sizeof(ULONG)], BytesToRead);
			if(ret != BytesToRead)
			{
				printf("%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
				fRet = FALSE;
			}
		}
		else
		{
			printf("%s: Setup Data Magic Mis-Match: read=0x%x, expect=0x%x\r\n", __func__, m_SetupData.magic, SETUP_DATA_MAGIC);
			fRet = FALSE;
		}
	}

	Close();

	printf("%s: Load %s\r\n", __func__, (fRet) ? "Success" : "Failure");
	LeaveCS();

	return fRet;
}

BOOL CSetupData::SaveSetupData()
{
	char szFilePath[128] = {0,};
	int ret;
	BOOL fRet = TRUE;

	EnterCS();

	sprintf(szFilePath, "%s%s\0", SETUP_DATA_PATH, SETUP_DATA_FILE);
	if(Open(szFilePath, "wb")==FALSE)
	{
		printf("%s: '%s' Saving Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	ret = Write(&m_SetupData, sizeof(m_SetupData));
	if(ret != sizeof(m_SetupData))
	{
		printf("%s: Invalid Write Size: write=%d, expect=%d\r\n", __func__, ret, sizeof(m_SetupData));
		fRet = FALSE;
	}

	Close();

	printf("%s: Save %s\r\n", __func__, (fRet) ? "Success" : "Failure");
	LeaveCS();

	return fRet;
}

void CSetupData::SetSetupDataDefault()
{
	time_t now;
	struct tm *t;

	now = time(NULL);
	t = localtime(&now);

	m_SetupData.magic				= SETUP_DATA_MAGIC;

//	설정값
	m_SetupData.volume				= SETUP_VOLUME_DEF;
	m_SetupData.adc_value			= SETUP_ADC_VALUE_DEF;

//	날씨정보
	m_SetupData.weather_year		= t->tm_year;
	m_SetupData.weather_month		= t->tm_mon + 1;
	m_SetupData.weather_day			= t->tm_mday;
	m_SetupData.weather_hour		= t->tm_hour;
	m_SetupData.weather_minute		= t->tm_min;
	m_SetupData.weather_second		= t->tm_sec;

	m_SetupData.today_temp_high		= SETUP_TODAY_TEMP_DEF;
	m_SetupData.today_temp_low		= SETUP_TODAY_TEMP_DEF;
	m_SetupData.today_weather		= SETUP_TODAY_WEATHER_DEF;

	m_SetupData.tomorrow_temp_high	= SETUP_TOMORROW_TEMP_DEF;
	m_SetupData.tomorrow_temp_low	= SETUP_TOMORROW_TEMP_DEF;
	m_SetupData.tomorrow_weather	= SETUP_TOMORROW_WEATHER_DEF;

//	월패드상태
	m_SetupData.alloff_stat			= 0;	//일괄소등 해제
	m_SetupData.gas_stat			= 1;	//가스밸브 열림
	m_SetupData.security_stat		= 0;	//외출설정 해제

//	주차정보
	m_SetupData.in_out				= 0;
	memset(&m_SetupData.car_num, ' ', sizeof(m_SetupData.car_num));
	m_SetupData.car_floor[0]		= ' ';	//층명칭, 층번호
	m_SetupData.car_floor[1]		= ' ';	//
	m_SetupData.car_zone[0]			= ' ';	//구역명칭, 구역번호
	m_SetupData.car_zone[1]			= ' ';	//
	m_SetupData.car_zone[2]			= ' ';	//
	m_SetupData.car_zone[3]			= ' ';	//
	m_SetupData.car_zone[4]			= ' ';	//
	m_SetupData.car_zone[5]			= ' ';	//

}


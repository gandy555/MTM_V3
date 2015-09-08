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
		DBGMSG(DBG_SETUP, "%s: '%s' Loading Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	pSetupData = (UCHAR *)&m_SetupData;

	BytesToRead = sizeof(ULONG);	//for read magic
	ret = Read(pSetupData, BytesToRead);
	if(ret != BytesToRead)
	{
		DBGMSG(DBG_SETUP, "%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
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
				DBGMSG(DBG_SETUP, "%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
				fRet = FALSE;
			}
		}
		else
		{
			DBGMSG(DBG_SETUP, "%s: Setup Data Magic Mis-Match: read=0x%x, expect=0x%x\r\n", __func__, m_SetupData.magic, SETUP_DATA_MAGIC);
			fRet = FALSE;
		}
	}

	Close();

	DBGMSG(DBG_SETUP, "%s: Load %s\r\n", __func__, (fRet) ? "Success" : "Failure");
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
		DBGMSG(DBG_SETUP, "%s: '%s' Saving Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	ret = Write(&m_SetupData, sizeof(m_SetupData));
	if(ret != sizeof(m_SetupData))
	{
		DBGMSG(DBG_SETUP, "%s: Invalid Write Size: write=%d, expect=%d\r\n", __func__, ret, sizeof(m_SetupData));
		fRet = FALSE;
	}

	Close();

	DBGMSG(DBG_SETUP, "%s: Save %s\r\n", __func__, (fRet) ? "Success" : "Failure");
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
	m_SetupData.wallpad_type		= WALLPAD_TYPE_DEFAULT;
	m_SetupData.ui_option			= UI_OPTION_DEFAULT;
	m_SetupData.key_option			= KEY_OPTION_ALL;
	m_SetupData.volume				= SETUP_VOLUME_DEF;
	m_SetupData.adc_value			= SETUP_ADC_VALUE_DEF;

//	날씨정보
	m_SetupData.weather_year		= t->tm_year;
	m_SetupData.weather_month		= t->tm_mon + 1;
	m_SetupData.weather_day			= t->tm_mday;
	m_SetupData.weather_hour		= t->tm_hour;
	m_SetupData.weather_minute		= t->tm_min;
	m_SetupData.weather_second		= t->tm_sec;

	m_SetupData.current_temp		= SETUP_TEMP_DEF;

	m_SetupData.weather_left		= SETUP_WEATHER_DEF;
	m_SetupData.temp_low_left		= SETUP_TEMP_DEF;
	m_SetupData.temp_high_left		= SETUP_TEMP_DEF;

	m_SetupData.weather_right		= SETUP_WEATHER_DEF;
	m_SetupData.temp_low_right		= SETUP_TEMP_DEF;
	m_SetupData.temp_high_right		= SETUP_TEMP_DEF;

//	기기상태
	m_SetupData.light_stat			= 0;	//일괄소등 해제
	m_SetupData.gas_stat			= 0;	//가스밸브 사용중
	m_SetupData.security_stat		= 0;	//보안설정 해제

//	주차정보
	m_SetupData.status				= 0;	//주차상태 정보없음
	memset(&m_SetupData.car_floor, 0, MAX_PARKING_FLOOR_NAME);	//층명칭
	memset(&m_SetupData.car_zone,  0, MAX_PARKING_ZONE_NAME);	//구역명칭
	memset(&m_SetupData.car_num,   0, MAX_PARKING_CAR_NUM);		//차량번호

}


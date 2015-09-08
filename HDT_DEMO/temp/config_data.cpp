#include "common.h"
#include "config_data.h"
#include "config_site.h"
#include "config_wallpad.h"
#include "main.h"

//
// Construction/Destruction
//
CConfigData::CConfigData()
{
	m_CriticalSection = 0;

	memset(&m_ConfigData, 0, sizeof(m_ConfigData));
}

CConfigData::~CConfigData()
{
}

//
// Member Function
//

void CConfigData::Init()
{
}

BOOL CConfigData::EnterCS(BOOL isBlock)
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

void CConfigData::LeaveCS()
{
	m_CriticalSection = 0;
}

//-----------------------------------------------------------------------------

BOOL CConfigData::LoadConfigData()
{
	char szFilePath[128] = {0,};
	UCHAR* pConfigData;
	int ret, BytesToRead;
	BOOL fRet = TRUE;

	EnterCS();

	sprintf(szFilePath, "%s%s\0", CONFG_DATA_PATH, CONFG_DATA_FILE);
	if(Open(szFilePath, "rb")==FALSE)
	{
		printf("[Failure]\r\n%s: '%s' Loading Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	pSetupData = (UCHAR *)&m_ConfigData;

	BytesToRead = sizeof(ULONG);	//for read magic
	ret = Read(pConfigData, BytesToRead);
	if(ret != BytesToRead)
	{
		printf("%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
		fRet = FALSE;
	}

	if(fRet)
	{
		if(m_ConfigData.magic == CONFG_DATA_MAGIC)
		{
			BytesToRead = sizeof(m_ConfigData)-sizeof(ULONG);
			ret = Read(&pSetupData[sizeof(ULONG)], BytesToRead);
			if(ret != BytesToRead)
			{
				printf("%s: Invalid Read Size: read=%d, expect=%d\r\n", __func__, ret, BytesToRead);
				fRet = FALSE;
			}
		}
		else
		{
			printf("%s: Setup Data Magic Mis-Match: read=0x%x, expect=0x%x\r\n", __func__, m_ConfigData.magic, CONFG_DATA_MAGIC);
			fRet = FALSE;
		}
	}

	Close();

	printf("%s: Load %s\r\n", __func__, (fRet) ? "Success" : "Failure");
	LeaveCS();

	return fRet;
}

BOOL CConfigData::SaveSetupData()
{
	char szFilePath[128] = {0,};
	int ret;
	BOOL fRet = TRUE;

	EnterCS();

	sprintf(szFilePath, "%s%s\0", CONFG_DATA_PATH, CONFG_DATA_FILE);
	if(Open(szFilePath, "wb")==FALSE)
	{
		printf("%s: '%s' Saving Failure\r\n", __func__, szFilePath);
		LeaveCS();

		return FALSE;;
	}

	ret = Write(&m_ConfigData, sizeof(m_ConfigData));
	if(ret != sizeof(m_ConfigData))
	{
		printf("%s: Invalid Write Size: write=%d, expect=%d\r\n", __func__, ret, sizeof(m_ConfigData));
		fRet = FALSE;
	}

	Close();

	printf("%s: Save %s\r\n", __func__, (fRet) ? "Success" : "Failure");
	LeaveCS();

	return fRet;
}

void CConfigData::SetConfigDataDefault()
{
	memset(&m_ConfigData, 0, sizeof(CONFIG_DATA));

	m_ConfigData.magic				= CONFG_DATA_MAGIC;

//	사이트 설정값
	m_ConfigData.site_maker			= SITE_ID_DAEWOO_GUN;			//대우건설
	m_ConfigData.site_region		= SITE_REGION_SEOUL_GANGGYO;	//프로지오-광교
	m_ConfigData.site_option		= 0;

//	월패드 설정값
	m_ConfigData.wallpad_maker		= WALLPAD_MAKER_HDT;			//현대통신
	m_ConfigData.wallpad_model		= WALLPAD_MODEL_HDT;			//통합프로토콜 지원모델

}


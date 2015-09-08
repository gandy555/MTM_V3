/*

*/
#include "common.h"

//
// Construction/Destruction
//
CFile::CFile()
{
	m_pFile = NULL;
}

CFile::~CFile()
{
	Close();
}

//
// Member Function
//
BOOL CFile::Open(const char *pszFileName, const char *pszMode)
{
	m_pFile = fopen(pszFileName, pszMode);
	if(m_pFile == NULL)
	{
		printf("%s: %s can not open : error=%d %s\r\n", __func__, pszFileName, errno, strerror(errno));
		return FALSE;
	}
	
	return TRUE;
}

void CFile::Close()
{
	if(m_pFile)
		fclose(m_pFile);

	m_pFile = NULL;
}


int CFile::Write(const void *pBuffer, int size)
{
	int ret = ERROR;
	
	if(m_pFile)
		ret = fwrite(pBuffer, 1, (size_t)size, m_pFile);

	return ret;
}


int CFile::Writef(const char *pszFormat, ...)
{
	char szBuffer[MAX_BUFFER];
	int ret;

	memset(szBuffer, 0, MAX_BUFFER);

	va_list list;
	va_start(list, pszFormat);
	if(m_pFile)
		ret = vfprintf(m_pFile, pszFormat, list);
	va_end(list);

	return ret;
}


int CFile::Read(void *pBuffer, int size)
{
	int ret = ERROR;
	
	if(m_pFile)
		ret = fread(pBuffer, 1, (size_t)size, m_pFile);

	return ret;
}


int CFile::Readf(const char *pszFormat, ...)
{
	char szBuffer[MAX_BUFFER];
	int ret;

	memset(szBuffer, 0, MAX_BUFFER);

	va_list list;
	va_start(list, pszFormat);
	if(m_pFile)
		ret = vfscanf(m_pFile, pszFormat, list);
	va_end(list);

	return ret;
}

//whence: SEEK_SET, SEEK_END, SEEK_CUR
int CFile::Seek(long offset, int whence)
{
	return fseek(m_pFile, offset, whence);
}

void CFile::Rewind()
{
	rewind(m_pFile);
}

BOOL CFile::ExistFile(const char *pszFileName)
{
	struct stat fs;

	if(pszFileName == NULL) return FALSE;

	return ((stat(pszFileName, &fs)==0) ? TRUE : FALSE);
}

long CFile::GetSize()
{
	long size;

	if(m_pFile == NULL) 
		return 0;

	if(fseek(m_pFile, 0, SEEK_END)==0)
		size = ftell(m_pFile);

	rewind(m_pFile);

	return size;
}

long CFile::GetSize(const char *pszFileName)
{
	struct stat fs;

	if(stat( pszFileName, &fs )==0)
		return fs.st_size;

	return 0;
}



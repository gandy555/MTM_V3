/*

*/

#ifndef __FILE_H__
#define __FILE_H__

#define	MAX_BUFFER		512

class CFile
{
public:
	CFile();
	~CFile();

	//Member Function
	BOOL Open(const char *pszFileName, const char *pszMode);
	void Close();

	int Write(const void *pBuffer, int size);
	int Writef(const char *pszFormat, ...);
	int Read(void *pBuffer, int size);
	int Readf(const char *pszFormat, ...);

	int Seek(long offset, int whence);
	void Rewind();

	BOOL ExistFile(const char *pszFileName);
	long GetSize();
	long GetSize(const char *pszFileName);

	//Member Variable
	FILE		*m_pFile;
};

#endif //__FILE_H__

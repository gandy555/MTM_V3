/*

*/

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <termios.h>

typedef void* (*PFN_SERIAL_DATA_HANDLER)(void *pParam);

class CSerial
{
public:
	CSerial();
	~CSerial();

	//Member Function
	BOOL Open(const char *pszDev, unsigned int baudRate=B38400); //default=38400,8N1
	void Close();

	BOOL IsOpen();

	int Write(const unsigned char *pszBuffer, int size);
	int Read(unsigned char *pBuffer, int size);

	BOOL StartListener(PFN_SERIAL_DATA_HANDLER pfnHandler = NULL);
	void StopListener();

	static void* SerialListener(void *pParam);

	//Member Variable
	int			m_fdDev;
	BOOL		m_fListenerRunning;
	pthread_t	m_ListenerThread;
	PFN_SERIAL_DATA_HANDLER	m_pfnHandler;
};

#endif //__SERIAL_H__


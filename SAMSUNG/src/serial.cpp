/*

*/
#include "common.h"
#include "serial.h"

CSerial::CSerial()
{
	m_fdDev				= ERROR;
	m_fListenerRunning	= FALSE;
	m_ListenerThread	= 0;
	m_pfnHandler		= NULL;
}

CSerial::~CSerial()
{
	Close();
}

BOOL CSerial::Open(const char *pszDev, unsigned int baudRate)
{
	struct termios tio;

	if(pszDev == NULL) 
	{
		printf("%s: invalid dev name\r\n", __func__);
		return FALSE;
	}

	if( (strcmp(pszDev, UART0_DEV)!=0) &&
		(strcmp(pszDev, UART1_DEV)!=0) &&
		(strcmp(pszDev, UART2_DEV)!=0) )
	{
		printf("%s: unsupported dev name '%s'\r\n", __func__, pszDev);
		return FALSE;
	}

	m_fdDev = open(pszDev, O_RDWR);
	if(m_fdDev == ERROR)
	{
		printf("%s: '%s' device open failure\r\n", __func__, pszDev);
		return FALSE;
	}

    memset(&tio, 0, sizeof(termios));
    tio.c_iflag = IGNPAR;				// non-parity
    tio.c_oflag = 0;					//
	tio.c_cflag = CS8 | CLOCAL | CREAD;	// NO-rts/cts	

	tio.c_cflag |= baudRate;

    tio.c_lflag = 0;

    tio.c_cc[VTIME] = 1;				// timeout 0.1초 단위
    tio.c_cc[VMIN]  = 1;				// 최소 n 문자 받을 때까진 대기

    tcflush  ( m_fdDev, TCIFLUSH );
    tcsetattr( m_fdDev, TCSANOW, &tio );

	printf("%s: '%s' device open success\r\n", __func__, pszDev);
	
	return TRUE;
}

void CSerial::Close()
{
	if(m_fdDev>=0)
	{
		close(m_fdDev);
		m_fdDev = ERROR;
	}
}

BOOL CSerial::IsOpen()
{
	return (m_fdDev != ERROR) ? TRUE : FALSE;
}

int CSerial::Write(const unsigned char *pszBuffer, int size)
{
	int ret = ERROR;

	if(m_fdDev>=0)
	{
		ret = write(m_fdDev, pszBuffer, size);
		if(ret == ERROR)
		{
			printf("%s: write failure : errno=%d %s\r\n", __func__, errno, strerror(errno));
			return ERROR;
		}
		//printf("%s: write success\r\n", __func__);
	}

	return ret;
}

int CSerial::Read(unsigned char *pBuffer, int size)
{
	int ret = ERROR;

	if(m_fdDev>=0)
	{
		ret = read(m_fdDev, pBuffer, size);
		if(ret == ERROR)
		{
			printf("%s: read failure : errno=%d %s\r\n", __func__, errno, strerror(errno));
			return ERROR;
		}
		//printf("%s: read success\r\n", __func__);
	}

	return ret;
}

BOOL CSerial::StartListener(PFN_SERIAL_DATA_HANDLER pfnHandler)
{
	int create_error;

	m_fListenerRunning = TRUE;

	if(pfnHandler)
		create_error = pthread_create(&m_ListenerThread, NULL, pfnHandler, this);
	else
		create_error = pthread_create(&m_ListenerThread, NULL, SerialListener, this);

	if(create_error)
	{
		printf("%s: pthread_create failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		m_fListenerRunning = FALSE;
		Close();
		return FALSE;
	}

	m_pfnHandler = pfnHandler;

	return TRUE;
}

void CSerial::StopListener()
{
	void* thread_result;
	int ret;

	if(m_fListenerRunning)
	{
		m_fListenerRunning = FALSE;

		ret = pthread_join(m_ListenerThread, &thread_result);
		if(ret)
		{
			printf("%s: pthread_join failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		}
	}
}

void* CSerial::SerialListener(void *pParam)
{
	CSerial *pThis = (CSerial *)pParam;
	unsigned char buffer[1024];
	struct timeval timeout = { 10, 0 };
	fd_set	fdRead;
	int ret;
	

	while(pThis->m_fListenerRunning)
	{
		FD_SET((unsigned int)pThis->m_fdDev, &fdRead);
		ret = select(pThis->m_fdDev+1, &fdRead, NULL, NULL, &timeout);
		if((ret != ERROR)&&(FD_ISSET(pThis->m_fdDev, &fdRead))) 
		{
			memset(buffer, 0, 1024);
			ret = pThis->Read(buffer, 1024);
			if((ret != ERROR)&&(ret != 0))
			{
				buffer[ret] = 0;
				printf("%s\r\n", (char *)buffer);
			}
		}
	}

	pthread_exit(NULL); 
}



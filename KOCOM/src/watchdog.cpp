/*

*/

#include "common.h"

CWatchdog::CWatchdog()
{
	m_fdMem = -1;
	m_pWatchdog = NULL;
}

CWatchdog::~CWatchdog()
{
	DeInit();
}

BOOL CWatchdog::Init()
{
	m_fdMem = open(MEM_DEV, O_RDWR|O_SYNC);
	if(m_fdMem == ERROR)
	{
		printf("[Failure]\r\n--> %s: %s open failure: errno=%d %s\r\n", __func__, MEM_DEV, errno, strerror(errno));
		return FALSE;
	}

	m_pWatchdog = (volatile WATCHreg *)mmap(
				0,						// start
				sizeof(WATCHreg),		// length
				PROT_READ|PROT_WRITE,	// prot
				MAP_SHARED,				// flag
				m_fdMem,				// fd
				WDT_TIMER_REG_PHYSICAL	// offset
				);

	if((int)m_pWatchdog == ERROR)
	{
		printf("[Failure]\r\n--> %s: mmap failure: errno=%d %s\r\n", __func__, errno, strerror(errno));
		close(m_fdMem);
		return FALSE;
	}

	return TRUE;
}

void CWatchdog::DeInit()
{
	Disable();

	if(m_pWatchdog > 0)
		munmap((void *)m_pWatchdog, sizeof(WATCHreg));

	if(m_fdMem > 0)
		close(m_fdMem);

	m_pWatchdog	= NULL;
	m_fdMem		= -1;
}

void CWatchdog::SetPeriod(UINT msPeriod)
{
	if(m_pWatchdog)
	{
		if(msPeriod > MAX_WATCHDOG_PERIOD)
		{
			msPeriod = MAX_WATCHDOG_PERIOD;
		}
		m_pWatchdog->rWTDAT = TICKS_PER_SECOND / 1000 * msPeriod;
		m_pWatchdog->rWTCNT = TICKS_PER_SECOND / 1000 * msPeriod;
	}
}

void CWatchdog::Enable()
{
	USHORT usReg;

	if(m_pWatchdog)
	{
		usReg = m_pWatchdog->rWTCON;

		usReg |=  (255 << 8);	//Prescaler Value			0 ~ 255
		usReg |=  (1 << 5);		//Watchdog Timer			0=Disable, 1=Enable
		usReg &= ~(3 << 3);		//Clock Division Factor		0=16, 1=32, 2=64, 3=128
		usReg |=  (1 << 0);		//Watchdog timer output for reset	0=Disable, 1=Enable

		m_pWatchdog->rWTCON = usReg;
	}
}

void CWatchdog::Disable()
{
	USHORT usReg;

	if(m_pWatchdog)
	{
		usReg = m_pWatchdog->rWTCON;

	//	usReg |=  (255 << 8);	//Prescaler Value			0 ~ 255
		usReg &= ~(1 << 5);		//Watchdog Timer			0=Disable, 1=Enable
	//	usReg &= ~(3 << 3);		//Clock Division Factor		0=16, 1=32, 2=64, 3=128
		usReg &= ~(1 << 0);		//Watchdog timer output for reset	0=Disable, 1=Enable

		m_pWatchdog->rWTCON = usReg;
	}
}

void CWatchdog::Refresh()
{
	if(m_pWatchdog)
	{
		m_pWatchdog->rWTCNT = m_pWatchdog->rWTDAT;
	}
}

void CWatchdog::Reboot()
{
	SetPeriod(10);
	Enable();
	while(1);
}


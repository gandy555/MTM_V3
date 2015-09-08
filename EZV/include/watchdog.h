/*

*/
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
/*
	계산상 4030[ms]정도의 주기까지 가능
	FCLK=399.65M, PCLK=FCLK/6, Prescale=255, ClockDiv=16 
	--> TicksPerSec=PCLK/256/16 = 16261

	16비트 타이머 이므로 65535 / 16261 = 4.03
*/
#define MAX_WATCHDOG_PERIOD		4000
#define TICKS_PER_SECOND		16261

class CWatchdog
{
public:
	CWatchdog();
	~CWatchdog();

	//Member Function
	BOOL Init();
	void DeInit();

	void SetPeriod(UINT msPeriod);
	void Enable();
	void Disable();
	void Refresh();

	void Reboot();
	int CheckResetCause();

	//Member Variable
	int m_fdMem;
	volatile WATCHreg	*m_pWatchdog;
};

#endif //__WATCHDOG_H__

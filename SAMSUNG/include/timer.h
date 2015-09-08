/*

*/

#ifndef __TIMER_H__
#define __TIMER_H__

typedef void (*PFN_TIMER_HANDLER)(void *pParam);

//Timer ID
#define TIMER_IDLE_CLOCK			0


#define	MAX_TIMER					10	//LJK, modify, 2007-02-16
#define MAX_TIMER_NAME				32
typedef struct _TIMER_TABLE
{
	struct timeval		start;			//Timer start time (set by gettimeofday)
	int					second;			//Timer Time of each ID
	PFN_TIMER_HANDLER	pfnHandler;		//Handler of each ID
	char				szTimerName[MAX_TIMER_NAME+1];
	void*				pParam;
} __attribute__ ((packed)) TIMER_TABLE, *PTIMER_TABLE;


class CTimer
{
public:
	CTimer();
	~CTimer();

	//Member Function
	BOOL Run();
	void Stop();
	BOOL SetTimer(int id, int second, PFN_TIMER_HANDLER pfnHandler, const char *pTimerName=NULL, void* pParam=NULL);
	void KillTimer(int id);
	void ResetTimer(int id);
	BOOL GetTimer(int id);
	void Refresh();
	static void* TimerHandler(void *pParam);

	//Member Variable
	pthread_t		m_HandlerThread;
	TIMER_TABLE		m_table[MAX_TIMER];
	BOOL			m_fRunning;
};

#endif //__TIMER_H__

/*

*/
#include "common.h"
#include "main.h"

//
// CTimer Construction/Destruction
//

CTimer::CTimer()
{
	m_HandlerThread = 0;
	m_fRunning = FALSE;

	for(int i=0; i<MAX_TIMER; i++)
	{
		m_table[i].start.tv_sec	 = 0;
		m_table[i].start.tv_usec = 0;
		m_table[i].second		 = 0;
		m_table[i].pfnHandler	 = NULL;
		memset(m_table[i].szTimerName, 0, MAX_TIMER_NAME+1);
		m_table[i].pParam = NULL;
	}
}

CTimer::~CTimer()
{
}

//
// CTimer Implementation
//

BOOL CTimer::Run()
{
	int create_error;

#if 0
	if(m_fRunning)
	{
		DBGMSG(DBG_TIMER, "%s: Warning!! : Thread already running\r\n", __func__);
		return TRUE;
	}
#endif
	pthread_mutex_init(&m_timer_mutex, NULL);

	m_fRunning = TRUE;

	create_error = pthread_create(&m_HandlerThread, NULL, TimerHandler, this);
	if(create_error)
	{
		DBGMSG(DBG_TIMER, "%s: pthread_create failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		m_fRunning = FALSE;
	}

	return m_fRunning;
}

void CTimer::Stop()
{
	void* thread_result;
	int ret;

	if(m_fRunning)
	{
		m_fRunning = FALSE;

		ret = pthread_join(m_HandlerThread, &thread_result);
		if(ret)
		{
			DBGMSG(DBG_TIMER, "%s: pthread_join failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		}

		pthread_mutex_destroy(&m_timer_mutex);
	}
}

BOOL CTimer::SetTimer(int id, int second, PFN_TIMER_HANDLER pfnHandler, const char *pTimerName, void* pParam)
{
	struct timezone	tz;
	time_t now;
	struct tm *t;

	if((id<0)||(id>=MAX_TIMER))
	{
		DBGMSG(DBG_TIMER, "%s: invalid timer id. valid range is (0~%d)\r\n", __func__, MAX_TIMER-1);
		return FALSE;
	}

#if 0
	if(pfnHandler == NULL)
	{
		DBGMSG(DBG_TIMER, "%s: invalid timer handler\r\n", __func__);
		return FALSE;
	}
#endif

	pthread_mutex_lock(&m_timer_mutex);
	gettimeofday(&(m_table[id].start), &tz);
	m_table[id].second		= second;
	m_table[id].pfnHandler	= pfnHandler;
	memset(m_table[id].szTimerName, 0, MAX_TIMER_NAME+1);
	if(pTimerName)
	{
		if(strlen(pTimerName)>30)
			strncpy(m_table[id].szTimerName, pTimerName, 30);
		else
			strcpy(m_table[id].szTimerName, pTimerName);
	}
	m_table[id].pParam = pParam;

	now = time(NULL);
	t = localtime(&now);
	pthread_mutex_unlock(&m_timer_mutex);
	
	DBGMSG(DBG_TIMER, "%s: '%s' id=%d timeout=%d(sec) was started at %02d:%02d:%02d\r\n", 
			__func__, m_table[id].szTimerName, id, second, t->tm_hour, t->tm_min, t->tm_sec);

	return TRUE;
}

void CTimer::KillTimer(int id)
{
	if((id<0)||(id>=MAX_TIMER))
	{
		DBGMSG(DBG_TIMER, "%s: invalid timer id. valid range is (0~%d)\r\n", __func__, MAX_TIMER-1);
		return;
	}

	pthread_mutex_lock(&m_timer_mutex);
	m_table[id].second		= 0;
	m_table[id].pfnHandler	= NULL;
	m_table[id].pParam		= NULL;
	pthread_mutex_unlock(&m_timer_mutex);
	
	DBGMSG(DBG_TIMER, "%s: '%s' id=%d was stoped\r\n", __func__, m_table[id].szTimerName, id);
}

void CTimer::ResetTimer(int id)
{
	struct timezone	tz;

	if((id<0)||(id>=MAX_TIMER))
	{
		DBGMSG(DBG_TIMER, "%s: invalid timer id. valid range is (0~%d)\r\n", __func__, MAX_TIMER-1);
		return;
	}

	pthread_mutex_lock(&m_timer_mutex);
	gettimeofday(&(m_table[id].start), &tz);
	pthread_mutex_unlock(&m_timer_mutex);
}

BOOL CTimer::GetTimer(int id)
{
	if(m_table[id].pfnHandler)
		return TRUE;
	else
		return FALSE;
}

// NTP의 재설정등으로 시간이 틀어지면 타이머가 오동작 할 수 있음.
// 이런 경우 바뀐 시간으로 타이머를 재설정함
void CTimer::Refresh()
{
	struct timezone	tz;
	int i;

	for(i=0; i<MAX_TIMER; i++)
	{
		pthread_mutex_lock(&m_timer_mutex);
		if(m_table[i].second)
		{
			if(m_table[i].second)
				gettimeofday(&(m_table[i].start), &tz);
		}
		pthread_mutex_unlock(&m_timer_mutex);
	}
}

void* CTimer::TimerHandler(void *pParam)
{
	CTimer *pThis = (CTimer *)pParam;
	struct timeval  curr_time, elapsedTime;
	struct timezone	tz;
	int i;
	double d, d1, d2;

	if(pThis == NULL) return NULL;

	DBGMSG(DBG_TIMER, "%s: Start\r\n", __func__);

	while(pThis->m_fRunning)
	{
		for(i=0; i<MAX_TIMER; i++)
		{
			pthread_mutex_lock(&pThis->m_timer_mutex);
			if(pThis->m_table[i].second)
			{
				gettimeofday(&curr_time, &tz);
				d1 = (pThis->m_table[i].start.tv_sec) + (pThis->m_table[i].start.tv_usec) / 1000000.0;
				d2 = curr_time.tv_sec + curr_time.tv_usec / 1000000.0;
				d = d2 - d1;
				elapsedTime.tv_sec  = (long)d;
				elapsedTime.tv_usec = (long)((d - elapsedTime.tv_sec) * 1000000);
				//LJK, append, 시계표시 타이머의 정밀도를 100msec로 조정
				if(i == TIMER_IDLE_CLOCK)
				{
					if(elapsedTime.tv_usec > 100000)
					{
						if(pThis->m_table[i].pfnHandler)
							pThis->m_table[i].pfnHandler(pThis->m_table[i].pParam);
						else
							g_message.SendMessage(MSG_TIMER_EVENT, (ULONG)i, (ULONG)(pThis->m_table[i].pParam));
						gettimeofday(&(pThis->m_table[i].start), &tz);
					}
				}
				else
				{
					if(elapsedTime.tv_sec>=(pThis->m_table[i].second))
					{
						if(pThis->m_table[i].pfnHandler)
							pThis->m_table[i].pfnHandler(pThis->m_table[i].pParam);
						else
							g_message.SendMessage(MSG_TIMER_EVENT, (ULONG)i, (ULONG)(pThis->m_table[i].pParam));
						gettimeofday(&(pThis->m_table[i].start), &tz);
					}
				}
			}
			pthread_mutex_unlock(&pThis->m_timer_mutex);
		}
		usleep(1000);
	}

	DBGMSG(DBG_TIMER, "%s: End\r\n", __func__);

	pthread_exit(NULL);
}

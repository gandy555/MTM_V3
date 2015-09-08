/*


*/

#include "common.h"
#include "main.h"

CMicrowave::CMicrowave()
{
	m_fdAdc	= -1;
	m_HandlerThread = 0;
	m_fRunning = FALSE;
	m_isMeasureEnable = FALSE;

	memset(m_adc_sample_value, 0, sizeof(m_adc_sample_value));
	m_adc_sample_measure = 0;
	m_adc_sample_step = 0;
}

CMicrowave::~CMicrowave()
{
	DeInit();
}

BOOL CMicrowave::Init()
{
//	m_fdAdc = open(ADC_DEV, O_RDWR|O_SYNC);
	m_fdAdc = open(ADC_DEV, O_RDWR);
	if(m_fdAdc == ERROR)
	{
		DBGMSG(DBG_MICROWAVE, "%s: %s open failure: errno=%d %s\r\n", __func__, ADC_DEV, errno, strerror(errno));
		return FALSE;
	}

	ioctl(m_fdAdc, ADC_INPUT_PIN, 0);

	return TRUE;
}

void CMicrowave::DeInit()
{
	Stop();

	if(m_fdAdc > 0) close(m_fdAdc);

	m_fdAdc		= -1;
}

BOOL CMicrowave::Run()
{
	int create_error;

	if(m_fRunning)
	{
		DBGMSG(DBG_MICROWAVE, "%s: Warning!! : Thread already running\r\n", __func__);
		return TRUE;
	}

	m_fRunning = TRUE;

	create_error = pthread_create(&m_HandlerThread, NULL, MicrowaveHandler, this);
	if(create_error)
	{
		DBGMSG(DBG_MICROWAVE, "%s: pthread_create failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		Stop();
	}

	return m_fRunning;
}

void CMicrowave::Stop()
{
	void* thread_result;
	int ret;

	if(m_fRunning)
	{
		m_fRunning = FALSE;

		ret = pthread_join(m_HandlerThread, &thread_result);
		if(ret)
		{
			DBGMSG(DBG_MICROWAVE, "%s: pthread_join failure: error=%d %s\r\n", __func__, errno, strerror(errno));
		}
	}
}

int fn_qsort_intcmp( const void *a, const void *b )
{
	return( *(int *)a - *(int *)b);
}

BOOL CMicrowave::DoAdcSamplingStep()
{
	UINT uAdcValue = 0;
	BOOL fRet = FALSE;

	if(m_fdAdc < 0) return FALSE;
	if(m_adc_sample_step >= SAMPLE_COUNT) return TRUE;

//	if(m_adc_sample_step==0) m_adc_sample_measure = 0;

	if(read(m_fdAdc, &uAdcValue, sizeof(UINT)) == sizeof(UINT))
	{
		m_adc_sample_value[m_adc_sample_step] = uAdcValue;
		m_adc_sample_measure += uAdcValue;
		m_adc_sample_step++;

		if(m_adc_sample_step==SAMPLE_COUNT) fRet = TRUE;
	}

//	printf("%s: step=%d, value=%d, measure=%d\r\n", __func__, m_adc_sample_step, uAdcValue, m_adc_sample_measure);

	return fRet;
}

UINT CMicrowave::GetAdcSamplingResult()
{
	UINT uAdcSampleResult;
	int i;

	//샘플링된 ADC값을 정렬하고 
	qsort(m_adc_sample_value, SAMPLE_COUNT, sizeof(UINT), fn_qsort_intcmp);

	//정렬된 배열에서 MAX_FILTER, MIN_FILTER 갯수만큼 합계에서 제외한뒤 평균냄
	for(i=0; i<MIN_FILTER; i++)
	{
		m_adc_sample_measure -= m_adc_sample_value[i];					//앞에서 뺌
	}
	for(i=0; i<MAX_FILTER; i++)
	{
		m_adc_sample_measure -= m_adc_sample_value[SAMPLE_COUNT-1-i];	//뒤에서 뺌
	}

	uAdcSampleResult = (m_adc_sample_measure / (SAMPLE_COUNT - MAX_FILTER - MIN_FILTER));

	m_adc_sample_step = 0;
	m_adc_sample_measure = 0;

//	printf("%s: AdcSampleResult=%d\r\n", __func__, uAdcSampleResult);

	return uAdcSampleResult;
}

UINT CMicrowave::GetAdcSamplingValue()
{
	UINT i, adc_value[SAMPLE_COUNT], adc_measure=0, adc_error=0;

	if(m_fdAdc < 0) return 0;

	//ADC값을 SAMPLE_COUNT만큼 샘플링
	for(i=0; i<SAMPLE_COUNT; i++)
	{
		if(read(m_fdAdc, &(adc_value[i]), sizeof(UINT)) == sizeof(UINT))
		{
			adc_measure += adc_value[i];
		}
		else
		{
			adc_error++;	//ADC값 읽기 실패횟수
			if(i)
			{
				adc_value[i] = adc_value[i-1];	//못읽을 경우 이전값으로 보정
				adc_measure += adc_value[i];
				DBGMSG(DBG_MICROWAVE, "%s: ADC Read Error Adjusted\r\n", __func__);
			}
		}

		usleep(MEASURE_PERIOD);
	}

	//샘플링된 ADC값을 정렬하고 
	qsort(adc_value, SAMPLE_COUNT, sizeof(UINT), fn_qsort_intcmp);

	//정렬된 배열에서 MAX_FILTER, MIN_FILTER 갯수만큼 합계에서 제외한뒤 평균냄
	for(i=0; i<MIN_FILTER; i++)
	{
		adc_measure -= adc_value[i];					//앞에서 뺌
	}
	for(i=0; i<MAX_FILTER; i++)
	{
		adc_measure -= adc_value[SAMPLE_COUNT-1-i];		//뒤에서 뺌
	}

	adc_measure = (adc_measure / (SAMPLE_COUNT - MAX_FILTER - MIN_FILTER));

	return adc_measure;
}

void* CMicrowave::MicrowaveHandler(void *pParam)
{
	CMicrowave *pThis = (CMicrowave *)pParam;
	UINT uValue, ulTick;

	DBGMSG(DBG_MICROWAVE, "%s: Start\r\n", __func__);

	ulTick = GetTickCount();

	while(pThis->m_fRunning)
	{
		if(pThis->m_isMeasureEnable)
		{
		#if 0
			uValue = pThis->GetAdcSamplingValue();
			if(uValue > g_setup_data.m_SetupData.adc_value)
			{
			//	post_event(GR_EVENT_TYPE_USER, EVT_MICROWAVE_DETECT, uValue, 0);
			}
		#else
			if(pThis->DoAdcSamplingStep())
			{
				uValue = pThis->GetAdcSamplingResult();
				if(uValue > g_setup_data.m_SetupData.adc_value)
				{
					if(GetElapsedTick(ulTick) >= MEASURE_REPORT_TERM)
					{
						g_message.SendMessage(MSG_MICROWAVE_EVENT);
						ulTick = GetTickCount();
					}
				}
			}
		#endif
		}

		usleep(MEASURE_PERIOD);
	}

	DBGMSG(DBG_MICROWAVE, "%s: End\r\n", __func__);

	pthread_exit(NULL); 
}

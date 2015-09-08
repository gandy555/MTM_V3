/*

*/

#ifndef __MICROWAVE_H__
#define __MICROWAVE_H__

#define ADC_INPUT_PIN _IOW('S', 0x0c, unsigned long)

#define SAMPLE_COUNT		14		//30
#define MAX_FILTER			2		//3
#define MIN_FILTER			2		//3

#define MEASURE_PERIOD		1000	//1000

class CMicrowave
{
public:
	CMicrowave();
	~CMicrowave();

	//Member Function
	BOOL Init();
	void DeInit();

	BOOL Run();
	void Stop();

	void StartMeasure()	{ m_isMeasureEnable = TRUE; }
	void StopMeasure()	{ m_isMeasureEnable = FALSE; }

	BOOL DoAdcSamplingStep();
	UINT GetAdcSamplingResult();

	UINT GetAdcSamplingValue();

	static void* MicrowaveHandler(void *pParam);

	//Member Variable
	int				m_fdAdc;

	pthread_t		m_HandlerThread;
	BOOL			m_fRunning;

	BOOL			m_isMeasureEnable;

	UINT			m_adc_sample_value[SAMPLE_COUNT];
	UINT			m_adc_sample_measure;
	int				m_adc_sample_step;
};

#endif	//__MICROWAVE_H__

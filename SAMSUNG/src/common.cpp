/*
*/
#include "common.h"

void DumpData(UCHAR *data, int size, BOOL isHex)
{
	int i;
	UCHAR *p = data;

	for(i=0; i<size; i++)
	{
		if(isHex)
		{
			printf("%02x ", *p++);
			if( (i+1)%16 == 0 ) printf("\r\n");
		}
		else
			printf("%c", *p++);
	}
	printf("\r\n");
}

ULONG Conv_Asc2Bin(UCHAR* pData, int size)
{
	int i;
	ULONG ulValue = 0;
	UCHAR ucValue;

	if(size > 8) size = 8;

	for(i=0; i<size; i++)
	{
		if(i>0) ulValue <<= 4;

		ucValue = 0;
		if( (pData[i] >= '0') && (pData[i] <= '9') )
		{
			ucValue = pData[i] - '0';
		}
		else if( (pData[i] >= 'A') && (pData[i] <= 'F') )
		{
			ucValue = pData[i] - 'A' + 10;
		}
		else if( (pData[i] >= 'a') && (pData[i] <= 'f') )
		{
			ucValue = pData[i] - 'a' + 10;
		}

		ulValue += ucValue;
	}

	return ulValue;
}

void Conv_Bin2Asc(ULONG value, UCHAR* pData, int size)
{
	int i;
	char szData[9] = {0,};

	if(size > 8) size = 8;

	sprintf(szData, "%08x\0", value);

	for(i=0; i<size; i++)
	{
		pData[i] = szData[8-size+i];
	
	#if 0
		//Hex Lower
		if( (pData[i] >= 'A') && (pData[i] <= 'F') )
		{
			pData[i] += 0x20;
		}
	#endif
	}
}

ULONG Conv_BE2UL(UCHAR* pData, int size)
{
	int i;
	ULONG ulValue = 0;

	for(i=0; i<size; i++)
	{
		if(i>0) ulValue <<= 8;
		ulValue += pData[i];
	}

	return ulValue;
}

ULONG Conv_LE2UL(UCHAR* pData, int size)
{
	int i;
	ULONG ulValue = 0;

	for(i=(size-1); i>=0; i--)
	{
		if(i<(size-1)) ulValue <<= 8;
		ulValue += pData[i];
	}

	return ulValue;
}

float Conv_UL2FLOAT(ULONG ulValue)
{
	FLOAT_CONV conv;
	conv.ulValue = ulValue;
	return conv.fValue;
}

ULONG Conv_FLOAT2UL(float fValue)
{
	FLOAT_CONV conv;
	conv.fValue = fValue;
	return conv.ulValue;
}

void SetRect(RECT* pRect, int x, int y, int w, int h)
{
	if(pRect)
	{
		pRect->x = x;
		pRect->y = y;
		pRect->w = w;
		pRect->h = h;
	}
}

BOOL isLeapYear(int year)
{
	BOOL isDiv4, isDiv100, isDiv400, isLeapYear=FALSE;

	isDiv4   = ((year/4)==0)   ? TRUE : FALSE;
	isDiv100 = ((year/100)==0) ? TRUE : FALSE;
	isDiv400 = ((year/400)==0) ? TRUE : FALSE;

	if(isDiv4)
	{
		if(isDiv100)
		{
			if(isDiv400)
			{
				isLeapYear = TRUE;
			}
		}
		else
		{
			isLeapYear = TRUE;
		}
	}

	return isLeapYear;
}

void CalcDate(struct tm* t, int day)
{
	int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	
	//윤년체크
	if(isLeapYear(t->tm_year + 1900)) days[1] = 29;

	t->tm_mday += day;

	if(day > 0)
	{
		//더한 날짜가 해당월의 말일을 넘으면
		while( t->tm_mday > days[t->tm_mon] )
		{
			t->tm_mday -= days[t->tm_mon];

			t->tm_mon++;
			if(t->tm_mon==12)
			{
				t->tm_mon = 0;
				t->tm_year++;
				days[1] = (isLeapYear(t->tm_year + 1900)) ? 29 : 28;
			}
		}
	}
	else
	{
		//뺀 날짜가 0 이하가 되면
		while( t->tm_mday <= 0 )
		{
			t->tm_mon--;
			if(t->tm_mon<0)
			{
				t->tm_mon==11;
				t->tm_year--;
				days[1] = (isLeapYear(t->tm_year + 1900)) ? 29 : 28;
			}
			
			t->tm_mday += days[t->tm_mon];	//바뀐달의 날수를 적용하기 위해 뒤로뺌
		}
	}
}

/*
	gettimeofday의 리턴값을 ms단위의 tick count로 리턴함
	tick count는 0 ~ 86399999 (1일)
*/
#define MAX_TICK	86400000
ULONG GetTickCount()
{
	struct timeval  tv;
	struct timezone	tz;
	ULONG ulTick;
	double d;

	gettimeofday(&tv, &tz);

	// tv_sec값을 1일단위(86400초)로 자른다음 ms로 환산하고
	// tv_usec값도 ms로 환산하여 더한값을 tick count로 한다
#if 1
	ulTick = (tv.tv_sec % 86400)*1000 + tv.tv_usec/1000;
#else
	d = tv.tv_sec / 86400.0;
	ulTick = (ULONG)((d - (ULONG)d)*86400.0)*1000 + tv.tv_usec/1000;
#endif

	return ulTick;
}

ULONG GetElapsedTick(ULONG ulStartTick)
{
	ULONG ulCurrentTick, ulElpasedTick;

	if(ulStartTick > MAX_TICK) return 0;	//Error Case

	ulCurrentTick = GetTickCount();

	if(ulCurrentTick >= ulStartTick)
		ulElpasedTick = ulCurrentTick - ulStartTick;
	else
		ulElpasedTick = MAX_TICK - ulStartTick + ulCurrentTick;

	return ulElpasedTick;
}

void SetVolume(UINT nVolumeLevel)
{
	char wm9713_volume[9][6] = { "f0f0\0", "1818\0", "1212\0", "1010\0", "0808\0", "0707\0", "0606\0", "0303\0", "0000\0" };
	char cmd[128] = {0,};

	if(nVolumeLevel <= MAX_VOLUME_LEVEL)
	{
		sprintf(cmd, "echo %s > /proc/driver/wm9713/04-AC97_HEADPHONE_VOL", wm9713_volume[nVolumeLevel]);
		system(cmd);
	//	play_wavplay("/app/sound/touch.wav");
	}
}

void PlayWavFile(const char* pszWavFileName)
{
#if 1
	char szCommand[128] = {0,};

	if(pszWavFileName==NULL) return;

	system("killall -q wavplay\0");

	sprintf(szCommand, "/app/bin/wavplay -d /dev/sound_dsp -q %s & \0", pszWavFileName);

	system(szCommand);
	printf("Wav Playing : %s\r\n", pszWavFileName);
#endif
}

void StopWavPlay()
{
#if 1
	system("killall -q wavplay\0");
#endif
}


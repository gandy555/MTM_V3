#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <linux/input.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
/*
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
*/

#include "type.h"
#include "timer.h"

#include "dev.h"
#include "s3c2440.h"
#include "gpio.h"
#include "gpio_mtm.h"
#include "watchdog.h"

#include "file.h"
#include "setup_data.h"

#include "serial.h"
#include "microwave.h"

#include "sns_wallpad_protocol.h"
#include "serial_wallpad_sns.h"

#include "nano-X.h"
#include "nano-X_custom.h"

#include "object.h"
#include "ksc5601.h"

#include "context.h"
#include "state.h"

//#include "inverter.h"

#define FONT_PATH			"/app/font/NANUMBD.ttf"

#define MAX_VOLUME_LEVEL	8
#define MAX_ADC_VALUE		1023	//10bit - 0~0x3ff

//< 버전정보
#define VER_MAJOR			2					//Mandatory
#define VER_MINOR			34					//M
#define VER_REVISION		'A'					//Optional
#define VER_DATE			"20150818\0"		//M
#define VER_STRING			"for SHN-8810\0"	//O
//>

#define FLAG(bit)			(1<<(bit))
#define SET_FLAG(v,f)		((v) |= (f))
#define CLR_FLAG(v,f)		((v) &= ~(f))
#define CHK_FLAG(v,f)		(((v)&(f))==(f))

//
// Global Function
//
void DumpData(UCHAR *data, int size, BOOL isHex);

ULONG Conv_Asc2Bin(UCHAR* pData, int size);
void Conv_Bin2Asc(ULONG value, UCHAR* pData, int size);

ULONG Conv_BE2UL(UCHAR* pData, int size);
ULONG Conv_LE2UL(UCHAR* pData, int size);

float Conv_UL2FLOAT(ULONG ulValue);
ULONG Conv_FLOAT2UL(float fValue);
float Conv_UL2FLOAT(ULONG ulValue);
ULONG Conv_FLOAT2UL(float fValue);

void SetRect(RECT* pRect, int x, int y, int w, int h);

BOOL isLeapYear(int year);
void CalcDate(struct tm* t, int day);

ULONG GetTickCount();
ULONG GetElapsedTick(ULONG ulStartTick);

void SetVolume(UINT nVolumeLevel);
void PlayWavFile(const char* pszWavFileName);
void StopWavPlay();

#endif //__COMMON_H__

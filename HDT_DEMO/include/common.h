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
#include <poll.h>
//#include <sys/mqueue.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#include "type.h"

#include "dev.h"
#include "s3c2440.h"
#include "gpio.h"
#include "gpio_mtm.h"
#include "watchdog.h"
#include "serial.h"
#include "sysv_mq.h"
#include "message.h"

#include "timer.h"
#include "file.h"
#include "microwave.h"

#include "wallpad_protocol_mtm.h"
#include "wallpad_protocol_hdt.h"
#include "wallpad_protocol_shn.h"
#include "wallpad_protocol_kcm.h"

#include "wallpad.h"
#include "wallpad_hdt.h"
#include "wallpad_shn.h"
#include "wallpad_kcm.h"

#include "nano-X.h"
#include "nano-X_custom.h"

#include "object.h"
#include "ksc5601.h"

#include "context.h"
#include "state.h"

#include "setup_data.h"
#include "debug_zone.h"

#define FONT_PATH			"/app/font/NANUMBD.ttf"

#define MAX_VOLUME_LEVEL	8
#define MAX_ADC_VALUE		1023	//10bit - 0~0x3ff

//< 버전정보
#define VER_MAJOR			3					//Mandatory
#define VER_MINOR			5					//M
#define VER_REVISION		' '					//Optional
#define VER_DATE			"20150909\0"		//M
#define VER_STRING			"for HDT_DEMO\0"			//O
//>

#define BIT_FLAG(bit)		(1<<(bit))
#define SET_FLAG(v, b)		(v |= (b))
#define CLR_FLAG(v, b)		(v &= ~(b))
#define CHK_FLAG(v, b)		((v & (b))==(b))
#define MSK_FLAG(v, b)		(v & (b))

//#define DBGMSG(c,m...)	if(c) { printf("%s: ", __func__); printf(m); }
#define DBGMSG(c,m...)		if(c) { printf(m); }
#define DBGMSGC(c,m...)		if(c) { PrintClass(__PRETTY_FUNCTION__); printf(m); }
//#define DBGMSGC(c,m...)	if(c) { printf("%s: ", __func__); printf(m); }
#define DBGDMP(c,d,s,o)		if(c) DumpData(d,s,o)

//
// Global Function
//
void DumpData(UCHAR *data, int size, BOOL isHex);
void PrintClass(const char* pszPrettyFunction);

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
UCHAR CalcCheckSum(UCHAR* pBuffer, UINT size);

ULONG GetTickCount();
ULONG GetElapsedTick(ULONG ulStartTick);

void SetVolume(UINT nVolumeLevel);
void PlayWavFile(const char* pszWavFileName);
void StopWavPlay();

#endif //__COMMON_H__

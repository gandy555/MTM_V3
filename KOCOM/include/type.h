/*

*/

#ifndef __TYPE_H__
#define __TYPE_H__

#ifndef NULL
#define NULL	0
#endif

#ifndef ERROR
#define	ERROR	-1
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef LEVEL
	typedef int			LEVEL;
	#define HIGH		1
	#define LOW			0
	#define OPEN		-1
#endif

#ifndef UCHAR
typedef unsigned char UCHAR;
#endif

#ifndef USHORT
typedef unsigned short USHORT;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef POINT
typedef struct _POINT
{
	int	x;
	int y;
} __attribute__ ((packed)) POINT;
#endif

#ifndef RECT
typedef struct _RECT
{
	int	x;
	int y;
	int w;
	int h;
} __attribute__ ((packed)) RECT;
#endif

typedef struct input_event InputData;

typedef union
{
	ULONG		ulValue;
	float		fValue;
} __attribute__ ((packed)) FLOAT_CONV;

#endif //__TYPE_H__

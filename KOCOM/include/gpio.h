/*

*/

#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_BIT(num)				(1<<num)
#define GPIO_AF1(num,af)			(af<<num)
#define GPIO_AF2(num,af)			(af<<(num*2))

typedef enum _GPIO_GROUP
{
	GPIOA,	//25 output
	GPIOB,	//11 input/output
	GPIOC,	//16 input/output
	GPIOD,	//16 input/output
	GPIOE,	//16 input/output
	GPIOF,	//8  input/output
	GPIOG,	//16 input/output
	GPIOH,	//9  input/output
	GPIOJ	//13 input/output
} GPIO_GROUP;

// can apply from GPB. GPA is reserved control pin.
typedef enum _GPIO_AF
{
	AF_INPUT,
	AF_OUTPUT,
	AF_PRIMARY,
	AF_SECOND
} GPIO_AF;

typedef enum _GPIO_DIR
{
	GPIO_IN,
	GPIO_OUT
} GPIO_DIR;

typedef struct _GPIO_PIN
{
	GPIO_GROUP	group;
	int			num;
	GPIO_DIR	dir;
} __attribute__ ((packed)) GPIO_PIN, *PGPIO_PIN;

class CGpio
{
public:
	CGpio();
	~CGpio();

	//Member Function
	BOOL Init();
	void DeInit();

	ULONG GetCon(GPIO_GROUP group);
	ULONG GetDat(GPIO_GROUP group);

	void SetPin(GPIO_GROUP group, int num, LEVEL level);
	LEVEL GetPin(GPIO_GROUP group, int num);

	void SetAF(GPIO_GROUP group, int num, GPIO_AF af);
	void SetDir(GPIO_GROUP group, int num, GPIO_DIR dir);

	USHORT ChkResetState();

	//Member Variable
	int m_fdMem;
	volatile IOPreg *m_pGPIO;
};

#endif	//__GPIO_H__

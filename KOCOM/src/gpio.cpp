/*


*/

#define BUILD_GPIO_TABLE

#include "common.h"

CGpio::CGpio()
{
	m_pGPIO		= NULL;
	m_fdMem		= -1;
}

CGpio::~CGpio()
{
	DeInit();
}

BOOL CGpio::Init()
{
	m_fdMem = open(MEM_DEV, O_RDWR|O_SYNC);
	if(m_fdMem == ERROR)
	{
		printf("[Failure]\r\n--> %s: %s open failure: errno=%d %s\r\n", __func__, MEM_DEV, errno, strerror(errno));
		return FALSE;
	}

	m_pGPIO = (volatile IOPreg *)mmap(
				0,						// start
				sizeof(IOPreg),			// length
				PROT_READ|PROT_WRITE,	// prot
				MAP_SHARED,				// flag
				m_fdMem,				// fd
				GPIO_REG_PHYSICAL		// offset
				);
	if((int)m_pGPIO == ERROR)
	{
		printf("[Failure]\r\n--> %s: mmap failure: errno=%d %s\r\n", __func__, errno, strerror(errno));
		close(m_fdMem);
		return FALSE;
	}

//	printf("%s: success : m_pGPIO=0x%x\r\n", __func__, m_pGPIO);

	return TRUE;
}

void CGpio::DeInit()
{
	if(m_pGPIO > 0)
		munmap((void *)m_pGPIO, sizeof(IOPreg));

	if(m_fdMem > 0)
		close(m_fdMem);

	m_pGPIO		= NULL;
	m_fdMem		= -1;
}

ULONG CGpio::GetDat(GPIO_GROUP group)
{
	volatile ULONG *pDAT = NULL;

	switch(group)
	{
//	case GPIOA:
//		pDAT = &(m_pGPIO->rGPADAT);
//		break;
	case GPIOB:
		pDAT = &(m_pGPIO->rGPBDAT);
		break;
	case GPIOC:
		pDAT = &(m_pGPIO->rGPCDAT);
		break;
	case GPIOD:
		pDAT = &(m_pGPIO->rGPDDAT);
		break;
	case GPIOE:
		pDAT = &(m_pGPIO->rGPEDAT);
		break;
	case GPIOF:
		pDAT = &(m_pGPIO->rGPFDAT);
		break;
	case GPIOG:
		pDAT = &(m_pGPIO->rGPGDAT);
		break;
	case GPIOH:
		pDAT = &(m_pGPIO->rGPHDAT);
		break;
	case GPIOJ:
		pDAT = &(m_pGPIO->rGPJDAT);
		break;
	default:
		printf("%s: invalid GPIO group\r\n", __func__);
	}

	return (ULONG)pDAT;
}

ULONG CGpio::GetCon(GPIO_GROUP group)
{
	volatile ULONG *pCON = NULL;

	switch(group)
	{
//	case GPIOA:
//		pCON = &(m_pGPIO->rGPACON);
//		break;
	case GPIOB:
		pCON = &(m_pGPIO->rGPBCON);
		break;
	case GPIOC:
		pCON = &(m_pGPIO->rGPCCON);
		break;
	case GPIOD:
		pCON = &(m_pGPIO->rGPDCON);
		break;
	case GPIOE:
		pCON = &(m_pGPIO->rGPECON);
		break;
	case GPIOF:
		pCON = &(m_pGPIO->rGPFCON);
		break;
	case GPIOG:
		pCON = &(m_pGPIO->rGPGCON);
		break;
	case GPIOH:
		pCON = &(m_pGPIO->rGPHCON);
		break;
	case GPIOJ:
		pCON = &(m_pGPIO->rGPJCON);
		break;
	default:
		printf("%s: invalid GPIO group\r\n", __func__);
	}

	return (ULONG)pCON;
}


void CGpio::SetPin(GPIO_GROUP group, int num, LEVEL level)
{
	volatile ULONG *pDAT = (volatile ULONG *)GetDat(group);

	if(pDAT == NULL) return;

//	SetDir(group, num, GPIO_OUT);

	(level) ? (*pDAT |= GPIO_BIT(num)) : (*pDAT &= ~GPIO_BIT(num));
}

LEVEL CGpio::GetPin(GPIO_GROUP group, int num)
{
	volatile ULONG *pDAT = (volatile ULONG *)GetDat(group);

	if(pDAT == NULL) return ERROR;

//	SetDir(group, num, GPIO_IN);

	if((*pDAT & GPIO_BIT(num)) == GPIO_BIT(num))
		return HIGH;
	else
		return LOW;
}

void CGpio::SetAF(GPIO_GROUP group, int num, GPIO_AF af)
{
	volatile ULONG *pCON = (volatile ULONG *)GetCon(group);

	if(pCON == NULL) return;

	*pCON &= ~GPIO_AF2(num, 3);
	*pCON |= GPIO_AF2(num, af);
}

void CGpio::SetDir(GPIO_GROUP group, int num, GPIO_DIR dir)
{
	if(dir == GPIO_IN)
		SetAF(group, num, AF_INPUT);
	else
		SetAF(group, num, AF_OUTPUT);
}

USHORT CGpio::ChkResetState()
{
	USHORT usReg = m_pGPIO->rGSTATUS2;

	if(m_pGPIO == NULL) return 0;

	if(usReg) m_pGPIO->rGSTATUS2 = usReg;

#if 1
	printf("%s: GSTATUS2=0x%x\r\n", __func__, usReg);
#else
	if(usReg & 0x01)		//Power On Reset
		nRet = 1;
	else if(usReg & 0x02)	//Wakeup Reset
		nRet = 2;
	else if(usReg & 0x04)	//Watch Dog Reset
		nRet = 3;
#endif

	return usReg;
}

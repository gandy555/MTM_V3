#include "common.h"
#include "main.h"


//
// Construction/Destruction
//

CSerialWallPad::CSerialWallPad()
{
	int i;

	m_usMaker = WALLPAD_MAKER_UNDEFINED;
	m_usModel = WALLPAD_MODEL_UNDEFINED;

	for(i=0; i<MAX_QUE_COUNT; i++)
	{
		memset(&m_BufferQue[i][0], 0, MAX_RECV_BUFFER);
	}
	m_idxQuePush = 0;
	m_idxQuePop = 0;
	m_isQueFull = FALSE;
}

CSerialWallPad::~CSerialWallPad()
{

}

//
// Member Function
//
BOOL CSerialWallPad::Open(const char *pszDev, unsigned int baudRate)
{
	BOOL fRet = FALSE;

	if( (fRet = CSerial::Open(pszDev, baudRate)) == FALSE )
	{
		printf("[Failure]\r\n--> %s: open failure\r\n", __func__);
	}

	return fRet;
}

BOOL CSerialWallPad::Run()
{
	return FALSE;
}

void CSerialWallPad::Stop()
{
	StopListener();
}

int CSerialWallPad::Write(const UCHAR *pszBuffer, int size)
{
	int ret;

	if(m_fdDev < 0) return ERROR;

	while(m_isRecv) { usleep(1000); }

	// 485 Enable
	g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, LOW);

	// TX Enable�� ���� ��Ŷ�� �����ϱ���� 1.5ms ����
	usleep(1500);

	ret = write(m_fdDev, pszBuffer, size);

	usleep(size * 834);		// 

	//������ ��Ŷ ������ TX Disable �ϱ���� 500us ����
	usleep(500);

	// 485 Disable
	g_gpio_mtm.SetGpioOutPin(GPIO_RS485_CTRL, HIGH);

	if(ret == ERROR)
	{
		printf("%s: write failure : errno=%d %s\r\n", __func__, errno, strerror(errno));
	}

	return ret;
}

UCHAR CSerialWallPad::CalcCheckSum(UCHAR* pBuffer, UINT size)
{
	UCHAR cs = 0;
	UINT i;

	if(pBuffer==NULL) return 0;

	for(i=0; i<size; i++)
	{
		cs ^= pBuffer[i];
	}

	return cs;
}

int CSerialWallPad::GetPacketCountInQue()
{
	int count = 0;

	if(m_isQueFull)
		count = MAX_QUE_COUNT;
	else if(m_idxQuePush != m_idxQuePop)
		count = (m_idxQuePush > m_idxQuePop) ? (m_idxQuePush-m_idxQuePop) : (MAX_QUE_COUNT+m_idxQuePush-m_idxQuePop);

	return count;
}

//1. ��Ŷ�� ȣ���� �˻�� ��Ŷ���� ����
//2. InitData(0x5A), FreeData(0x41), ���� ���� ACK���� Que�� ���� ����
//   --> ���ÿ��� �������� ���ɸ��� �͵鸸 �ְԵ�
//3. Que�� Full�̸� ������� Push�� ��Ŷ�ڸ��� OverWrite��
BOOL CSerialWallPad::PushPacketIntoQue(UCHAR* pPacket, UCHAR len)
{
	if(pPacket==NULL) return ERROR;

	//��Ŷ�� ���̰� ����ũ�⸦ �Ѿ�� ��� --> �߸��� ��Ŷ�̹Ƿ� ����
	if( (len==0) || ((len+4) > MAX_RECV_BUFFER) ) return ERROR;

	//m_isQueFull: Pop�� ���� Push�� MAX_QUE_COUNT��ŭ �̷������ ���̻� Push�� ������ ����
	//			   --> Empty���¿� Full���´� Push Index�� Pop Index�� ������
	if(m_isQueFull)
	{
		//Pop Index�� Plus Shift�����ν� Que���� ������� Push�� ����Ÿ�� �������� ó����
		m_idxQuePop++;
		if(m_idxQuePop == MAX_QUE_COUNT) m_idxQuePop=0;

	//	m_isQueFull = FALSE;	//������ �ٽ� ä���� ��Ȳ�̹Ƿ� �ǹ� ����
	}

	memset(&m_BufferQue[m_idxQuePush][0], 0, MAX_RECV_BUFFER);
	memcpy(&m_BufferQue[m_idxQuePush][0], pPacket, len+4);

	//Push Index Plus Shift
	m_idxQuePush++;
	if(m_idxQuePush == MAX_QUE_COUNT) m_idxQuePush=0;

	if(m_idxQuePush == m_idxQuePop) m_isQueFull=TRUE;

	return TRUE;
}

BOOL CSerialWallPad::PopPacketFromQue(UCHAR* pPacket)
{
	if(pPacket==NULL) return ERROR;

	//Full�� �ƴѵ� Pop Index�� Push Index�� ���ٸ� Que�� ��� �ִ� ����
	//--> ���� Full�̰ų� Pop Index�� Push Index�� �ٸ��� Que�� ��Ŷ�� �ִ� ����
	if( (m_isQueFull) || (m_idxQuePop!=m_idxQuePush) )
	{
		//�����͸� ���� �� ��� Full�� ��� ���Ⱑ ���� �ʾ� 
		//Que�� ó�� ��Ŷ�� �ƴ϶� ������ ��Ŷ�� ó���� �� ����
		//����, Que�� ��Ŷ�� �������ִ� ������� ��ȯ
		memcpy(pPacket, &m_BufferQue[m_idxQuePop][0], MAX_RECV_BUFFER);

		//Pop Index Plus Shift
		m_idxQuePop++;
		if(m_idxQuePop == MAX_QUE_COUNT) m_idxQuePop=0;

		m_isQueFull=FALSE;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UINT CSerialWallPad::ConvWeatherIcon(UINT nWallPadIcon)
{
	return 0;
}

void CSerialWallPad::CallContextReceiveProc(void *pParam)
{
#if 1	//�������

	#if 0
		CContext* pContext = NULL;

		pContext = g_state.GetCurrContext();
		if(pContext && pParam)
		{
			pContext->RecvProc(pParam);
		}
	#else
		g_state.RecvProc((UCHAR *)pParam);
	#endif

#else	//����� - Que���

	PushPacketIntoQue((UCHAR *)pParam);

#endif
}



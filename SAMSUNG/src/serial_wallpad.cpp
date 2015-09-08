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

	// TX Enable후 최초 패킷을 전송하기까지 1.5ms 지연
	usleep(1500);

	ret = write(m_fdDev, pszBuffer, size);

	usleep(size * 834);		// 

	//마지막 패킷 전송후 TX Disable 하기까지 500us 지연
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

//1. 패킷은 호출전 검사된 패킷으로 가정
//2. InitData(0x5A), FreeData(0x41), 값이 없는 ACK들은 Que에 넣지 않음
//   --> 어플에서 응답으로 사용될만한 것들만 넣게됨
//3. Que가 Full이면 가장먼저 Push된 패킷자리에 OverWrite됨
BOOL CSerialWallPad::PushPacketIntoQue(UCHAR* pPacket, UCHAR len)
{
	if(pPacket==NULL) return ERROR;

	//패킷의 길이가 버퍼크기를 넘어서는 경우 --> 잘못된 패킷이므로 무시
	if( (len==0) || ((len+4) > MAX_RECV_BUFFER) ) return ERROR;

	//m_isQueFull: Pop이 없이 Push만 MAX_QUE_COUNT만큼 이루어지면 더이상 Push할 공감이 없음
	//			   --> Empty상태와 Full상태는 Push Index와 Pop Index가 같아짐
	if(m_isQueFull)
	{
		//Pop Index를 Plus Shift함으로써 Que에서 가장먼저 Push된 데이타를 버리도록 처리함
		m_idxQuePop++;
		if(m_idxQuePop == MAX_QUE_COUNT) m_idxQuePop=0;

	//	m_isQueFull = FALSE;	//어차피 다시 채워질 상황이므로 의미 없음
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

	//Full이 아닌데 Pop Index가 Push Index와 같다면 Que가 비어 있는 상태
	//--> 따라서 Full이거나 Pop Index와 Push Index가 다르면 Que에 패킷이 있는 상태
	if( (m_isQueFull) || (m_idxQuePop!=m_idxQuePush) )
	{
		//포인터만 전달 할 경우 Full일 경우 동기가 맞지 않아 
		//Que의 처음 패킷이 아니라 마지막 패킷을 처리할 수 있음
		//따라서, Que의 패킷을 복사해주는 방식으로 전환
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
#if 1	//이전방식

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

#else	//새방식 - Que사용

	PushPacketIntoQue((UCHAR *)pParam);

#endif
}



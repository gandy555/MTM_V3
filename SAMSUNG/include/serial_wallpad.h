#ifndef __SERIAL_WALLPAD_H__
#define __SERIAL_WALLPAD_H__

#define MAX_RECV_BUFFER					64
#define MAX_QUE_COUNT					10

#define SELECT_TIMEOUT_SEC				0
#define SELECT_TIMEOUT_USEC				500000		//500ms

#define RECEIVE_WAIT_TIMEOUT			5			//sec
#define MAX_SELECT_TIMEOUT_COUNT		10			//500ms x 10 = 5sec

#define MAX_WAIT_COUNT_FOR_RESPONSE		3

#define PACKET_DUMP_ENABLE
#define DEBUG_RESPONSE


class CSerialWallPad : public CSerial
{
public:
	CSerialWallPad();
	~CSerialWallPad();

	//Member Function
	virtual BOOL Open(const char *pszDev, UINT baudRate=B9600);

	virtual BOOL Run();
	virtual void Stop();

	virtual int Write(const UCHAR *pszBuffer, int size);

	UCHAR CalcCheckSum(UCHAR* pBuffer, UINT size);

	int  GetPacketCountInQue();
	BOOL PushPacketIntoQue(UCHAR* pPacket, UCHAR len);
	BOOL PopPacketFromQue(UCHAR* pPacket);

	UINT ConvWeatherIcon(UINT nWallPadIcon);

	void CallContextReceiveProc(void *pParam);

	//Member Variable
	USHORT		m_usMaker;
	USHORT		m_usModel;

	UCHAR		m_BufferQue[MAX_QUE_COUNT][MAX_RECV_BUFFER];
	UCHAR		m_idxQuePush;
	UCHAR		m_idxQuePop;
	BOOL		m_isQueFull;

};

#endif //__SERIAL_WALLPAD_H__


#include "common.h"

//
// Constructor / Destructor
//
CMessage::CMessage()
{
	m_msgid = -1;
}

CMessage::~CMessage()
{
}


//
// Member Function
//
BOOL CMessage::Init()
{
	BOOL fRet = TRUE;
	int ret;

	m_msgid = m_mq.Create(MSG_KEY_MAIN);
	if(m_msgid < 0) 
	{
		DBGMSGC(DBG_MESSAGE, "MSG_KEY_MAIN create failure\r\n");
		fRet = FALSE;
	}
	
	if( (ret = m_mq.GetMsgCount()) )
	{
		DBGMSG(DBG_MESSAGE, "[MSG:INIT] MsgCount Result=%d\r\n", ret);
		if(ret > 0)
		{
			ret = m_mq.Remove();
			DBGMSG(DBG_MESSAGE, "[MSG:INIT] Remove Result=%d\r\n", ret);

			m_msgid = m_mq.Create(MSG_KEY_MAIN);
		}
	}

	return fRet;
}

void CMessage::DeInit()
{
	if(m_msgid > 0)
	{
		m_mq.Remove(m_msgid);
		m_msgid = -1;
	}
}

int CMessage::SendMessage(long msg_type, ULONG param1, ULONG param2, int msg_flag)
{
	MTM_MSG msg = {0,};
	int ret;

	if( (ret = m_mq.GetMsgCount()) >= MSG_MAX_COUNT )
	{
		DBGMSG(DBG_MESSAGE, "Unread Message Ramain In System Que [%d]. SendMessage Skip!\r\n", ret);
		//m_mq.Remove();
		//m_msgid = m_mq.Create(MSG_KEY_MAIN);
	//	return -1;
		return -1;
	}

	msg.msg_type = msg_type;
	msg.param1 = param1;
	msg.param2 = param2;

	ret = m_mq.Send(m_msgid, (struct msgbuf *)&msg, sizeof(MTM_MSG)-sizeof(long), msg_flag);

	return ret;
}

int CMessage::RecvMessage(MTM_MSG *pmsg, BOOL isWait)
{	
	if (pmsg == NULL) {
		DBGMSG(DBG_MESSAGE, "Invalid pmsg: NULL\r\n");
		return -1;
	}

	return m_mq.Recv(m_msgid, (struct msgbuf *)pmsg, sizeof(MTM_MSG)-sizeof(long), 0, (isWait) ? 0 : IPC_NOWAIT);
}

int CMessage::SendMessageData(long msg_type, void* pData, ULONG ulSize, int msg_flag)
{
	MTM_MSG msg = {0,};
	int msg_count;

	if(pData == NULL)
	{
		DBGMSG(DBG_MESSAGE, "Invalid pData: NULL\r\n");
		return -1;
	}
	if(ulSize > MSG_DATA_SIZE)
	{
		DBGMSG(DBG_MESSAGE, "Invalid Data Size\r\n");
		return -1;
	}
/*
	if( (msg_count = m_mq.GetMsgCount()) >= MSG_MAX_COUNT )
	{
		DBGMSG(DBG_MESSAGE, "Unread Message Ramain In System Que [%d]. SendMessage Skip!\r\n", msg_count);
		return -1;
	}
*/

	msg.msg_type = msg_type;
	msg.param1 = ulSize;
	msg.param2 = 0;
	memcpy(msg.data, pData, ulSize);

	return m_mq.Send(m_msgid, (struct msgbuf *)&msg, sizeof(MTM_MSG)-sizeof(long), msg_flag);
}

int CMessage::GetMessageCount()
{
	return m_mq.GetMsgCount();
}

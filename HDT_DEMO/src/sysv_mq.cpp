/*
	Description:
		SystemV Message Function Manipulation Class

	History:
		LJK		2014-04-15		First Coding

*/
#include "common.h"

//
// Constructor / Destructor
//
CSysvMessageQue::CSysvMessageQue()
{
	m_msg_id = -1;
}

CSysvMessageQue::~CSysvMessageQue()
{
}

//
// Member Function
//
int CSysvMessageQue::Create(key_t key, int msg_flag)
{
	m_msg_id = msgget(key, msg_flag);
	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgget error=%d, %s\r\n", errno, strerror(errno));
	}

	return m_msg_id;
}

int CSysvMessageQue::Send(int msg_id, struct msgbuf *pmsg, int msg_size, int msg_flag)
{
	int ret;

	if(msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_id\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg\r\n");
		return -3;
	}

	//msgsnd retun 0 on success
	ret = msgsnd(msg_id, (void *)pmsg, msg_size, msg_flag);
	if(ret < 0)
	{
		if( (msg_flag & IPC_NOWAIT) && (errno == EAGAIN) )
			ret = 0;	//substitute : ret==-1 && EAGAIN
		else
			DBGMSG(DBG_SYSV_MQ, "msgsnd error=%d, %s\r\n", errno, strerror(errno));
	}
	else if(ret > 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgsnd un-expected return: %d\r\n", ret);
		ret = -4;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

int CSysvMessageQue::Send(struct msgbuf *pmsg, int msg_size, int msg_flag)
{
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg\r\n");
		return -3;
	}

	//msgsnd retun 0 on success
	ret = msgsnd(m_msg_id, (void *)pmsg, msg_size, msg_flag);
	if(ret < 0)
	{
		if( (msg_flag & IPC_NOWAIT) && (errno == EAGAIN) )
			ret = 0;	//substitute : ret==-1 && EAGAIN
		else
			DBGMSG(DBG_SYSV_MQ, "msgsnd error=%d, %s\r\n", errno, strerror(errno));
	}
	else if(ret > 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgsnd un-expected return: %d\r\n", ret);
		ret = -4;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

int CSysvMessageQue::Send(long msg_type, void *pmsg, int msg_size, int msg_flag)
{
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg\r\n");
		return -3;
	}
	if(msg_size > MAX_MSG_SIZE)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_size. over the limit(%d)\r\n", MAX_MSG_SIZE);
		return -4;
	}

	memset(&m_msg, 0, sizeof(m_msg));
	m_msg.msg_type = msg_type;
	memcpy(m_msg.msg_buffer, pmsg, msg_size);

	//msgsnd retun 0 on success
	ret = msgsnd(m_msg_id, (void *)&m_msg, msg_size, msg_flag);
	if(ret < 0)
	{
		if( (msg_flag & IPC_NOWAIT) && (errno == EAGAIN) )
			ret = 0;	//substitute : ret==-1 && EAGAIN
		else
			DBGMSG(DBG_SYSV_MQ, "msgsnd error=%d, %s\r\n", errno, strerror(errno));
	}
	else if(ret > 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgsnd un-expected return: %d\r\n", ret);
		ret = -5;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

int CSysvMessageQue::Recv(int msg_id, struct msgbuf *pmsg, int msg_size, long msg_type, int msg_flag)
{
	int ret;

	if (msg_id < 0) {
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_id\r\n");
		return -2;
	}
	
	if (pmsg == NULL) {
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg\r\n");
		return -3;
	}
	
	if (msg_size > MAX_MSG_SIZE) {
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_size. over the limit(%d)\r\n", MAX_MSG_SIZE);
		return -4;
	}

	ret = msgrcv(msg_id, (void *)pmsg, msg_size, msg_type, msg_flag);
	if (ret < 0) {
		if( (msg_flag & IPC_NOWAIT) && (errno == ENOMSG) )
			ret = 0;	//substitute : ret==-1 && ENOMSG
		else
			DBGMSG(DBG_SYSV_MQ, "msgrcv error=%d, %s\r\n", errno, strerror(errno));
	}
	//is it possible ret==0

	return ret;
}

int CSysvMessageQue::Recv(struct msgbuf *pmsg, int msg_size, long msg_type, int msg_flag)
{
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg\r\n");
		return -3;
	}
	if(msg_size > MAX_MSG_SIZE)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_size. over the limit(%d)\r\n", MAX_MSG_SIZE);
		return -4;
	}

	ret = msgrcv(m_msg_id, (void *)pmsg, msg_size, msg_type, msg_flag);
	if(ret < 0)
	{
		if( (msg_flag & IPC_NOWAIT) && (errno == ENOMSG) )
			ret = 0;	//substitute : ret==-1 && ENOMSG
		else
			DBGMSG(DBG_SYSV_MQ, "msgrcv error=%d, %s\r\n", errno, strerror(errno));
	}
	//is it possible ret==0

	return ret;
}

int CSysvMessageQue::Recv(long msg_type, void *pmsg, int msg_size, int msg_flag)
{
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg\r\n");
		return -3;
	}
	if(msg_size > MAX_MSG_SIZE)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_size. over the limit(%d)\r\n", MAX_MSG_SIZE);
		return -4;
	}

	memset(&m_msg, 0, sizeof(m_msg));

	ret = msgrcv(m_msg_id, (void *)&m_msg, msg_size, msg_type, msg_flag);
	if(ret < 0)
	{
		if( (msg_flag & IPC_NOWAIT) && (errno == ENOMSG) )
			ret = 0;	//substitute : ret==-1 && ENOMSG
		else
			DBGMSG(DBG_SYSV_MQ, "msgrcv error=%d, %s\r\n", errno, strerror(errno));
	}
	else if(ret > 0)
	{
	//	memcpy(pmsg, m_msg.msg_buffer, msg_size);
		memcpy(pmsg, &m_msg, msg_size+sizeof(long));
	}
	//is it possible ret==0

	return ret;
}

long CSysvMessageQue::RecvAll(int msg_flag)
{
	long ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}

	memset(&m_msg, 0, sizeof(m_msg));

	ret = msgrcv(m_msg_id, (void *)&m_msg, MAX_MSG_SIZE, 0, msg_flag);
	if(ret < 0)
	{
		if( (msg_flag & IPC_NOWAIT) && (errno == ENOMSG) )
			ret = 0;	//substitute : ret==-1 && ENOMSG
		else
			DBGMSG(DBG_SYSV_MQ, "msgrcv error=%d, %s\r\n", errno, strerror(errno));
	}
	else if(ret > 0)
	{
		ret = m_msg.msg_type;
	}
	//is it possible ret==0

	return ret;
}

int CSysvMessageQue::Control(int msg_id, int cmd, struct msqid_ds *pmsg_ds)
{
	int ret;

	if(pmsg_ds == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg_ds\r\n");
		return -2;
	}

	ret = msgctl(msg_id, cmd, pmsg_ds);
	if(ret < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl (id=%d, cmd=%d) error=%d, %s\r\n", msg_id, cmd, errno, strerror(errno));
	}

	//Success Case:
	//	cmd - IPC_STAT, IPC_SET, and IPC_RMID --> ret==0
	//  cmd - IPC_INFO or MSG_INFO			  --> ret==index

	return ret;
}

int CSysvMessageQue::Control(int cmd, struct msqid_ds *pmsg_ds)
{
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}
	if(pmsg_ds == NULL)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid pmsg_ds\r\n");
		return -3;
	}

	ret = msgctl(m_msg_id, cmd, pmsg_ds);
	if(ret < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl (id=%d, cmd=%d) error=%d, %s\r\n", m_msg_id, cmd, errno, strerror(errno));
	}

	//Success Case:
	//	cmd - IPC_STAT, IPC_SET, and IPC_RMID --> ret==0
	//  cmd - IPC_INFO or MSG_INFO			  --> ret==index

	return ret;
}

int CSysvMessageQue::GetMsgCount(int msg_id)
{
	struct msqid_ds msg_ds;
	int ret;

	if(msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}

	ret = msgctl(msg_id, IPC_STAT, &msg_ds);
	if(ret < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl (id=%d, cmd=%d) error=%d, %s\r\n", msg_id, IPC_STAT, errno, strerror(errno));
	}
	else if(ret == 0)	//success
	{
		ret = (int)msg_ds.msg_qnum;		//msg_qnum type is unsigned long
	}
	else
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl un-expected return: %d\r\n", ret);
		ret = -3;
	}

	return ret;
}

int CSysvMessageQue::GetMsgCount()
{
	struct msqid_ds msg_ds;
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -2;
	}

	ret = msgctl(m_msg_id, IPC_STAT, &msg_ds);
	if(ret < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl (id=%d, cmd=%d) error=%d, %s\r\n", m_msg_id, IPC_STAT, errno, strerror(errno));
	}
	else if(ret == 0)	//success
	{
		ret = (int)msg_ds.msg_qnum;		//msg_qnum type is unsigned long
	}
	else
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl un-expected return: %d\r\n", ret);
		ret = -3;
	}

	return ret;
}

int CSysvMessageQue::Remove(int msg_id)
{
	int ret;

	if(msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid msg_id\r\n");
		return -2;
	}

	ret = msgctl(msg_id, IPC_RMID, NULL);
	if(ret < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl (id=%d, cmd=%d) error=%d, %s\r\n", msg_id, IPC_RMID, errno, strerror(errno));
	}

	// ret==0 on success

	return ret;
}

int CSysvMessageQue::Remove()
{
	int ret;

	if(m_msg_id < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "Invalid m_msg_id\r\n");
		return -1;
	}

	ret = msgctl(m_msg_id, IPC_RMID, NULL);
	if(ret < 0)
	{
		DBGMSG(DBG_SYSV_MQ, "msgctl (id=%d, cmd=%d) error=%d, %s\r\n", m_msg_id, IPC_RMID, errno, strerror(errno));
	}

	// ret==0 on success

	return ret;
}


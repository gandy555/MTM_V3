/*
	Description:
		Posix Message Function Manipulation Class

	History:
		LJK		2014-04-14		First Coding

*/
#include "common.h"

//
// Constructor / Destructor
//
CPosixMessageQue::CPosixMessageQue()
{
	memset(m_mq_name, 0, MAX_MQ_NAME_SIZE+1);
	m_mqd = -1;
	memset(&m_mq_attr, 0, sizeof(struct mq_attr));
	m_mq_attr.mq_maxmsg = DEFAULT_MQ_COUNT;
	m_mq_attr.mq_msgsize = DEFAULT_MSG_SIZE;
}

CPosixMessageQue::CPosixMessageQue(const char *mq_name)
{
	memset(m_mq_name, 0, MAX_MQ_NAME_SIZE+1);
	m_mqd = -1;
	memset(&m_mq_attr, 0, sizeof(struct mq_attr));
	m_mq_attr.mq_maxmsg = DEFAULT_MQ_COUNT;
	m_mq_attr.mq_msgsize = DEFAULT_MSG_SIZE;

	if(mq_name)
	{
		strcpy(m_mq_name, mq_name);
	}
	else
	{
		DBGMSG(DBG_POSIX_MQ, "invalid message que name\r\n");
	}
}

CPosixMessageQue::~CPosixMessageQue()
{
	if(m_mqd > 0)
	{
		if(Close() > 0)
		{
			Remove();
		}
	}
}

//
// Member Function
//
int CPosixMessageQue::Create(const char *mq_name, int mq_maxmsg, int mq_msgsize, int oflag, mode_t mode)
{
	if(mq_name == NULL) 
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mq_name : NULL\r\n");
		return -2;
	}

	memset(m_mq_name, 0, MAX_MQ_NAME_SIZE+1);
	strcpy(m_mq_name, mq_name);

	memset(&m_mq_attr, 0, sizeof(struct mq_attr));
	m_mq_attr.mq_maxmsg = mq_maxmsg;
	m_mq_attr.mq_msgsize = mq_msgsize;

	m_mqd = mq_open(m_mq_name, oflag, mode, &m_mq_attr);
	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_open error = %d, %s\r\n", errno, strerror(errno));
	}

	return m_mqd;
}

int CPosixMessageQue::Create(int mq_maxmsg, int mq_msgsize, int oflag, mode_t mode)
{
	if(strlen(m_mq_name)==0) 
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid m_mq_name\r\n");
		return -2;
	}

	memset(&m_mq_attr, 0, sizeof(struct mq_attr));
	m_mq_attr.mq_maxmsg = mq_maxmsg;
	m_mq_attr.mq_msgsize = mq_msgsize;

	m_mqd = mq_open(m_mq_name, oflag, mode, &m_mq_attr);
	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_open error = %d, %s\r\n", errno, strerror(errno));
	}

	return m_mqd;
}

int CPosixMessageQue::Open(const char *mq_name, int oflag)
{
	mqd_t mqd;

	if(mq_name == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mq_name: NULL\r\n");
		return -2;
	}

	mqd = mq_open(mq_name, oflag);
	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_open error = %d, %s\r\n", errno, strerror(errno));
	}

	return mqd;
}

int CPosixMessageQue::Open(int oflag)
{
	if(strlen(m_mq_name)==0) 
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid message que name\r\n");
		return -2;
	}

#if 0
	if(m_mqd > 0)
	{
		DBGMSG(DBG_POSIX_MQ, "already mq_open\r\n");
		return -3;
	}
#endif

	m_mqd = mq_open(m_mq_name, oflag);
	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_open error = %d, %s\r\n", errno, strerror(errno));
	}

	return m_mqd;
}

int CPosixMessageQue::Close(mqd_t mqd)
{
	int ret;

	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mqd descriptor\r\n");
		return -2;
	}

	ret = mq_close(mqd);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_close error = %d, %s\r\n", errno, strerror(errno));
	}

	//On success mq_close() returns 0

	return ret;
}

int CPosixMessageQue::Close()
{
	int ret;

	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mq_open descriptor\r\n");
		return -2;
	}

	ret = mq_close(m_mqd);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_close error = %d, %s\r\n", errno, strerror(errno));
	}

	//On success mq_close() returns 0

	return ret;
}

int CPosixMessageQue::Remove(const char *mq_name)
{
	int ret;

	if(mq_name == NULL) 
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mq_name: NULL\r\n");
		return -2;
	}

	ret = mq_unlink(mq_name);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_unlink error=%d, %s\r\n", errno, strerror(errno));
	}

	//On success mq_unlink() returns 0

	return ret;
}

int CPosixMessageQue::Remove()
{
	int ret;

	if(strlen(m_mq_name)==0) 
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid message que name\r\n");
		return -2;
	}

	ret = mq_unlink(m_mq_name);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_unlink error=%d, %s\r\n", errno, strerror(errno));
	}

	//On success mq_unlink() returns 0

	return ret;
}

int CPosixMessageQue::Send(mqd_t mqd, void *pmsg, int msg_size, int to_sec)
{
	unsigned int msg_prio = DEFAULT_MQ_PRIORITY;
//	struct timespec ts = { .tv_sec = to_sec, .tv_nsec = 0 };
	struct timespec ts = {0,};
	int ret;

	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mqd\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid pmsg: NULL\r\n");
		return -3;
	}
	if(msg_size > DEFAULT_MSG_SIZE)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid msg_size: too big\r\n");
		return -4;
	}

	ts.tv_sec  = to_sec;
//	ts.tv_nsec = 0;

	if(to_sec == 0)
		ret = mq_send(mqd, (char *)pmsg, (size_t)msg_size, msg_prio);
	else
		ret = mq_timedsend(mqd, (char *)pmsg, (size_t)msg_size, msg_prio, &ts);

	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "%s error=%d, %s\r\n", (to_sec == 0) ? "mq_send" : "mq_timedsend", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::Send(void *pmsg, int msg_size, int to_sec)
{
	unsigned int msg_prio = DEFAULT_MQ_PRIORITY;
	struct timespec ts = {0,};
	int ret;

	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid m_mqd\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid pmsg: NULL\r\n");
		return -3;
	}
	if(msg_size > DEFAULT_MSG_SIZE)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid msg_size: too big\r\n");
		return -4;
	}

	ts.tv_sec  = to_sec;
//	ts.tv_nsec = 0;

	if(to_sec == 0)
		ret = mq_send(m_mqd, (char *)pmsg, (size_t)msg_size, msg_prio);
	else
		ret = mq_timedsend(m_mqd, (char *)pmsg, (size_t)msg_size, msg_prio, &ts);

	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "%s error=%d, %s\r\n", (to_sec == 0) ? "mq_send" : "mq_timedsend", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::Recv(mqd_t mqd, void *pmsg, int msg_size, int to_sec)
{
	unsigned int msg_prio = DEFAULT_MQ_PRIORITY;
	struct timespec ts = {0,};
	int ret;

	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mqd\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid pmsg: NULL\r\n");
		return -3;
	}
	if(msg_size > DEFAULT_MSG_SIZE)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid msg_size: too big\r\n");
		return -4;
	}

	ts.tv_sec  = to_sec;
//	ts.tv_nsec = 0;

	if(to_sec == 0)
		ret = mq_receive(mqd, (char *)pmsg, (size_t)msg_size, &msg_prio);
	else
		ret = mq_timedreceive(mqd, (char *)pmsg, (size_t)msg_size, &msg_prio, &ts);

	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "%s error=%d, %s\r\n", (to_sec == 0) ? "mq_receive" : "mq_timedreceive", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::Recv(void *pmsg, int msg_size, int to_sec)
{
	unsigned int msg_prio = DEFAULT_MQ_PRIORITY;
	struct timespec ts = {0,};
	int ret;

	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid m_mqd\r\n");
		return -2;
	}
	if(pmsg == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid pmsg: NULL\r\n");
		return -3;
	}
	if(msg_size > DEFAULT_MSG_SIZE)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid msg_size: too big\r\n");
		return -4;
	}

	ts.tv_sec  = to_sec;
//	ts.tv_nsec = 0;

	if(to_sec == 0)
		ret = mq_receive(m_mqd, (char *)pmsg, (size_t)msg_size, &msg_prio);
	else
		ret = mq_timedreceive(m_mqd, (char *)pmsg, (size_t)msg_size, &msg_prio, &ts);

	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "%s error=%d, %s\r\n", (to_sec == 0) ? "mq_receive" : "mq_timedreceive", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::SetAttribute(mqd_t mqd, struct mq_attr *set_attr)
{
	struct mq_attr old_attr;
	int ret;

	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mqd\r\n");
		return -2;
	}
	if(set_attr == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid set_attr: NULL\r\n");
		return -3;
	}

	ret = mq_setattr(mqd, set_attr, &old_attr);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_setattr error=%d, %s\r\n", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::SetAttribute(struct mq_attr *set_attr)
{
	struct mq_attr old_attr;
	int ret;

	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid m_mqd\r\n");
		return -2;
	}
	if(set_attr == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid set_attr: NULL\r\n");
		return -3;
	}

	ret = mq_setattr(m_mqd, set_attr, &old_attr);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_setattr error=%d, %s\r\n", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::GetAttribute(mqd_t mqd, struct mq_attr *get_attr)
{
	int ret;

	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mqd\r\n");
		return -2;
	}
	if(get_attr == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid get_attr: NULL\r\n");
		return -3;
	}

	ret = mq_getattr(mqd, get_attr);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_getattr error=%d, %s\r\n", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::GetAttribute(struct mq_attr *get_attr)
{
	int ret;

	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mq_open descriptor\r\n");
		return -2;
	}
	if(get_attr == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid get_attr: NULL\r\n");
		return -3;
	}

	ret = mq_getattr(m_mqd, get_attr);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_getattr error=%d, %s\r\n", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::SetNotifySig(mqd_t mqd, const struct sigevent *sevp)
{
	int ret;

	if(mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mqd\r\n");
		return -2;
	}
	if(sevp == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid sevp: NULL\r\n");
		return -3;
	}

	ret = mq_notify(m_mqd, sevp);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_notify error=%d, %s\r\n", errno, strerror(errno));
	}

	return ret;
}

int CPosixMessageQue::SetNotifySig(const struct sigevent *sevp)
{
	int ret;

	if(m_mqd < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid mq_open descriptor\r\n");
		return -2;
	}
	if(sevp == NULL)
	{
		DBGMSG(DBG_POSIX_MQ, "Invalid sevp: NULL\r\n");
		return -3;
	}

	ret = mq_notify(m_mqd, sevp);
	if(ret < 0)
	{
		DBGMSG(DBG_POSIX_MQ, "mq_notify error=%d, %s\r\n", errno, strerror(errno));
	}

	return ret;
}


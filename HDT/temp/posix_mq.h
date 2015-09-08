/*
	Description:
		Posix Message Function Manipulation Class

	History:
		LJK		2014-04-14		First Coding

	Reference:
		typedef unsigned int		mode_t;
		typedef int					mqd_t;

		# sysctl -a | grep mqueue
		fs.mqueue.queues_max = 256		//시스템내에서 만들 수 있는 메세지큐의 갯수
		fs.mqueue.msg_max = 10			//mq_attr상의 mq_maxmsg의 제한값
		fs.mqueue.msgsize_max = 8192	//mq_attr상의 mq_msgsize의 제한값

	Usage:

		//
		// Listener
		//
		CPosixMessageQue mq("/my_mq");
		unsigned long msg;

	#ifndef APPLY_NONBLOCK
		if(mq.Create(10, sizeof(msg))<=0) return;	//block
	#else
		if(mq.Create(10, sizeof(msg), O_CREAT|O_RDWR|O_EXCL|O_NONBLOCK)<=0 return;	//non-block
	#endif

		if(mq.Recv(&msg, sizeof(msg))	
		{
			//Do Something
		}

		mq.Close();
		mq.Unlink();

		//
		// Sender
		//
		CPosixMessageQue mq("/my_mq");
		unsigned long msg;

		if(mq.Open()<=0) return;

		msg = 0x1234;
		mq.Send(&msg, sizeof(msg));

		mq.Close();
*/

#ifndef __POSIX_MESSAGE_QUE_H__
#define __POSIX_MESSAGE_QUE_H__

#define MAX_MQ_NAME_SIZE				128
#define DEFAULT_MQ_PRIORITY				1

#define DEFAULT_MQ_MODE					0666
#define DEFAULT_MQ_COUNT				10
#define DEFAULT_MSG_SIZE				256

class CPosixMessageQue
{
public:
	CPosixMessageQue();
	CPosixMessageQue(const char *mq_name);
	~CPosixMessageQue();

	//Member Function
	int Create(const char *mq_name, int mq_maxmsg=DEFAULT_MQ_COUNT, int mq_msgsize=DEFAULT_MSG_SIZE, int oflag=O_CREAT|O_RDWR|O_EXCL, mode_t mode=DEFAULT_MQ_MODE);
	int Create(int mq_maxmsg=DEFAULT_MQ_COUNT, int mq_msgsize=DEFAULT_MSG_SIZE, int oflag=O_CREAT|O_RDWR|O_EXCL, mode_t mode=DEFAULT_MQ_MODE);

	int Open(const char *mq_name, int oflag=O_RDWR);
	int Open(int oflag=O_RDWR);

	int Close(mqd_t mqd);
	int Close();

	int Remove(const char *mq_name);
	int Remove();

	int Send(mqd_t mqd, void *pmsg, int msg_size, int to_sec=0);
	int Send(void *pmsg, int msg_size, int to_sec=0);

	int Recv(mqd_t mqd, void *pmsg, int msg_size, int to_sec=0);
	int Recv(void *pmsg, int msg_size, int to_sec=0);

	int SetAttribute(mqd_t mqd, struct mq_attr *set_attr);
	int SetAttribute(struct mq_attr *set_attr);

	int GetAttribute(mqd_t mqd, struct mq_attr *get_attr);
	int GetAttribute(struct mq_attr *get_attr);

	int SetNotifySig(mqd_t mqd, const struct sigevent *sevp);
	int SetNotifySig(const struct sigevent *sevp);

	//Member Variable
	char				m_mq_name[MAX_MQ_NAME_SIZE+1];
	mqd_t				m_mqd;
	struct mq_attr		m_mq_attr;
};

#endif //__POSIX_MESSAGE_QUE_H__

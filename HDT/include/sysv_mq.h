/*
	Description:
		SystemV Message Function Manipulation Class

	History:
		LJK		2014-04-15		First Coding

	Note:
		Send, Recv에서 msg_size는 struct msgbuf 변수의 크기가 아니고
		struct msgbuf의 버퍼크기임. 
		Send의 경우는 보내는 크기고 Recv의 경우는 최대크기.
*/

#ifndef __SYSV_MESSAGE_QUE_H__
#define __SYSV_MESSAGE_QUE_H__

#define MAX_MSG_SIZE					512

typedef struct _MSG_BUF
{
	long			msg_type;
	unsigned char	msg_buffer[MAX_MSG_SIZE];
} __attribute__ ((packed)) MSG_BUF;

class CSysvMessageQue
{
public:
	CSysvMessageQue();
	~CSysvMessageQue();

	//Member Function
	int Create(key_t key, int msg_flag=IPC_CREAT|0666);

	int Send(int msg_id, struct msgbuf *pmsg, int msg_size, int msg_flag=0);
	int Send(struct msgbuf *pmsg, int msg_size, int msg_flag=0);
	int Send(long msg_type, void *pmsg, int msg_size, int msg_flag=0);
	
	int Recv(int msg_id, struct msgbuf *pmsg, int msg_size, long msg_type, int msg_flag=0);
	int Recv(struct msgbuf *pmsg, int msg_size, long msg_type, int msg_flag=0);
	int Recv(long msg_type, void *pmsg, int msg_size, int msg_flag=0);

	long RecvAll(int msg_flag=0);

	int Control(int msg_id, int cmd, struct msqid_ds *pmsg_ds);
	int Control(int cmd, struct msqid_ds *pmsg_ds);

	int GetMsgCount(int msg_id);
	int GetMsgCount();

	int Remove(int msg_id);
	int Remove();

	//Member Variable
	int				m_msg_id;
	MSG_BUF			m_msg;
};

#endif //__SYSV_MESSAGE_QUE_H__

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MSG_KEY_MAIN					0xA000
#define MSG_DATA_SIZE					256			// msg_type을 제외한 메세지의 길이

#define MSG_MAX_COUNT					32

enum
{
	MSG_IDLE,
	MSG_STATE_CHANGE,
	MSG_TIMER_EVENT,
	MSG_KEY_EVENT,
	MSG_DOOR_EVENT,
	MSG_MICROWAVE_EVENT,
	MSG_WALLPAD_DATA,
	MSG_WALLPAD_RSP_ERR,
	MSG_EXIT,
	MSG_COUNT
};

typedef struct _MTM_MSG
{
	long	msg_type;
	ULONG	param1;
	ULONG	param2;
	UCHAR	data[MSG_DATA_SIZE];
} __attribute__ ((packed)) MTM_MSG;

class CMessage
{
public:
	CMessage();
	~CMessage();

	// Member Function
	BOOL Init();
	void DeInit();

	int SendMessage(long msg_type, ULONG param1=0, ULONG param2=0, int msg_flag=0);
	int RecvMessage(MTM_MSG *pmsg, BOOL isWait=TRUE);

	int SendMessageData(long msg_type, void* pData, ULONG ulSize, int msg_flag=0);

	int GetMessageCount();

	// Member Variable
	CSysvMessageQue		m_mq;
	int					m_msgid;
};

#endif //__MESSAGE_H__

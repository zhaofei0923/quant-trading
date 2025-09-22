#ifndef _MSG_QUEUE_SERVER_
#define _MSG_QUEUE_SERVER_
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <map>
#include <list>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thread_group.h"
#include "const.h"
#include "message.h"
#include "struct.h"
#include "share_mem_queue.h"
#define TRADE_SHARE_MEMORY_START_KEY   2000  //�����ڴ��KEY��ʼֵ
#define TRADE_MSG_QUEUE_ITEM_COUNT   1000  //��Ϣ����Ԫ�ظ���
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

typedef  std::map<string, ShmQueue<MsgQueueData>*>  AccountMsgQueue;   //�˺ţ���Ϣ����

class  CMsgQueueServer :public CThreadGroup
{
private:
	CMsgQueueServer();
public:
	~CMsgQueueServer();

	static CMsgQueueServer *GetInstance()
	{
		// C++11 style singleton
		static CMsgQueueServer CMsgQueueServerImpl;
		return &CMsgQueueServerImpl;
	}

public:
	int Init();
	int Start();
	int Stop();
	int Release();
	virtual int DealMsg(void*pMsg, int iThreadId);
public:
	int CreateMsgQueue(string&szUserId, int&iReadKey, int&iWriteKey);
	int CreateReadQueue(string&szUserId, int&iReadKey);
	int CreateWriteQueue(string&szUserId, int& iWriteKey);
	int DelMsgQueue(string&szUserId);
	int DelReadQueue(string&szUserId);
	int DelWriteQueue(string&szUserId);
	int ReadQueue();
	int WriteQueue(ServerMsg*pServerMsg);
public:
	int GetKeyValue();
	int GetMsgIndex();
	int AddClientHead(Head&HeadData);
	int DelClientHead(unsigned int iIndex);
	bool GetClientHead(unsigned int iIndex, Head&HeadData);



public:
	AccountMsgQueue m_ReadMap; //�����ݶ���
	AccountMsgQueue m_WriteMap; //д���ݶ���
	map<int, Head> m_IndexHeadMap;  //������ţ�Head

	std::thread*m_pThread;
	unsigned int m_iKeyIndex;
	unsigned int m_iMsgIndex;

	mutex m_ReadMtx; //��m_ReadMap
	mutex m_WriteMtx; //��m_WriteMap
	mutex m_KeyMtx; //��m_iKeyIndex
	mutex m_MsgIndexMtx; //��m_iMsgIndex
	mutex m_IndexHeadMtx;//��m_IndexHeadMap

};


#endif


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
#define MARKET_SHARE_MEMORY_START_KEY   8000  //�����ڴ��KEY��ʼֵ
#define MARKET_MSG_QUEUE_ITEM_COUNT   (1024*100)  //��Ϣ����Ԫ�ظ���
#define MARKET_MSG_QUEUE_SEND_BUF_LENGTH (1024*1024) //���ͻ���������
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

typedef  std::map<int, ShmQueue<MsgQueueData>*>  ClientMsgQueue;   //�ͻ��˱�ţ���Ϣ����
typedef  std::map<string, list<int>>  StockSubInfo;   //��Ʊ���룬list<�ͻ��˱��>
typedef  std::map<int, vector<string>> ClientSubInfo;  //�ͻ��˱�ţ���Ʊ����

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
	int CreateWriteQueue(vector<string>&vStockCodes, int& iWriteKey, int&iClientNum);
	int DelWriteQueue(int iClientNum);
	int WriteQueue(ServerMsg*pServerMsg);
private:
	int GetKeyValue();

public:
	ClientMsgQueue m_WriteMap; //д���ݶ���
	StockSubInfo   m_StockSubInfo; //ÿ����Ʊ�Ķ�����Ϣ
	ClientSubInfo  m_ClientSubInfo; //ÿ�������߶��ĵĹ�Ʊ
	unsigned int m_iKeyIndex;
	unsigned  long long m_iCount;
	char *m_pSendBuf;

	mutex m_WriteMtx; //��m_WriteMap,m_StockSubInfo,m_ClientSubInfo
	mutex m_KeyMtx; //��m_iKeyIndex


};


#endif


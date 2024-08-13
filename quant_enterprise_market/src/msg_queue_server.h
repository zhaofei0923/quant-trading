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
#define MARKET_SHARE_MEMORY_START_KEY   8000  //共享内存的KEY起始值
#define MARKET_MSG_QUEUE_ITEM_COUNT   (1024*100)  //消息队列元素个数
#define MARKET_MSG_QUEUE_SEND_BUF_LENGTH (1024*1024) //发送缓冲区长度
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

typedef  std::map<int, ShmQueue<MsgQueueData>*>  ClientMsgQueue;   //客户端编号，消息队列
typedef  std::map<string, list<int>>  StockSubInfo;   //股票代码，list<客户端编号>
typedef  std::map<int, vector<string>> ClientSubInfo;  //客户端编号，股票代码

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
	ClientMsgQueue m_WriteMap; //写数据队列
	StockSubInfo   m_StockSubInfo; //每个股票的订阅信息
	ClientSubInfo  m_ClientSubInfo; //每个订阅者订阅的股票
	unsigned int m_iKeyIndex;
	unsigned  long long m_iCount;
	char *m_pSendBuf;

	mutex m_WriteMtx; //锁m_WriteMap,m_StockSubInfo,m_ClientSubInfo
	mutex m_KeyMtx; //锁m_iKeyIndex


};


#endif


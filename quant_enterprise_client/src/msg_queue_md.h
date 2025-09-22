#ifndef _MSG_QUEUE_MD_
#define _MSG_QUEUE_MD_

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
#include "callback.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class  CMsgQueueMd
{
private:
	CMsgQueueMd();
public:

	~CMsgQueueMd();

	static CMsgQueueMd *GetInstance()
	{
		// C++11 style singleton
		static CMsgQueueMd CMsgQueueMdImpl;
		return &CMsgQueueMdImpl;
	}

public:
	int Init(int iReadKey);
	int Start();
	int Stop();
	int Release();
	int CreateReadQueue(int iReadKey);
	int ReadQueue();
	void SetCallBack(CallBack*pCallBack);
public:
	std::thread*m_pThread;
	bool m_bIsInit;
	bool m_bIsRun;
	CallBack*m_pCallBack;
	ShmQueue<MsgQueueData>*m_pReadShmQueue;
};


#endif


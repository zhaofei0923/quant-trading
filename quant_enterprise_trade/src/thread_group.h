#ifndef _THREAD_GROUP_H_
#define _THREAD_GROUP_H_
#include <fstream>
#include <iostream>
#include <string>
#include <mutex>
#include <vector>
#include "thread_queue.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class  CThreadGroup
{
public:
	CThreadGroup()
	{
		m_iThreadCount = 1;
		m_iSelect = -1;
		m_bRunFlag = false;
	}

	virtual ~CThreadGroup()
	{
	}

public:
	int SetThreadCount(int iThreadCount = 1);
	int RunThreads();
	int StopThreads();
	virtual int PostMsg(void*pMsg);
	virtual int DealMsg(void*pMsg, int iThreadId);
public:
	bool m_bRunFlag;
	int m_iSelect;
	int m_iThreadCount;
	std::mutex m_Mutex;
	std::vector<CThreadQueue*> m_vThread;
};




#endif



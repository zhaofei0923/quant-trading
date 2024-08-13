#ifndef _SAVE_DATA_
#define _SAVE_DATA_
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
#define CONVERT_BUF_LENGTH  1024
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class  CSaveData :public CThreadGroup
{
private:
	CSaveData();
public:

	~CSaveData();

	static CSaveData *GetInstance()
	{
		// C++11 style singleton
		static CSaveData CSaveDataImpl;
		return &CSaveDataImpl;
	}

public:
	int Init();
	int Start();
	int Stop();
	int Release();
	virtual int DealMsg(void*pMsg, int iThreadId);
public:
	int ParseMsg(ServerMsg*pMsg, int iThreadId);
	int WriteData(MarketData&Quote);

public:
	fstream m_QuoteStream;
	char *m_pConvertBuf;

};


#endif


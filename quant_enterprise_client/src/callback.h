#ifndef CALL_BACK_H_
#define CALL_BACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <thread>
#include "message.h"
#include "struct.h"
#include "const.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class CallBack
{
public:
	CallBack();
	~CallBack();
public:
	void OnReceive(Head*pHead, char*pBody, int iBodyLength);

public:
	//交易的共享内存使用
	int m_iTradeReadKey;
	int m_iTradeWriteKey;

	//行情共享内存使用
	int m_iMarketKey;
	//行情客户端编号
	int m_iMarketClientNum;
};


#endif

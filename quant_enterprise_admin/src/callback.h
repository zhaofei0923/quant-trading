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
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

class CallBack
{
public:
	CallBack();
	~CallBack();
public:
	void OnReceive(Head*pHead, char*pBody, int iBodyLength);

public:
	//���׵Ĺ����ڴ�ʹ��
	int m_iTradeReadKey;
	int m_iTradeWriteKey;

	//���鹲���ڴ�ʹ��
	int m_iMarketKey;
	//����ͻ��˱��
	int m_iMarketClientNum;
};


#endif

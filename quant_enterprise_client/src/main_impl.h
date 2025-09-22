#ifndef MAIN_IMPL_H_
#define MAIN_IMPL_H_
#include "tcp_client.h"
#include "msg_queue_td.h"
#include "msg_queue_md.h"
#include <vector>
using namespace std;

/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class MainImpl
{
public:
	MainImpl()
	{
		m_bIsInit = false;
		m_pTrade = NULL;
		m_pMarket = NULL;
	}

	~MainImpl()
	{

	}
public:
	int Init();
	int Start();
	int Stop();
	int Release();
	//管理员demo
	void AdminDemo();
	//交易员查询
	void TraderQueryDemo();
	//交易员查询
	void TraderQueryDemo2();
	//交易员委托
	void TraderEntrustDemo();
	//行情demo
	void MarketDemo();

public:
	bool m_bIsInit;
	CTcpClient *m_pTrade; //交易
	CMsgQueueTd*m_pCMsgQueueTd;//交易
	CTcpClient *m_pMarket; //行情
	CMsgQueueMd*m_pCMsgQueueMd;//行情

};




#endif

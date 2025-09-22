#ifndef MAIN_IMPL_H_
#define MAIN_IMPL_H_
#include "tcp_client.h"
#include "msg_queue_td.h"
#include "msg_queue_md.h"
#include <vector>
using namespace std;

/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//����Աdemo
	void AdminDemo();
	//����Ա��ѯ
	void TraderQueryDemo();
	//����Ա��ѯ
	void TraderQueryDemo2();
	//����Աί��
	void TraderEntrustDemo();
	//����demo
	void MarketDemo();

public:
	bool m_bIsInit;
	CTcpClient *m_pTrade; //����
	CMsgQueueTd*m_pCMsgQueueTd;//����
	CTcpClient *m_pMarket; //����
	CMsgQueueMd*m_pCMsgQueueMd;//����

};




#endif

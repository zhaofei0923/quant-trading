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
		Stop();
		Release();
	}
public:
	int Init();
	int Start();
	int Stop();
	int Release();
	//����Ա��¼
	bool TraderLogin();
	//����Ա�˳�
	void TraderLogout();
	//��������
	bool SubMarket(const vector<string>&vStockCodes);

public:
	bool m_bIsInit;
	CTcpClient *m_pTrade; //����
	CMsgQueueTd*m_pCMsgQueueTd;//����
	CTcpClient *m_pMarket; //����
	CMsgQueueMd*m_pCMsgQueueMd;//����

};




#endif

#ifndef MAIN_IMPL_H_
#define MAIN_IMPL_H_
#include "tcp_client.h"
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
	}

	~MainImpl()
	{

	}
public:
	int Init();
	int Start();
	int Stop();
	int Release();
	void AdminOper();

public:
	bool m_bIsInit;
	CTcpClient *m_pTrade; //交易

};




#endif

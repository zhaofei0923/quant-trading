#ifndef _TCP_SERVER_
#define _TCP_SERVER_
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <string.h>
#include <memory>
#include "thread_group.h"
#include "const.h"
#include "message.h"
#include "struct.h"
#define MAX_EPOLL_EVENTS  1000
#define MAX_MSG_LEN  1024
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

class  CTcpServer
{
private:
	CTcpServer();
public:

	~CTcpServer();

	static CTcpServer *GetInstance()
	{
		// C++11 style singleton
		static CTcpServer CTcpServerImpl;
		return &CTcpServerImpl;
	}

public:
	int Init();
	int Start();
	int Stop();
	int Release();
public:
	//���÷�����ģʽ
	int SetFdNonBlock(int iFd);
	//��������ģʽ
	int SetFdBlock(int iFd);
	//ѭ��������
	int ReadData(int iFd, void *pBuf, int iCount);
	//ѭ��д����
	int WriteData(int iFd, const void *pBuf, int iCount);
	int CreateSocket(int iPort);
	int StartServer();
public:
	//������Ϣ���ͻ���
	int SendMsg(Head*pHead, void*pBody, int iSockFd);
	//��������
	int DealReq(Head*pHead, void*pBody, int iSockFd);
	//ȡ������
	int CancelSub(int iSockFd);

public:
	std::thread*m_pThread;
	int m_iEpollFd;
	map<int, int> m_FdMap; //socket,iClientNum
};


#endif


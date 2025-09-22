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
作者：赵志根
微信：401985690
qq群号：450286917
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
	//设置非阻塞模式
	int SetFdNonBlock(int iFd);
	//设置阻塞模式
	int SetFdBlock(int iFd);
	//循环读数据
	int ReadData(int iFd, void *pBuf, int iCount);
	//循环写数据
	int WriteData(int iFd, const void *pBuf, int iCount);
	int CreateSocket(int iPort);
	int StartServer();
public:
	//发送消息到客户端
	int SendMsg(Head*pHead, void*pBody, int iSockFd);
	//处理请求
	int DealReq(Head*pHead, void*pBody, int iSockFd);
	//取消订阅
	int CancelSub(int iSockFd);

public:
	std::thread*m_pThread;
	int m_iEpollFd;
	map<int, int> m_FdMap; //socket,iClientNum
};


#endif


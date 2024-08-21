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

struct TcpLink
{
	int iConnectId;
	string szUserId;
	bool bIsShareMem; //�Ƿ��й����ڴ�ͨѶ

	TcpLink()  //Ĭ�Ϲ��캯��
	{
		iConnectId = 0;
		szUserId = "";
		bIsShareMem = false;
	}
	void set(TcpLink* s1, TcpLink* s2)//��ֵ����
	{
		s1->iConnectId = s2->iConnectId;
		s1->szUserId = s2->szUserId;
		s1->bIsShareMem = s2->bIsShareMem;
	}
	TcpLink& operator=(const TcpLink& s)//���������
	{
		set(this, (TcpLink*)&s);
		return *this;
	}
	TcpLink(const TcpLink& s)//���ƹ��캯��
	{
		*this = s;
	}
};

class  CTcpServer :public CThreadGroup
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
	virtual int DealMsg(void*pMsg, int iThreadId);
public:
	//���÷�����ģʽ
	int SetFdNonBlock(int iFd);
	//��������ģʽ
	int SetFdBlock(int iFd);
	int CreateSocket(int iPort);
	//ѭ�������ݺ���
	int ReadData(int iFd, void *pBuf, int iCount);
	//ѭ��д���ݺ���
	int WriteData(int iFd, const void *pBuf, int iCount);
	int StartServer();
	int SendMsg(ServerMsg*pMsg);
public:
	int GetMsgIndex();
	int GetConnectId();
	int GetAccount(const Head&HeadBuf, char*pBodyBuf, int iFd, TcpLink&TcpLinkData);
	//ɾ��socket
	int DelFd(int iFd);
	int AddClientHead(Head&HeadData);
	int DelClientHead(unsigned int iIndex);
	bool GetClientHead(unsigned int iIndex, Head&HeadData);

public:
	map<int, int> m_ConnectMap; //ConnectId,iSockFd
	map<int, TcpLink> m_FdMap;  //iSockFd,TcpLink
	map<int, Head> m_IndexHeadMap;  //������ţ�Head

	std::thread*m_pThread;
	int m_iEpollFd;
	unsigned int m_iConnectId;
	unsigned int m_iMsgIndex;

	mutex m_FdMtx; //��m_FdMap,m_ConnectMap
	mutex m_ConnectIdMtx; //��m_iConnectId
	mutex m_MsgIndexMtx; //��m_iMsgIndex
	mutex m_IndexHeadMtx;//��m_IndexHeadMap
};


#endif


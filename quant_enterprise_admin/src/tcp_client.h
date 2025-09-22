#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <vector>
#include <map>
#include <thread>
#include "message.h"
#include "struct.h"
#include "const.h"
#include "callback.h"
using namespace std;
#define READ_BUF_LENGTH  (1024*1024)
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

typedef std::vector<struct epoll_event>    EventList;

class CTcpClient
{
public:
	CTcpClient(string szIp, int iPort);
	~CTcpClient();
public:
	int Init();
	int Connect();
	int Start();
	int ReadData(int iFd, void *pBuf, int iCount);
	int WriteData(int iFd, const void *pBuf, int iCount);
	//设置非阻塞
	int SetFdNonBlock(int iFd);
	//设置阻塞
	int SetFdBlock(int iFd);
	void SetCallBack(CallBack*pCallBack);
	int EpollRecv();
	int SendMsg(Head*pHead, void*pBody, int iBodyLength);
	bool GetTradeKey(int&iReadKey, int&iWriteKey);
	bool GetMarketKey(int&iMarketKey, int&iClientNum);

public:
	string m_szServerIp;
	int  m_iPort;
	int m_iSockFd;
	bool m_bIsConnect;
	std::thread*m_pThread;
	char *m_pReadBuf;
	CallBack*m_pCallBack;

};




#endif

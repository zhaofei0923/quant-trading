#include "tcp_server.h"
#include "config.h"
#include "sim_log.h"
#include "msg_queue_server.h"
#include "mem_pool_mng.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CTcpServer::CTcpServer()
{
}

CTcpServer::~CTcpServer()
{

}

int CTcpServer::Init()
{
	return 0;
}
int CTcpServer::Start()
{
	LInfo("Start CTcpServer ");

	m_pThread = new std::thread(&CTcpServer::StartServer, this);
	m_pThread->detach();

	return 0;
}
int CTcpServer::Stop()
{
	return 0;
}
int CTcpServer::Release()
{
	return 0;
}
//循环读数据
int CTcpServer::ReadData(int iFd, void *pBuf, int iCount)
{
	int iLeft = iCount;
	int iReadCount;
	char *pBufData = (char*)pBuf;

	while (iLeft > 0)
	{
		iReadCount = read(iFd, pBufData, iLeft);
		if (iReadCount < 0)
		{

			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			{
				continue;
			}

			//如果出现读错误，即read函数返回值为-1，此时要查看errno的值，根据errno的值查找原因
			LError("tcp read error,iReadCount=[{0}],errno=[{1}]", iReadCount, errno);

			return iReadCount;
		}
		else if (iReadCount == 0)
		{
			//return iCount - iLeft;
			return iReadCount;
		}

		pBufData += iReadCount;
		iLeft -= iReadCount;
	}

	return iCount;
}
//循环写数据
int CTcpServer::WriteData(int iFd, const void *pBuf, int iCount)
{
	int iLeft = iCount;
	int iWriteCount;
	char *pBufData = (char*)pBuf;

	while (iLeft > 0)
	{
		iWriteCount = write(iFd, pBufData, iLeft);
		if (iWriteCount < 0)
		{
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			{
				continue;
			}

			//如果出现写错误，即write函数返回值为-1，此时要查看errno的值，根据errno的值查找原因
			LError("tcp write error,iWriteCount=[{0}],errno=[{1}]", iWriteCount, errno);

			return  iWriteCount;
		}
		else if (iWriteCount == 0)
		{
			continue;
		}

		pBufData += iWriteCount;
		iLeft -= iWriteCount;
	}

	return iCount;
}

//设置非阻塞模式
int CTcpServer::SetFdNonBlock(int iFd)
{
	//fcntl(iFd, F_SETFL, fcntl(iFd, F_GETFL) | O_NONBLOCK);

	int iFlags;
	if (iFlags = fcntl(iFd, F_GETFL, 0) < 0)
	{
		perror("fcntl");
		return -1;
	}

	iFlags |= O_NONBLOCK;
	if (fcntl(iFd, F_SETFL, iFlags) < 0)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}
//设置阻塞模式
int CTcpServer::SetFdBlock(int iFd)
{
	//fcntl(iFd, F_SETFL, fcntl(iFd, F_GETFL) | O_NONBLOCK);

	int iFlags;
	if (iFlags = fcntl(iFd, F_GETFL, 0) < 0)
	{
		perror("fcntl");
		return -1;
	}
	iFlags &= ~O_NONBLOCK;
	if (fcntl(iFd, F_SETFL, iFlags) < 0)
	{
		perror("fcntl");
		return -1;
	}

	return 0;
}
int CTcpServer::CreateSocket(int iPort)
{
	struct sockaddr_in ServerAddrInfo;
	memset(&ServerAddrInfo, 0x00, sizeof(ServerAddrInfo));

	/* 设置ipv4模式 */
	ServerAddrInfo.sin_family = AF_INET;           /* ipv4 */
	/* 设置端口号 */
	ServerAddrInfo.sin_port = htons(iPort);
	ServerAddrInfo.sin_addr.s_addr = 0;
	//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* 建立socket */
	int iSockFd = socket(PF_INET, SOCK_STREAM, 0);
	if (iSockFd == -1) {
		LError("create socket error");
	}
	/* 设置复用模式 */
	int iReuse = 1;
	if (setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, &iReuse, sizeof(iReuse)) == -1)
	{
		LError("setsockopt error");
	}
	/* 绑定端口 */
	if (bind(iSockFd, (sockaddr *)&ServerAddrInfo, sizeof(ServerAddrInfo)) == -1)
	{
		LError("bind port error");
	}
	/* 设置被动打开 */
	if (listen(iSockFd, 5) == -1)
	{
		LError("start listen  error");
	}
	return iSockFd;
}

int CTcpServer::StartServer()
{
	int iPort = ConfigData::GetInstance()->TcpCfgData.iPort;
	int iSockFd = CreateSocket(iPort);
	SetFdNonBlock(iSockFd);

	/* 创建epoll */
	int m_iEpollFd = epoll_create(1);
	if (m_iEpollFd == -1)
	{
		LError("epoll_create  error");
	}

	/* 添加sockfd到m_iEpollFd列表 */
	struct epoll_event EvInfo;
	EvInfo.data.fd = iSockFd;
	EvInfo.events = EPOLLIN;
	if (epoll_ctl(m_iEpollFd, EPOLL_CTL_ADD, iSockFd, &EvInfo) == -1)
	{
		LError("epoll_ctl  error");
	}

	printf("start tcp server,port=%d\n", iPort);
	LInfo("start tcp server,port=[{0}]", iPort);

	/* 创建一个列表用于存放wait所返回的events */
	struct epoll_event events[MAX_EPOLL_EVENTS] = { 0 };
	/* 开始等待所有在epoll上挂上去的事件 */
	while (1) {
		/* 等待事件 */
		int number = epoll_wait(m_iEpollFd, events, MAX_EPOLL_EVENTS, -1);
		if (number > 0) {
			/* 遍历所有事件 */
			for (int i = 0; i < number; i++)
			{
				int iEventFd = events[i].data.fd;
				/* 如果触发事件的fd是sockfd，则说明有人连接上来了，我们需要accept他 */
				if (iEventFd == iSockFd) {
					printf("accept new client...\n");
					struct sockaddr_in ClientAddr;
					socklen_t ClientAddrLen = sizeof(ClientAddr);
					int iConnFd = accept(iSockFd, (struct sockaddr *)&ClientAddr, &ClientAddrLen);
					SetFdNonBlock(iConnFd);

					/* accept之后，需要将文件描述符加入到监听列表中 */
					struct epoll_event EvData;
					EvData.data.fd = iConnFd;
					EvData.events = EPOLLIN;

					if (epoll_ctl(m_iEpollFd, EPOLL_CTL_ADD, iConnFd, &EvData) == -1)
					{
						LError("EPOLL_CTL_ADD  error,iConnFd=[{0}]", iConnFd);
					}

					std::cout << "accept new client end,iConnFd=" << iConnFd << endl;
					LInfo("accept new client end,iConnFd=[{0}]", iConnFd);

				}
				else
				{
					/* 如果触发的fd不是sockfd，那就是新加的iConnFd */

					Head HeadBuf;
					memset(&HeadBuf, 0, sizeof(HeadBuf));
					int iRet = 0;
					iRet = ReadData(iEventFd, &HeadBuf, sizeof(HeadBuf));
					if (0 == iRet)
					{
						std::cout << "client close,iConnFd=" << iEventFd << endl;
						LInfo("client close,iConnFd=[{0}]", iEventFd);

						close(iEventFd);
						epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, iEventFd, NULL);
						CancelSub(iEventFd);

						continue;
					}
					else if (iRet < 0)
					{
						LInfo("Read HeadBuf,iRet=[{0}],iConnFd=[{1}]", iRet, iEventFd);
						continue;
					}

					int iBodyLength = HeadBuf.iBodyLength;
					char szBodyBuf[1024] = { 0 };
					if (iBodyLength > 0)
					{
						iRet = ReadData(iEventFd, szBodyBuf, iBodyLength);
						if (0 == iRet)
						{
							std::cout << "client close,iConnFd=" << iEventFd << endl;
							LInfo("client close,iConnFd=[{0}]", iEventFd);
							close(iEventFd);
							epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, iEventFd, NULL);
							CancelSub(iEventFd);

							continue;
						}
						else if (iRet < 0)
						{
							LInfo("Read BodyBuf,iRet=[{0}],iConnFd=[{1}]", iRet, iEventFd);
							continue;
						}
					}
					DealReq(&HeadBuf, szBodyBuf, iEventFd);
				}
			}
		}
	}

	return 0;
}

//发送消息到客户端
int CTcpServer::SendMsg(Head*pHead, void*pBody, int iSockFd)
{
	int iRet = 0;
	int iHeadLength = sizeof(Head);

	//发送head		
	iRet = WriteData(iSockFd, pHead, iHeadLength);
	if (iRet != iHeadLength)
	{
		LError("send head fail，iRet=[{0}],iSockFd=[{1}]", iRet, iSockFd);
		return -1;
	}

	if ((pHead->iBodyLength > 0) && (pBody != NULL))
	{
		//发送body
		iRet = WriteData(iSockFd, pBody, pHead->iBodyLength);
		if (iRet != pHead->iBodyLength)
		{
			LError("send body fail，iRet=[{0}],iSockFd=[{1}]", iRet, iSockFd);
		}
	}

	return 0;
}
//处理请求
int CTcpServer::DealReq(Head*pHead, void*pBody, int iSockFd)
{
	CMsgQueueServer*pCMsgQueueServer = CMsgQueueServer::GetInstance();
	if (FUNC_SUB_QUOTE == pHead->iFunctionId)
	{
		LInfo("收到订阅请求,订阅的股票代码:");

		int iNum = *((int*)pBody);
		char *p = (char *)pBody;
		char *pData = &p[sizeof(iNum)];

		vector <string> vStockCodes;
		for (int i = 0; i < iNum; i++)
		{
			SubItem Item;
			memcpy(&Item, &pData[i * sizeof(Item)], sizeof(Item));
			string szStockCode = Item.szStockCode;
			vStockCodes.push_back(szStockCode);
			LInfo("szStockCode=[{0}]", szStockCode);
		}


		SubQuoteResT  SubQuoteResData;
		int iWriteKey = 0;
		int iClientNum = 0;
		if (0 == pCMsgQueueServer->CreateWriteQueue(vStockCodes, iWriteKey, iClientNum))
		{
			LInfo("CreateWriteQueue OK,iWriteKey=[{0}],iClientNum=[{1}]", iWriteKey, iClientNum);

			pHead->iErrorCode = RET_OK;
			snprintf(pHead->szErrorInfo, sizeof(pHead->szErrorInfo), "OK");
			pHead->iBodyLength = sizeof(SubQuoteResData);
			m_FdMap[iSockFd] = iClientNum;

		}
		else
		{
			LError("CreateWriteQueue Fail,iWriteKey=[{0}],iClientNum=[{1}]", iWriteKey, iClientNum);

			pHead->iErrorCode = RET_CREATE_MSG_QUEUE_FAIL;
			snprintf(pHead->szErrorInfo, sizeof(pHead->szErrorInfo), "创建消息队列失败");
			pHead->iBodyLength = 0;
		}

		SubQuoteResData.iClientNum = iClientNum;
		SubQuoteResData.iDataKey = iWriteKey;

		pHead->bIsLast = true;
		pHead->iMsgtype = MSG_TYPE_RES;

		SendMsg(pHead, &SubQuoteResData, iSockFd);
	}
	else if (FUNC_CANCEL_SUB_QUOTE == pHead->iFunctionId)
	{
		CancelSubQuoteReqT*pCancelSubQuoteReq = (CancelSubQuoteReqT*)pBody;
		pCMsgQueueServer->DelWriteQueue(pCancelSubQuoteReq->iClientNum);

		LInfo("DelWriteQueue  OK,iClientNum=[{0}]", pCancelSubQuoteReq->iClientNum);

		pHead->iErrorCode = RET_OK;
		snprintf(pHead->szErrorInfo, sizeof(pHead->szErrorInfo), "OK");
		pHead->iBodyLength = 0;
		pHead->bIsLast = true;
		pHead->iMsgtype = MSG_TYPE_RES;

		SendMsg(pHead, NULL, iSockFd);
	}

	return 0;
}
//客户端断开连接时，根据socket取消订阅
int CTcpServer::CancelSub(int iSockFd)
{
	if (m_FdMap.count(iSockFd) > 0)
	{
		int iClientNum = m_FdMap[iSockFd];
		CMsgQueueServer*pCMsgQueueServer = CMsgQueueServer::GetInstance();
		pCMsgQueueServer->DelWriteQueue(iClientNum);
		m_FdMap.erase(iSockFd);

		LInfo("DelWriteQueue OK,iClientNum=[{0}]", iClientNum);
	}

	return 0;
}
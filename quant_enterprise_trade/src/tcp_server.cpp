#include "tcp_server.h"
#include "config.h"
#include "sim_log.h"
#include "execute_engine.h"
#include "mem_pool_mng.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CTcpServer::CTcpServer()
{
	m_iMsgIndex = 0;
	m_iConnectId = 0;
}

CTcpServer::~CTcpServer()
{

}

int CTcpServer::Init()
{
	SetThreadCount(1);
	return 0;
}
int CTcpServer::Start()
{
	LInfo("Start CTcpServer ");

	m_pThread = new std::thread(&CTcpServer::StartServer, this);
	m_pThread->detach();

	RunThreads();

	return 0;
}
int CTcpServer::Stop()
{
	StopThreads();
	return 0;
}
int CTcpServer::Release()
{
	return 0;
}
int  CTcpServer::DealMsg(void*pMsg, int iThreadId)
{
	if (pMsg != NULL)
	{
		ServerMsg*pServerMsg = (ServerMsg*)pMsg;
		SendMsg(pServerMsg);

		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
		char*pBody = pServerMsg->pBody;
		if (pBody != NULL)
		{
			pMemPoolMng->DeleteMaxMemBlock(pBody);
		}

		pMemPoolMng->DeleteMaxMemBlock((char*)pServerMsg);
	}

	return 0;

}


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

//设置非阻塞
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
//设置阻塞
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

	std::cout << "Start tcp  server! port=" << iPort << endl;
	LInfo("Start tcp  server! port=[{0}]", iPort);

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
						DelFd(iEventFd);
						epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, iEventFd, NULL);

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
							DelFd(iEventFd);
							epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, iEventFd, NULL);

							continue;
						}
						else if (iRet < 0)
						{
							LInfo("Read BodyBuf,iRet=[{0}],iConnFd=[{1}]", iRet, iEventFd);
							continue;
						}
					}

					TcpLink TcpLinkData;
					if (GetAccount(HeadBuf, szBodyBuf, iEventFd, TcpLinkData) != 0)
					{
						continue;
					}
					LDebug("iConnectId=[{0}],szUserId=[{1}]", TcpLinkData.iConnectId, TcpLinkData.szUserId);

					unsigned int iMsgIndex = AddClientHead(HeadBuf);

					MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
					char*pBody = pMemPoolMng->GetMaxMemBlock(iBodyLength);
					if (NULL == pBody)
					{
						LError("pBody is NULL!  iConnectId=[{0}],szUserId=[{1}]", TcpLinkData.iConnectId, TcpLinkData.szUserId);
						continue;
					}
					memset(pBody, 0, iBodyLength);
					memcpy(pBody, szBodyBuf, iBodyLength);

					char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
					if (NULL == pMemBlock)
					{
						LError("pMemBlock is NULL!  iConnectId=[{0}],szUserId=[{1}]", TcpLinkData.iConnectId, TcpLinkData.szUserId);
						continue;
					}

					ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
					pServerMsg->iConnectId = TcpLinkData.iConnectId;
					snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", TcpLinkData.szUserId.c_str());
					pServerMsg->iFunctionId = HeadBuf.iFunctionId;
					pServerMsg->iRoletype = HeadBuf.iRoletype;
					pServerMsg->iMsgIndex = iMsgIndex;
					pServerMsg->iBodyLen = iBodyLength;
					pServerMsg->pBody = pBody;
					pServerMsg->bIsLast = true;
					pServerMsg->iErrorCode = RET_OK;
					snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "OK");

					ExecuteEngine::GetInstance()->PostMsg(pServerMsg);

				}
			}
		}
	}


	return 0;
}
//发送消息
int CTcpServer::SendMsg(ServerMsg*pServerMsg)
{
	lock_guard<mutex> lk(m_FdMtx);

	int iConnectId = pServerMsg->iConnectId;
	int iSockFd = 0;
	if (m_ConnectMap.count(iConnectId) > 0)
	{
		iSockFd = m_ConnectMap[iConnectId];
	}
	else
	{
		//客户端关闭连接，iConnectId被删除
		LInfo("客户端关闭连接，iConnectId被删除，iConnectId=[{0}]", iConnectId);
		return 0;
	}

	string szUserId = pServerMsg->szUserId;
	int iFunctionId = pServerMsg->iFunctionId;

	//如果功能号是登录
	if (FUNC_LOGIN == iFunctionId)
	{
		if (pServerMsg->iErrorCode != RET_OK)
		{
			//如果登录失败
			m_ConnectMap.erase(iConnectId);
			m_FdMap.erase(iSockFd);
			LInfo("login fail，delete socket，iConnectId=[{0}],iSockFd=[{1}]", iConnectId, iSockFd);

		}

	}
	else if (FUNC_LOGOUT == iFunctionId)
	{
		m_ConnectMap.erase(iConnectId);
		m_FdMap.erase(iSockFd);
	}
	else if (FUNC_ADD_MSG_QUEUE == iFunctionId)
	{
		if (RET_OK == pServerMsg->iErrorCode)
		{
			if (m_FdMap.count(iSockFd))
			{
				m_FdMap[iSockFd].bIsShareMem = true;
			}
		}
	}
	else if (FUNC_DEL_MSG_QUEUE == iFunctionId)
	{
		if (RET_OK == pServerMsg->iErrorCode)
		{
			if (m_FdMap.count(iSockFd))
			{
				m_FdMap[iSockFd].bIsShareMem = false;
			}
		}
	}

	Head HeadData;
	if (false == GetClientHead(pServerMsg->iMsgIndex, HeadData))
	{
		LInfo("iMsgIndex is not exist，iConnectId=[{0}],iMsgIndex=[{1}]", iConnectId, pServerMsg->iMsgIndex);
		return -1;
	}

	//如果是查询，则会返回多笔数据，返回最后一条数据时删除head
	if (pServerMsg->bIsLast)
	{
		DelClientHead(pServerMsg->iMsgIndex);
	}


	//修改Head
	HeadData.iMsgtype = MSG_TYPE_RES;
	HeadData.iBodyLength = pServerMsg->iBodyLen;
	HeadData.bIsLast = pServerMsg->bIsLast;
	HeadData.iErrorCode = pServerMsg->iErrorCode;
	snprintf(HeadData.szErrorInfo, sizeof(HeadData.szErrorInfo), "%s", pServerMsg->szErrorInfo);

	int iRet = 0;
	int iHeadLength = sizeof(HeadData);

	//发送head		
	iRet = WriteData(iSockFd, &HeadData, iHeadLength);
	if (iRet != iHeadLength)
	{
		LError("send head fail，iConnectId=[{0}],iSockFd=[{1}]", iConnectId, iSockFd);
		return -1;
	}

	if ((pServerMsg->iBodyLen > 0) && (pServerMsg->pBody != NULL))
	{
		//发送body
		iRet = WriteData(iSockFd, pServerMsg->pBody, pServerMsg->iBodyLen);
		if (iRet != pServerMsg->iBodyLen)
		{
			LError("send body fail，iConnectId=[{0}],iSockFd=[{1}]", iConnectId, iSockFd);
		}
	}

	return 0;
}

int CTcpServer::DelFd(int iFd)
{
	lock_guard<mutex> lk(m_FdMtx);

	if (m_FdMap.count(iFd) > 0)
	{
		int iConnectId = m_FdMap[iFd].iConnectId;
		string szUserId = m_FdMap[iFd].szUserId;

		//没有删除共享内存
		if (m_FdMap[iFd].bIsShareMem)
		{
			MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
			char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
			if (NULL == pMemBlock)
			{
				LError("pMemBlock is NULL!");
				return -1;
			}

			ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
			pServerMsg->iConnectId = iConnectId;
			snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", szUserId.c_str());
			pServerMsg->iFunctionId = FUNC_DEL_MSG_QUEUE;
			pServerMsg->iRoletype = ROLE_TYPE_TRADER;
			pServerMsg->iMsgIndex = 0;
			pServerMsg->iBodyLen = 0;
			pServerMsg->pBody = NULL;
			pServerMsg->bIsLast = true;
			pServerMsg->iErrorCode = RET_OK;
			snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "OK");

			LInfo("断开连接，删除共享内存队列,iConnectId=[{0}],szUserId=[{1}]", iConnectId, szUserId);

			ExecuteEngine::GetInstance()->PostMsg(pServerMsg);
		}

		if (m_ConnectMap.count(iConnectId) > 0)
		{
			m_ConnectMap.erase(iConnectId);
		}

		m_FdMap.erase(iFd);
	}

	return 0;

}

int CTcpServer::GetAccount(const Head&HeadBuf, char*pBodyBuf, int iFd, TcpLink&TcpLinkData)
{
	lock_guard<mutex> lk(m_FdMtx);

	if (0 == m_FdMap.count(iFd))
	{
		if (HeadBuf.iFunctionId != FUNC_LOGIN)
		{
			LError("首次连接不是登录报文,丢弃，iFd=[{0}]", iFd);
			return -1;
		}
		else
		{
			//登录报文
			LoginReqT*pLoginReqT = (LoginReqT*)pBodyBuf;
			TcpLinkData.szUserId = pLoginReqT->szUserId;
			TcpLinkData.iConnectId = GetConnectId();

			//添加iFd到m_FdMap
			m_FdMap[iFd] = TcpLinkData;
			m_ConnectMap[TcpLinkData.iConnectId] = iFd;
		}
	}
	else
	{
		TcpLinkData = m_FdMap[iFd];
	}

	return 0;
}

int CTcpServer::GetConnectId()
{
	lock_guard<mutex> lk(m_ConnectIdMtx);
	m_iConnectId++;
	return m_iConnectId;
}
int CTcpServer::GetMsgIndex()
{
	lock_guard<mutex> lk(m_MsgIndexMtx);
	m_iMsgIndex++;
	return m_iMsgIndex;
}

int CTcpServer::AddClientHead(Head&HeadData)
{
	lock_guard<mutex> lk(m_IndexHeadMtx);

	unsigned int iIndex = GetMsgIndex();
	m_IndexHeadMap[iIndex] = HeadData;

	return iIndex;
}
int CTcpServer::DelClientHead(unsigned int iIndex)
{
	lock_guard<mutex> lk(m_IndexHeadMtx);
	m_IndexHeadMap.erase(iIndex);
	return 0;
}
bool CTcpServer::GetClientHead(unsigned int iIndex, Head&HeadData)
{
	lock_guard<mutex> lk(m_IndexHeadMtx);
	if (m_IndexHeadMap.count(iIndex) > 0)
	{
		HeadData = m_IndexHeadMap[iIndex];
		return  true;
	}
	else
	{
		return false;
	}

	return true;
}


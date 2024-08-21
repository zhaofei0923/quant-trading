#include "tcp_server.h"
#include "config.h"
#include "sim_log.h"
#include "msg_queue_server.h"
#include "mem_pool_mng.h"
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
//ѭ��������
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

			//������ֶ����󣬼�read��������ֵΪ-1����ʱҪ�鿴errno��ֵ������errno��ֵ����ԭ��
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
//ѭ��д����
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

			//�������д���󣬼�write��������ֵΪ-1����ʱҪ�鿴errno��ֵ������errno��ֵ����ԭ��
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

//���÷�����ģʽ
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
//��������ģʽ
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

	/* ����ipv4ģʽ */
	ServerAddrInfo.sin_family = AF_INET;           /* ipv4 */
	/* ���ö˿ں� */
	ServerAddrInfo.sin_port = htons(iPort);
	ServerAddrInfo.sin_addr.s_addr = 0;
	//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* ����socket */
	int iSockFd = socket(PF_INET, SOCK_STREAM, 0);
	if (iSockFd == -1) {
		LError("create socket error");
	}
	/* ���ø���ģʽ */
	int iReuse = 1;
	if (setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, &iReuse, sizeof(iReuse)) == -1)
	{
		LError("setsockopt error");
	}
	/* �󶨶˿� */
	if (bind(iSockFd, (sockaddr *)&ServerAddrInfo, sizeof(ServerAddrInfo)) == -1)
	{
		LError("bind port error");
	}
	/* ���ñ����� */
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

	/* ����epoll */
	int m_iEpollFd = epoll_create(1);
	if (m_iEpollFd == -1)
	{
		LError("epoll_create  error");
	}

	/* ���sockfd��m_iEpollFd�б� */
	struct epoll_event EvInfo;
	EvInfo.data.fd = iSockFd;
	EvInfo.events = EPOLLIN;
	if (epoll_ctl(m_iEpollFd, EPOLL_CTL_ADD, iSockFd, &EvInfo) == -1)
	{
		LError("epoll_ctl  error");
	}

	printf("start tcp server,port=%d\n", iPort);
	LInfo("start tcp server,port=[{0}]", iPort);

	/* ����һ���б����ڴ��wait�����ص�events */
	struct epoll_event events[MAX_EPOLL_EVENTS] = { 0 };
	/* ��ʼ�ȴ�������epoll�Ϲ���ȥ���¼� */
	while (1) {
		/* �ȴ��¼� */
		int number = epoll_wait(m_iEpollFd, events, MAX_EPOLL_EVENTS, -1);
		if (number > 0) {
			/* ���������¼� */
			for (int i = 0; i < number; i++)
			{
				int iEventFd = events[i].data.fd;
				/* ��������¼���fd��sockfd����˵���������������ˣ�������Ҫaccept�� */
				if (iEventFd == iSockFd) {
					printf("accept new client...\n");
					struct sockaddr_in ClientAddr;
					socklen_t ClientAddrLen = sizeof(ClientAddr);
					int iConnFd = accept(iSockFd, (struct sockaddr *)&ClientAddr, &ClientAddrLen);
					SetFdNonBlock(iConnFd);

					/* accept֮����Ҫ���ļ����������뵽�����б��� */
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
					/* ���������fd����sockfd���Ǿ����¼ӵ�iConnFd */

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

//������Ϣ���ͻ���
int CTcpServer::SendMsg(Head*pHead, void*pBody, int iSockFd)
{
	int iRet = 0;
	int iHeadLength = sizeof(Head);

	//����head		
	iRet = WriteData(iSockFd, pHead, iHeadLength);
	if (iRet != iHeadLength)
	{
		LError("send head fail��iRet=[{0}],iSockFd=[{1}]", iRet, iSockFd);
		return -1;
	}

	if ((pHead->iBodyLength > 0) && (pBody != NULL))
	{
		//����body
		iRet = WriteData(iSockFd, pBody, pHead->iBodyLength);
		if (iRet != pHead->iBodyLength)
		{
			LError("send body fail��iRet=[{0}],iSockFd=[{1}]", iRet, iSockFd);
		}
	}

	return 0;
}
//��������
int CTcpServer::DealReq(Head*pHead, void*pBody, int iSockFd)
{
	CMsgQueueServer*pCMsgQueueServer = CMsgQueueServer::GetInstance();
	if (FUNC_SUB_QUOTE == pHead->iFunctionId)
	{
		LInfo("�յ���������,���ĵĹ�Ʊ����:");

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
			snprintf(pHead->szErrorInfo, sizeof(pHead->szErrorInfo), "������Ϣ����ʧ��");
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
//�ͻ��˶Ͽ�����ʱ������socketȡ������
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
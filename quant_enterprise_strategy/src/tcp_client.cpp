#include "tcp_client.h"
#include "sim_log.h"
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

//-------------------------------------CTcpClient-------------------------------------------
CTcpClient::CTcpClient(string szIp, int iPort)
{
	m_iSockFd = 0;
	m_bIsConnect = false;
	m_pReadBuf = new char[READ_BUF_LENGTH];
	m_pCallBack = NULL;

	m_szServerIp = szIp;
	m_iPort = iPort;

}

CTcpClient::~CTcpClient()
{

}
int CTcpClient::Init()
{
	return 0;
}
int CTcpClient::ReadData(int iFd, void *pBuf, int iCount)
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

int CTcpClient::WriteData(int iFd, const void *pBuf, int iCount)
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

//���÷�����
int CTcpClient::SetFdNonBlock(int iFd)
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
//��������
int CTcpClient::SetFdBlock(int iFd)
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

int CTcpClient::Connect()
{
	m_iSockFd = socket(AF_INET, SOCK_STREAM, 0);// ����ͨ�Ŷ˵㣺�׽���
	if (m_iSockFd < 0)
	{
		perror("socket");
		exit(-1);
	}

	// ���÷�������ַ�ṹ��
	struct sockaddr_in ServerAddr;
	bzero(&ServerAddr, sizeof(ServerAddr)); // ��ʼ����������ַ
	ServerAddr.sin_family = AF_INET;	// IPv4
	ServerAddr.sin_port = htons(m_iPort);	// �˿�
											//inet_pton(AF_INET, szServerIp, &ServerAddr.sin_addr);	// ip
	ServerAddr.sin_addr.s_addr = inet_addr(m_szServerIp.c_str());//��inet_pton�ȼ�


	// �������ӷ�����
	int iRet = connect(m_iSockFd, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if (iRet != 0)
	{
		printf("connect fail,iRet=[%d],errno=%d\n", iRet, errno);
		printf("Is Trade or Market TcpSever running? \n");
		close(m_iSockFd);

	}
	else
	{
		printf("connect ok\n");
		m_bIsConnect = true;
	}

	//���ӳɹ���fd����Ϊ������
	SetFdNonBlock(m_iSockFd);

	return iRet;
}

int CTcpClient::EpollRecv()
{
	int iEpollFd = epoll_create(EPOLL_CLOEXEC); // �������ø���������close-on-exec(FD_CLOEXEC)��־

	struct epoll_event event;
	event.data.fd = m_iSockFd;
	event.events = EPOLLIN | EPOLLET;  //EPOLLET��ETģʽ��ϵͳĬ����LTģʽ����EPOLLLT
	epoll_ctl(iEpollFd, EPOLL_CTL_ADD, m_iSockFd, &event);


	EventList events(4);
	int i;
	int iReady = 0;
	while (1)
	{
		iReady = epoll_wait(iEpollFd, &*events.begin(), static_cast<int>(events.size()), -1);
		if (iReady == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}

			return iReady;
		}

		if (iReady == 0)
		{
			continue;
		}

		if ((size_t)iReady == events.size())
		{
			events.resize(events.size() * 2);
		}

		for (i = 0; i < iReady; ++i)
		{
			if (events[i].data.fd == m_iSockFd)
			{
				printf("m_iSockFd recv data...\n");

				while (true)
				{
					int iRet = 0;
					Head HeadBuf;
					memset(&HeadBuf, 0, sizeof(HeadBuf));
					iRet = ReadData(m_iSockFd, &HeadBuf, sizeof(HeadBuf));
					if (0 == iRet)
					{
						printf("server close...\n");
						close(m_iSockFd);
						epoll_ctl(iEpollFd, EPOLL_CTL_DEL, m_iSockFd, NULL);
						return 0;

					}
					else if (iRet < 0)
					{
						printf("receive end\n");
						break;
					}

					memset(m_pReadBuf, 0, READ_BUF_LENGTH);
					int iBodyLength = HeadBuf.iBodyLength;
					if (iBodyLength > 0)
					{
						iRet = ReadData(m_iSockFd, m_pReadBuf, iBodyLength);
						if (0 == iRet)
						{
							printf("server close...\n");
							close(m_iSockFd);
							epoll_ctl(iEpollFd, EPOLL_CTL_DEL, m_iSockFd, NULL);

							return 0;
						}
						else if (iRet < 0)
						{
							printf("receive end\n");
							break;
						}
					}

					m_pCallBack->OnReceive(&HeadBuf, m_pReadBuf, iBodyLength);
				}


			}
		}
	}


	return 0;
}

int CTcpClient::Start()
{
	if (false == m_bIsConnect)
	{
		return -1;
	}

	m_pThread = new std::thread(&CTcpClient::EpollRecv, this);
	m_pThread->detach();
	return 0;
}
int CTcpClient::SendMsg(Head*pHead, void*pBody, int iBodyLength)
{
	if (m_bIsConnect)
	{
		int iRet = 0;
		int iHeadLength = sizeof(Head);
		iRet = WriteData(m_iSockFd, pHead, iHeadLength);
		if (iRet != iHeadLength)
		{
			return iRet;
		}

		if (pHead->iBodyLength != 0)
		{
			iRet = WriteData(m_iSockFd, pBody, iBodyLength);
			if (iRet != iBodyLength)
			{
				return iRet;
			}
		}

	}
	else
	{
		return -1;
	}

	return 0;
}
void  CTcpClient::SetCallBack(CallBack*pCallBack)
{
	if (pCallBack != NULL)
	{
		m_pCallBack = pCallBack;
	}
}

bool CTcpClient::GetTradeKey(int&iReadKey, int&iWriteKey)
{
	iReadKey = m_pCallBack->m_iTradeReadKey;
	iWriteKey = m_pCallBack->m_iTradeWriteKey;

	if (0 == iReadKey && 0 == iWriteKey)
	{
		//û�л�ȡ��key,key�ǳ�ʼֵ
		return false;
	}

	return true;
}

bool CTcpClient::GetMarketKey(int&iMarketKey, int&iClientNum)
{
	iMarketKey = m_pCallBack->m_iMarketKey;
	iClientNum = m_pCallBack->m_iMarketClientNum;

	if (0 == iMarketKey)
	{
		//û�л�ȡ��key,key�ǳ�ʼֵ
		return false;
	}

	return true;
}
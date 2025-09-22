#include "msg_queue_md.h"
#include "config.h"
#include "sim_log.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CMsgQueueMd::CMsgQueueMd()
{
	m_pReadShmQueue = NULL;
	m_pThread = NULL;

	m_bIsInit = false;
	m_bIsRun = false;
}

CMsgQueueMd::~CMsgQueueMd()
{

}
int CMsgQueueMd::Init(int iReadKey)
{
	LInfo("Init CMsgQueueMd ");

	if (0 == CreateReadQueue(iReadKey))
	{
		m_bIsInit = true;
	}

	return 0;
}
int CMsgQueueMd::Start()
{
	if (false == m_bIsInit)
	{
		return -1;
	}

	LInfo("Start CMsgQueueMd ");

	m_bIsRun = true;
	m_pThread = new std::thread(&CMsgQueueMd::ReadQueue, this);
	m_pThread->detach();

	return 0;
}
int CMsgQueueMd::Stop()
{
	m_bIsRun = false;
	return 0;
}
int CMsgQueueMd::Release()
{
	return 0;
}

int CMsgQueueMd::CreateReadQueue(int iReadKey)
{
	m_pReadShmQueue = new ShmQueue<MsgQueueData>(iReadKey, 'c', 0);
	if (m_pReadShmQueue->Init() != 0)
	{
		LError("create ShmQueue fail, iReadKey=[{0}]", iReadKey);
		return RET_CREATE_MSG_QUEUE_FAIL;
	}

	return 0;

}

int CMsgQueueMd::ReadQueue()
{
	if (NULL == m_pReadShmQueue)
	{
		LError("NULL == m_pReadShmQueue");
		return -1;
	}
	while (true)
	{
		if (false == m_bIsRun)
		{
			break;
		}

		MsgQueueData Msg;
		if (m_pReadShmQueue->Read(Msg))
		{
			Head*pHead = (Head*)Msg.szData;
			int iBodyLength = pHead->iBodyLength;
			int iHeadlength = sizeof(Head);
			m_pCallBack->OnReceive(pHead, &Msg.szData[iHeadlength], iBodyLength);
		}
	}

	return 0;
}
void  CMsgQueueMd::SetCallBack(CallBack*pCallBack)
{
	if (pCallBack != NULL)
	{
		m_pCallBack = pCallBack;
	}
}
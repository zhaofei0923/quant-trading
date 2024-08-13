#include "msg_queue_td.h"
#include "config.h"
#include "sim_log.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CMsgQueueTd::CMsgQueueTd()
{
	m_pReadShmQueue = NULL;
	m_pWriteShmQueue = NULL;
	m_pThread = NULL;

	m_bIsInit = false;
	m_bIsRun = false;
}

CMsgQueueTd::~CMsgQueueTd()
{

}
int CMsgQueueTd::Init(int iReadKey, int iWriteKey)
{
	LInfo("Init CMsgQueueTd ");

	if (0 == CreateMsgQueue(iReadKey, iWriteKey))
	{
		m_bIsInit = true;
	}

	return 0;
}
int CMsgQueueTd::Start()
{
	if (false == m_bIsInit)
	{
		return -1;
	}

	LInfo("Start CMsgQueueTd ");

	m_bIsRun = true;
	m_pThread = new std::thread(&CMsgQueueTd::ReadQueue, this);
	m_pThread->detach();

	return 0;
}
int CMsgQueueTd::Stop()
{
	m_bIsRun = false;
	return 0;
}
int CMsgQueueTd::Release()
{
	return 0;
}
int CMsgQueueTd::CreateMsgQueue(int iReadKey, int iWriteKey)
{
	int iRet = 0;
	iRet = CreateReadQueue(iReadKey);
	if (iRet != 0)
	{
		return iRet;
	}
	LInfo("create CreateReadQueue Ok, iReadKey=[{0}]", iReadKey);

	iRet = CreateWriteQueue(iWriteKey);
	if (iRet != 0)
	{

		return iRet;
	}
	LInfo("create CreateWriteQueue Ok, iWriteKey=[{0}]", iWriteKey);

	return 0;
}
int CMsgQueueTd::CreateReadQueue(int iReadKey)
{
	m_pReadShmQueue = new ShmQueue<MsgQueueData>(iReadKey, 'c', 0);
	if (m_pReadShmQueue->Init() != 0)
	{
		LError("create ShmQueue fail, iReadKey=[{0}]", iReadKey);
		return RET_CREATE_MSG_QUEUE_FAIL;
	}

	return 0;

}
int CMsgQueueTd::CreateWriteQueue(int iWriteKey)
{
	m_pWriteShmQueue = new ShmQueue<MsgQueueData>(iWriteKey, 'c', 0);
	if (m_pWriteShmQueue->Init() != 0)
	{
		LError("create ShmQueue fail, iWriteKey=[{0}]", iWriteKey);
		return RET_CREATE_MSG_QUEUE_FAIL;
	}

	return 0;
}

int CMsgQueueTd::ReadQueue()
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
int CMsgQueueTd::WriteQueue(Head*pHead, void*pBody, int iBodyLength)
{
	if (NULL == m_pWriteShmQueue)
	{
		LError("NULL == m_pWriteShmQueue");
		return -1;
	}

	if (m_bIsRun)
	{
		int iFunctionId = pHead->iFunctionId;
		MsgQueueData Msg;
		memcpy(&Msg.szData[0], pHead, sizeof(Head));
		if (iBodyLength > 0)
		{
			memcpy(&Msg.szData[sizeof(Head)], pBody, iBodyLength);
		}
		if (false == m_pWriteShmQueue->Write(Msg))
		{
			LError("write msg fail, iFunctionId=[{0}]", iFunctionId);
			return  -1;
		}
	}

	return 0;
}
void  CMsgQueueTd::SetCallBack(CallBack*pCallBack)
{
	if (pCallBack != NULL)
	{
		m_pCallBack = pCallBack;
	}
}
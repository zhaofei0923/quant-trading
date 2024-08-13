#include "msg_queue_server.h"
#include "config.h"
#include "sim_log.h"
#include "execute_engine.h"
#include "mem_pool_mng.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CMsgQueueServer::CMsgQueueServer()
{
	m_iMsgIndex = 0;
	m_iKeyIndex = 0;
}

CMsgQueueServer::~CMsgQueueServer()
{

}
int CMsgQueueServer::Init()
{
	SetThreadCount(1);

	m_iKeyIndex = TRADE_SHARE_MEMORY_START_KEY;

	return 0;
}
int CMsgQueueServer::Start()
{
	std::cout << "Start CMsgQueueServer" << endl;
	LInfo("Start CMsgQueueServer ");

	m_pThread = new std::thread(&CMsgQueueServer::ReadQueue, this);
	m_pThread->detach();

	RunThreads();

	return 0;
}
int CMsgQueueServer::Stop()
{
	StopThreads();
	return 0;
}
int CMsgQueueServer::Release()
{

	return 0;
}

int  CMsgQueueServer::DealMsg(void*pMsg, int iThreadId)
{
	if (pMsg != NULL)
	{
		ServerMsg*pServerMsg = (ServerMsg*)pMsg;
		WriteQueue(pServerMsg);

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
int CMsgQueueServer::CreateMsgQueue(string&szUserId, int&iReadKey, int&iWriteKey)
{
	int iRet = 0;
	iRet = CreateReadQueue(szUserId, iReadKey);
	if (iRet != 0)
	{
		return iRet;
	}

	std::cout << "CreateReadQueue OK, szUserId=" << szUserId << endl;
	LInfo("CreateReadQueue OK, szUserId=[{0}]", szUserId);

	iRet = CreateWriteQueue(szUserId, iWriteKey);
	if (iRet != 0)
	{
		DelReadQueue(szUserId);
		return iRet;
	}

	std::cout << "CreateWriteQueue OK, szUserId=" << szUserId << endl;
	LInfo("CreateWriteQueue OK, szUserId=[{0}]", szUserId);

	return 0;
}
int CMsgQueueServer::CreateReadQueue(string&szUserId, int&iReadKey)
{
	lock_guard<mutex> lk(m_ReadMtx);
	iReadKey = GetKeyValue();
	if (m_ReadMap.count(szUserId) > 0)
	{
		return RET_MSG_QUEUE_EXIST;
	}
	else
	{
		ShmQueue<MsgQueueData>*pShmQueue = new ShmQueue<MsgQueueData>(iReadKey, 's', TRADE_MSG_QUEUE_ITEM_COUNT);
		if (pShmQueue->Init() != 0)
		{
			LError("create ShmQueue fail, szUserId=[{0}]", szUserId);
			return RET_CREATE_MSG_QUEUE_FAIL;
		}
		m_ReadMap[szUserId] = pShmQueue;
	}

	return 0;

}
int CMsgQueueServer::CreateWriteQueue(string&szUserId, int& iWriteKey)
{
	lock_guard<mutex> lk(m_WriteMtx);

	iWriteKey = GetKeyValue();
	if (m_WriteMap.count(szUserId) > 0)
	{
		return RET_MSG_QUEUE_EXIST;
	}
	else
	{
		ShmQueue<MsgQueueData>*pShmQueue = new ShmQueue<MsgQueueData>(iWriteKey, 's', TRADE_MSG_QUEUE_ITEM_COUNT);
		if (pShmQueue->Init() != 0)
		{
			LError("create ShmQueue fail, szUserId=[{0}]", szUserId);
			return RET_CREATE_MSG_QUEUE_FAIL;
		}
		m_WriteMap[szUserId] = pShmQueue;
	}

	return 0;
}
int CMsgQueueServer::DelMsgQueue(string&szUserId)
{
	DelReadQueue(szUserId);
	DelWriteQueue(szUserId);

	std::cout << "DelMsgQueue, szUserId=" << szUserId << endl;
	LInfo("DelMsgQueue, szUserId=[{0}]", szUserId);

	return 0;
}
int CMsgQueueServer::DelReadQueue(string&szUserId)
{
	lock_guard<mutex> lk(m_ReadMtx);

	if (m_ReadMap.count(szUserId) > 0)
	{
		m_ReadMap.erase(szUserId);
	}

	return 0;

}
int CMsgQueueServer::DelWriteQueue(string&szUserId)
{
	lock_guard<mutex> lk(m_WriteMtx);

	if (m_WriteMap.count(szUserId) > 0)
	{
		m_WriteMap.erase(szUserId);
	}

	return 0;
}

int CMsgQueueServer::ReadQueue()
{
	while (true)
	{
		lock_guard<mutex> lk(m_ReadMtx);

		for (auto& ReadPair : m_ReadMap)
		{
			string szUserId = ReadPair.first;
			ShmQueue<MsgQueueData>*pShmQueue = ReadPair.second;
			MsgQueueData Msg;
			if (pShmQueue->Read(Msg))
			{
				Head*pHead = (Head*)Msg.szData;
				int iBodyLength = pHead->iBodyLength;
				int iHeadlength = sizeof(Head);
				MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
				char*pBodyBuf = pMemPoolMng->GetMaxMemBlock(iBodyLength);
				if (NULL == pBodyBuf)
				{
					LError("pBodyBuf is NULL!");
					return -1;
				}
				memcpy(pBodyBuf, &Msg.szData[iHeadlength], iBodyLength);
				unsigned int iMsgIndex = AddClientHead(*pHead);


				char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
				if (NULL == pMemBlock)
				{
					LError("pMemBlock is NULL!");
					return -1;
				}
				ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
				pServerMsg->iConnectId = 0;
				snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", szUserId.c_str());
				pServerMsg->iFunctionId = pHead->iFunctionId;
				pServerMsg->iRoletype = pHead->iRoletype;
				pServerMsg->iMsgIndex = iMsgIndex;
				pServerMsg->iBodyLen = iBodyLength;
				pServerMsg->pBody = pBodyBuf;
				pServerMsg->bIsLast = true;
				pServerMsg->iErrorCode = RET_OK;
				snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "OK");

				ExecuteEngine::GetInstance()->PostMsg(pServerMsg);
			}

		}
	}

	return 0;
}
int CMsgQueueServer::WriteQueue(ServerMsg*pServerMsg)
{
	lock_guard<mutex> lk(m_WriteMtx);
	string szUserId = pServerMsg->szUserId;
	int iFunctionId = pServerMsg->iFunctionId;

	if (m_WriteMap.count(szUserId) > 0)
	{
		Head HeadData;
		if (iFunctionId == FUNC_PLACE_ORDER || iFunctionId == FUNC_CANCEL_ORDER)
		{
			if (false == GetClientHead(pServerMsg->iMsgIndex, HeadData))
			{
				LInfo("iMsgIndex is not exist，iMsgIndex=[{0}]", pServerMsg->iMsgIndex);
				return -1;
			}
			DelClientHead(pServerMsg->iMsgIndex);
		}
		else
		{
			HeadData.iFunctionId = iFunctionId;
			HeadData.iRoletype = ROLE_TYPE_TRADER;
		}


		//修改Head
		HeadData.iMsgtype = MSG_TYPE_RES;
		HeadData.iBodyLength = pServerMsg->iBodyLen;
		HeadData.bIsLast = pServerMsg->bIsLast;
		HeadData.iErrorCode = pServerMsg->iErrorCode;
		snprintf(HeadData.szErrorInfo, sizeof(HeadData.szErrorInfo), "%s", pServerMsg->szErrorInfo);

		//复制数据
		MsgQueueData Msg;
		memcpy(&Msg.szData[0], &HeadData, sizeof(HeadData));
		if (pServerMsg->iBodyLen != 0)
		{
			memcpy(&Msg.szData[sizeof(Head)], pServerMsg->pBody, pServerMsg->iBodyLen);
		}

		//发送
		ShmQueue<MsgQueueData>*pShmQueue = m_WriteMap[szUserId];
		if (false == pShmQueue->Write(Msg))
		{
			LError("write msg fail, szUserId=[{0}]", szUserId);
		}
	}
	else
	{
		LError("ShmQueue is not exist, szUserId=[{0}]", szUserId);
		return RET_MSG_QUEUE_NOT_EXIST;
	}

	return 0;
}

int CMsgQueueServer::GetKeyValue()
{
	lock_guard<mutex> lk(m_KeyMtx);
	m_iKeyIndex++;
	return  m_iKeyIndex;
}
int CMsgQueueServer::GetMsgIndex()
{
	lock_guard<mutex> lk(m_MsgIndexMtx);
	m_iMsgIndex++;
	return m_iMsgIndex;
}

int CMsgQueueServer::AddClientHead(Head&HeadData)
{
	lock_guard<mutex> lk(m_IndexHeadMtx);

	unsigned int iIndex = GetMsgIndex();
	m_IndexHeadMap[iIndex] = HeadData;

	return iIndex;
}
int CMsgQueueServer::DelClientHead(unsigned int iIndex)
{
	lock_guard<mutex> lk(m_IndexHeadMtx);
	m_IndexHeadMap.erase(iIndex);
	return 0;
}
bool CMsgQueueServer::GetClientHead(unsigned int iIndex, Head&HeadData)
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
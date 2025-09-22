#include "msg_queue_server.h"
#include "config.h"
#include "sim_log.h"
#include "const.h"
#include "struct.h"
#include "message.h"
#include "convert.h"
#include "mem_pool_mng.h"
#include "ThostFtdcMdApi.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CMsgQueueServer::CMsgQueueServer()
{
	m_iKeyIndex = 0;
	m_iCount = 0;
	m_pSendBuf = new char[MARKET_MSG_QUEUE_SEND_BUF_LENGTH];
}

CMsgQueueServer::~CMsgQueueServer()
{

}
int CMsgQueueServer::Init()
{
	SetThreadCount(1);

	m_iKeyIndex = MARKET_SHARE_MEMORY_START_KEY;

	return RET_OK;
}
int CMsgQueueServer::Start()
{
	LInfo("Start CMsgQueueServer ");
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
//vector<string>&vStockCodes:订阅的股票代码
int CMsgQueueServer::CreateWriteQueue(vector<string>&vStockCodes, int& iWriteKey, int&iClientNum)
{
	lock_guard<mutex> lk(m_WriteMtx);

	LDebug("CreateWriteQueue");

	iWriteKey = GetKeyValue();
	ShmQueue<MsgQueueData>*pShmQueue = new ShmQueue<MsgQueueData>(iWriteKey, 's', MARKET_MSG_QUEUE_ITEM_COUNT);
	if (pShmQueue->Init() != 0)
	{
		LError("create ShmQueue fail, iWriteKey=[{0}]", iWriteKey);
		return RET_CREATE_MSG_QUEUE_FAIL;
	}

	iClientNum = iWriteKey;
	m_WriteMap[iClientNum] = pShmQueue;

	//存储此客户端订阅的股票
	m_ClientSubInfo[iClientNum] = vStockCodes;

	//遍历客户端订阅的股票，把客户端编号添加到股票代码的订阅者
	for (int i = 0; i < vStockCodes.size(); i++)
	{
		string szStockCode = vStockCodes[i];

		if (m_StockSubInfo.count(szStockCode) > 0)
		{
			//添加iClientNum到szStockCode的订阅列表
			LDebug("add iClientNum=[{0}] to szStockCode=[{1}] sub list", iClientNum, szStockCode);
			m_StockSubInfo[szStockCode].push_back(iClientNum);
		}
		else
		{
			//创建szStockCode的订阅列表
			//添加iClientNum到szStockCode的订阅列表
			LDebug("create szStockCode=[{0}] sub list", szStockCode);
			LDebug("add iClientNum=[{0}] to szStockCode=[{1}] sub list", iClientNum, szStockCode);

			list<int> ClientNumList;
			m_StockSubInfo[szStockCode] = ClientNumList;
			m_StockSubInfo[szStockCode].push_back(iClientNum);
		}
	}

	return 0;
}
int CMsgQueueServer::DelWriteQueue(int iClientNum)
{
	lock_guard<mutex> lk(m_WriteMtx);

	LDebug("DelWriteQueue,iClientNum=[{0}]", iClientNum);

	if (m_ClientSubInfo.count(iClientNum) > 0)
	{
		//查找此客户端编号订阅的所有股票
		//从股票的订阅者中删除客户端编号
		vector<string>&vStockCodes = m_ClientSubInfo[iClientNum];
		for (int i = 0; i < vStockCodes.size(); i++)
		{
			string szStockCode = vStockCodes[i];
			m_StockSubInfo[szStockCode].remove(iClientNum);
		}

		//删除订阅者订阅的股票
		m_ClientSubInfo.erase(iClientNum);
	}
	else
	{
		LError("iClientNum is not exist, iClientNum=[{0}]", iClientNum);
		return -1;
	}


	if (m_WriteMap.count(iClientNum) > 0)
	{
		m_WriteMap.erase(iClientNum);
	}

	return 0;
}
int CMsgQueueServer::WriteQueue(ServerMsg*pServerMsg)
{
	lock_guard<mutex> lk(m_WriteMtx);

	int iFunctionId = pServerMsg->iFunctionId;
	m_iCount++;

	Head  HeadData;
	HeadData.iMsgtype = MSG_TYPE_REQ;
	HeadData.iReqId = m_iCount;
	HeadData.iRoletype = ROLE_TYPE_TRADER;
	HeadData.bIsLast = false;

	HeadData.iFunctionId = iFunctionId;
	HeadData.iErrorCode = pServerMsg->iErrorCode;
	snprintf(HeadData.szErrorInfo, sizeof(HeadData.szErrorInfo), "%s", pServerMsg->szErrorInfo);

	memset(m_pSendBuf, 0, MARKET_MSG_QUEUE_SEND_BUF_LENGTH);
	int iBodyLength = 0;

	string szStockCode;
	if (FUNC_MARKET_QUOTE == HeadData.iFunctionId)
	{
		//转换格式
		MarketData*pQuote = (MarketData*)m_pSendBuf;
		ConvertMarketData((CThostFtdcDepthMarketDataField *)pServerMsg->pBody, pQuote);
		iBodyLength = sizeof(MarketData);
		szStockCode = pQuote->szStockCode;
	}

	HeadData.iBodyLength = iBodyLength;

	//复制数据
	MsgQueueData Msg;
	memcpy(&Msg.szData[0], &HeadData, sizeof(HeadData)); //复制Head
	if (iBodyLength != 0)
	{
		//复制Body
		memcpy(&Msg.szData[sizeof(Head)], m_pSendBuf, iBodyLength);
	}

	if (m_StockSubInfo.count(szStockCode) > 0)
	{
		//获取此股票的订阅者列表
		list<int>& ClientNumList = m_StockSubInfo[szStockCode];
		list<int>::iterator it; //声明一个迭代器
		for (it = ClientNumList.begin(); it != ClientNumList.end(); it++)
		{
			//根据订阅者列表中的客户端编号查找消息队列
			int iClientNum = *it;
			if (m_WriteMap.count(iClientNum) > 0)
			{
				//发送
				ShmQueue<MsgQueueData>*pShmQueue = m_WriteMap[iClientNum];
				if (false == pShmQueue->Write(Msg))
				{
					LError("write msg fail, iClientNum=[{0}]", iClientNum);
				}
			}
		}
	}

	return 0;
}

int CMsgQueueServer::GetKeyValue()
{
	lock_guard<mutex> lk(m_KeyMtx);
	m_iKeyIndex++;
	return  m_iKeyIndex;
}

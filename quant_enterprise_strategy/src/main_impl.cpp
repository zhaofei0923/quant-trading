#include "main_impl.h"
#include "config.h"
#include "sim_log.h"
#include "mem_pool_mng.h"
#include "strategy.h"
#include <chrono>

/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

int MainImpl::Init()
{

	ConfigData*pConfigData = ConfigData::GetInstance();
	if (false == pConfigData->LoadConfig())
	{
		return -1;
	}

	SimLog::Instance().InitSimLog(pConfigData->iLogLevel);

	//内存池
	MemPoolMng::GetInstance()->Init();

	//策略
	CStrategy::GetInstance()->Init();

	//交易
	m_pTrade = new CTcpClient(pConfigData->szTradeIp, pConfigData->iTradePort);
	CallBack*pTdTcpCall = new CallBack();
	m_pTrade->SetCallBack(pTdTcpCall);
	m_pTrade->Init();

	//交易
	CallBack*pTdMqCall = new CallBack();
	m_pCMsgQueueTd = CMsgQueueTd::GetInstance();
	m_pCMsgQueueTd->SetCallBack(pTdMqCall);


	//行情
	m_pMarket = new CTcpClient(pConfigData->szMarketIp, pConfigData->iMarketPort);
	CallBack*pMdTcpCall = new CallBack();
	m_pMarket->SetCallBack(pMdTcpCall);
	m_pMarket->Init();

	//行情
	CallBack*pMdMqCall = new CallBack();
	m_pCMsgQueueMd = CMsgQueueMd::GetInstance();
	m_pCMsgQueueMd->SetCallBack(pMdMqCall);

	m_bIsInit = true;
	return 0;
}
int MainImpl::Start()
{
	if (false == m_bIsInit)
	{
		return -1;
	}

	//策略
	CStrategy::GetInstance()->Start();

	if (m_pTrade->Connect() != 0)
	{
		return -1;
	}

	if (m_pTrade->Start() != 0)
	{
		return -1;
	}

	if (m_pMarket->Connect() != 0)
	{
		return -1;
	}

	if (m_pMarket->Start() != 0)
	{
		return -1;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));

	//交易员登录
	if (false == TraderLogin())
	{
		LError("TraderLogin  Fail");
		return -1;
	}

	//订阅行情
	vector<string> vStockCodes;
	string szStockCode = "cu2408";
	vStockCodes.push_back(szStockCode);
	//szStockCode = "cu2402";
	//vStockCodes.push_back(szStockCode);

	if (false == SubMarket(vStockCodes))
	{
		LError("SubMarket  Fail");
		return -1;
	}

	return 0;
}
int MainImpl::Stop()
{
	TraderLogout();
	//策略
	CStrategy::GetInstance()->Stop();
	return 0;
}
int MainImpl::Release()
{
	//策略
	CStrategy::GetInstance()->Release();
	//内存池
	MemPoolMng::GetInstance()->Release();
	return 0;
}

//交易员登录
bool MainImpl::TraderLogin()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 300;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);

	printf("------ login------\n");

	LoginReqT LoginData;

	sprintf(LoginData.szUserId, "%s", pConfigData->szTrader.c_str());
	sprintf(LoginData.szPassword, "%s", pConfigData->szTraderPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
	printf("------add msg queue------\n");

	HeadBuf.iFunctionId = FUNC_ADD_MSG_QUEUE;
	HeadBuf.iReqId = 301;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	//启动共享内存通讯
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
	int iReadKey = 0;
	int	iWriteKey = 0;
	m_pTrade->GetTradeKey(iReadKey, iWriteKey);
	if (iReadKey != 0 && iWriteKey != 0)
	{
		m_pCMsgQueueTd->Init(iReadKey, iWriteKey);
		m_pCMsgQueueTd->Start();
	}
	else
	{
		printf("------start share memory fail-----\n");
		return false;
	}

	return true;

}
//交易员退出
void MainImpl::TraderLogout()
{
	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_DEL_MSG_QUEUE;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 300;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = 0;


	printf("------stop  local  msg queue-----\n");
	//发送删除共享内存通讯队列报文之前先停止本地共享内存通讯线程
	m_pCMsgQueueTd->Stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	printf("------logout------\n");

	HeadBuf.iFunctionId = FUNC_DEL_MSG_QUEUE;
	HeadBuf.iReqId = 301;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


}
//订阅行情
bool MainImpl::SubMarket(const vector<string>&vStockCodes)
{
	printf("-------SubMarket----Start------\n");
	vector<SubItem> vSubStockCodes;
	for (int i = 0; i < vStockCodes.size(); i++)
	{
		SubItem Item;
		snprintf(Item.szStockCode, sizeof(Item.szStockCode), "%s", vStockCodes[i].c_str());
		vSubStockCodes.push_back(Item);
	}

	int iNum = vSubStockCodes.size();
	int iLength = sizeof(iNum) + iNum * sizeof(SubItem);

	char*pBuf = new char[iLength];
	memcpy(&pBuf[0], &iNum, sizeof(iNum));
	memcpy(&pBuf[sizeof(iNum)], &vSubStockCodes[0], iNum * sizeof(SubItem));


	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_SUB_QUOTE;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 500;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = iLength;

	printf("-------SubMarket-----SendMsg---FUNC_SUB_QUOTE--\n");
	m_pMarket->SendMsg(&HeadBuf, pBuf, iLength);
	delete[] pBuf;

	//启动共享内存通讯
	int iMarketKey = 0;
	int	iClientNum = 0;

	//由于是异步通讯，此处采用循环查询
	unsigned long long  i = 0;
	while (1)
	{
		i++;

		if (i > 1000000)
		{
			printf("------not receive res msg!   start share memory fail-----\n");
			return false;
		}

		m_pMarket->GetMarketKey(iMarketKey, iClientNum);
		if (iMarketKey != 0 && iClientNum != 0)
		{
			m_pCMsgQueueMd->Init(iMarketKey);
			m_pCMsgQueueMd->Start();
			return true;
		}

		//休眠1微秒
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	printf("-------SubMarket----End------\n");
	return true;
}
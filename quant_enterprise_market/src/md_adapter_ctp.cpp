#include "helper_time.h"
#include "md_adapter_ctp.h"
#include "msg_queue_server.h"
#include "save_data.h"
#include "sim_log.h"
#include "trade_data.h"
#include "mem_pool_mng.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

MdAdapterCtp::MdAdapterCtp() : m_pQuoteApi(nullptr), m_iRequestId(0), m_bIsLogin(false)
{
}

MdAdapterCtp::~MdAdapterCtp()
{
	Logout();
	if (m_pQuoteApi)
	{
		m_pQuoteApi->Release();
		m_pQuoteApi = nullptr;
	}
}
int MdAdapterCtp::GetRequestId()
{
	lock_guard<mutex> lk(m_RequestIdMtx);
	m_iRequestId++;
	return m_iRequestId;
}

void MdAdapterCtp::Connect(CtpConfig &ConfigData)
{
	m_CtpConfig = ConfigData;

	m_pQuoteApi = CThostFtdcMdApi::CreateFtdcMdApi();
	if (!m_pQuoteApi)
	{
		LError("ctp create quoteapi Error");
		return;
	}

	m_pQuoteApi->RegisterSpi(this);
	string szFrontAddr = "tcp://" + m_CtpConfig.szMarketIp + ':' + to_string(m_CtpConfig.iMarketPort);
	std::cout << "Market FrontAddress=" << szFrontAddr << endl;
	//const_cast转换符是用来移除变量的const或volatile限定符
	m_pQuoteApi->RegisterFront(const_cast<char *>(szFrontAddr.c_str()));

	m_pQuoteApi->Init();
}


bool MdAdapterCtp::IsValidTick(string sTickTime)
{
	//8:59:00-11:31:00,13:29:00-15:01:00,20:59:00-02:30:00
	//iHour>0 && iHour<23 , iMinute>0 && iMinute<59 , iSecond>0 && iSecond<59

	int iHour = atoi(sTickTime.substr(0, 2).c_str());
	int iMinute = atoi(sTickTime.substr(3, 2).c_str());
	int iSecond = atoi(sTickTime.substr(6, 2).c_str());

	if ((iHour == 8 && iMinute == 59) || iHour == 9 || iHour == 10 || (iHour == 11 && iMinute <= 30))
	{
		return true;
	}
	else if ((iHour == 13 && iMinute >= 29) || iHour == 14 || (iHour == 15 && iMinute == 0))
	{
		return true;
	}
	else if ((iHour == 20 && iMinute == 59) || iHour == 21 || iHour == 22 || iHour == 23 || iHour == 0 || iHour == 1 || (iHour == 2 && iMinute <= 30))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MdAdapterCtp::Logout()
{
	if (!m_pQuoteApi)
	{
		return;
	}

	CThostFtdcUserLogoutField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_CtpConfig.szBrokerId.c_str());
	strcpy(ReqParam.UserID, m_CtpConfig.szUserId.c_str());
	int iRet = m_pQuoteApi->ReqUserLogout(&ReqParam, GetRequestId());
	if (iRet != 0)
	{
		LError("send ctp ReqUserLogout Error,iRet=[{0}]", iRet);
	}

}

// --- req functions for outside call --- //

//vInstrumentId：合约代码
void MdAdapterCtp::ReqSubscribeMarketdata(const vector<string> &vInstrumentId)
{
	LInfo("ctp ReqSubscribeMarketdata");

	if (false == m_bIsLogin)
	{
		LError("MdAdapterCtp  is not login");
	}

	const int iCount = vInstrumentId.size();
	char **pInstrumentIdArray = new char*[iCount];
	for (int i = 0; i < iCount; ++i)
	{
		pInstrumentIdArray[i] = const_cast<char *>(vInstrumentId[i].c_str());
		//printf("[%d]=%s\n",i, pInstrumentIdArray[i]);
		//LInfo("ReqSubscribeMarketdata i=[{0}],StockCode=[{1}]", i, pInstrumentIdArray[i]);
	}

	//必须输入合约代码才能订阅
	int iRet = m_pQuoteApi->SubscribeMarketData(pInstrumentIdArray, iCount);
	if (iRet != 0)
	{
		LError("send ctp SubscribeMarketData Error,iRet=[{0}]", iRet);
	}
	else
	{
		LInfo("send ctp SubscribeMarketData OK");
	}

	delete[] pInstrumentIdArray;
}

// --- ctp aync response --- //
void MdAdapterCtp::OnFrontConnected()
{
	printf("ctp MdAdapterCtp  OnFrontConnected\n");
	LInfo("---- ctp MdAdapterCtp  OnFrontConnected ----");

	CThostFtdcReqUserLoginField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_CtpConfig.szBrokerId.c_str());
	strcpy(ReqParam.UserID, m_CtpConfig.szUserId.c_str());
	strcpy(ReqParam.Password, m_CtpConfig.szPassword.c_str());

	int iRet = m_pQuoteApi->ReqUserLogin(&ReqParam, GetRequestId());
	if (iRet != 0)
	{
		LError("send ctp MdAdapterCtp ReqUserLogin Error,iRet=[{0}]", iRet);
	}
	else
	{
		LInfo("send ctp MdAdapterCtp  ReqUserLogin OK");
	}

}

void MdAdapterCtp::OnFrontDisconnected(int nReason)
{
	LError("---- ctp MdAdapterCtp  OnFrontDisconnected ----,nReason=[{0}]", nReason);
	m_bIsLogin = false;

}

void MdAdapterCtp::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		printf("ctp MdAdapterCtp OnRspUserLogin success\n");
		LInfo("ctp MdAdapterCtp OnRspUserLogin success");

		m_bIsLogin = true;

		//获取合约数据
		vector<shared_ptr<InstrumentT>> InstrumentDatas;
		TradeData::GetInstruments(InstrumentDatas);
		if (0 == InstrumentDatas.size())
		{
			LError("database Instrument is empty");
			return;
		}

		vector<string>  vInstrumentId;

		for (int i = 0; i < InstrumentDatas.size(); i++)
		{
			if(InstrumentDatas[i]->szInstrumentId == "cu2408")
			{
				vInstrumentId.push_back(InstrumentDatas[i]->szInstrumentId);
			}
			//vInstrumentId.push_back(InstrumentDatas[i]->szInstrumentId);
		}
		//订阅
		ReqSubscribeMarketdata(vInstrumentId);
	}
	else
	{
		std::cout << "ctp MdAdapterCtp OnRspUserLogin Error,error_msg=" << pRspInfo->ErrorMsg << endl;
		LError("ctp MdAdapterCtp OnRspUserLogin  Error, error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

	}
}

void MdAdapterCtp::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp MdAdapterCtp OnRspUserLogout success");
		m_bIsLogin = false;
	}
	else
	{
		LError("ctp MdAdapterCtp OnRspUserLogout  Error, error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}

void MdAdapterCtp::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		//LInfo("ctp MdAdapterCtp OnRspSubMarketData success");
	}
	else
	{
		LError("ctp MdAdapterCtp OnRspSubMarketData  Error, error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

	}
}

void MdAdapterCtp::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	// printf("---------pDepthMarketData--%s------------------\n", pDepthMarketData->InstrumentID);
	// printf("pDepthMarketData->TradingDay=[%s]\n", pDepthMarketData->TradingDay);
	// printf("pDepthMarketData->UpdateTime=[%s]\n", pDepthMarketData->UpdateTime);
	// printf("pDepthMarketData->ActionDay=[%s]\n", pDepthMarketData->ActionDay);

	string sTickTime = pDepthMarketData->UpdateTime;

	if (IsValidTick(sTickTime))
	{

		//printf("UpdateTime=[%s],InstrumentID=[%s]\n", pDepthMarketData->UpdateTime, pDepthMarketData->InstrumentID);

		MemPoolMng *pMemPoolMng = MemPoolMng::GetInstance();

		// 发送到CMsgQueueServer
		{
			char *pData = pMemPoolMng->GetMaxMemBlock(sizeof(CThostFtdcDepthMarketDataField));
			if (NULL == pData)
			{
				LError("pData is NULL!");
				return;
			}
			CThostFtdcDepthMarketDataField *pMarketData = (CThostFtdcDepthMarketDataField *)pData;
			memcpy(pMarketData, pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));

			char *pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
			if (NULL == pMemBlock)
			{
				LError("pMemBlock is NULL!");
				return;
			}
			ServerMsg *pServerMsg = (ServerMsg *)pMemBlock;
			pServerMsg->iFunctionId = FUNC_MARKET_QUOTE;
			pServerMsg->pBody = (char *)pMarketData;
			pServerMsg->iBodyLen = sizeof(CThostFtdcDepthMarketDataField);

			CMsgQueueServer::GetInstance()->PostMsg(pServerMsg);
		}

		// 发送到CSaveData
		{
			char *pData = pMemPoolMng->GetMaxMemBlock(sizeof(CThostFtdcDepthMarketDataField));
			if (NULL == pData)
			{
				LError("pData is NULL!");
				return;
			}
			CThostFtdcDepthMarketDataField *pMarketData = (CThostFtdcDepthMarketDataField *)pData;
			memcpy(pMarketData, pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));

			char *pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
			if (NULL == pMemBlock)
			{
				LError("pMemBlock is NULL!");
				return;
			}
			ServerMsg *pServerMsg = (ServerMsg *)pMemBlock;
			pServerMsg->iFunctionId = FUNC_MARKET_QUOTE;
			pServerMsg->pBody = (char *)pMarketData;
			pServerMsg->iBodyLen = sizeof(CThostFtdcDepthMarketDataField);

			CSaveData::GetInstance()->PostMsg(pServerMsg);
		}
	}
	else
	{
		printf("Invalid Tick UpdateTime=[%s],InstrumentID=[%s]\n", pDepthMarketData->UpdateTime, pDepthMarketData->InstrumentID);
		// LInfo("ctp MdAdapterCtp OnRtnDepthMarketData  Error, error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}

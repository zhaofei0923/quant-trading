#include "config.h"
#include "sim_log.h"
#include "helper_time.h"
#include "helper_tools.h"
#include "execute_engine.h"
#include "td_adapter_ctp.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

//去空格
std::string& Trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

static inline int ConvertMarketData(const CThostFtdcDepthMarketDataField *pDepthMarketData, MarketData&Snapshot)
{
	MarketData *pMarketData = &Snapshot;

	memset(pMarketData, 0, sizeof(MarketData));

	sprintf(pMarketData->szTradingDay, "%s", pDepthMarketData->TradingDay);
	sprintf(pMarketData->szActionDay, "%s", pDepthMarketData->ActionDay);
	sprintf(pMarketData->szStockCode, "%s", pDepthMarketData->InstrumentID);
	sprintf(pMarketData->szExchangeId, "%s", pDepthMarketData->ExchangeID);
	sprintf(pMarketData->szUpdateTime, "%s", pDepthMarketData->UpdateTime);
	pMarketData->iUpdateMillisec = pDepthMarketData->UpdateMillisec;

	pMarketData->iLastPrice = zutil::ConvertDoubleToInt(pDepthMarketData->LastPrice, PRICE_MULTIPLE);
	pMarketData->iPreSettlePrice = zutil::ConvertDoubleToInt(pDepthMarketData->PreSettlementPrice, PRICE_MULTIPLE);
	pMarketData->iPreClosePrice = zutil::ConvertDoubleToInt(pDepthMarketData->PreClosePrice, PRICE_MULTIPLE);
	pMarketData->iPreOpenInterest = pDepthMarketData->PreOpenInterest;
	pMarketData->iOpenPrice = zutil::ConvertDoubleToInt(pDepthMarketData->OpenPrice, PRICE_MULTIPLE);
	pMarketData->iLowPrice = zutil::ConvertDoubleToInt(pDepthMarketData->LowestPrice, PRICE_MULTIPLE);
	pMarketData->iHighPrice = zutil::ConvertDoubleToInt(pDepthMarketData->HighestPrice, PRICE_MULTIPLE);
	pMarketData->iVolume = pDepthMarketData->Volume;
	pMarketData->iTurnOver = zutil::ConvertDoubleToInt(pDepthMarketData->Turnover, PRICE_MULTIPLE);
	pMarketData->iOpenInterest = pDepthMarketData->OpenInterest;
	pMarketData->iClosePrice = zutil::ConvertDoubleToInt(pDepthMarketData->ClosePrice, PRICE_MULTIPLE);
	pMarketData->iSettlePrice = zutil::ConvertDoubleToInt(pDepthMarketData->SettlementPrice, PRICE_MULTIPLE);
	pMarketData->iUpperLimitPrice = zutil::ConvertDoubleToInt(pDepthMarketData->UpperLimitPrice, PRICE_MULTIPLE);
	pMarketData->iLowerLimitPrice = zutil::ConvertDoubleToInt(pDepthMarketData->LowerLimitPrice, PRICE_MULTIPLE);
	pMarketData->iPreDelta = zutil::ConvertDoubleToInt(pDepthMarketData->PreDelta, PRICE_MULTIPLE);
	pMarketData->iCurrDelta = zutil::ConvertDoubleToInt(pDepthMarketData->CurrDelta, PRICE_MULTIPLE);
	pMarketData->iAveragePrice = zutil::ConvertDoubleToInt(pDepthMarketData->AveragePrice, PRICE_MULTIPLE);

	pMarketData->iSellPrice1 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice1, PRICE_MULTIPLE);
	pMarketData->iSellPrice2 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice2, PRICE_MULTIPLE);
	pMarketData->iSellPrice3 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice3, PRICE_MULTIPLE);
	pMarketData->iSellPrice4 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice4, PRICE_MULTIPLE);
	pMarketData->iSellPrice5 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice5, PRICE_MULTIPLE);
	pMarketData->iBuyPrice1 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice1, PRICE_MULTIPLE);
	pMarketData->iBuyPrice2 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice2, PRICE_MULTIPLE);
	pMarketData->iBuyPrice3 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice3, PRICE_MULTIPLE);
	pMarketData->iBuyPrice4 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice4, PRICE_MULTIPLE);
	pMarketData->iBuyPrice5 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice5, PRICE_MULTIPLE);
	pMarketData->iSellQuantity1 = pDepthMarketData->AskVolume1;
	pMarketData->iSellQuantity2 = pDepthMarketData->AskVolume2;
	pMarketData->iSellQuantity3 = pDepthMarketData->AskVolume3;
	pMarketData->iSellQuantity4 = pDepthMarketData->AskVolume4;
	pMarketData->iSellQuantity5 = pDepthMarketData->AskVolume5;
	pMarketData->iBuyQuantity1 = pDepthMarketData->BidVolume1;
	pMarketData->iBuyQuantity2 = pDepthMarketData->BidVolume2;
	pMarketData->iBuyQuantity3 = pDepthMarketData->BidVolume3;
	pMarketData->iBuyQuantity4 = pDepthMarketData->BidVolume4;
	pMarketData->iBuyQuantity5 = pDepthMarketData->BidVolume5;

	//LDebug("InstrumentID=[{0}],old PreSettlementPrice=[{1}],new PreSettlementPrice=[{2}]", pDepthMarketData->InstrumentID, pDepthMarketData->PreSettlementPrice, Snapshot.iPreSettlePrice);

	return 0;
}
static inline char ConvertAction(const TThostFtdcDirectionType cDirection, const TThostFtdcOffsetFlagType cOffsetFlag)
{
	char cAction = ORDER_ACTION_BUY_UNKNOWN;
	if (cDirection == THOST_FTDC_D_Buy)
	{
		switch (cOffsetFlag)
		{
		case THOST_FTDC_OF_Open:
			cAction = ORDER_ACTION_BUY_OPEN;
			break;
		case THOST_FTDC_OF_Close:
		case THOST_FTDC_OF_CloseToday:
		case THOST_FTDC_OF_CloseYesterday:
		case THOST_FTDC_OF_ForceOff:
		case THOST_FTDC_OF_LocalForceClose:
			cAction = ORDER_ACTION_BUY_CLOSE; // TODO: diff today and yesterday
			break;
		default:
			LError("Unkown OffsetFlag:{1}", cOffsetFlag);
			break;
		}
	}
	else if (cDirection == THOST_FTDC_D_Sell)
	{
		switch (cOffsetFlag)
		{
		case THOST_FTDC_OF_Open:
			cAction = ORDER_ACTION_SELL_OPEN;
			break;
		case THOST_FTDC_OF_Close:
		case THOST_FTDC_OF_CloseToday:
		case THOST_FTDC_OF_CloseYesterday:
		case THOST_FTDC_OF_ForceOff:
		case THOST_FTDC_OF_LocalForceClose:
			cAction = ORDER_ACTION_SELL_CLOSE; // TODO: diff today and yesterday
			break;
		default:
			LError("Unkown OffsetFlag:{1}", cOffsetFlag);
			break;
		}
	}
	else
	{
		LError("Unkown Direction:{1}", cDirection);
	}
	return cAction;
}

static inline char ConvertStatus(const CThostFtdcOrderField *pOrder)
{
	char cStatus = ORDER_STATUS_UNKNOWN;

	switch (pOrder->OrderSubmitStatus)
	{
	case THOST_FTDC_OSS_InsertSubmitted:
		cStatus = ORDER_STATUS_SUBMIT;
		LDebug("THOST_FTDC_OSS_InsertSubmitted");
		break;
	case THOST_FTDC_OSS_CancelSubmitted:
		cStatus = ORDER_STATUS_SUBMIT;
		LDebug("THOST_FTDC_OSS_CancelSubmitted");
		break;
	case THOST_FTDC_OSS_ModifySubmitted:
		cStatus = ORDER_STATUS_SUBMIT;
		LDebug("THOST_FTDC_OSS_ModifySubmitted");
		break;
	case THOST_FTDC_OSS_Accepted:
		cStatus = ORDER_STATUS_ACCEPT;
		LDebug("THOST_FTDC_OSS_Accepted");
		break;
	case THOST_FTDC_OSS_InsertRejected:
		cStatus = ORDER_STATUS_INSERT_REFUSE;
		LDebug("THOST_FTDC_OSS_InsertRejected");
		break;
	case THOST_FTDC_OSS_CancelRejected:
		cStatus = ORDER_STATUS_CANCEL_REFUSE;
		LDebug("THOST_FTDC_OSS_CancelRejected");
		break;
	case THOST_FTDC_OSS_ModifyRejected:
		LDebug("THOST_FTDC_OSS_ModifyRejected");
		break;
	}

	switch (pOrder->OrderStatus)
	{
	case THOST_FTDC_OST_AllTraded:
		cStatus = ORDER_STATUS_ALL_TRADE;
		LDebug("THOST_FTDC_OST_AllTraded");
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
		cStatus = ORDER_STATUS_PART_TRADE;
		LDebug("THOST_FTDC_OST_PartTradedQueueing");
		break;
	case THOST_FTDC_OST_PartTradedNotQueueing:
		cStatus = ORDER_STATUS_PART_TRADE;
		LDebug("THOST_FTDC_OST_PartTradedNotQueueing");
		break;
	case THOST_FTDC_OST_NoTradeQueueing:
		LDebug("THOST_FTDC_OST_NoTradeQueueing");
		break;
	case THOST_FTDC_OST_NoTradeNotQueueing:
		LDebug("THOST_FTDC_OST_NoTradeNotQueueing");
		break;
	case THOST_FTDC_OST_Canceled:
		if (cStatus != ORDER_STATUS_INSERT_REFUSE)
		{
			cStatus = ORDER_STATUS_CANCEL;
			LDebug("THOST_FTDC_OST_Canceled--1");
		}
		LDebug("THOST_FTDC_OST_Canceled--2");
		break;
	case THOST_FTDC_OST_Unknown:
		LDebug("THOST_FTDC_OST_Unknown");
		break;
	case THOST_FTDC_OST_NotTouched:
		LDebug("THOST_FTDC_OST_NotTouched");
		break;
	case THOST_FTDC_OST_Touched:
		LDebug("THOST_FTDC_OST_Touched");
		break;
	}

	return cStatus;
}

BalanceT TdAdapterCtp::ConvertBalance(const CThostFtdcTradingAccountField *pAccount)
{
	BalanceT BalanceData;

	BalanceData.szUserId = "";
	BalanceData.dUseMargin = pAccount->PreMargin;
	BalanceData.dFrozenMargin = pAccount->FrozenMargin;
	BalanceData.dFrozenCash = pAccount->FrozenCash;
	BalanceData.dFrozenCommission = pAccount->FrozenCommission;
	BalanceData.dCurrMargin = pAccount->CurrMargin;
	BalanceData.dCommission = pAccount->Commission;
	BalanceData.dAvailable = pAccount->Available;

	BalanceData.iModifyDate = zutil::GetToday();
	BalanceData.iModifyTime = zutil::GetNowSedondTime();

	return BalanceData;
}

OrderT TdAdapterCtp::ConvertOrder(const CThostFtdcOrderField *pOrder)
{
	OrderT OrderData;

	OrderData.iInitDate = zutil::GetToday();
	OrderData.iInitTime = zutil::GetNowSedondTime();
	OrderData.iModifyDate = zutil::GetToday();
	OrderData.iModifyTime = zutil::GetNowSedondTime();
	OrderData.szJysInsertDate = pOrder->InsertDate;
	OrderData.szJysInsertTime = pOrder->InsertTime;
	OrderData.szExchangeID = pOrder->ExchangeID;
	OrderData.szStockCode = pOrder->InstrumentID;
	OrderData.szOrderRef = pOrder->OrderRef;
	OrderData.szOrderSysID = pOrder->OrderSysID;
	OrderData.cAction = ConvertAction(pOrder->Direction, pOrder->CombOffsetFlag[0]);
	OrderData.dPrice = pOrder->LimitPrice;
	OrderData.iEntrustNum = pOrder->VolumeTotalOriginal;
	OrderData.cStatus = ConvertStatus(pOrder);
	OrderData.iTradeNum = pOrder->VolumeTraded;
	OrderData.iFrontID = pOrder->FrontID;
	OrderData.iSessionID = pOrder->SessionID;

	return OrderData;
}

TradeT TdAdapterCtp::ConvertTrade(const CThostFtdcTradeField *pTrade)
{
	TradeT Tradedata;

	Tradedata.iInitDate = zutil::GetToday();
	Tradedata.iInitTime = zutil::GetNowSedondTime();
	Tradedata.szJysTradeDate = pTrade->TradeDate;
	Tradedata.szJysTradeTime = pTrade->TradeTime;
	Tradedata.szExchangeID = pTrade->ExchangeID;
	Tradedata.szStockCode = pTrade->InstrumentID;

	//确定唯一TradeID的3个字段：ExchangeID，买卖方向，TradeID
	char cDirection = 'z';
	if (THOST_FTDC_D_Buy == pTrade->Direction)
	{
		cDirection = TRADE_DIRECTION_BUY;
	}
	else if (THOST_FTDC_D_Sell == pTrade->Direction)
	{
		cDirection = TRADE_DIRECTION_SELL;
	}

	string szTradeID = pTrade->TradeID;
	LDebug("old szTradeID=[{0}]", szTradeID);
	Trim(szTradeID);
	Tradedata.szTradeID = Tradedata.szExchangeID + string("_") + cDirection + string("_") + szTradeID;
	LDebug("new szTradeID=[{0}]", Tradedata.szTradeID);
	Tradedata.szOrderRef = pTrade->OrderRef;
	Tradedata.szOrderSysID = pTrade->OrderSysID;
	Tradedata.cAction = ConvertAction(pTrade->Direction, pTrade->OffsetFlag);
	Tradedata.dPrice = pTrade->Price;
	Tradedata.iTradeNum = pTrade->Volume;

	return Tradedata;
}

PositionT TdAdapterCtp::ConvertPosition(const CThostFtdcInvestorPositionField *pPos)
{
	PositionT PositionData;

	PositionData.iInitDate = zutil::GetToday();
	PositionData.iInitTime = zutil::GetNowSedondTime();
	PositionData.iModifyDate = zutil::GetToday();
	PositionData.iModifyTime = zutil::GetNowSedondTime();
	PositionData.szUserId = "";
	PositionData.szExchangeID = pPos->ExchangeID;
	PositionData.szStockCode = pPos->InstrumentID;

	if (THOST_FTDC_PD_Long == pPos->PosiDirection)
	{
		PositionData.cDirection = POSITION_DIRECTION_LONG;
	}
	else if (THOST_FTDC_PD_Short == pPos->PosiDirection)
	{
		PositionData.cDirection = POSITION_DIRECTION_SHORT;
	}
	else if (THOST_FTDC_PD_Net == pPos->PosiDirection)
	{
		PositionData.cDirection = POSITION_DIRECTION_NET;
	}

	PositionData.cHedgeFlag = pPos->HedgeFlag;
	PositionData.iYdPosition = pPos->YdPosition;
	PositionData.iPosition = pPos->Position;
	PositionData.iTodayPosition = pPos->TodayPosition;
	PositionData.iLongFrozen = pPos->LongFrozen;
	PositionData.iShortFrozen = pPos->ShortFrozen;
	PositionData.dLongFrozenAmount = pPos->LongFrozenAmount;
	PositionData.dShortFrozenAmount = pPos->ShortFrozenAmount;
	PositionData.dUseMargin = pPos->UseMargin;
	PositionData.dFrozenMargin = pPos->FrozenMargin;
	PositionData.dFrozenCash = pPos->FrozenCash;
	PositionData.dFrozenCommission = pPos->FrozenCommission;
	PositionData.dCommission = pPos->Commission;
	PositionData.dPreSettlementPrice = pPos->PreSettlementPrice;
	PositionData.dSettlementPrice = pPos->SettlementPrice;
	PositionData.dMarginRateByMoney = pPos->MarginRateByMoney;
	PositionData.dMarginRateByVolume = pPos->MarginRateByVolume;

	return PositionData;
}
int TdAdapterCtp::GetRequestId()
{
	lock_guard<mutex> lk(m_RequestIdMtx);
	m_iRequestId++;
	return m_iRequestId;
}

TdAdapterCtp::TdAdapterCtp() : m_pTraderApi(nullptr), m_IsLogin(false)
{
	m_iRequestId = 0;
}

TdAdapterCtp::~TdAdapterCtp()
{
	LogOut();
	if (m_pTraderApi)
	{
		m_pTraderApi->Release();
		m_pTraderApi = nullptr;
	}
}

void TdAdapterCtp::Connect(CtpConfig &config)
{
	m_Config = config;

	m_pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	if (!m_pTraderApi)
	{
		LError("ctp create traderapi failed");
		return;
	}

	m_pTraderApi->RegisterSpi(this);
	string szTmpFrontAddr = "tcp://" + m_Config.szTradeIp + ':' + to_string(m_Config.iTradePort);
	printf("Trade FrontAddress=%s\n", szTmpFrontAddr.c_str());
	char szFrontAddr[100] = { 0 };
	sprintf(szFrontAddr, "%s", szTmpFrontAddr.c_str());
	m_pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	m_pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	m_pTraderApi->RegisterFront(szFrontAddr);
	m_pTraderApi->Init(); //调用Init，开始连接

}

void TdAdapterCtp::LogOut()
{
	if (!m_pTraderApi)
	{
		return;
	}

	CThostFtdcUserLogoutField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.UserID, m_Config.szUserId.c_str());

	int iRet = m_pTraderApi->ReqUserLogout(&ReqParam, GetRequestId());
	if (iRet != 0)
	{
		LError("send ctp ReqUserLogout Error,iRet=[{0}]", iRet);
	}
}

bool TdAdapterCtp::IsLogin() const
{
	return m_IsLogin;
}

//查询资金
int TdAdapterCtp::ReqQueryBalance()
{
	LInfo("ctp ReqQueryBalance");

	CThostFtdcQryTradingAccountField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryTradingAccount(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryTradingAccount Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
//查询委托
int TdAdapterCtp::ReqQueryOrder()
{
	LInfo("ctp ReqQueryOrder");

	CThostFtdcQryOrderField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	// query all orders
	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryOrder(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryOrder Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
//查询成交
int TdAdapterCtp::ReqQueryTrade()
{
	LInfo("ctp ReqQueryTrade");

	CThostFtdcQryTradeField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	// query all trades
	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryTrade(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryTrade Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}
	return iRequestId;
}
//查询持仓
int TdAdapterCtp::ReqQueryPosition()
{
	LInfo("ctp ReqQueryPosition");

	CThostFtdcQryInvestorPositionField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	// query all positions
	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryInvestorPosition(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryInvestorPosition Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
//查询合约
int TdAdapterCtp::ReqQryInstrument()
{
	LInfo("ctp ReqQryInstrument");

	CThostFtdcQryInstrumentField Req;
	memset(&Req, 0, sizeof(Req));
	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryInstrument(&Req, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryTrade Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
///请求查询合约保证金率
int TdAdapterCtp::ReqQryInstrumentMarginRate(string szInstrumentId)
{
	LInfo("ctp ReqQryInstrumentMarginRate");

	CThostFtdcQryInstrumentMarginRateField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	strcpy(ReqParam.InstrumentID, szInstrumentId.c_str());
	ReqParam.HedgeFlag = THOST_FTDC_HF_Speculation;

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryInstrumentMarginRate(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryInstrumentMarginRate Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
///请求查询合约手续费率
int TdAdapterCtp::ReqQryInstrumentCommissionRate(string szInstrumentId)
{
	LInfo("ctp ReqQryInstrumentCommissionRate");

	CThostFtdcQryInstrumentCommissionRateField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	strcpy(ReqParam.InstrumentID, szInstrumentId.c_str());

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryInstrumentCommissionRate(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryInstrumentCommissionRate Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
///请求查询报单手续费
int TdAdapterCtp::ReqQryInstrumentOrderCommRate(string szInstrumentId)
{
	LInfo("ctp ReqQryInstrumentOrderCommRate");

	CThostFtdcQryInstrumentOrderCommRateField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	strcpy(ReqParam.InstrumentID, szInstrumentId.c_str());

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryInstrumentOrderCommRate(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryInstrumentOrderCommRate Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
//请求查询经纪公司交易参数
int TdAdapterCtp::ReqQryBrokerTradingParams()
{
	LInfo("ctp ReqQryBrokerTradingParams");

	CThostFtdcQryBrokerTradingParamsField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	strcpy(ReqParam.CurrencyID, "CNY");

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryBrokerTradingParams(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryBrokerTradingParams Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
//请求查询结算信息确认
int TdAdapterCtp::ReqQrySettlementInfoConfirm()
{
	LInfo("ctp ReqQrySettlementInfoConfirm");

	CThostFtdcQrySettlementInfoConfirmField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	//strcpy(ReqParam.CurrencyID, "CNY");

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQrySettlementInfoConfirm(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQrySettlementInfoConfirm Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
///请求查询投资者结算结果
int TdAdapterCtp::ReqQrySettlementInfo()
{
	LInfo("ctp ReqQrySettlementInfo");

	CThostFtdcQrySettlementInfoField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());

	// 不填日期，表示取上一交易日结算单

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQrySettlementInfo(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQrySettlementInfo Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
///投资者结算结果确认
int TdAdapterCtp::ReqSettlementInfoConfirm()
{
	LInfo("ctp ReqSettlementInfoConfirm");

	CThostFtdcSettlementInfoConfirmField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqSettlementInfoConfirm(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqSettlementInfoConfirm Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}

//请求查询行情
int  TdAdapterCtp::ReqQryDepthMarketData(string szInstrumentId, string szExchangeId)
{
	LInfo("ctp ReqQryInstrumentOrderCommRate");

	CThostFtdcQryDepthMarketDataField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));

	if (szExchangeId != "")
	{
		strcpy(ReqParam.ExchangeID, szExchangeId.c_str());
	}
	if (szInstrumentId != "")
	{
		strcpy(ReqParam.InstrumentID, szInstrumentId.c_str());
	}

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqQryDepthMarketData(&ReqParam, iRequestId);
	if (iRet != 0)
	{
		LError("send ctp ReqQryInstrumentOrderCommRate Error,iRet=[{0}]", iRet);
		return RET_SEND_JYS_FAIL;
	}

	return iRequestId;
}
//委托
TdResponse<string> TdAdapterCtp::ReqPlaceOrder(const string& szOrderRef, const string &szStockCode, const char &cAction, const double dPrice, const int iQuantity)
{
	LInfo("ctp ReqPlaceOrder  szOrderRef=[{0}],szStockCode=[{1}],cAction=[{2}],dPrice=[{3}],iQuantity=[{4}]", szOrderRef, szStockCode, cAction, dPrice, iQuantity);

	CThostFtdcInputOrderField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	snprintf(ReqParam.OrderRef, sizeof(ReqParam.OrderRef), "%s", szOrderRef.c_str());
	snprintf(ReqParam.InstrumentID, sizeof(ReqParam.InstrumentID), "%s", szStockCode.c_str());
	ReqParam.LimitPrice = dPrice; // always use limit
	ReqParam.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	ReqParam.TimeCondition = THOST_FTDC_TC_GFD;

	if (cAction == ORDER_ACTION_BUY_OPEN)
	{
		ReqParam.Direction = THOST_FTDC_D_Buy;
		ReqParam.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	}
	else if (cAction == ORDER_ACTION_SELL_OPEN)
	{
		ReqParam.Direction = THOST_FTDC_D_Sell;
		ReqParam.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	}
	else if (cAction == ORDER_ACTION_BUY_CLOSE)
	{
		ReqParam.Direction = THOST_FTDC_D_Buy;
		ReqParam.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
	}
	else if (cAction == ORDER_ACTION_SELL_CLOSE)
	{
		ReqParam.Direction = THOST_FTDC_D_Sell;
		ReqParam.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
	}
	else
	{
		LError("unkown action to ctp");
	}

	// quantity
	ReqParam.VolumeTotalOriginal = iQuantity;
	// other options
	ReqParam.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	ReqParam.VolumeCondition = THOST_FTDC_VC_AV;
	ReqParam.MinVolume = 1;
	ReqParam.ContingentCondition = THOST_FTDC_CC_Immediately;
	ReqParam.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	ReqParam.IsAutoSuspend = 0;
	ReqParam.UserForceClose = 0;

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqOrderInsert(&ReqParam, iRequestId);
	if (0 == iRet)
	{
		LInfo(" send ctp ReqPlaceOrder success");

		TdResponse<string> TdResponseData;
		TdResponseData.RspResult = szOrderRef;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "OK";
		TdResponseData.iRequestId = iRequestId;

		return TdResponseData;
	}
	else
	{
		LError("send ctp ReqPlaceOrder Error,iRet=[{0}]", iRet);

		TdResponse<string> TdResponseData;
		TdResponseData.RspResult = szOrderRef;
		TdResponseData.iErrCode = RET_SEND_JYS_FAIL;
		TdResponseData.szMsg = "send ctp ReqOrderInsert Error";
		TdResponseData.iRequestId = iRequestId;

		return TdResponseData;
	}
}
//撤单
TdResponse<string> TdAdapterCtp::ReqCancelOrder(int iFrontID, int iSessionID, const string& szOrderRef, const string& szInstrumentID)
{
	LInfo("ctp ReqCancelOrder,iFrontID=[{0}],iSessionID=[{1}],szOrderRef=[{2}],szInstrumentID=[{3}]", iFrontID, iSessionID, szOrderRef, szInstrumentID);

	CThostFtdcInputOrderActionField ReqParam;
	memset(&ReqParam, 0, sizeof(ReqParam));
	strcpy(ReqParam.BrokerID, m_Config.szBrokerId.c_str());
	strcpy(ReqParam.InvestorID, m_Config.szUserId.c_str());
	ReqParam.FrontID = iFrontID;
	ReqParam.SessionID = iSessionID;
	strcpy(ReqParam.OrderRef, szOrderRef.c_str());
	strcpy(ReqParam.InstrumentID, szInstrumentID.c_str());

	ReqParam.ActionFlag = THOST_FTDC_AF_Delete;

	int iRequestId = GetRequestId();
	int iRet = m_pTraderApi->ReqOrderAction(&ReqParam, iRequestId);
	if (0 == iRet)
	{
		LInfo("send ctp ReqCancelOrder success");

		TdResponse<string> TdResponseData;
		TdResponseData.RspResult = szOrderRef;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "Ok";
		TdResponseData.iRequestId = iRequestId;

		return TdResponseData;
	}
	else
	{
		LError("send ctp ReqCancelOrder Error,iRet=[{0}]", iRet);

		TdResponse<string> TdResponseData;
		TdResponseData.RspResult = szOrderRef;
		TdResponseData.iErrCode = RET_SEND_JYS_FAIL;
		TdResponseData.szMsg = "send ctp ReqOrderAction Error";
		TdResponseData.iRequestId = iRequestId;

		return TdResponseData;
	}
}

void TdAdapterCtp::OnFrontConnected()
{
	printf("ctp OnFrontConnected\n");
	LInfo("ctp OnFrontConnected");

	CThostFtdcReqAuthenticateField  ReqAuthenticateField;
	memset(ReqAuthenticateField.BrokerID, 0, sizeof(ReqAuthenticateField.BrokerID));
	memset(ReqAuthenticateField.UserID, 0, sizeof(ReqAuthenticateField.UserID));
	memset(ReqAuthenticateField.AuthCode, 0, sizeof(ReqAuthenticateField.AuthCode));
	memset(ReqAuthenticateField.AppID, 0, sizeof(ReqAuthenticateField.AppID));
	sprintf(ReqAuthenticateField.BrokerID, "%s", m_Config.szBrokerId.c_str());
	sprintf(ReqAuthenticateField.UserID, "%s", m_Config.szUserId.c_str());
	sprintf(ReqAuthenticateField.AuthCode, "%s", m_Config.szAuthCode.c_str());
	sprintf(ReqAuthenticateField.AppID, "%s", m_Config.szAppID.c_str());

	int iRet = m_pTraderApi->ReqAuthenticate(&ReqAuthenticateField, GetRequestId());
	if (iRet != 0)
	{
		LError("send ctp  ReqAuthenticate Error ,iRet=[{0}]", iRet);
	}

}

void TdAdapterCtp::OnFrontDisConnected(int nReason)
{
	LError("ctp TdAdapterCtp  OnFrontDisConnected  reason=[{0}]", nReason);
	m_IsLogin = false;
}
//登录响应
void TdAdapterCtp::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (0 == pRspInfo->ErrorID)
	{
		printf("ctp TdAdapterCtp OnRspUserLogin success\n");
		LInfo("ctp TdAdapterCtp OnRspUserLogin success");
		m_IsLogin = true;
	}
	else
	{

		printf("ctp TdAdapterCtp OnRspUserLogin Error,error_msg=[%s]\n", pRspInfo->ErrorMsg);
		LError("ctp TdAdapterCtp OnRspUserLogin  Error, error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

		//首次登录需要修改登录密码
		/*
		CThostFtdcUserPasswordUpdateField UserPasswordUpdate;
		memset(&UserPasswordUpdate, 0, sizeof(UserPasswordUpdate));
		sprintf(UserPasswordUpdate.BrokerID, "%s", m_Config.szBrokerId.c_str());
		sprintf(UserPasswordUpdate.UserID, "%s", m_Config.szUser.c_str());
		sprintf(UserPasswordUpdate.OldPassword, "%s", m_Config.szPassword.c_str());
		sprintf(UserPasswordUpdate.NewPassword, "%s", "simnow!!123456");
		int  iRet = m_pTraderApi->ReqUserPasswordUpdate(&UserPasswordUpdate, GetRequestId());
		if (iRet != 0)
		{
			LError("ctp  send  ReqUserPasswordUpdate fail ,iRet=[{0}]", iRet);
		}
		*/
	}
}
//退出响应
void TdAdapterCtp::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp TdAdapterCtp OnRspUserLogout success ----");
		m_IsLogin = false;
	}
	else
	{
		LError("ctp TdAdapterCtp OnRspUserLogout Error,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}
//查询资金响应
void TdAdapterCtp::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryTradingAccount success ----");

		if (NULL == pTradingAccount)
		{
			LDebug("没有数据");
			return;
		}


		LDebug("Available=[{0}]", pTradingAccount->Available);
		LDebug("Balance=[{0}]", pTradingAccount->Balance);
		LDebug("PreBalance=[{0}]", pTradingAccount->PreBalance);
		LDebug("CurrMargin=[{0}]", pTradingAccount->CurrMargin);
		LDebug("PreMargin=[{0}]", pTradingAccount->PreMargin);
		LDebug("FrozenMargin=[{0}]", pTradingAccount->FrozenMargin);
		LDebug("FrozenCash=[{0}]", pTradingAccount->FrozenCash);
		LDebug("CloseProfit=[{0}]", pTradingAccount->CloseProfit);
		LDebug("PositionProfit=[{0}]", pTradingAccount->PositionProfit);


		BalanceT BalanceData = ConvertBalance(pTradingAccount);

		TdResponse<BalanceT> TdResponseData;
		TdResponseData.RspResult = BalanceData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryBalanceRsp(TdResponseData);
	}
	else
	{
		LError("ctp OnRspQryTradingAccount Error ,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}
//查询委托响应
void TdAdapterCtp::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryOrder success ----");

		if (NULL == pOrder)
		{
			LDebug("没有数据");
			return;
		}

		LDebug("pOrder=[{0}]", pOrder->ExchangeID);
		LDebug("InstrumentID=[{0}]", pOrder->InstrumentID);
		LDebug("OrderRef=[{0}]", pOrder->OrderRef);
		LDebug("Direction=[{0}]", pOrder->Direction);
		LDebug("LimitPrice=[{0}]", pOrder->LimitPrice);
		LDebug("VolumeTotalOriginal=[{0}]", pOrder->VolumeTotalOriginal);
		LDebug("ForceCloseReason=[{0}]", pOrder->ForceCloseReason);
		LDebug("OrderLocalID=[{0}]", pOrder->OrderLocalID);
		LDebug("TradingDay=[{0}]", pOrder->TradingDay);
		LDebug("OrderSysID=[{0}]", pOrder->OrderSysID);
		LDebug("OrderSource=[{0}]", pOrder->OrderSource);
		LDebug("OrderStatus=[{0}]", pOrder->OrderStatus);
		LDebug("VolumeTraded=[{0}]", pOrder->VolumeTraded);
		LDebug("VolumeTotal=[{0}]", pOrder->VolumeTotal);
		LDebug("InsertDate=[{0}]", pOrder->InsertDate);
		LDebug("InsertTime=[{0}]", pOrder->InsertTime);


		OrderT OrderData = ConvertOrder(pOrder);

		TdResponse<OrderT> TdResponseData;
		TdResponseData.RspResult = OrderData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryOrderRsp(TdResponseData);
	}
	else
	{
		LError("ctp OnRspQryOrder Error,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}
//查询成交响应
void TdAdapterCtp::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryTrade success ----");

		if (NULL == pTrade)
		{
			LDebug("没有数据");
			return;
		}

		LDebug("ExchangeID=[{0}]", pTrade->ExchangeID);
		LDebug("InstrumentID=[{0}]", pTrade->InstrumentID);
		LDebug("OrderRef=[{0}]", pTrade->OrderRef);
		LDebug("TradeID=[{0}]", pTrade->TradeID);
		LDebug("Direction=[{0}]", pTrade->Direction);
		LDebug("OrderSysID=[{0}]", pTrade->OrderSysID);
		LDebug("OffsetFlag=[{0}]", pTrade->OffsetFlag);
		LDebug("Price=[{0}]", pTrade->Price);
		LDebug("Volume=[{0}]", pTrade->Volume);
		LDebug("TradeDate=[{0}]", pTrade->TradeDate);
		LDebug("TradeTime=[{0}]", pTrade->TradeTime);
		LDebug("OrderLocalID=[{0}]", pTrade->OrderLocalID);

		TradeT TradeData = ConvertTrade(pTrade);

		TdResponse<TradeT> TdResponseData;
		TdResponseData.RspResult = TradeData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryTradeRsp(TdResponseData);
	}
	else
	{
		LError("ctp OnRspQryTrade Error ,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}
//查询持仓响应
void TdAdapterCtp::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryInvestorPosition success ----");

		if (NULL == pInvestorPosition)
		{
			LDebug("没有数据");
			return;
		}

		LDebug("ExchangeID=[{0}]", pInvestorPosition->ExchangeID);
		LDebug("InstrumentID=[{0}]", pInvestorPosition->InstrumentID);
		LDebug("PosiDirection=[{0}]", pInvestorPosition->PosiDirection);
		LDebug("HedgeFlag=[{0}]", pInvestorPosition->HedgeFlag);
		LDebug("PositionDate=[{0}]", pInvestorPosition->PositionDate);
		LDebug("YdPosition=[{0}]", pInvestorPosition->YdPosition);
		LDebug("Position=[{0}]", pInvestorPosition->Position);
		LDebug("LongFrozen=[{0}]", pInvestorPosition->LongFrozen);
		LDebug("ShortFrozen=[{0}]", pInvestorPosition->ShortFrozen);
		LDebug("LongFrozenAmount=[{0}]", pInvestorPosition->LongFrozenAmount);
		LDebug("ShortFrozenAmount=[{0}]", pInvestorPosition->ShortFrozenAmount);
		LDebug("OpenVolume=[{0}]", pInvestorPosition->OpenVolume);
		LDebug("CloseVolume=[{0}]", pInvestorPosition->CloseVolume);
		LDebug("OpenAmount=[{0}]", pInvestorPosition->OpenAmount);
		LDebug("CloseAmount=[{0}]", pInvestorPosition->CloseAmount);
		LDebug("PositionCost=[{0}]", pInvestorPosition->PositionCost);
		LDebug("PreMargin=[{0}]", pInvestorPosition->PreMargin);
		LDebug("UseMargin=[{0}]", pInvestorPosition->UseMargin);
		LDebug("FrozenMargin=[{0}]", pInvestorPosition->FrozenMargin);
		LDebug("FrozenCash=[{0}]", pInvestorPosition->FrozenCash);
		LDebug("FrozenCommission=[{0}]", pInvestorPosition->FrozenCommission);
		LDebug("Commission=[{0}]", pInvestorPosition->Commission);
		LDebug("CashIn=[{0}]", pInvestorPosition->CashIn);
		LDebug("CloseProfit=[{0}]", pInvestorPosition->CloseProfit);
		LDebug("PositionProfit=[{0}]", pInvestorPosition->PositionProfit);
		LDebug("PreSettlementPrice=[{0}]", pInvestorPosition->PreSettlementPrice);
		LDebug("SettlementPrice=[{0}]", pInvestorPosition->SettlementPrice);
		LDebug("TradingDay=[{0}]", pInvestorPosition->TradingDay);
		LDebug("SettlementID=[{0}]", pInvestorPosition->SettlementID);
		LDebug("OpenCost=[{0}]", pInvestorPosition->OpenCost);
		LDebug("ExchangeMargin=[{0}]", pInvestorPosition->ExchangeMargin);
		LDebug("CloseProfitByDate=[{0}]", pInvestorPosition->CloseProfitByDate);
		LDebug("CloseProfitByTrade=[{0}]", pInvestorPosition->CloseProfitByTrade);
		LDebug("TodayPosition=[{0}]", pInvestorPosition->TodayPosition);
		LDebug("MarginRateByMoney=[{0}]", pInvestorPosition->MarginRateByMoney);
		LDebug("MarginRateByVolume=[{0}]", pInvestorPosition->MarginRateByVolume);
		LDebug("StrikeFrozenAmount=[{0}]", pInvestorPosition->StrikeFrozenAmount);
		LDebug("AbandonFrozen=[{0}]", pInvestorPosition->AbandonFrozen);
		LDebug("YdStrikeFrozen=[{0}]", pInvestorPosition->YdStrikeFrozen);
		LDebug("InvestUnitID=[{0}]", pInvestorPosition->InvestUnitID);

		PositionT PositionData = ConvertPosition(pInvestorPosition);

		TdResponse<PositionT> TdResponseData;
		TdResponseData.RspResult = PositionData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryPositionRsp(TdResponseData);
	}
	else
	{
		LError("ctp OnRspQryInvestorPosition Error,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}

///请求查询合约响应
void TdAdapterCtp::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryInstrument success ----");

		if (NULL == pInstrument)
		{
			LDebug("没有数据");
			return;
		}

		/*
		LDebug("InstrumentID=[{0}]", pInstrument->InstrumentID);
		LDebug("ExchangeID=[{0}]", pInstrument->ExchangeID);
		LDebug("InstrumentName=[{0}]", pInstrument->InstrumentName);
		LDebug("ExchangeInstID=[{0}]", pInstrument->ExchangeInstID);
		LDebug("ProductID=[{0}]", pInstrument->ProductID);
		LDebug("ProductClass=[{0}]", pInstrument->ProductClass);
		LDebug("DeliveryYear=[{0}]", pInstrument->DeliveryYear);
		LDebug("DeliveryMonth=[{0}]", pInstrument->DeliveryMonth);

		LDebug("IsTrading=[{0}]", pInstrument->IsTrading);
		LDebug("PriceTick=[{0}]", pInstrument->PriceTick);
		LDebug("StrikePrice=[{0}]", pInstrument->StrikePrice);
		*/

		InstrumentT InstrumentData;
		InstrumentData.szExchangeId = pInstrument->ExchangeID;
		InstrumentData.szInstrumentId = pInstrument->InstrumentID;
		InstrumentData.szInstrumentName = pInstrument->InstrumentName;
		InstrumentData.szProductId = pInstrument->ProductID;
		InstrumentData.szProductClass = pInstrument->ProductClass;
		InstrumentData.iDeliveryYear = pInstrument->DeliveryYear;
		InstrumentData.iDeliveryMonth = pInstrument->DeliveryMonth;
		InstrumentData.iMaxMarketOrderVolume = pInstrument->MaxMarketOrderVolume;
		InstrumentData.iMinMarketOrderVolume = pInstrument->MinMarketOrderVolume;
		InstrumentData.iMaxLimitOrderVolume = pInstrument->MaxLimitOrderVolume;
		InstrumentData.iMinLimitOrderVolume = pInstrument->MinLimitOrderVolume;
		InstrumentData.iVolumeMultiple = pInstrument->VolumeMultiple;
		InstrumentData.dPriceTick = pInstrument->PriceTick;
		InstrumentData.szExpireDate = pInstrument->ExpireDate;
		InstrumentData.iIsTrading = pInstrument->IsTrading;
		InstrumentData.szPositionType = pInstrument->PositionType;
		InstrumentData.szPositionDateType = pInstrument->PositionDateType;
		InstrumentData.dLongMarginRatio = pInstrument->LongMarginRatio;
		InstrumentData.dShortMarginRatio = pInstrument->ShortMarginRatio;
		InstrumentData.szMaxMarginSideAlgorithm = pInstrument->MaxMarginSideAlgorithm;
		InstrumentData.dStrikePrice = pInstrument->StrikePrice;
		InstrumentData.cOptionsType = pInstrument->OptionsType;
		InstrumentData.dUnderlyingMultiple = pInstrument->UnderlyingMultiple;


		TdResponse<InstrumentT> TdResponseData;
		TdResponseData.RspResult = InstrumentData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryInstrumentRsp(TdResponseData);

	}
	else
	{

		LError("ctp OnRspQryInstrument Error,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

	}
}
///客户端认证响应
void TdAdapterCtp::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		printf("ctp OnRspAuthenticate OK\n");
		LInfo("ctp  OnRspAuthenticate   OK");

		CThostFtdcReqUserLoginField ReqUserLoginField;
		memset(ReqUserLoginField.BrokerID, 0, sizeof(ReqUserLoginField.BrokerID));
		memset(ReqUserLoginField.UserID, 0, sizeof(ReqUserLoginField.UserID));
		memset(ReqUserLoginField.Password, 0, sizeof(ReqUserLoginField.Password));
		sprintf(ReqUserLoginField.BrokerID, "%s", m_Config.szBrokerId.c_str());
		sprintf(ReqUserLoginField.UserID, "%s", m_Config.szUserId.c_str());
		sprintf(ReqUserLoginField.Password, "%s", m_Config.szPassword.c_str());

		int  iRet = m_pTraderApi->ReqUserLogin(&ReqUserLoginField, GetRequestId());
		if (iRet != 0)
		{
			LError("ctp  send  ReqUserLogin fail ,iRet=[{0}]", iRet);
		}
	}
	else
	{
		printf("ctp OnRspAuthenticate error, ErrorMsg=[%s]\n", pRspInfo->ErrorMsg);
		LError("ctp OnRspAuthenticate  ErrorID=[{0},ErrorMsg=[{1}]]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}

}
///用户口令更新请求响应
void TdAdapterCtp::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp  OnRspUserPasswordUpdate  OK");
	}
	else
	{
		LError("ctp OnRspUserPasswordUpdate  Error,ErrorMsg=[{0}]", pRspInfo->ErrorMsg);
	}
};


///请求查询合约保证金率响应
void TdAdapterCtp::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryInstrumentMarginRate success ----");

		if (NULL == pInstrumentMarginRate)
		{
			LDebug("没有数据");
			return;
		}
		if (pInstrumentMarginRate->HedgeFlag != THOST_FTDC_HF_Speculation)
		{
			return;
		}

		/*
		LDebug("ExchangeID=[{0}]", pInstrumentMarginRate->ExchangeID);
		LDebug("InstrumentID=[{0}]", pInstrumentMarginRate->InstrumentID);
		LDebug("HedgeFlag=[{0}]", pInstrumentMarginRate->HedgeFlag);
		LDebug("LongMarginRatioByMoney=[{0}]", pInstrumentMarginRate->LongMarginRatioByMoney);
		LDebug("LongMarginRatioByVolume=[{0}]", pInstrumentMarginRate->LongMarginRatioByVolume);
		LDebug("ShortMarginRatioByMoney=[{0}]", pInstrumentMarginRate->ShortMarginRatioByMoney);
		LDebug("ShortMarginRatioByVolume=[{0}]", pInstrumentMarginRate->ShortMarginRatioByVolume);
		LDebug("IsRelative=[{0}]", pInstrumentMarginRate->IsRelative);
		*/


		InstrumentMarginRate InstrumentMarginRateData;
		InstrumentMarginRateData.szExchangeId = pInstrumentMarginRate->ExchangeID;
		InstrumentMarginRateData.szInstrumentId = pInstrumentMarginRate->InstrumentID;
		InstrumentMarginRateData.cHedgeFlag = pInstrumentMarginRate->HedgeFlag;
		InstrumentMarginRateData.dLongMarginRatioByMoney = pInstrumentMarginRate->LongMarginRatioByMoney;
		InstrumentMarginRateData.dLongMarginRatioByVolume = pInstrumentMarginRate->LongMarginRatioByVolume;
		InstrumentMarginRateData.dShortMarginRatioByMoney = pInstrumentMarginRate->ShortMarginRatioByMoney;
		InstrumentMarginRateData.dShortMarginRatioByVolume = pInstrumentMarginRate->ShortMarginRatioByVolume;
		InstrumentMarginRateData.iIsRelative = pInstrumentMarginRate->IsRelative;


		TdResponse<InstrumentMarginRate> TdResponseData;
		TdResponseData.RspResult = InstrumentMarginRateData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryInstrumentMarginRateRsp(TdResponseData);
	}
	else
	{
		LError("ctp OnRspQryInstrumentMarginRate Error ,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}
///请求查询合约手续费率响应
void TdAdapterCtp::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryInstrumentCommissionRate success ----");

		if (NULL == pInstrumentCommissionRate)
		{
			LDebug("没有数据");
			return;
		}

		/*
		LDebug("ExchangeID=[{0}]", pInstrumentCommissionRate->ExchangeID);
		LDebug("InstrumentID=[{0}]", pInstrumentCommissionRate->InstrumentID);
		LDebug("OpenRatioByMoney=[{0}]", pInstrumentCommissionRate->OpenRatioByMoney);
		LDebug("OpenRatioByVolume=[{0}]", pInstrumentCommissionRate->OpenRatioByVolume);
		LDebug("CloseRatioByMoney=[{0}]", pInstrumentCommissionRate->CloseRatioByMoney);
		LDebug("CloseRatioByVolume=[{0}]", pInstrumentCommissionRate->CloseRatioByVolume);
		LDebug("CloseTodayRatioByMoney=[{0}]", pInstrumentCommissionRate->CloseTodayRatioByMoney);
		LDebug("CloseTodayRatioByVolume=[{0}]", pInstrumentCommissionRate->CloseTodayRatioByVolume);
		LDebug("BizType=[{0}]", pInstrumentCommissionRate->BizType);
		*/

		InstrumentCommissionRate InstrumentCommissionRateData;
		InstrumentCommissionRateData.szExchangeId = pInstrumentCommissionRate->ExchangeID;
		InstrumentCommissionRateData.szInstrumentId = pInstrumentCommissionRate->InstrumentID;
		InstrumentCommissionRateData.dOpenRatioByMoney = pInstrumentCommissionRate->OpenRatioByMoney;
		InstrumentCommissionRateData.dOpenRatioByVolume = pInstrumentCommissionRate->OpenRatioByVolume;
		InstrumentCommissionRateData.dCloseRatioByMoney = pInstrumentCommissionRate->CloseRatioByMoney;
		InstrumentCommissionRateData.dCloseRatioByVolume = pInstrumentCommissionRate->CloseRatioByVolume;
		InstrumentCommissionRateData.dCloseTodayRatioByMoney = pInstrumentCommissionRate->CloseTodayRatioByMoney;
		InstrumentCommissionRateData.dCloseTodayRatioByVolume = pInstrumentCommissionRate->CloseTodayRatioByVolume;
		InstrumentCommissionRateData.cBizType = pInstrumentCommissionRate->BizType;


		TdResponse<InstrumentCommissionRate> TdResponseData;
		TdResponseData.RspResult = InstrumentCommissionRateData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryInstrumentCommissionRateRsp(TdResponseData);
	}
	else
	{

		LError("ctp OnRspQryInstrumentCommissionRate Error ,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

	}
}
///请求查询报单手续费响应
void TdAdapterCtp::OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryInstrumentOrderCommRate success ----");

		if (NULL == pInstrumentOrderCommRate)
		{
			LDebug("没有数据");
			return;
		}

		if (pInstrumentOrderCommRate->HedgeFlag != THOST_FTDC_HF_Speculation)
		{
			return;
		}

		/*
		LDebug("ExchangeID=[{0}]", pInstrumentOrderCommRate->ExchangeID);
		LDebug("InstrumentID=[{0}]", pInstrumentOrderCommRate->InstrumentID);
		LDebug("HedgeFlag=[{0}]", pInstrumentOrderCommRate->HedgeFlag);
		LDebug("OrderCommByVolume=[{0}]", pInstrumentOrderCommRate->OrderCommByVolume);
		LDebug("OrderActionCommByVolume=[{0}]", pInstrumentOrderCommRate->OrderActionCommByVolume);
		*/


		InstrumentOrderCommRate InstrumentOrderCommRateData;
		InstrumentOrderCommRateData.szExchangeId = pInstrumentOrderCommRate->ExchangeID;
		InstrumentOrderCommRateData.szInstrumentId = pInstrumentOrderCommRate->InstrumentID;
		InstrumentOrderCommRateData.cHedgeFlag = pInstrumentOrderCommRate->HedgeFlag;
		InstrumentOrderCommRateData.dOrderCommByVolume = pInstrumentOrderCommRate->OrderCommByVolume;
		InstrumentOrderCommRateData.dOrderActionCommByVolume = pInstrumentOrderCommRate->OrderActionCommByVolume;

		TdResponse<InstrumentOrderCommRate> TdResponseData;
		TdResponseData.RspResult = InstrumentOrderCommRateData;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryInstrumentOrderCommRateRsp(TdResponseData);
	}
	else
	{
		LError("ctp OnRspQryInstrumentOrderCommRate Error ,error_code=[{0}],error_msg=[{1}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

	}
}
///请求查询经纪公司交易参数响应
void TdAdapterCtp::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryBrokerTradingParams success ----");

		if (NULL == pBrokerTradingParams)
		{
			LDebug("没有数据");
			return;
		}

		LDebug("MarginPriceType=[{0}]", pBrokerTradingParams->MarginPriceType);
		LDebug("Algorithm=[{0}]", pBrokerTradingParams->Algorithm);
		LDebug("AvailIncludeCloseProfit=[{0}]", pBrokerTradingParams->AvailIncludeCloseProfit);
		LDebug("CurrencyID=[{0}]", pBrokerTradingParams->CurrencyID);


	}
}
///请求查询结算信息确认响应
void TdAdapterCtp::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQrySettlementInfoConfirm success ----");

		if (NULL == pSettlementInfoConfirm)
		{
			LDebug("没有数据");
			ReqSettlementInfoConfirm();
			return;
		}

		LDebug("InvestorID=[{0}]", pSettlementInfoConfirm->InvestorID);
		LDebug("ConfirmDate=[{0}]", pSettlementInfoConfirm->ConfirmDate);
		LDebug("SettlementID=[{0}]", pSettlementInfoConfirm->SettlementID);
		LDebug("AccountID=[{0}]", pSettlementInfoConfirm->AccountID);

	}
}
///请求查询投资者结算结果响应
void TdAdapterCtp::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQrySettlementInfo success ----");

		if (NULL == pSettlementInfo)
		{
			LDebug("没有数据");
			return;
		}

		LDebug("InvestorID=[{0}]", pSettlementInfo->InvestorID);
		LDebug("TradingDay=[{0}]", pSettlementInfo->TradingDay);
		LDebug("SettlementID=[{0}]", pSettlementInfo->SettlementID);
		LDebug("AccountID=[{0}]", pSettlementInfo->AccountID);

	}
}

///投资者结算结果确认响应
void TdAdapterCtp::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspSettlementInfoConfirm success ----");

		if (NULL == pSettlementInfoConfirm)
		{
			LDebug("没有数据");
			return;
		}

		LDebug("InvestorID=[{0}]", pSettlementInfoConfirm->InvestorID);
		LDebug("ConfirmDate=[{0}]", pSettlementInfoConfirm->ConfirmDate);
		LDebug("SettlementID=[{0}]", pSettlementInfoConfirm->SettlementID);
		LDebug("AccountID=[{0}]", pSettlementInfoConfirm->AccountID);

	}
}


///请求查询行情响应
void TdAdapterCtp::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || 0 == pRspInfo->ErrorID)
	{
		LInfo("---- ctp OnRspQryDepthMarketData success ----");

		if (NULL == pDepthMarketData)
		{
			LDebug("没有数据");
			return;
		}

		MarketData MarketDataSnapshot;
		ConvertMarketData(pDepthMarketData, MarketDataSnapshot);

		TdResponse<MarketData> TdResponseData;
		TdResponseData.RspResult = MarketDataSnapshot;
		TdResponseData.iErrCode = 0;
		TdResponseData.szMsg = "";
		TdResponseData.bIsLast = bIsLast;
		TdResponseData.iRequestId = nRequestID;
		ExecuteEngine::GetInstance()->HandleQueryDepthMarketDataRsp(TdResponseData);

	}
}

//委托回报
void TdAdapterCtp::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	LDebug("--------ctp OnRtnOrder-------------");

	LDebug("pOrder->BrokerID=[{0}]", pOrder->BrokerID);
	LDebug("pOrder->InvestorID=[{0}]", pOrder->InvestorID);
	LDebug("pOrder->InstrumentID=[{0}]", pOrder->InstrumentID);
	LDebug("pOrder->UserID=[{0}]", pOrder->UserID);
	LDebug("pOrder->Direction=[{0}]", pOrder->Direction);
	LDebug("pOrder->LimitPrice=[{0}]", pOrder->LimitPrice);
	LDebug("pOrder->VolumeTotalOriginal=[{0}]", pOrder->VolumeTotalOriginal);
	LDebug("pOrder->ClientID=[{0}]", pOrder->ClientID);
	LDebug("pOrder->ExchangeID=[{0}]", pOrder->ExchangeID);
	LDebug("pOrder->ExchangeInstID=[{0}]", pOrder->ExchangeInstID);
	LDebug("pOrder->OrderLocalID=[{0}]", pOrder->OrderLocalID);
	LDebug("pOrder->OrderRef=[{0}]", pOrder->OrderRef);
	LDebug("pOrder->OrderSysID=[{0}]", pOrder->OrderSysID);
	LDebug("pOrder->OrderStatus=[{0}]", pOrder->OrderStatus);
	LDebug("pOrder->StatusMsg=[{0}]", pOrder->StatusMsg);

	if (THOST_FTDC_OST_AllTraded == pOrder->OrderStatus)
	{
		LDebug("全部成交");
	}
	else if (THOST_FTDC_OST_PartTradedQueueing == pOrder->OrderStatus)
	{
		LDebug("部分成交还在队列中");
	}
	else if (THOST_FTDC_OST_PartTradedNotQueueing == pOrder->OrderStatus)
	{
		LDebug("部分成交不在队列中");
	}
	else if (THOST_FTDC_OST_NoTradeQueueing == pOrder->OrderStatus)
	{
		LDebug("未成交还在队列中");
	}
	else if (THOST_FTDC_OST_NoTradeNotQueueing == pOrder->OrderStatus)
	{
		LDebug("未成交不在队列中");
	}
	else if (THOST_FTDC_OST_Canceled == pOrder->OrderStatus)
	{
		LDebug("撤单");
	}
	else if (THOST_FTDC_OST_Unknown == pOrder->OrderStatus)
	{
		LDebug("未知");
	}
	else if (THOST_FTDC_OST_NotTouched == pOrder->OrderStatus)
	{
		LDebug("尚未触发");
	}
	else if (THOST_FTDC_OST_Touched == pOrder->OrderStatus)
	{
		LDebug("已触发");
	}

	LDebug("pOrder->OrderSubmitStatus=[{0}]", pOrder->OrderSubmitStatus);
	if (THOST_FTDC_OSS_InsertSubmitted == pOrder->OrderSubmitStatus)
	{
		LDebug("已经提交");
	}
	else if (THOST_FTDC_OSS_CancelSubmitted == pOrder->OrderSubmitStatus)
	{
		LDebug("撤单已经提交");
	}
	else if (THOST_FTDC_OSS_ModifySubmitted == pOrder->OrderSubmitStatus)
	{
		LDebug("修改已经提交");
	}
	else if (THOST_FTDC_OSS_Accepted == pOrder->OrderSubmitStatus)
	{
		LDebug("已经接受");
	}
	else if (THOST_FTDC_OSS_InsertRejected == pOrder->OrderSubmitStatus)
	{
		LDebug("报单已经被拒绝");
	}
	else if (THOST_FTDC_OSS_CancelRejected == pOrder->OrderSubmitStatus)
	{
		LDebug("撤单已经被拒绝");
	}
	else if (THOST_FTDC_OSS_ModifyRejected == pOrder->OrderSubmitStatus)
	{
		LDebug("改单已经被拒绝");
	}
	LDebug("pOrder->VolumeTraded=[{0}]", pOrder->VolumeTraded);
	LDebug("pOrder->VolumeTotal=[{0}]", pOrder->VolumeTotal);
	LDebug("pOrder->AccountID=[{0}]", pOrder->AccountID);
	LDebug("pOrder->RequestID=[{0}]", pOrder->RequestID);


	OrderT OrderData = ConvertOrder(pOrder);
	string szUserId = OrderData.szOrderRef.substr(0, VIR_ACCOUNT_LENGTH);
	LDebug("szUserId=[{0}]", szUserId);
	OrderData.szUserId = szUserId;
	TdResponse<OrderT> TdResponseData;
	TdResponseData.RspResult = OrderData;
	TdResponseData.iErrCode = 0;
	TdResponseData.szMsg = "";
	TdResponseData.iRequestId = pOrder->RequestID;
	ExecuteEngine::GetInstance()->HandleOrderNotify(TdResponseData);
}
//成交回报
void TdAdapterCtp::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	LDebug("-----------ctp OnRtnTrade------------");
	LDebug("pTrade->BrokerID=[{0}]", pTrade->BrokerID);
	LDebug("pTrade->InvestorID=[{0}]", pTrade->InvestorID);
	LDebug("pTrade->InstrumentID=[{0}]", pTrade->InstrumentID);
	LDebug("pTrade->OrderRef=[{0}]", pTrade->OrderRef);
	LDebug("pTrade->UserID=[{0}]", pTrade->UserID);
	LDebug("pTrade->ExchangeID=[{0}]", pTrade->ExchangeID);
	LDebug("pTrade->Direction=[{0}]", pTrade->Direction);
	LDebug("pTrade->OrderSysID=[{0}]", pTrade->OrderSysID);
	LDebug("pTrade->ClientID=[{0}]", pTrade->ClientID);
	LDebug("pTrade->ExchangeInstID=[{0}]", pTrade->ExchangeInstID);
	LDebug("pTrade->OffsetFlag=[{0}]", pTrade->OffsetFlag);
	LDebug("pTrade->TradeID=[{0}]", pTrade->TradeID);
	LDebug("pTrade->Price=[{0}]", pTrade->Price);
	LDebug("pTrade->Volume=[{0}]", pTrade->Volume);
	LDebug("pTrade->OrderLocalID=[{0}]", pTrade->OrderLocalID);


	TradeT TradeData = ConvertTrade(pTrade);

	string szUserId = TradeData.szOrderRef.substr(0, VIR_ACCOUNT_LENGTH);
	LDebug("szUserId=[{0}]", szUserId);
	TradeData.szUserId = szUserId;

	// notify trade
	TdResponse<TradeT> TdResponseData;
	TdResponseData.RspResult = TradeData;
	TdResponseData.iErrCode = 0;
	TdResponseData.szMsg = "";
	TdResponseData.iRequestId = 0;
	ExecuteEngine::GetInstance()->HandleTradeNotify(TdResponseData);
}
///错误应答
void TdAdapterCtp::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp  OnRspError  OK");
	}
	else
	{
		LError("ctp OnRspError  Error,ErrorMsg=[{0}]", pRspInfo->ErrorMsg);
	}

}
///报单录入错误回报
void TdAdapterCtp::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp  OnErrRtnOrderInsert  OK");
	}
	else
	{
		LError("ctp OnErrRtnOrderInsert  Error,ErrorMsg=[{0}]", pRspInfo->ErrorMsg);

		if (pInputOrder != NULL)
		{
			LDebug("----------------OnErrRtnOrderInsert---------------");
			LDebug("OrderRef=[{0}]", pInputOrder->OrderRef);
			LDebug("ExchangeID=[{0}]", pInputOrder->ExchangeID);
			LDebug("InstrumentID=[{0}]", pInputOrder->InstrumentID);

			string szOrderRef = pInputOrder->OrderRef;
			string szUserId = szOrderRef.substr(0, VIR_ACCOUNT_LENGTH);
			LDebug("szUserId=[{0}]", szUserId);
			int iErrorCode = pRspInfo->ErrorID;
			string szErrorMsg = pRspInfo->ErrorMsg;

			PlaceOrderErrNotifyT  NotifyData;
			snprintf(NotifyData.szOrderRef, sizeof(NotifyData.szOrderRef), "%s", szOrderRef.c_str());
			snprintf(NotifyData.szUserId, sizeof(NotifyData.szUserId), "%s", szUserId.c_str());
			NotifyData.iErrorCode = iErrorCode;
			snprintf(NotifyData.szErrorInfo, sizeof(NotifyData.szErrorInfo), "%s", szErrorMsg.c_str());

			TdResponse<PlaceOrderErrNotifyT> TdResponseData;
			TdResponseData.RspResult = NotifyData;
			TdResponseData.iErrCode = iErrorCode;
			TdResponseData.szMsg = szErrorMsg;
			TdResponseData.iRequestId = pInputOrder->RequestID;

			ExecuteEngine::GetInstance()->HandlePlaceOrderErrNotify(TdResponseData);
		}

	}
}
///报单录入请求响应
void TdAdapterCtp::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp  OnRspOrderInsert  OK");
	}
	else
	{
		LError("ctp OnRspOrderInsert  Error,ErrorMsg=[{0}]", pRspInfo->ErrorMsg);

		if (pInputOrder != NULL)
		{
			LDebug("----------------OnRspOrderInsert---------------");
			LDebug("OrderRef=[{0}]", pInputOrder->OrderRef);
			LDebug("ExchangeID=[{0}]", pInputOrder->ExchangeID);
			LDebug("InstrumentID=[{0}]", pInputOrder->InstrumentID);
			LDebug("pInputOrder->RequestID=[{0}]", pInputOrder->RequestID);

			string szOrderRef = pInputOrder->OrderRef;
			string szUserId = szOrderRef.substr(0, VIR_ACCOUNT_LENGTH);
			LDebug("szUserId=[{0}]", szUserId);
			int iErrorCode = pRspInfo->ErrorID;
			string szErrorMsg = pRspInfo->ErrorMsg;

			PlaceOrderErrNotifyT  NotifyData;
			snprintf(NotifyData.szOrderRef, sizeof(NotifyData.szOrderRef), "%s", szOrderRef.c_str());
			snprintf(NotifyData.szUserId, sizeof(NotifyData.szUserId), "%s", szUserId.c_str());
			NotifyData.iErrorCode = iErrorCode;
			snprintf(NotifyData.szErrorInfo, sizeof(NotifyData.szErrorInfo), "%s", szErrorMsg.c_str());

			TdResponse<PlaceOrderErrNotifyT> TdResponseData;
			TdResponseData.RspResult = NotifyData;
			TdResponseData.iErrCode = iErrorCode;
			TdResponseData.szMsg = szErrorMsg;
			TdResponseData.iRequestId = pInputOrder->RequestID;

			ExecuteEngine::GetInstance()->HandlePlaceOrderErrNotify(TdResponseData);
		}
	}


}
///报单操作错误回报
void TdAdapterCtp::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp  OnErrRtnOrderAction  OK");
	}
	else
	{
		LError("ctp OnErrRtnOrderAction  Error,ErrorMsg=[{0}]", pRspInfo->ErrorMsg);

		if (pOrderAction != NULL)
		{
			LDebug("----------------OnErrRtnOrderAction---------------");
			LDebug("OrderRef=[{0}]", pOrderAction->OrderRef);
			LDebug("OrderActionRef=[{0}]", pOrderAction->OrderActionRef);
			LDebug("ExchangeID=[{0}]", pOrderAction->ExchangeID);
			LDebug("InstrumentID=[{0}]", pOrderAction->InstrumentID);
			LDebug("ActionFlag=[{0}]", pOrderAction->ActionFlag);
			LDebug("OrderSysID=[{0}]", pOrderAction->OrderSysID);
			LDebug("RequestID=[{0}]", pOrderAction->RequestID);

			string szOrderRef = pOrderAction->OrderRef;
			string szUserId = szOrderRef.substr(0, VIR_ACCOUNT_LENGTH);
			LDebug("szUserId=[{0}]", szUserId);
			int iErrorCode = pRspInfo->ErrorID;
			string szErrorMsg = pRspInfo->ErrorMsg;

			CancelOrderErrNotifyT  NotifyData;
			snprintf(NotifyData.szOrderRef, sizeof(NotifyData.szOrderRef), "%s", szOrderRef.c_str());
			snprintf(NotifyData.szUserId, sizeof(NotifyData.szUserId), "%s", szUserId.c_str());
			NotifyData.iErrorCode = iErrorCode;
			snprintf(NotifyData.szErrorInfo, sizeof(NotifyData.szErrorInfo), "%s", szErrorMsg.c_str());

			TdResponse<CancelOrderErrNotifyT> TdResponseData;
			TdResponseData.RspResult = NotifyData;
			TdResponseData.iErrCode = iErrorCode;
			TdResponseData.szMsg = szErrorMsg;
			TdResponseData.iRequestId = pOrderAction->RequestID;

			ExecuteEngine::GetInstance()->HandleCancelOrderErrNotify(TdResponseData);
		}
	}


}
///报单操作请求响应，撤单发生错误时，会调用此函数
void TdAdapterCtp::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!pRspInfo || pRspInfo->ErrorID == 0)
	{
		LInfo("ctp  OnRspOrderAction  OK");
	}
	else
	{
		LError("ctp OnRspOrderAction  Error,ErrorMsg=[{0}]", pRspInfo->ErrorMsg);

		if (pInputOrderAction != NULL)
		{
			LDebug("----------------OnRspOrderAction---------------");
			LDebug("OrderRef=[{0}]", pInputOrderAction->OrderRef);
			LDebug("OrderActionRef=[{0}]", pInputOrderAction->OrderActionRef);
			LDebug("ExchangeID=[{0}]", pInputOrderAction->ExchangeID);
			LDebug("InstrumentID=[{0}]", pInputOrderAction->InstrumentID);
			LDebug("ActionFlag=[{0}]", pInputOrderAction->ActionFlag);
			LDebug("OrderSysID=[{0}]", pInputOrderAction->OrderSysID);

			string szOrderRef = pInputOrderAction->OrderRef;
			string szUserId = szOrderRef.substr(0, VIR_ACCOUNT_LENGTH);
			LDebug("szUserId=[{0}]", szUserId);
			int iErrorCode = pRspInfo->ErrorID;
			string szErrorMsg = pRspInfo->ErrorMsg;

			CancelOrderErrNotifyT  NotifyData;
			snprintf(NotifyData.szOrderRef, sizeof(NotifyData.szOrderRef), "%s", szOrderRef.c_str());
			snprintf(NotifyData.szUserId, sizeof(NotifyData.szUserId), "%s", szUserId.c_str());
			NotifyData.iErrorCode = iErrorCode;
			snprintf(NotifyData.szErrorInfo, sizeof(NotifyData.szErrorInfo), "%s", szErrorMsg.c_str());

			TdResponse<CancelOrderErrNotifyT> TdResponseData;
			TdResponseData.RspResult = NotifyData;
			TdResponseData.iErrCode = iErrorCode;
			TdResponseData.szMsg = szErrorMsg;
			TdResponseData.iRequestId = pInputOrderAction->RequestID;

			ExecuteEngine::GetInstance()->HandleCancelOrderErrNotify(TdResponseData);
		}
	}


}




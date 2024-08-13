#include "execute_engine.h"
#include "helper_time.h"
#include "helper_tools.h"
#include "sim_log.h"
#include "print.h"
#include "tcp_server.h"
#include "msg_queue_server.h"
#include "config.h"
#include "trade_data.h"
#include "mem_pool_mng.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

ExecuteEngine::ExecuteEngine()
{
	m_TdAdapterCtp = NULL;
	m_iOrderRefNum = 0;
}

ExecuteEngine::~ExecuteEngine()
{

}

int ExecuteEngine::Init()
{
	//设置2个线程
	SetThreadCount(2);

	InitErrMsg();

	ConfigData*pConfigData = ConfigData::GetInstance();
	//加载账户
	for (int i = 0; i < pConfigData->TradeUsers.size(); i++)
	{
		string szUserId = pConfigData->TradeUsers[i].szUserId;
		m_UserMap[szUserId] = pConfigData->TradeUsers[i];

		BalanceT BalanceData;
		TradeData::GetBalance(szUserId, BalanceData);
		if ("" == BalanceData.szUserId)
		{
			std::cout << "Warn: create record in td_balance,please add money to this UserId! UserId=" << szUserId << endl;
			LWarn("Warn: create record in td_balance,please add money to this UserId!, szUserId=[{0}]", szUserId);

			BalanceData.szUserId = szUserId;
			BalanceData.dStartMoney = 0;

			BalanceData.iSettleDate = zutil::GetToday();
			BalanceData.iModifyDate = zutil::GetToday();
			BalanceData.iModifyTime = zutil::GetNowSedondTime();
			TradeData::InsertBalance(BalanceData);
		}

		shared_ptr<RuntimeManagerCtp> pRuntimeManager(new RuntimeManagerCtp(szUserId));
		m_RuntimeManagerCtpMap[szUserId] = pRuntimeManager;
		pRuntimeManager->Init();
	}

	for (int i = 0; i < pConfigData->AdminUsers.size(); i++)
	{
		string szUserId = pConfigData->AdminUsers[i].szUserId;
		m_UserMap[szUserId] = pConfigData->AdminUsers[i];
	}


	m_TdAdapterCtp = new TdAdapterCtp;
	m_TdAdapterCtp->Connect(pConfigData->CtpCfgData);


	return 0;
}
int ExecuteEngine::InitErrMsg()
{
	m_ErrMap[RET_OK] = "OK";
	m_ErrMap[RET_FAIL] = "失败";
	m_ErrMap[RET_ACCOUNT_NOT_EXIST] = "账号不存在";
	m_ErrMap[RET_ACCOUNT_PASSWORD_ERROR] = "密码错误";
	m_ErrMap[RET_JYS_ERROR] = "交易所错误";
	m_ErrMap[RET_ORDER_REF_EXIST] = "OrderRef已经存在";
	m_ErrMap[RET_ORDER_REF_NOT_EXIST] = "OrderRef不存在";
	m_ErrMap[RET_SEND_JYS_FAIL] = "发送到交易所失败";
	m_ErrMap[RET_CLIENT_ORDER_ID_EXIST] = "ClientOrderId已经存在";
	m_ErrMap[RET_CLIENT_ORDER_ID_NOT_EXIST] = "ClientOrderId不存在";
	m_ErrMap[RET_CLIENT_MSG_ID_EXIST] = "客户端ReqId已经存在";
	m_ErrMap[RET_RECORD_NOT_EXIST] = "记录不存在";
	m_ErrMap[RET_PASSWORD_ERROR] = "密码错误";
	m_ErrMap[RET_ROLE_ERROR] = "角色类型错误";
	m_ErrMap[RET_CREATE_MSG_QUEUE_FAIL] = "创建消息队列失败";
	m_ErrMap[RET_MSG_QUEUE_EXIST] = "消息队列已经存在";
	m_ErrMap[RET_MSG_QUEUE_NOT_EXIST] = "消息队列不存在";
	m_ErrMap[RET_NO_RIGHT] = "没有权限";
	m_ErrMap[RET_NO_RCV_FRONTID_SESSIONID] = "没有收到FrontID，SessionID";
	m_ErrMap[RET_UP_HIGH_PRICE] = "高于涨停价";
	m_ErrMap[RET_DOWN_LOW_PRICE] = "低于跌停价";
	m_ErrMap[RET_NO_POSITION] = "没有持仓";
	m_ErrMap[RET_NO_BALANCE] = "没有资金";
	m_ErrMap[RET_NO_STOCK_CODE] = "没有股票代码";
	m_ErrMap[RET_ENTRUST_NUM_ERROR] = "委托数量错误";
	m_ErrMap[RET_ONESELF_TRADE_ERROR] = "自成交错误";

	return 0;
}
int ExecuteEngine::Start()
{
	RunThreads();

	LInfo("休眠3秒，等待ctp连接服务器");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));

	//判断是否登录成功
	while (false == m_TdAdapterCtp->IsLogin())
	{
		LError("ctp登录失败");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	}

	//获取合约数据。调用ReqCtpInstrumentData()函数1次就可以了。
	//不要重复调用ReqCtpInstrumentData()函数。重复调用会导致数据表的数据重复
	//ReqCtpInstrumentData();

	std::cout << "Start ReqQrySettlementInfoConfirm" << endl;
	LInfo("Start ReqQrySettlementInfoConfirm");

	//请求查询结算信息确认
	m_TdAdapterCtp->ReqQrySettlementInfoConfirm();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));

	std::cout << "Start ReqQryDepthMarketData" << endl;
	LInfo("Start ReqQryDepthMarketData");

	//查询行情
	m_TdAdapterCtp->ReqQryDepthMarketData("", "");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 10));

	std::cout << "Start ReqQryBrokerTradingParams" << endl;
	LInfo("Start ReqQryBrokerTradingParams");

	//请求查询经纪公司交易参数
	m_TdAdapterCtp->ReqQryBrokerTradingParams();

	std::cout << "Start get ctp instrument data" << endl;
	LInfo("Start get ctp instrument data");

	//加载数据库合约数据
	//合约
	vector<shared_ptr<InstrumentT>> InstrumentDatas;
	TradeData::GetInstruments(InstrumentDatas);
	for (int i = 0; i < InstrumentDatas.size(); i++)
	{
		string szKey = InstrumentDatas[i]->szExchangeId + string("_") + InstrumentDatas[i]->szInstrumentId;
		m_InstrumentMap[szKey] = InstrumentDatas[i];
		m_InstrumentExchangeMap[InstrumentDatas[i]->szInstrumentId] = InstrumentDatas[i]->szExchangeId;
	}

	//合约保证金率
	vector<shared_ptr<InstrumentMarginRate>> InstrumentMarginRateDatas;
	TradeData::GetInstrumentMarginRates(InstrumentMarginRateDatas);
	for (int i = 0; i < InstrumentMarginRateDatas.size(); i++)
	{
		string  szInstrumentId = InstrumentMarginRateDatas[i]->szInstrumentId;
		InstrumentT InstrumentData;
		TradeData::GetInstruments(szInstrumentId, InstrumentData);
		string szExchangeId = InstrumentData.szExchangeId;

		string szKey = szExchangeId + string("_") + szInstrumentId;
		m_InstrumentMarginRateMap[szKey] = InstrumentMarginRateDatas[i];

	}

	//合约手续费率
	vector<shared_ptr<InstrumentCommissionRate>> InstrumentCommissionRateDatas;
	TradeData::GetInstrumentCommissionRates(InstrumentCommissionRateDatas);
	for (int i = 0; i < InstrumentCommissionRateDatas.size(); i++)
	{
		string szPartInstrumentId = InstrumentCommissionRateDatas[i]->szInstrumentId;
		vector<shared_ptr<InstrumentT>> InstrumentDatas;
		TradeData::GetInstruments(szPartInstrumentId, InstrumentDatas);

		for (int j = 0; j < InstrumentDatas.size(); j++)
		{
			string szKey = InstrumentDatas[j]->szExchangeId + string("_") + InstrumentDatas[j]->szInstrumentId;
			m_InstrumentCommissionRateMap[szKey] = InstrumentCommissionRateDatas[i];

		}
	}

	//报单手续费
	vector<shared_ptr<InstrumentOrderCommRate>> InstrumentOrderCommRateDatas;
	TradeData::GetInstrumentOrderCommRates(InstrumentOrderCommRateDatas);
	for (int i = 0; i < InstrumentOrderCommRateDatas.size(); i++)
	{
		string szPartInstrumentId = InstrumentOrderCommRateDatas[i]->szInstrumentId;

		vector<shared_ptr<InstrumentT>> InstrumentDatas;
		TradeData::GetInstruments(szPartInstrumentId, InstrumentDatas);
		for (int j = 0; j < InstrumentDatas.size(); j++)
		{
			string szKey = InstrumentDatas[j]->szExchangeId + string("_") + InstrumentDatas[j]->szInstrumentId;
			m_InstrumentOrderCommRateMap[szKey] = InstrumentOrderCommRateDatas[i];

		}

	}

	std::cout << "Start SettlePosition" << endl;
	LInfo("Start SettlePosition");

	//m_RuntimeManagerCtpMap
	for (auto&RuntimeManagerPair : m_RuntimeManagerCtpMap)
	{
		//结算
		RuntimeManagerPair.second->SettlePosition(m_MarketDataMap, m_InstrumentMap, m_InstrumentMarginRateMap);
		//更新order费率信息
		RuntimeManagerPair.second->UpdateOrderRate(m_MarketDataMap, m_InstrumentMap, m_InstrumentMarginRateMap, m_InstrumentCommissionRateMap, m_InstrumentOrderCommRateMap);
	}

	return 0;
}
int ExecuteEngine::Stop()
{
	StopThreads();
	return 0;
}
int ExecuteEngine::Release()
{
	return 0;
}
int  ExecuteEngine::DealMsg(void*pMsg, int iThreadId)
{
	if (pMsg != NULL)
	{
		ServerMsg*pServerMsg = (ServerMsg*)pMsg;
		DisPatchMsg(pServerMsg, iThreadId);

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
int ExecuteEngine::PostMsg(void*pMsg)
{
	if (false == m_bRunFlag)
	{
		return -1;
	}

	int iSelect = 0;
	if (pMsg != NULL)
	{
		ServerMsg*pServerMsg = (ServerMsg*)pMsg;
		int iFunctionId = pServerMsg->iFunctionId;

		//交易功能选用0号线程，其他功能选用1号线程
		if (FUNC_PLACE_ORDER == iFunctionId)
		{
			iSelect = 0;
		}
		else if (FUNC_CANCEL_ORDER == iFunctionId)
		{
			iSelect = 0;
		}
		else if (FUNC_LOGIN == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_LOGOUT == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_QUERY_BALANCE == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_QUERY_ORDER == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_QUERY_TRADE == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_QUERY_POSITION == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_ADD_MSG_QUEUE == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_DEL_MSG_QUEUE == iFunctionId)
		{
			iSelect = 1;
		}
		else if (FUNC_ADD_DEL_MONEY == iFunctionId)
		{
			iSelect = 1;
		}
	}

	m_vThread[iSelect]->PostMsg(pMsg);

	return 0;
}

int ExecuteEngine::DisPatchMsg(ServerMsg*pServerMsg, int iThreadId)
{
	int iFunctionId = pServerMsg->iFunctionId;
	if (FUNC_LOGIN == iFunctionId)
	{
		OnLogin(pServerMsg);
	}
	else if (FUNC_LOGOUT == iFunctionId)
	{
		OnLogout(pServerMsg);
	}
	else if (FUNC_QUERY_BALANCE == iFunctionId)
	{
		OnQueryBalance(pServerMsg);
	}
	else if (FUNC_QUERY_ORDER == iFunctionId)
	{
		OnQueryOrder(pServerMsg);
	}
	else if (FUNC_QUERY_TRADE == iFunctionId)
	{
		OnQueryTrade(pServerMsg);
	}
	else if (FUNC_QUERY_POSITION == iFunctionId)
	{
		OnQueryPosition(pServerMsg);
	}
	else if (FUNC_PLACE_ORDER == iFunctionId)
	{
		OnSendOrder(pServerMsg);
	}
	else if (FUNC_CANCEL_ORDER == iFunctionId)
	{
		OnCancelOrder(pServerMsg);
	}
	else if (FUNC_ADD_MSG_QUEUE == iFunctionId)
	{
		OnAddMsgQueue(pServerMsg);
	}
	else if (FUNC_DEL_MSG_QUEUE == iFunctionId)
	{
		OnDelMsgQueue(pServerMsg);
	}
	else if (FUNC_ADD_DEL_MONEY == iFunctionId)
	{
		OnAddDelMoney(pServerMsg);
	}
	return 0;
}

//登录
void ExecuteEngine::OnLogin(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	int iRet = 0;
	//登录报文
	LoginReqT*pLoginReqT = (LoginReqT*)(pReqMsg->pBody);
	string szUserId = pLoginReqT->szUserId;
	string szPassword = pLoginReqT->szPassword;

	if (m_UserMap.count(szUserId) > 0)
	{
		if (szPassword == m_UserMap[szUserId].szPassword)
		{
			if (iRoletype == m_UserMap[szUserId].iRoletype)
			{
				LInfo("login ok, szUserId=[{0}]", szUserId);
				iRet = RET_OK;
			}
			else
			{
				LError("login fail,iRoletype is error, szUserId=[{0}]", szUserId);
				iRet = RET_ROLE_ERROR;
			}

		}
		else
		{
			//密码错误
			LError("login fail,password is error, szUserId=[{0}]", szUserId);
			iRet = RET_PASSWORD_ERROR;
		}

	}
	else
	{
		//用户不存在
		LError("login fail,szUserId is not exist, szUserId=[{0}]", szUserId);
		iRet = RET_ACCOUNT_NOT_EXIST;
	}

	ServerMsg*pResMsg = GetResMsg(pReqMsg, iRet, 0, NULL);
	if (pResMsg != NULL)
	{
		CTcpServer::GetInstance()->PostMsg(pResMsg);
	}

}
//退出
void ExecuteEngine::OnLogout(ServerMsg*pReqMsg)
{
	ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, 0, NULL);
	if (pResMsg != NULL)
	{
		CTcpServer::GetInstance()->PostMsg(pResMsg);
	}
}
//查询资金
void ExecuteEngine::OnQueryBalance(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//管理员查Balance
		int iRequestId = m_TdAdapterCtp->ReqQueryBalance();
		if (iRequestId >= 0)
		{
			//发送成功
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//发送失败
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_SEND_JYS_FAIL, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}

		}
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		BalanceT TraderBalance = m_RuntimeManagerCtpMap[szUserId]->GetBalance();

		QueryBalanceResT BalanceData;
		BalanceData.iUseMargin = TraderBalance.dUseMargin*PRICE_MULTIPLE;
		BalanceData.iFrozenMargin = TraderBalance.dFrozenMargin*PRICE_MULTIPLE;
		BalanceData.iFrozenCash = TraderBalance.dFrozenCash*PRICE_MULTIPLE;
		BalanceData.iFrozenCommission = TraderBalance.dFrozenCommission*PRICE_MULTIPLE;
		BalanceData.iCurrMargin = TraderBalance.dCurrMargin*PRICE_MULTIPLE;
		BalanceData.iCommission = TraderBalance.dCommission*PRICE_MULTIPLE;
		BalanceData.iAvailable = TraderBalance.dAvailable*PRICE_MULTIPLE;

		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, sizeof(BalanceData), &BalanceData);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}

	}


}
//查询委托
void ExecuteEngine::OnQueryOrder(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号

	QueryOrderReqT*pQueryOrderReqT = (QueryOrderReqT*)(pReqMsg->pBody);
	int iFlag = pQueryOrderReqT->iFlag;//0-查当天全部，1-根据szClientOrderId查询
	string szClientOrderId = pQueryOrderReqT->szClientOrderId;

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		if (1 == iFlag)
		{
			//没有权限
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}

		int iRequestId = m_TdAdapterCtp->ReqQueryOrder();
		if (iRequestId >= 0)
		{
			//发送成功
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//发送失败
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_SEND_JYS_FAIL, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
		}

		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		//0-查当天全部，1-根据szClientOrderId查询
		if (0 == iFlag)
		{
			vector<shared_ptr<OrderT>> Orders;
			m_RuntimeManagerCtpMap[szUserId]->GetOrders(Orders);

			int iCount = Orders.size();
			for (int i = 0; i < iCount; i++)
			{
				QueryOrderResT  OrderRes;

				OrderRes.OrderData.iInitDate = Orders[i]->iInitDate;
				OrderRes.OrderData.iInitTime = Orders[i]->iInitTime;
				snprintf(OrderRes.OrderData.szJysInsertDate, sizeof(OrderRes.OrderData.szJysInsertDate), "%s", Orders[i]->szJysInsertDate.c_str());
				snprintf(OrderRes.OrderData.szJysInsertTime, sizeof(OrderRes.OrderData.szJysInsertTime), "%s", Orders[i]->szJysInsertTime.c_str());
				snprintf(OrderRes.OrderData.szUserId, sizeof(OrderRes.OrderData.szUserId), "%s", Orders[i]->szUserId.c_str());
				snprintf(OrderRes.OrderData.szExchangeID, sizeof(OrderRes.OrderData.szExchangeID), "%s", Orders[i]->szExchangeID.c_str());
				snprintf(OrderRes.OrderData.szStockCode, sizeof(OrderRes.OrderData.szStockCode), "%s", Orders[i]->szStockCode.c_str());
				snprintf(OrderRes.OrderData.szOrderRef, sizeof(OrderRes.OrderData.szOrderRef), "%s", Orders[i]->szOrderRef.c_str());
				snprintf(OrderRes.OrderData.szOrderSysID, sizeof(OrderRes.OrderData.szOrderSysID), "%s", Orders[i]->szOrderSysID.c_str());
				OrderRes.OrderData.cAction = Orders[i]->cAction;
				OrderRes.OrderData.iPrice = Orders[i]->dPrice*PRICE_MULTIPLE;
				OrderRes.OrderData.iEntrustNum = Orders[i]->iEntrustNum;
				OrderRes.OrderData.iTradeNum = Orders[i]->iTradeNum;
				OrderRes.OrderData.cStatus = Orders[i]->cStatus;

				bool bIsLast = false;
				if ((iCount - 1) == i)
				{
					bIsLast = true;
				}

				ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, sizeof(OrderRes), &OrderRes, bIsLast);
				if (pResMsg != NULL)
				{
					CTcpServer::GetInstance()->PostMsg(pResMsg);
				}
			}
		}
		else if (1 == iFlag)
		{
			shared_ptr<OrderT> pOrderT = nullptr;
			if (m_RuntimeManagerCtpMap[szUserId]->GetOrderByClientOrderId(szClientOrderId, pOrderT))
			{
				QueryOrderResT  OrderRes;

				OrderRes.OrderData.iInitDate = pOrderT->iInitDate;
				OrderRes.OrderData.iInitTime = pOrderT->iInitTime;
				snprintf(OrderRes.OrderData.szJysInsertDate, sizeof(OrderRes.OrderData.szJysInsertDate), "%s", pOrderT->szJysInsertDate.c_str());
				snprintf(OrderRes.OrderData.szJysInsertTime, sizeof(OrderRes.OrderData.szJysInsertTime), "%s", pOrderT->szJysInsertTime.c_str());
				snprintf(OrderRes.OrderData.szUserId, sizeof(OrderRes.OrderData.szUserId), "%s", pOrderT->szUserId.c_str());
				snprintf(OrderRes.OrderData.szExchangeID, sizeof(OrderRes.OrderData.szExchangeID), "%s", pOrderT->szExchangeID.c_str());
				snprintf(OrderRes.OrderData.szStockCode, sizeof(OrderRes.OrderData.szStockCode), "%s", pOrderT->szStockCode.c_str());
				snprintf(OrderRes.OrderData.szOrderRef, sizeof(OrderRes.OrderData.szOrderRef), "%s", pOrderT->szOrderRef.c_str());
				snprintf(OrderRes.OrderData.szOrderSysID, sizeof(OrderRes.OrderData.szOrderSysID), "%s", pOrderT->szOrderSysID.c_str());
				OrderRes.OrderData.cAction = pOrderT->cAction;
				OrderRes.OrderData.iPrice = pOrderT->dPrice*PRICE_MULTIPLE;
				OrderRes.OrderData.iEntrustNum = pOrderT->iEntrustNum;
				OrderRes.OrderData.iTradeNum = pOrderT->iTradeNum;
				OrderRes.OrderData.cStatus = pOrderT->cStatus;

				bool bIsLast = true;
				ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, sizeof(OrderRes), &OrderRes, bIsLast);
				if (pResMsg != NULL)
				{
					CTcpServer::GetInstance()->PostMsg(pResMsg);
				}
			}
		}
	}

}
//查询成交
void ExecuteEngine::OnQueryTrade(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号

	QueryTradeReqT*pQueryTradeReqT = (QueryTradeReqT*)(pReqMsg->pBody);
	int iFlag = pQueryTradeReqT->iFlag;//0-查当天全部，1-根据szClientOrderId查询
	string szClientOrderId = pQueryTradeReqT->szClientOrderId;

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		if (1 == iFlag)
		{
			//没有权限
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}

		int iRequestId = m_TdAdapterCtp->ReqQueryTrade();
		if (iRequestId >= 0)
		{
			//发送成功
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//发送失败
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_SEND_JYS_FAIL, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
		}

		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		//0-查当天全部，1-根据szClientOrderId查询
		if (0 == iFlag)
		{
			vector<shared_ptr<TradeT>> Trades;
			m_RuntimeManagerCtpMap[szUserId]->GetTrades(Trades);

			int iCount = Trades.size();
			for (int i = 0; i < iCount; i++)
			{
				QueryTradeResT  TradeRes;
				TradeRes.TradeData.iInitDate = Trades[i]->iInitDate;
				TradeRes.TradeData.iInitTime = Trades[i]->iInitTime;
				snprintf(TradeRes.TradeData.szJysTradeDate, sizeof(TradeRes.TradeData.szJysTradeDate), "%s", Trades[i]->szJysTradeDate.c_str());
				snprintf(TradeRes.TradeData.szJysTradeTime, sizeof(TradeRes.TradeData.szJysTradeTime), "%s", Trades[i]->szJysTradeTime.c_str());
				snprintf(TradeRes.TradeData.szUserId, sizeof(TradeRes.TradeData.szUserId), "%s", Trades[i]->szUserId.c_str());
				snprintf(TradeRes.TradeData.szExchangeID, sizeof(TradeRes.TradeData.szExchangeID), "%s", Trades[i]->szExchangeID.c_str());
				snprintf(TradeRes.TradeData.szStockCode, sizeof(TradeRes.TradeData.szStockCode), "%s", Trades[i]->szStockCode.c_str());
				snprintf(TradeRes.TradeData.szOrderRef, sizeof(TradeRes.TradeData.szOrderRef), "%s", Trades[i]->szOrderRef.c_str());
				snprintf(TradeRes.TradeData.szOrderSysID, sizeof(TradeRes.TradeData.szOrderSysID), "%s", Trades[i]->szOrderSysID.c_str());
				snprintf(TradeRes.TradeData.szTradeID, sizeof(TradeRes.TradeData.szTradeID), "%s", Trades[i]->szTradeID.c_str());
				TradeRes.TradeData.cAction = Trades[i]->cAction;
				TradeRes.TradeData.iPrice = Trades[i]->dPrice*PRICE_MULTIPLE;
				TradeRes.TradeData.iTradeNum = Trades[i]->iTradeNum;

				bool bIsLast = false;
				if ((iCount - 1) == i)
				{
					bIsLast = true;
				}

				ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, sizeof(TradeRes), &TradeRes, bIsLast);
				if (pResMsg != NULL)
				{
					CTcpServer::GetInstance()->PostMsg(pResMsg);
				}
			}
		}
		else if (1 == iFlag)
		{
			vector<shared_ptr<TradeT>> Trades;
			if (m_RuntimeManagerCtpMap[szUserId]->GetTradesByClientOrderId(szClientOrderId, Trades))
			{
				int iCount = Trades.size();
				for (int i = 0; i < iCount; i++)
				{
					QueryTradeResT  TradeRes;
					TradeRes.TradeData.iInitDate = Trades[i]->iInitDate;
					TradeRes.TradeData.iInitTime = Trades[i]->iInitTime;
					snprintf(TradeRes.TradeData.szJysTradeDate, sizeof(TradeRes.TradeData.szJysTradeDate), "%s", Trades[i]->szJysTradeDate.c_str());
					snprintf(TradeRes.TradeData.szJysTradeTime, sizeof(TradeRes.TradeData.szJysTradeTime), "%s", Trades[i]->szJysTradeTime.c_str());
					snprintf(TradeRes.TradeData.szUserId, sizeof(TradeRes.TradeData.szUserId), "%s", Trades[i]->szUserId.c_str());
					snprintf(TradeRes.TradeData.szExchangeID, sizeof(TradeRes.TradeData.szExchangeID), "%s", Trades[i]->szExchangeID.c_str());
					snprintf(TradeRes.TradeData.szStockCode, sizeof(TradeRes.TradeData.szStockCode), "%s", Trades[i]->szStockCode.c_str());
					snprintf(TradeRes.TradeData.szOrderRef, sizeof(TradeRes.TradeData.szOrderRef), "%s", Trades[i]->szOrderRef.c_str());
					snprintf(TradeRes.TradeData.szOrderSysID, sizeof(TradeRes.TradeData.szOrderSysID), "%s", Trades[i]->szOrderSysID.c_str());
					snprintf(TradeRes.TradeData.szTradeID, sizeof(TradeRes.TradeData.szTradeID), "%s", Trades[i]->szTradeID.c_str());
					TradeRes.TradeData.cAction = Trades[i]->cAction;
					TradeRes.TradeData.iPrice = Trades[i]->dPrice*PRICE_MULTIPLE;
					TradeRes.TradeData.iTradeNum = Trades[i]->iTradeNum;

					bool bIsLast = false;
					if ((iCount - 1) == i)
					{
						bIsLast = true;
					}

					ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, sizeof(TradeRes), &TradeRes, bIsLast);
					if (pResMsg != NULL)
					{
						CTcpServer::GetInstance()->PostMsg(pResMsg);
					}
				}
			}
		}
	}
}
//查询持仓
void ExecuteEngine::OnQueryPosition(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		int iRequestId = m_TdAdapterCtp->ReqQueryPosition();
		if (iRequestId >= 0)
		{
			//发送成功
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//发送失败
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_SEND_JYS_FAIL, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
		}

		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		vector<shared_ptr<PositionT>> Positions;
		m_RuntimeManagerCtpMap[szUserId]->GetPositions(Positions);

		int iCount = Positions.size();
		for (int i = 0; i < iCount; i++)
		{
			QueryPositionResT  PositionRes;
			PositionRes.PositionData.iInitDate = Positions[i]->iInitDate;
			snprintf(PositionRes.PositionData.szUserId, sizeof(PositionRes.PositionData.szUserId), "%s", Positions[i]->szUserId.c_str());
			snprintf(PositionRes.PositionData.szExchangeID, sizeof(PositionRes.PositionData.szExchangeID), "%s", Positions[i]->szExchangeID.c_str());
			snprintf(PositionRes.PositionData.szStockCode, sizeof(PositionRes.PositionData.szStockCode), "%s", Positions[i]->szStockCode.c_str());
			PositionRes.PositionData.cDirection = Positions[i]->cDirection;
			PositionRes.PositionData.cHedgeFlag = Positions[i]->cHedgeFlag;
			PositionRes.PositionData.iYdPosition = Positions[i]->iYdPosition;
			PositionRes.PositionData.iTodayPosition = Positions[i]->iTodayPosition;
			PositionRes.PositionData.iPosition = Positions[i]->iPosition;
			PositionRes.PositionData.iLongFrozen = Positions[i]->iLongFrozen;
			PositionRes.PositionData.iShortFrozen = Positions[i]->iShortFrozen;
			PositionRes.PositionData.iUseMargin = Positions[i]->dUseMargin*PRICE_MULTIPLE;
			PositionRes.PositionData.iFrozenMargin = Positions[i]->dFrozenMargin*PRICE_MULTIPLE;
			PositionRes.PositionData.iFrozenCash = Positions[i]->dFrozenCash*PRICE_MULTIPLE;
			PositionRes.PositionData.iFrozenCommission = Positions[i]->dFrozenCommission*PRICE_MULTIPLE;
			PositionRes.PositionData.iCommission = Positions[i]->dCommission*PRICE_MULTIPLE;
			PositionRes.PositionData.iPreSettlementPrice = Positions[i]->dPreSettlementPrice*PRICE_MULTIPLE;
			PositionRes.PositionData.iSettlementPrice = Positions[i]->dSettlementPrice*PRICE_MULTIPLE;

			bool bIsLast = false;
			if ((iCount - 1) == i)
			{
				bIsLast = true;
			}

			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, sizeof(PositionRes), &PositionRes, bIsLast);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
		}
	}

}
//增加通讯消息队列
void ExecuteEngine::OnAddMsgQueue(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//管理员没有权限，管理员不发交易请求
		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}
		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		int iReadKey = 0;
		int iWriteKey = 0;
		int iRet = CMsgQueueServer::GetInstance()->CreateMsgQueue(szUserId, iReadKey, iWriteKey);

		AddMsgQueueResT  ResData;
		ResData.iReadKey = iReadKey;
		ResData.iWriteKey = iWriteKey;

		ServerMsg*pResMsg = GetResMsg(pReqMsg, iRet, sizeof(ResData), &ResData);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}

	}
}
//删除通讯消息队列
void ExecuteEngine::OnDelMsgQueue(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//管理员没有权限，管理员不发交易请求
		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}
		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		int iRet = CMsgQueueServer::GetInstance()->DelMsgQueue(szUserId);
		ServerMsg*pResMsg = GetResMsg(pReqMsg, iRet, 0, NULL);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}

	}

}
//增减交易账号资金
void ExecuteEngine::OnAddDelMoney(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		AddDelMoneyReqT*pAddDelMoneyReqT = (AddDelMoneyReqT*)pReqMsg->pBody;
		string szTradeUserId = pAddDelMoneyReqT->szUserId;
		LInfo("增减资金:szTradeUserId=[{0}],iFlag=[{1}],iAmount=[{2}] ", szTradeUserId, pAddDelMoneyReqT->iFlag, pAddDelMoneyReqT->iAmount);

		if (0 == m_RuntimeManagerCtpMap.count(szTradeUserId))
		{
			//账号不存在
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_ACCOUNT_NOT_EXIST, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}

		BalanceT BalanceData;
		//操作标志：0-增加资金，1-减少资金
		if (0 == pAddDelMoneyReqT->iFlag)
		{
			BalanceData.dAddMoney = pAddDelMoneyReqT->iAmount / PRICE_MULTIPLE;
		}
		else
		{
			double dValue = pAddDelMoneyReqT->iAmount / PRICE_MULTIPLE;
			BalanceData.dAddMoney = 0 - dValue;
			LInfo("szTradeUserId=[{0}],dValue=[{1}],BalanceData.dAddMoney=[{2}] ", szTradeUserId, dValue, BalanceData.dAddMoney);

		}
		m_RuntimeManagerCtpMap[szTradeUserId]->UpdateBalance(BalanceData);

		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, 0, NULL);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		//没有权限
		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}
	}


}
//委托
void ExecuteEngine::OnSendOrder(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//管理员没有权限，管理员不发交易请求
		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
		if (pResMsg != NULL)
		{
			CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
		}
		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		PlaceOrderReqT*pPlaceOrderReqT = (PlaceOrderReqT*)pReqMsg->pBody;

		//ctp行情没有ExchangeID，此处获取ExchangeID
		string szExchangeID;
		string szStockCode = pPlaceOrderReqT->szStockCode;
		if (0 == m_InstrumentExchangeMap.count(szStockCode))
		{
			LError("m_InstrumentExchangeMap: StockCode  is  not exist, szStockCode=[{0}]", szStockCode);

			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_STOCK_CODE, 0, NULL);
			if (pResMsg != NULL)
			{
				CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}
		else
		{
			szExchangeID = m_InstrumentExchangeMap[szStockCode];
		}

		OrderT OrderData;
		OrderData.iInitDate = zutil::GetToday();
		OrderData.iInitTime = zutil::GetNowSedondTime();
		OrderData.iModifyDate = zutil::GetToday();
		OrderData.iModifyTime = zutil::GetNowSedondTime();
		OrderData.szExchangeID = szExchangeID;
		OrderData.szStockCode = pPlaceOrderReqT->szStockCode;
		OrderData.szClientOrderId = pPlaceOrderReqT->szClientOrderId;
		OrderData.szUserId = szUserId;
		OrderData.szOrderRef = GetOrderRef(szUserId);
		OrderData.cAction = pPlaceOrderReqT->cAction;
		OrderData.dPrice = pPlaceOrderReqT->iPrice / PRICE_MULTIPLE;
		OrderData.iEntrustNum = pPlaceOrderReqT->iEntrustNum;
		OrderData.cStatus = ORDER_STATUS_NO_SEND;

		string szKey = OrderData.szExchangeID + string("_") + OrderData.szStockCode;
		if (0 == m_InstrumentMap.count(szKey))
		{
			LError("m_InstrumentMap:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);

			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_STOCK_CODE, 0, NULL);
			if (pResMsg != NULL)
			{
				CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}
		if (0 == m_MarketDataMap.count(szKey))
		{
			LError("m_LastSettlementPriceMap:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);

			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_STOCK_CODE, 0, NULL);
			if (pResMsg != NULL)
			{
				CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}
		if (0 == m_InstrumentMarginRateMap.count(szKey))
		{
			LInfo("m_InstrumentMarginRateMap:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);
			m_InstrumentMarginRateMap[szKey] = make_shared<InstrumentMarginRate>();
		}
		if (0 == m_InstrumentCommissionRateMap.count(szKey))
		{
			LInfo("m_InstrumentCommissionRateMap:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);
			m_InstrumentCommissionRateMap[szKey] = make_shared<InstrumentCommissionRate>();
		}


		OrderData.iVolumeMultiple = m_InstrumentMap[szKey]->iVolumeMultiple;
		OrderData.dPreSettlementPrice = m_MarketDataMap[szKey].iPreSettlePrice / PRICE_MULTIPLE;
		LDebug("szStockCode=[{0}],iPreSettlePrice=[{1}],dPreSettlementPrice=[{2}]", OrderData.szStockCode, m_MarketDataMap[szKey].iPreSettlePrice, OrderData.dPreSettlementPrice);


		if (0 == m_InstrumentOrderCommRateMap.count(szKey))
		{
			//有的不收取报单手续费
			OrderData.dOrderCommByVolume = 0;
			OrderData.dOrderActionCommByVolume = 0;
		}
		else
		{
			OrderData.dOrderCommByVolume = m_InstrumentOrderCommRateMap[szKey]->dOrderCommByVolume;
			OrderData.dOrderActionCommByVolume = m_InstrumentOrderCommRateMap[szKey]->dOrderActionCommByVolume;
		}

		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction)
		{
			//多头
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dLongMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dLongMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByVolume;
		}
		else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//多头
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dCloseRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dCloseRatioByVolume;
			OrderData.dCloseTodayRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dCloseTodayRatioByMoney;
			OrderData.dCloseTodayRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dCloseTodayRatioByVolume;

		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction)
		{
			//空头
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByVolume;

		}
		else if (ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			//空头
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dLongMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dLongMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dCloseRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dCloseRatioByVolume;
			OrderData.dCloseTodayRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dCloseTodayRatioByMoney;
			OrderData.dCloseTodayRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dCloseTodayRatioByVolume;

		}

		int iRet = SendOrder(OrderData);

		PlaceOrderResT  ResData;
		snprintf(ResData.szOrderRef, sizeof(ResData.szOrderRef), "%s", OrderData.szOrderRef.c_str());
		ServerMsg*pResMsg = GetResMsg(pReqMsg, iRet, sizeof(ResData), &ResData);
		if (pResMsg != NULL)
		{
			CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
		}

	}


}
//撤单
void ExecuteEngine::OnCancelOrder(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //账号
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//管理员没有权限，管理员不发交易请求
		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
		if (pResMsg != NULL)
		{
			CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
		}
		return;
	}
	else if (ROLE_TYPE_TRADER == iRoletype)
	{
		CancelOrderReqT*pPlaceOrderReqT = (CancelOrderReqT*)pReqMsg->pBody;
		string szClientOrderId = pPlaceOrderReqT->szClientOrderId;

		int iRet = CancelOldOrder(szUserId, szClientOrderId);

		ServerMsg*pResMsg = GetResMsg(pReqMsg, iRet, 0, NULL);
		if (pResMsg != NULL)
		{
			CMsgQueueServer::GetInstance()->PostMsg(pResMsg);
		}

	}

}

int ExecuteEngine::SendOrder(OrderT&OrderData)
{
	int iRet = RET_OK;

	//风控检查
	iRet = RiskRule(OrderData);
	if (iRet != RET_OK)
	{
		return iRet;
	}

	//管理自成交
	char cSellBuyFlag = '\0';
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		//买
		cSellBuyFlag = TRADE_DIRECTION_BUY;
	}
	else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
	{
		//卖
		cSellBuyFlag = TRADE_DIRECTION_SELL;
	}
	MngOneselfTrade(cSellBuyFlag, FLAG_ADD, OrderData.szStockCode, zutil::ConvertDoubleToInt(OrderData.dPrice, PRICE_MULTIPLE), OrderData.iEntrustNum);

	//更新order
	OrderData.cStatus = ORDER_STATUS_NO_SEND;
	m_RuntimeManagerCtpMap[OrderData.szUserId]->UpdateOrder(OrderData);

	TdResponse<string> TdResponse;
	TdResponse = m_TdAdapterCtp->ReqPlaceOrder(OrderData.szOrderRef, OrderData.szStockCode, OrderData.cAction, OrderData.dPrice, OrderData.iEntrustNum);

	if (0 == TdResponse.iErrCode)
	{
		OrderData.cStatus = ORDER_STATUS_SEND;

		if (OrderData.dOrderCommByVolume != 0)
		{
			BalanceT BalanceData;
			BalanceData.dCommission = 0 - OrderData.dOrderCommByVolume;//报单手续费
			LDebug("szOrderRef=[{0}],OrderData.dOrderCommByVolume=[{1}]", OrderData.szOrderRef, OrderData.dOrderCommByVolume);

			m_RuntimeManagerCtpMap[OrderData.szUserId]->UpdateBalance(BalanceData);
		}

	}
	else
	{
		OrderData.cStatus = ORDER_STATUS_SEND_FAIL;
		iRet = RET_SEND_JYS_FAIL;
	}

	//更新order
	m_RuntimeManagerCtpMap[OrderData.szUserId]->UpdateOrder(OrderData);

	return iRet;
}

int ExecuteEngine::CancelOldOrder(const string&szUserId, const string& szClientOrderId)
{
	LInfo("CancelOrder, szUserId=[{0}],szClientOrderId=[{1}]", szUserId, szClientOrderId);

	int iRet = RET_OK;
	shared_ptr<OrderT>  pOrderT = nullptr;
	if (false == m_RuntimeManagerCtpMap[szUserId]->GetOrderByClientOrderId(szClientOrderId, pOrderT))
	{
		LError("szClientOrderId is not exist, szClientOrderId=[{0}]", szClientOrderId);
		iRet = RET_CLIENT_ORDER_ID_NOT_EXIST;
		return iRet;
	}

	string szOrderRef = pOrderT->szOrderRef;
	int iFrontID = pOrderT->iFrontID;
	int iSessionID = pOrderT->iSessionID;
	string szStockCode = pOrderT->szStockCode;

	if (0 == iFrontID && 0 == iSessionID)
	{
		LError("iFrontID and  iSessionID  is not received");
		return RET_NO_RCV_FRONTID_SESSIONID;
	}

	LDebug("iFrontID=[{0}],iSessionID=[{1}]", iFrontID, iSessionID);

	TdResponse<string> TdResponse;
	TdResponse = m_TdAdapterCtp->ReqCancelOrder(iFrontID, iSessionID, szOrderRef, szStockCode);
	if (0 == TdResponse.iErrCode)
	{
		iRet = RET_OK;

		if (pOrderT->dOrderActionCommByVolume != 0)
		{
			BalanceT BalanceData;
			BalanceData.dCommission = 0 - pOrderT->dOrderActionCommByVolume;//撤单手续费
			LDebug("szClientOrderId=[{0}],pOrderT->dOrderActionCommByVolume=[{1}]", szClientOrderId, pOrderT->dOrderActionCommByVolume);

			m_RuntimeManagerCtpMap[szUserId]->UpdateBalance(BalanceData);
		}

	}
	else
	{
		iRet = RET_SEND_JYS_FAIL;
	}

	return iRet;

}
//管理自成交
int ExecuteEngine::MngOneselfTrade(char cSellBuyFlag, int iAddDelFlag, string&szStockCode, unsigned long long iPrice, int iNum)
{
	lock_guard<mutex> lk(m_SellBuyMtx);

	if (TRADE_DIRECTION_BUY == cSellBuyFlag)
	{
		if (FLAG_ADD == iAddDelFlag)
		{
			if (0 == m_BuyMap.count(szStockCode))
			{
				map<unsigned long long, unsigned long long> PriceNumMap;
				m_BuyMap[szStockCode] = PriceNumMap;
				m_BuyMap[szStockCode][iPrice] = iNum;
			}
			else
			{
				if (0 == m_BuyMap[szStockCode].count(iPrice))
				{
					m_BuyMap[szStockCode][iPrice] = iNum;
				}
				else
				{
					m_BuyMap[szStockCode][iPrice] = m_BuyMap[szStockCode][iPrice] + iNum;
				}
			}
		}
		else if (FLAG_DEL == iAddDelFlag)
		{
			if (0 == m_BuyMap.count(szStockCode))
			{
				//不存在此情况
			}
			else
			{
				if (0 == m_BuyMap[szStockCode].count(iPrice))
				{
					//不存在此情况
				}
				else
				{
					m_BuyMap[szStockCode][iPrice] = m_BuyMap[szStockCode][iPrice] - iNum;
				}

			}
		}

		LDebug("MngOneselfTrade, szStockCode=[{0}] iPrice=[{1}]  m_BuyMap[szStockCode][iPrice] = [{2}]", szStockCode, iPrice, m_BuyMap[szStockCode][iPrice]);
	}
	else if (TRADE_DIRECTION_SELL == cSellBuyFlag)
	{
		if (FLAG_ADD == iAddDelFlag)
		{
			if (0 == m_SellMap.count(szStockCode))
			{
				map<unsigned long long, unsigned long long> PriceNumMap;
				m_SellMap[szStockCode] = PriceNumMap;
				m_SellMap[szStockCode][iPrice] = iNum;
			}
			else
			{
				if (0 == m_SellMap[szStockCode].count(iPrice))
				{
					m_SellMap[szStockCode][iPrice] = iNum;
				}
				else
				{
					m_SellMap[szStockCode][iPrice] = m_SellMap[szStockCode][iPrice] + iNum;
				}

			}
		}
		else if (FLAG_DEL == iAddDelFlag)
		{
			if (0 == m_SellMap.count(szStockCode))
			{
				//不存在此情况
			}
			else
			{
				if (0 == m_SellMap[szStockCode].count(iPrice))
				{
					//不存在此情况
				}
				else
				{
					m_SellMap[szStockCode][iPrice] = m_SellMap[szStockCode][iPrice] - iNum;
				}

			}
		}

		LDebug("MngOneselfTrade, szStockCode=[{0}] iPrice=[{1}]  m_SellMap[szStockCode][iPrice] = [{2}]", szStockCode, iPrice, m_SellMap[szStockCode][iPrice]);

	}

	return 0;
}

//查询资金响应
void ExecuteEngine::HandleQueryBalanceRsp(TdResponse<BalanceT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryBalanceRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		PrintData::PrintBalance(TdResponse.RspResult);

		QueryBalanceResT BalanceData;
		BalanceData.iUseMargin = TdResponse.RspResult.dUseMargin*PRICE_MULTIPLE;
		BalanceData.iFrozenMargin = TdResponse.RspResult.dFrozenMargin*PRICE_MULTIPLE;
		BalanceData.iFrozenCash = TdResponse.RspResult.dFrozenCash*PRICE_MULTIPLE;
		BalanceData.iFrozenCommission = TdResponse.RspResult.dFrozenCommission*PRICE_MULTIPLE;
		BalanceData.iCurrMargin = TdResponse.RspResult.dCurrMargin*PRICE_MULTIPLE;
		BalanceData.iCommission = TdResponse.RspResult.dCommission*PRICE_MULTIPLE;
		BalanceData.iAvailable = TdResponse.RspResult.dAvailable*PRICE_MULTIPLE;

		ServerMsg*pResMsg = GetResMsg(TdResponse.iRequestId, RET_OK, sizeof(BalanceData), &BalanceData);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}
	}

}
//查询委托响应
void ExecuteEngine::HandleQueryOrderRsp(TdResponse<OrderT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryOrderRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		OrderT&OrderData = TdResponse.RspResult;
		//打印委托
		PrintData::PrintOrder(OrderData);

		QueryOrderResT OrderRes;
		OrderRes.OrderData.iInitDate = OrderData.iInitDate;
		OrderRes.OrderData.iInitTime = OrderData.iInitTime;
		snprintf(OrderRes.OrderData.szJysInsertDate, sizeof(OrderRes.OrderData.szJysInsertDate), "%s", OrderData.szJysInsertDate.c_str());
		snprintf(OrderRes.OrderData.szJysInsertTime, sizeof(OrderRes.OrderData.szJysInsertTime), "%s", OrderData.szJysInsertTime.c_str());
		snprintf(OrderRes.OrderData.szUserId, sizeof(OrderRes.OrderData.szUserId), "%s", OrderData.szUserId.c_str());
		snprintf(OrderRes.OrderData.szExchangeID, sizeof(OrderRes.OrderData.szExchangeID), "%s", OrderData.szExchangeID.c_str());
		snprintf(OrderRes.OrderData.szStockCode, sizeof(OrderRes.OrderData.szStockCode), "%s", OrderData.szStockCode.c_str());
		snprintf(OrderRes.OrderData.szOrderRef, sizeof(OrderRes.OrderData.szOrderRef), "%s", OrderData.szOrderRef.c_str());
		snprintf(OrderRes.OrderData.szOrderSysID, sizeof(OrderRes.OrderData.szOrderSysID), "%s", OrderData.szOrderSysID.c_str());
		memcpy(OrderRes.OrderData.szUserId, OrderRes.OrderData.szOrderRef, VIR_ACCOUNT_LENGTH);
		OrderRes.OrderData.cAction = OrderData.cAction;
		OrderRes.OrderData.iPrice = OrderData.dPrice*PRICE_MULTIPLE;
		OrderRes.OrderData.iEntrustNum = OrderData.iEntrustNum;
		OrderRes.OrderData.iTradeNum = OrderData.iTradeNum;
		OrderRes.OrderData.cStatus = OrderData.cStatus;

		ServerMsg*pResMsg = GetResMsg(TdResponse.iRequestId, RET_OK, sizeof(OrderRes), &OrderRes, TdResponse.bIsLast);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}

		//发送到最后一条记录时，删除存储的请求信息
		if (TdResponse.bIsLast)
		{
			DelReqBase(TdResponse.iRequestId);
		}
	}
}
//查询成交响应
void ExecuteEngine::HandleQueryTradeRsp(TdResponse<TradeT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryTradeRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		TradeT &TradeData = TdResponse.RspResult;

		//打印交易
		PrintData::PrintTrade(TradeData);

		QueryTradeResT  TradeRes;
		TradeRes.TradeData.iInitDate = TradeData.iInitDate;
		TradeRes.TradeData.iInitTime = TradeData.iInitTime;
		snprintf(TradeRes.TradeData.szJysTradeDate, sizeof(TradeRes.TradeData.szJysTradeDate), "%s", TradeData.szJysTradeDate.c_str());
		snprintf(TradeRes.TradeData.szJysTradeTime, sizeof(TradeRes.TradeData.szJysTradeTime), "%s", TradeData.szJysTradeTime.c_str());
		snprintf(TradeRes.TradeData.szUserId, sizeof(TradeRes.TradeData.szUserId), "%s", TradeData.szUserId.c_str());
		snprintf(TradeRes.TradeData.szExchangeID, sizeof(TradeRes.TradeData.szExchangeID), "%s", TradeData.szExchangeID.c_str());
		snprintf(TradeRes.TradeData.szStockCode, sizeof(TradeRes.TradeData.szStockCode), "%s", TradeData.szStockCode.c_str());
		snprintf(TradeRes.TradeData.szOrderRef, sizeof(TradeRes.TradeData.szOrderRef), "%s", TradeData.szOrderRef.c_str());
		snprintf(TradeRes.TradeData.szOrderSysID, sizeof(TradeRes.TradeData.szOrderSysID), "%s", TradeData.szOrderSysID.c_str());
		snprintf(TradeRes.TradeData.szTradeID, sizeof(TradeRes.TradeData.szTradeID), "%s", TradeData.szTradeID.c_str());
		memcpy(TradeRes.TradeData.szUserId, TradeRes.TradeData.szOrderRef, VIR_ACCOUNT_LENGTH);
		TradeRes.TradeData.cAction = TradeData.cAction;
		TradeRes.TradeData.iPrice = TradeData.dPrice*PRICE_MULTIPLE;
		TradeRes.TradeData.iTradeNum = TradeData.iTradeNum;


		ServerMsg*pResMsg = GetResMsg(TdResponse.iRequestId, RET_OK, sizeof(TradeRes), &TradeRes, TdResponse.bIsLast);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}

		//发送到最后一条记录时，删除存储的请求信息
		if (TdResponse.bIsLast)
		{
			DelReqBase(TdResponse.iRequestId);
		}
	}
}
//查询持仓响应
void ExecuteEngine::HandleQueryPositionRsp(TdResponse<PositionT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryPositionRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		PositionT &PositionData = TdResponse.RspResult;
		//打印持仓
		PrintData::PrintPosition(PositionData);

		QueryPositionResT PositionRes;

		PositionRes.PositionData.iInitDate = PositionData.iInitDate;
		snprintf(PositionRes.PositionData.szUserId, sizeof(PositionRes.PositionData.szUserId), "%s", PositionData.szUserId.c_str());
		snprintf(PositionRes.PositionData.szExchangeID, sizeof(PositionRes.PositionData.szExchangeID), "%s", PositionData.szExchangeID.c_str());
		snprintf(PositionRes.PositionData.szStockCode, sizeof(PositionRes.PositionData.szStockCode), "%s", PositionData.szStockCode.c_str());
		PositionRes.PositionData.cDirection = PositionData.cDirection;
		PositionRes.PositionData.cHedgeFlag = PositionData.cHedgeFlag;

		PositionRes.PositionData.iYdPosition = PositionData.iYdPosition;
		PositionRes.PositionData.iTodayPosition = PositionData.iTodayPosition;
		PositionRes.PositionData.iPosition = PositionData.iPosition;
		PositionRes.PositionData.iLongFrozen = PositionData.iLongFrozen;
		PositionRes.PositionData.iShortFrozen = PositionData.iShortFrozen;

		PositionRes.PositionData.iUseMargin = PositionData.dUseMargin*PRICE_MULTIPLE;
		PositionRes.PositionData.iFrozenMargin = PositionData.dFrozenMargin*PRICE_MULTIPLE;
		PositionRes.PositionData.iFrozenCash = PositionData.dFrozenCash*PRICE_MULTIPLE;
		PositionRes.PositionData.iFrozenCommission = PositionData.dFrozenCommission*PRICE_MULTIPLE;
		PositionRes.PositionData.iCommission = PositionData.dCommission*PRICE_MULTIPLE;
		PositionRes.PositionData.iPreSettlementPrice = PositionData.dPreSettlementPrice*PRICE_MULTIPLE;
		PositionRes.PositionData.iSettlementPrice = PositionData.dSettlementPrice*PRICE_MULTIPLE;

		ServerMsg*pResMsg = GetResMsg(TdResponse.iRequestId, RET_OK, sizeof(PositionRes), &PositionRes, TdResponse.bIsLast);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}

		//发送到最后一条记录时，删除存储的请求信息
		if (TdResponse.bIsLast)
		{
			DelReqBase(TdResponse.iRequestId);
		}
	}

}

//查询合约
void ExecuteEngine::HandleQueryInstrumentRsp(TdResponse<InstrumentT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryInstrumentRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		InstrumentT  InstrumentData = TdResponse.RspResult;

		LDebug("------------------InstrumentT-------------");
		LDebug("szInstrumentId= {0}", InstrumentData.szInstrumentId);
		LDebug("szExchangeId= {0}", InstrumentData.szExchangeId);
		LDebug("szInstrumentName= {0}", InstrumentData.szInstrumentName);
		LDebug("szProductId= {0}", InstrumentData.szProductId);
		LDebug("szProductClass= {0}", InstrumentData.szProductClass);
		LDebug("iDeliveryYear= {0}", InstrumentData.iDeliveryYear);
		LDebug("iDeliveryMonth= {0}", InstrumentData.iDeliveryMonth);
		LDebug("iMaxMarketOrderVolume= {0}", InstrumentData.iMaxMarketOrderVolume);
		LDebug("iMinMarketOrderVolume= {0}", InstrumentData.iMinMarketOrderVolume);
		LDebug("iMaxLimitOrderVolume= {0}", InstrumentData.iMaxLimitOrderVolume);
		LDebug("iMinLimitOrderVolume= {0}", InstrumentData.iMinLimitOrderVolume);
		LDebug("iVolumeMultiple= {0}", InstrumentData.iVolumeMultiple);
		LDebug("dPriceTick= {0}", InstrumentData.dPriceTick);
		LDebug("szExpireDate= {0}", InstrumentData.szExpireDate);
		LDebug("iIsTrading= {0}", InstrumentData.iIsTrading);
		LDebug("szPositionType= {0}", InstrumentData.szPositionType);
		LDebug("szPositionDateType= {0}", InstrumentData.szPositionDateType);
		LDebug("dLongMarginRatio= {0}", InstrumentData.dLongMarginRatio);
		LDebug("dShortMarginRatio= {0}", InstrumentData.dShortMarginRatio);
		LDebug("szMaxMarginSideAlgorithm= {0}", InstrumentData.szMaxMarginSideAlgorithm);
		LDebug("dStrikePrice= {0}", InstrumentData.dStrikePrice);
		LDebug("cOptionsType= {0}", InstrumentData.cOptionsType);
		LDebug("dUnderlyingMultiple= {0}", InstrumentData.dUnderlyingMultiple);

		if (InstrumentData.dLongMarginRatio > NO_VALUE_START)
		{
			InstrumentData.dLongMarginRatio = 0;
		}

		if (InstrumentData.dShortMarginRatio > NO_VALUE_START)
		{
			InstrumentData.dShortMarginRatio = 0;
		}

		if (0 == InstrumentData.cOptionsType)
		{
			InstrumentData.cOptionsType = 'z';
		}

		if (InstrumentData.dUnderlyingMultiple > NO_VALUE_START)
		{
			InstrumentData.dUnderlyingMultiple = 0;
		}

		TradeData::InsertInstrument(InstrumentData);

	}
}

//查询合约保证金率响应
void ExecuteEngine::HandleQueryInstrumentMarginRateRsp(TdResponse<InstrumentMarginRate> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryInstrumentMarginRateRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		InstrumentMarginRate InstrumentMarginRateData = TdResponse.RspResult;


		LDebug("--------------InstrumentMarginRate---------------");
		LDebug("szInstrumentId= {0} ", InstrumentMarginRateData.szInstrumentId);
		LDebug("szExchangeId= {0} ", InstrumentMarginRateData.szExchangeId);
		LDebug("cHedgeFlag= {0} ", InstrumentMarginRateData.cHedgeFlag);
		LDebug("dLongMarginRatioByMoney= {0} ", InstrumentMarginRateData.dLongMarginRatioByMoney);
		LDebug("dLongMarginRatioByVolume= {0} ", InstrumentMarginRateData.dLongMarginRatioByVolume);
		LDebug("dShortMarginRatioByMoney= {0} ", InstrumentMarginRateData.dShortMarginRatioByMoney);
		LDebug("dShortMarginRatioByVolume= {0} ", InstrumentMarginRateData.dShortMarginRatioByVolume);
		LDebug("iIsRelative= {0} ", InstrumentMarginRateData.iIsRelative);

		//如果交易所没有赋值，则数值很大，此处改为0
		if (InstrumentMarginRateData.dLongMarginRatioByMoney > NO_VALUE_START)
		{
			InstrumentMarginRateData.dLongMarginRatioByMoney = 0;
		}
		if (InstrumentMarginRateData.dLongMarginRatioByVolume > NO_VALUE_START)
		{
			InstrumentMarginRateData.dLongMarginRatioByVolume = 0;
		}
		if (InstrumentMarginRateData.dShortMarginRatioByMoney > NO_VALUE_START)
		{
			InstrumentMarginRateData.dShortMarginRatioByMoney = 0;
		}
		if (InstrumentMarginRateData.dShortMarginRatioByVolume > NO_VALUE_START)
		{
			InstrumentMarginRateData.dShortMarginRatioByVolume = 0;
		}

		if (0 == InstrumentMarginRateData.cHedgeFlag)
		{
			InstrumentMarginRateData.cHedgeFlag = 'z';
		}


		TradeData::InsertInstrumentMarginRate(InstrumentMarginRateData);

	}
}
//查询合约手续费率响应
void ExecuteEngine::HandleQueryInstrumentCommissionRateRsp(TdResponse<InstrumentCommissionRate> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryInstrumentCommissionRateRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		InstrumentCommissionRate  InstrumentCommissionRateData = TdResponse.RspResult;

		LDebug("--------------InstrumentCommissionRate---------------");
		LDebug("szInstrumentId= {0} ", InstrumentCommissionRateData.szInstrumentId);
		LDebug("szExchangeId= {0} ", InstrumentCommissionRateData.szExchangeId);
		LDebug("dOpenRatioByMoney= {0} ", InstrumentCommissionRateData.dOpenRatioByMoney);
		LDebug("dOpenRatioByVolume= {0} ", InstrumentCommissionRateData.dOpenRatioByVolume);
		LDebug("dCloseRatioByMoney= {0} ", InstrumentCommissionRateData.dCloseRatioByMoney);
		LDebug("dCloseRatioByVolume= {0} ", InstrumentCommissionRateData.dCloseRatioByVolume);
		LDebug("dCloseTodayRatioByMoney= {0} ", InstrumentCommissionRateData.dCloseTodayRatioByMoney);
		LDebug("dCloseTodayRatioByVolume= {0} ", InstrumentCommissionRateData.dCloseTodayRatioByVolume);
		LDebug("cBizType= {0} ", InstrumentCommissionRateData.cBizType);

		if (InstrumentCommissionRateData.dOpenRatioByMoney > NO_VALUE_START)
		{
			InstrumentCommissionRateData.dOpenRatioByMoney = 0;
		}

		if (InstrumentCommissionRateData.dOpenRatioByVolume > NO_VALUE_START)
		{
			InstrumentCommissionRateData.dOpenRatioByVolume = 0;
		}

		if (InstrumentCommissionRateData.dCloseRatioByMoney > NO_VALUE_START)
		{
			InstrumentCommissionRateData.dCloseRatioByMoney = 0;
		}

		if (InstrumentCommissionRateData.dCloseRatioByVolume > NO_VALUE_START)
		{
			InstrumentCommissionRateData.dCloseRatioByVolume = 0;
		}

		if (InstrumentCommissionRateData.dCloseTodayRatioByMoney > NO_VALUE_START)
		{
			InstrumentCommissionRateData.dCloseTodayRatioByMoney = 0;
		}

		if (InstrumentCommissionRateData.dCloseTodayRatioByVolume > NO_VALUE_START)
		{
			InstrumentCommissionRateData.dCloseTodayRatioByVolume = 0;
		}

		if (0 == InstrumentCommissionRateData.cBizType)
		{
			InstrumentCommissionRateData.cBizType = 'z';
		}


		InstrumentCommissionRate TmpData;
		TradeData::GetInstrumentCommissionRates(InstrumentCommissionRateData.szInstrumentId, TmpData);
		if ("" == TmpData.szInstrumentId)
		{
			TradeData::InsertInstrumentCommissionRate(InstrumentCommissionRateData);
		}


	}
}
///查询报单手续费响应
void ExecuteEngine::HandleQueryInstrumentOrderCommRateRsp(TdResponse<InstrumentOrderCommRate> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryInstrumentOrderCommRateRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		InstrumentOrderCommRate InstrumentOrderCommRateData = TdResponse.RspResult;

		LDebug("--------------InstrumentOrderCommRate---------------");
		LDebug("szInstrumentId= {0} ", InstrumentOrderCommRateData.szInstrumentId);
		LDebug("szExchangeId= {0} ", InstrumentOrderCommRateData.szExchangeId);
		LDebug("cHedgeFlag= {0} ", InstrumentOrderCommRateData.cHedgeFlag);
		LDebug("dOrderCommByVolume= {0} ", InstrumentOrderCommRateData.dOrderCommByVolume);
		LDebug("dOrderActionCommByVolume= {0} ", InstrumentOrderCommRateData.dOrderActionCommByVolume);


		if (InstrumentOrderCommRateData.dOrderCommByVolume > NO_VALUE_START)
		{
			InstrumentOrderCommRateData.dOrderCommByVolume = 0;
		}

		if (InstrumentOrderCommRateData.dOrderActionCommByVolume > NO_VALUE_START)
		{
			InstrumentOrderCommRateData.dOrderActionCommByVolume = 0;
		}

		if (0 == InstrumentOrderCommRateData.cHedgeFlag)
		{
			InstrumentOrderCommRateData.cHedgeFlag = 'z';
		}


		InstrumentOrderCommRate TmpData;
		TradeData::GetInstrumentOrderCommRates(InstrumentOrderCommRateData.szInstrumentId, TmpData);
		if ("" == TmpData.szInstrumentId)
		{
			TradeData::InsertInstrumentOrderCommRate(InstrumentOrderCommRateData);
		}

	}
}
//查询行情响应
void ExecuteEngine::HandleQueryDepthMarketDataRsp(TdResponse<MarketData> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryDepthMarketDataRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		string szKey = TdResponse.RspResult.szExchangeId + string("_") + TdResponse.RspResult.szStockCode;
		m_MarketDataMap[szKey] = TdResponse.RspResult;
	}
}
//委托回报
void ExecuteEngine::HandleOrderNotify(TdResponse<OrderT> &TdNotify)
{
	LDebug("ExecuteEngine HandleOrderNotify , err_code= {0} msg={1}", TdNotify.iErrCode, TdNotify.szMsg);

	if (TdNotify.iErrCode == 0)
	{
		OrderT OrderData = TdNotify.RspResult;
		m_RuntimeManagerCtpMap[OrderData.szUserId]->UpdateOrder(OrderData);

		if (ORDER_STATUS_INSERT_REFUSE == OrderData.cStatus || ORDER_STATUS_CANCEL == OrderData.cStatus)
		{
			//报单被拒绝或者撤单
			//管理自成交
			char cSellBuyFlag = '\0';
			if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
			{
				//买
				cSellBuyFlag = TRADE_DIRECTION_BUY;
			}
			else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
			{
				//卖
				cSellBuyFlag = TRADE_DIRECTION_SELL;
			}

			shared_ptr<OrderT> pOrderT;
			if (m_RuntimeManagerCtpMap[OrderData.szUserId]->GetOrderByOrderRef(OrderData.szOrderRef, pOrderT))
			{
				//使用委托价格，减去此委托价格上的数量
				MngOneselfTrade(cSellBuyFlag, FLAG_DEL, OrderData.szStockCode, zutil::ConvertDoubleToInt(pOrderT->dPrice, PRICE_MULTIPLE), OrderData.iEntrustNum - OrderData.iTradeNum);
			}
		}


		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
		char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
		if (NULL == pMemBlock)
		{
			LError("pMemBlock is NULL!");
			return;
		}

		ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
		int iBodyLen = sizeof(OrderNotifyT);
		char*pBodyData = pMemPoolMng->GetMaxMemBlock(iBodyLen);
		if (NULL == pBodyData)
		{
			LError("pBodyData is NULL!");
			return;
		}
		memset(pBodyData, 0, iBodyLen);

		OrderNotifyT*pOrderNotify = (OrderNotifyT*)pBodyData;
		pOrderNotify->OrderData.iInitDate = OrderData.iInitDate;
		pOrderNotify->OrderData.iInitTime = OrderData.iInitTime;
		sprintf(pOrderNotify->OrderData.szJysInsertDate, "%s", OrderData.szJysInsertDate.c_str());
		sprintf(pOrderNotify->OrderData.szJysInsertTime, "%s", OrderData.szJysInsertTime.c_str());
		sprintf(pOrderNotify->OrderData.szUserId, "%s", OrderData.szUserId.c_str());
		sprintf(pOrderNotify->OrderData.szExchangeID, "%s", OrderData.szExchangeID.c_str());
		sprintf(pOrderNotify->OrderData.szStockCode, "%s", OrderData.szStockCode.c_str());
		sprintf(pOrderNotify->OrderData.szOrderRef, "%s", OrderData.szOrderRef.c_str());
		sprintf(pOrderNotify->OrderData.szOrderSysID, "%s", OrderData.szOrderSysID.c_str());
		pOrderNotify->OrderData.cAction = OrderData.cAction;
		pOrderNotify->OrderData.iPrice = OrderData.dPrice*PRICE_MULTIPLE;
		pOrderNotify->OrderData.iEntrustNum = OrderData.iEntrustNum;
		pOrderNotify->OrderData.iTradeNum = OrderData.iTradeNum;
		pOrderNotify->OrderData.cStatus = OrderData.cStatus;

		snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", OrderData.szUserId.c_str());
		pServerMsg->iFunctionId = FUNC_ORDER_NOTIFY;
		pServerMsg->iRoletype = ROLE_TYPE_TRADER;
		pServerMsg->iBodyLen = iBodyLen;
		pServerMsg->pBody = pBodyData;
		pServerMsg->bIsLast = true;
		pServerMsg->iErrorCode = RET_OK;
		snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "OK");

		CMsgQueueServer::GetInstance()->PostMsg(pServerMsg);
	}

}
//成交回报
void ExecuteEngine::HandleTradeNotify(TdResponse<TradeT> &TdNotify)
{
	LDebug("ExecuteEngine HandleTradeNotify , err_code= {0} msg={1}", TdNotify.iErrCode, TdNotify.szMsg);

	if (TdNotify.iErrCode == 0)
	{
		TradeT TradeData = TdNotify.RspResult;
		m_RuntimeManagerCtpMap[TradeData.szUserId]->UpdateTrade(TradeData);

		//管理自成交
		char cSellBuyFlag = '\0';
		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
		{
			//买
			cSellBuyFlag = TRADE_DIRECTION_BUY;
		}
		else if (ORDER_ACTION_SELL_CLOSE == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
		{
			//卖
			cSellBuyFlag = TRADE_DIRECTION_SELL;
		}
		shared_ptr<OrderT> pOrderT;
		if (m_RuntimeManagerCtpMap[TradeData.szUserId]->GetOrderByOrderRef(TradeData.szOrderRef, pOrderT))
		{
			//使用委托价格，减去此委托价格上的数量
			MngOneselfTrade(cSellBuyFlag, FLAG_DEL, TradeData.szStockCode, zutil::ConvertDoubleToInt(pOrderT->dPrice, PRICE_MULTIPLE), TradeData.iTradeNum);
		}


		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
		char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
		if (NULL == pMemBlock)
		{
			LError("pMemBlock is NULL!");
			return;
		}
		ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
		int iBodyLen = sizeof(TradeNotifyT);
		char*pBodyData = pMemPoolMng->GetMaxMemBlock(iBodyLen);
		if (NULL == pBodyData)
		{
			LError("pBodyData is NULL!");
			return;
		}
		memset(pBodyData, 0, iBodyLen);

		TradeNotifyT*pTradeNotify = (TradeNotifyT*)pBodyData;
		pTradeNotify->TradeData.iInitDate = TradeData.iInitDate;
		pTradeNotify->TradeData.iInitTime = TradeData.iInitTime;
		sprintf(pTradeNotify->TradeData.szJysTradeDate, "%s", TradeData.szJysTradeDate.c_str());
		sprintf(pTradeNotify->TradeData.szJysTradeTime, "%s", TradeData.szJysTradeTime.c_str());
		sprintf(pTradeNotify->TradeData.szUserId, "%s", TradeData.szUserId.c_str());
		sprintf(pTradeNotify->TradeData.szExchangeID, "%s", TradeData.szExchangeID.c_str());
		sprintf(pTradeNotify->TradeData.szStockCode, "%s", TradeData.szStockCode.c_str());
		sprintf(pTradeNotify->TradeData.szOrderRef, "%s", TradeData.szOrderRef.c_str());
		sprintf(pTradeNotify->TradeData.szOrderSysID, "%s", TradeData.szOrderSysID.c_str());
		sprintf(pTradeNotify->TradeData.szTradeID, "%s", TradeData.szTradeID.c_str());
		pTradeNotify->TradeData.cAction = TradeData.cAction;
		pTradeNotify->TradeData.iPrice = TradeData.dPrice * PRICE_MULTIPLE;
		pTradeNotify->TradeData.iTradeNum = TradeData.iTradeNum;

		snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", TradeData.szUserId.c_str());
		pServerMsg->iFunctionId = FUNC_TRADE_NOTIFY;
		pServerMsg->iRoletype = ROLE_TYPE_TRADER;
		pServerMsg->iBodyLen = iBodyLen;
		pServerMsg->pBody = pBodyData;
		pServerMsg->bIsLast = true;
		pServerMsg->iErrorCode = RET_OK;
		snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "OK");

		CMsgQueueServer::GetInstance()->PostMsg(pServerMsg);

	}

}
//委托错误通知
void ExecuteEngine::HandlePlaceOrderErrNotify(TdResponse<PlaceOrderErrNotifyT> &TdNotify)
{
	LDebug("ExecuteEngine HandlePlaceOrderErrNotify   err_code= {0} msg={1}", TdNotify.iErrCode, TdNotify.szMsg);

	if (TdNotify.iErrCode != 0)
	{
		shared_ptr<OrderT> pOrderT = nullptr;
		string szOrderRef = TdNotify.RspResult.szOrderRef;
		string szUserId = TdNotify.RspResult.szUserId;

		if (false == m_RuntimeManagerCtpMap[szUserId]->GetOrderByOrderRef(szOrderRef, pOrderT))
		{
			LError("GetOrderByOrderRef fail,  szOrderRef= {0}", szOrderRef);
			return;
		}
		string szClientOrderId = pOrderT->szClientOrderId;
		snprintf(TdNotify.RspResult.szClientOrderId, sizeof(TdNotify.RspResult.szClientOrderId), "%s", szClientOrderId.c_str());

		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
		char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
		if (NULL == pMemBlock)
		{
			LError("pMemBlock is NULL!");
			return;
		}
		ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
		int iBodyLen = sizeof(PlaceOrderErrNotifyT);
		char*pBodyData = pMemPoolMng->GetMaxMemBlock(iBodyLen);
		if (NULL == pBodyData)
		{
			LError("pBodyData is NULL!");
			return;
		}
		memset(pBodyData, 0, iBodyLen);
		memcpy(pBodyData, &TdNotify.RspResult, iBodyLen);

		snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", szUserId.c_str());
		pServerMsg->iFunctionId = FUNC_PLACE_ORDER_ERR_NOTIFY;
		pServerMsg->iRoletype = ROLE_TYPE_TRADER;
		pServerMsg->iBodyLen = iBodyLen;
		pServerMsg->pBody = pBodyData;
		pServerMsg->bIsLast = true;
		pServerMsg->iErrorCode = TdNotify.iErrCode;
		snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "%s", TdNotify.szMsg.c_str());

		CMsgQueueServer::GetInstance()->PostMsg(pServerMsg);
	}
}
//撤单错误通知
void ExecuteEngine::HandleCancelOrderErrNotify(TdResponse<CancelOrderErrNotifyT> &TdNotify)
{
	LDebug("ExecuteEngine HandleCancelOrderErrNotify, err_code= {0} msg={1}", TdNotify.iErrCode, TdNotify.szMsg);

	if (TdNotify.iErrCode != 0)
	{
		shared_ptr<OrderT> pOrderT = nullptr;
		string szOrderRef = TdNotify.RspResult.szOrderRef;
		string szUserId = TdNotify.RspResult.szUserId;

		if (false == m_RuntimeManagerCtpMap[szUserId]->GetOrderByOrderRef(szOrderRef, pOrderT))
		{
			LError("GetOrderByOrderRef fail,  szOrderRef= {0}", szOrderRef);
			return;
		}
		string szClientOrderId = pOrderT->szClientOrderId;
		snprintf(TdNotify.RspResult.szClientOrderId, sizeof(TdNotify.RspResult.szClientOrderId), "%s", szClientOrderId.c_str());


		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
		char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
		if (NULL == pMemBlock)
		{
			LError("pMemBlock is NULL!");
			return;
		}
		ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
		int iBodyLen = sizeof(CancelOrderErrNotifyT);
		char*pBodyData = pMemPoolMng->GetMaxMemBlock(iBodyLen);
		if (NULL == pBodyData)
		{
			LError("pBodyData is NULL!");
			return;
		}
		memset(pBodyData, 0, iBodyLen);
		memcpy(pBodyData, &TdNotify.RspResult, iBodyLen);

		snprintf(pServerMsg->szUserId, sizeof(pServerMsg->szUserId), "%s", szUserId.c_str());
		pServerMsg->iFunctionId = FUNC_CANCEL_ORDER_ERR_NOTIFY;
		pServerMsg->iRoletype = ROLE_TYPE_TRADER;
		pServerMsg->iBodyLen = iBodyLen;
		pServerMsg->pBody = pBodyData;
		pServerMsg->iErrorCode = TdNotify.iErrCode;
		snprintf(pServerMsg->szErrorInfo, sizeof(pServerMsg->szErrorInfo), "%s", TdNotify.szMsg.c_str());


		CMsgQueueServer::GetInstance()->PostMsg(pServerMsg);

	}
}

//风控
int ExecuteEngine::RiskRule(const OrderT&OrderData)
{
	int iRet = 0;

	iRet = RiskPosition(OrderData);
	if (iRet != 0)
	{
		return iRet;
	}

	iRet = RiskBalance(OrderData);
	if (iRet != 0)
	{
		return iRet;
	}

	iRet = RiskIndex(OrderData);
	if (iRet != 0)
	{
		return iRet;
	}

	iRet = RiskOneselfTrade(OrderData);
	if (iRet != 0)
	{
		return iRet;
	}

	iRet = RiskEntrustNum(OrderData);
	if (iRet != 0)
	{
		return iRet;
	}
	return RET_OK;
}

//检查持仓
int ExecuteEngine::RiskPosition(const OrderT&OrderData)
{
	if (false == m_RuntimeManagerCtpMap[OrderData.szUserId]->RiskPosition(OrderData))
	{
		LError("RET_NO_POSITION, szClientOrderId=[{0}]", OrderData.szClientOrderId);
		return RET_NO_POSITION;
	}

	return 0;
}
//检查资金
int ExecuteEngine::RiskBalance(const OrderT&OrderData)
{
	if (false == m_RuntimeManagerCtpMap[OrderData.szUserId]->RiskBalance(OrderData))
	{
		LError("RET_NO_BALANCE, szClientOrderId=[{0}]", OrderData.szClientOrderId);
		return RET_NO_BALANCE;
	}

	return 0;
}
//检查唯一性
int ExecuteEngine::RiskIndex(const OrderT&OrderData)
{
	int iRet = 0;
	if (m_RuntimeManagerCtpMap[OrderData.szUserId]->IsClientOrderIdExist(OrderData.szClientOrderId))
	{
		LError("ClientOrderId is  exist, szClientOrderId=[{0}]", OrderData.szClientOrderId);
		iRet = RET_CLIENT_ORDER_ID_EXIST;
		return iRet;
	}

	if (m_RuntimeManagerCtpMap[OrderData.szUserId]->IsOrderRefExist(OrderData.szOrderRef))
	{
		LError("szOrderRef is  exist, szOrderRef=[{0}]", OrderData.szOrderRef);
		iRet = RET_ORDER_REF_EXIST;
		return iRet;
	}

	return 0;
}
//检查自成交
int ExecuteEngine::RiskOneselfTrade(const OrderT&OrderData)
{
	lock_guard<mutex> lk(m_SellBuyMtx);

	unsigned long long iPrice = zutil::ConvertDoubleToInt(OrderData.dPrice, PRICE_MULTIPLE);
	unsigned long long iEntrustNum = OrderData.iEntrustNum;


	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		//买
		//检查是否有卖的
		if (m_SellMap.count(OrderData.szStockCode) > 0)
		{
			for (auto& PriceNumPair : m_SellMap[OrderData.szStockCode])
			{
				//未成交的价格和数量
				unsigned long long iNoTradePrice = PriceNumPair.first;
				unsigned long long iNoTradeEntrustNum = PriceNumPair.second;

				//卖：小于等于买的价格,数量大于0
				if (iNoTradePrice <= iPrice)
				{
					if (iNoTradeEntrustNum > 0)
					{
						LDebug("RET_ONESELF_TRADE_ERROR , iNoTradePrice=[{0}]  iNoTradeEntrustNum=[{1}]", iNoTradePrice,iNoTradeEntrustNum);
						return RET_ONESELF_TRADE_ERROR;
					}
				}
			}
		}
	}
	else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
	{
		//卖
		//检查是否有买的
		if (m_BuyMap.count(OrderData.szStockCode) > 0)
		{
			for (auto& PriceNumPair : m_BuyMap[OrderData.szStockCode])
			{
				//未成交的价格和数量
				unsigned long long iNoTradePrice = PriceNumPair.first;
				unsigned long long iNoTradeEntrustNum = PriceNumPair.second;

				//买：大于等于卖的价格,数量大于0
				if (iNoTradePrice >= iPrice)
				{
					if (iNoTradeEntrustNum > 0)
					{
						LDebug("RET_ONESELF_TRADE_ERROR , iNoTradePrice=[{0}]  iNoTradeEntrustNum=[{1}]", iNoTradePrice, iNoTradeEntrustNum);
						return RET_ONESELF_TRADE_ERROR;
					}
				}
			}
		}

	}

	return 0;
}
//检查委托数量
int ExecuteEngine::RiskEntrustNum(const OrderT&OrderData)
{
	if (OrderData.iEntrustNum <= 0)
	{
		return RET_ENTRUST_NUM_ERROR;
	}

	return 0;
}

//OrderRef:3位交易员账号+时间+3位序号
string ExecuteEngine::GetOrderRef(string&szUserId)
{
	m_iOrderRefNum++;
	if (m_iOrderRefNum > PART_ORDER_REF_MAX)
	{
		m_iOrderRefNum = 0;
	}
	char szBuf[10] = { 0 };
	//输入3位数字
	snprintf(szBuf, sizeof(szBuf), "%03d", m_iOrderRefNum);
	string szOrderRefNum = szBuf;
	string szRet = szUserId + zutil::GetStrSecondTime() + szOrderRefNum;

	return szRet;
}
/*
pReqMsg:请求消息
iErrorCode:错误码
iBodyLen：报文体数据的长度
pBody:报文体数据的首地址
*/
ServerMsg*ExecuteEngine::GetResMsg(ServerMsg*pReqMsg, int iErrorCode, int iBodyLen, void*pBody, bool bIsLast)
{
	MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
	char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
	if (NULL == pMemBlock)
	{
		LError("pMemBlock is NULL!");
		return NULL;
	}

	ServerMsg*pResMsg = (ServerMsg*)pMemBlock;
	*pResMsg = *pReqMsg;

	char*pBodyData = NULL;
	if (iBodyLen != 0)
	{
		pBodyData = pMemPoolMng->GetMaxMemBlock(iBodyLen);
		if (NULL == pBodyData)
		{
			LError("pBodyData is NULL!");
			return NULL;
		}

		memset(pBodyData, 0, iBodyLen);
		memcpy(pBodyData, pBody, iBodyLen);
	}
	pResMsg->iBodyLen = iBodyLen;
	pResMsg->pBody = pBodyData;
	pResMsg->bIsLast = bIsLast;

	string szErrMsg = "";
	if (m_ErrMap.count(iErrorCode) > 0)
	{
		szErrMsg = m_ErrMap[iErrorCode];
	}
	pResMsg->iErrorCode = iErrorCode;
	snprintf(pResMsg->szErrorInfo, sizeof(pResMsg->szErrorInfo), "%s", szErrMsg.c_str());

	return  pResMsg;
}
/*
iErrorCode:错误码
iBodyLen：报文体数据的长度
pBody:报文体数据的首地址
*/
ServerMsg*ExecuteEngine::GetResMsg(int iRequestId, int iErrorCode, int iBodyLen, void*pBody, bool bIsLast)
{
	ServerMsg  ReqData;
	if (GetReqBase(iRequestId, ReqData) != 0)
	{
		return NULL;
	}

	MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
	char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
	if (NULL == pMemBlock)
	{
		LError("pMemBlock is NULL!");
		return NULL;
	}

	ServerMsg*pResMsg = (ServerMsg*)pMemBlock;
	*pResMsg = ReqData;
	char*pBodyData = NULL;
	if (iBodyLen != 0)
	{
		pBodyData = pMemPoolMng->GetMaxMemBlock(iBodyLen);
		if (NULL == pBodyData)
		{
			LError("pBodyData is NULL!");
			return NULL;
		}

		memset(pBodyData, 0, iBodyLen);
		memcpy(pBodyData, pBody, iBodyLen);
	}
	pResMsg->iBodyLen = iBodyLen;
	pResMsg->pBody = pBodyData;
	pResMsg->bIsLast = bIsLast;

	string szErrMsg = "";
	if (m_ErrMap.count(iErrorCode) > 0)
	{
		szErrMsg = m_ErrMap[iErrorCode];
	}
	pResMsg->iErrorCode = iErrorCode;
	snprintf(pResMsg->szErrorInfo, sizeof(pResMsg->szErrorInfo), "%s", szErrMsg.c_str());

	return  pResMsg;
}


//增加请求基础信息
int ExecuteEngine::AddReqBase(int iRequestId, ServerMsg*pReqMsg)
{
	lock_guard<mutex> lk(m_RequestIdMtx);

	ServerMsg  MsgData = *pReqMsg;
	MsgData.iBodyLen = 0;
	MsgData.pBody = NULL;
	m_RequestIdMap[iRequestId] = MsgData;

	return 0;
}

//删除请求基础信息
int ExecuteEngine::DelReqBase(int iRequestId)
{
	lock_guard<mutex> lk(m_RequestIdMtx);

	m_RequestIdMap.erase(iRequestId);

	return 0;
}
//获取请求基础信息
int  ExecuteEngine::GetReqBase(int iRequestId, ServerMsg&ReqData)
{
	lock_guard<mutex> lk(m_RequestIdMtx);

	if (m_RequestIdMap.count(iRequestId) > 0)
	{
		ReqData = m_RequestIdMap[iRequestId];
	}
	else
	{
		return -1;
	}

	return 0;
}
//获取合约数据
int ExecuteEngine::ReqCtpInstrumentData()
{
	printf("---Start  ReqCtpInstrumentData---\n");

	//清空数据表
	TradeData::DeleteInstrument();
	TradeData::DeleteInstrumentMarginRate();
	TradeData::DeleteInstrumentCommissionRate();
	TradeData::DeleteInstrumentOrderCommRate();

	//查询合约
	m_TdAdapterCtp->ReqQryInstrument();

	LInfo("休眠30秒，等待接收合约");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 30));

	//加载数据库合约数据
	//合约
	vector<shared_ptr<InstrumentT>> InstrumentDatas;
	TradeData::GetInstruments(InstrumentDatas);
	for (int i = 0; i < InstrumentDatas.size(); i++)
	{
		string szKey = InstrumentDatas[i]->szExchangeId + string("_") + InstrumentDatas[i]->szInstrumentId;
		m_InstrumentMap[szKey] = InstrumentDatas[i];
		m_InstrumentExchangeMap[InstrumentDatas[i]->szInstrumentId] = InstrumentDatas[i]->szExchangeId;
	}

	//交易所有发送速度控制，不能发的太快
	const int iSleepInterval = 3 * 1000;

	for (auto& InstrumentData : m_InstrumentMap)
	{
		string szInstrumentId = InstrumentData.second->szInstrumentId;

		m_TdAdapterCtp->ReqQryInstrumentMarginRate(szInstrumentId);
		std::this_thread::sleep_for(std::chrono::milliseconds(iSleepInterval));

		printf("ReqQryInstrumentMarginRate--szInstrumentId=%s\n", szInstrumentId.c_str());

	}

	for (auto& InstrumentData : m_InstrumentMap)
	{
		string szInstrumentId = InstrumentData.second->szInstrumentId;

		m_TdAdapterCtp->ReqQryInstrumentCommissionRate(szInstrumentId);
		std::this_thread::sleep_for(std::chrono::milliseconds(iSleepInterval));


		printf("ReqQryInstrumentCommissionRate--szInstrumentId=%s\n", szInstrumentId.c_str());

	}

	for (auto& InstrumentData : m_InstrumentMap)
	{
		string szInstrumentId = InstrumentData.second->szInstrumentId;

		m_TdAdapterCtp->ReqQryInstrumentOrderCommRate(szInstrumentId);
		std::this_thread::sleep_for(std::chrono::milliseconds(iSleepInterval));

		printf("ReqQryInstrumentOrderCommRate--szInstrumentId=%s\n", szInstrumentId.c_str());

	}

	printf("---Stop  ReqCtpInstrumentData---\n");

	return 0;
}

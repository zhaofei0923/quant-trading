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
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//����2���߳�
	SetThreadCount(2);

	InitErrMsg();

	ConfigData*pConfigData = ConfigData::GetInstance();
	//�����˻�
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
	m_ErrMap[RET_FAIL] = "ʧ��";
	m_ErrMap[RET_ACCOUNT_NOT_EXIST] = "�˺Ų�����";
	m_ErrMap[RET_ACCOUNT_PASSWORD_ERROR] = "�������";
	m_ErrMap[RET_JYS_ERROR] = "����������";
	m_ErrMap[RET_ORDER_REF_EXIST] = "OrderRef�Ѿ�����";
	m_ErrMap[RET_ORDER_REF_NOT_EXIST] = "OrderRef������";
	m_ErrMap[RET_SEND_JYS_FAIL] = "���͵�������ʧ��";
	m_ErrMap[RET_CLIENT_ORDER_ID_EXIST] = "ClientOrderId�Ѿ�����";
	m_ErrMap[RET_CLIENT_ORDER_ID_NOT_EXIST] = "ClientOrderId������";
	m_ErrMap[RET_CLIENT_MSG_ID_EXIST] = "�ͻ���ReqId�Ѿ�����";
	m_ErrMap[RET_RECORD_NOT_EXIST] = "��¼������";
	m_ErrMap[RET_PASSWORD_ERROR] = "�������";
	m_ErrMap[RET_ROLE_ERROR] = "��ɫ���ʹ���";
	m_ErrMap[RET_CREATE_MSG_QUEUE_FAIL] = "������Ϣ����ʧ��";
	m_ErrMap[RET_MSG_QUEUE_EXIST] = "��Ϣ�����Ѿ�����";
	m_ErrMap[RET_MSG_QUEUE_NOT_EXIST] = "��Ϣ���в�����";
	m_ErrMap[RET_NO_RIGHT] = "û��Ȩ��";
	m_ErrMap[RET_NO_RCV_FRONTID_SESSIONID] = "û���յ�FrontID��SessionID";
	m_ErrMap[RET_UP_HIGH_PRICE] = "������ͣ��";
	m_ErrMap[RET_DOWN_LOW_PRICE] = "���ڵ�ͣ��";
	m_ErrMap[RET_NO_POSITION] = "û�гֲ�";
	m_ErrMap[RET_NO_BALANCE] = "û���ʽ�";
	m_ErrMap[RET_NO_STOCK_CODE] = "û�й�Ʊ����";
	m_ErrMap[RET_ENTRUST_NUM_ERROR] = "ί����������";
	m_ErrMap[RET_ONESELF_TRADE_ERROR] = "�Գɽ�����";

	return 0;
}
int ExecuteEngine::Start()
{
	RunThreads();

	LInfo("����3�룬�ȴ�ctp���ӷ�����");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));

	//�ж��Ƿ��¼�ɹ�
	while (false == m_TdAdapterCtp->IsLogin())
	{
		LError("ctp��¼ʧ��");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	}

	//��ȡ��Լ���ݡ�����ReqCtpInstrumentData()����1�ξͿ����ˡ�
	//��Ҫ�ظ�����ReqCtpInstrumentData()�������ظ����ûᵼ�����ݱ�������ظ�
	//ReqCtpInstrumentData();

	std::cout << "Start ReqQrySettlementInfoConfirm" << endl;
	LInfo("Start ReqQrySettlementInfoConfirm");

	//�����ѯ������Ϣȷ��
	m_TdAdapterCtp->ReqQrySettlementInfoConfirm();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));

	std::cout << "Start ReqQryDepthMarketData" << endl;
	LInfo("Start ReqQryDepthMarketData");

	//��ѯ����
	m_TdAdapterCtp->ReqQryDepthMarketData("", "");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 10));

	std::cout << "Start ReqQryBrokerTradingParams" << endl;
	LInfo("Start ReqQryBrokerTradingParams");

	//�����ѯ���͹�˾���ײ���
	m_TdAdapterCtp->ReqQryBrokerTradingParams();

	std::cout << "Start get ctp instrument data" << endl;
	LInfo("Start get ctp instrument data");

	//�������ݿ��Լ����
	//��Լ
	vector<shared_ptr<InstrumentT>> InstrumentDatas;
	TradeData::GetInstruments(InstrumentDatas);
	for (int i = 0; i < InstrumentDatas.size(); i++)
	{
		string szKey = InstrumentDatas[i]->szExchangeId + string("_") + InstrumentDatas[i]->szInstrumentId;
		m_InstrumentMap[szKey] = InstrumentDatas[i];
		m_InstrumentExchangeMap[InstrumentDatas[i]->szInstrumentId] = InstrumentDatas[i]->szExchangeId;
	}

	//��Լ��֤����
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

	//��Լ��������
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

	//����������
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
		//����
		RuntimeManagerPair.second->SettlePosition(m_MarketDataMap, m_InstrumentMap, m_InstrumentMarginRateMap);
		//����order������Ϣ
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

		//���׹���ѡ��0���̣߳���������ѡ��1���߳�
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

//��¼
void ExecuteEngine::OnLogin(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	int iRet = 0;
	//��¼����
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
			//�������
			LError("login fail,password is error, szUserId=[{0}]", szUserId);
			iRet = RET_PASSWORD_ERROR;
		}

	}
	else
	{
		//�û�������
		LError("login fail,szUserId is not exist, szUserId=[{0}]", szUserId);
		iRet = RET_ACCOUNT_NOT_EXIST;
	}

	ServerMsg*pResMsg = GetResMsg(pReqMsg, iRet, 0, NULL);
	if (pResMsg != NULL)
	{
		CTcpServer::GetInstance()->PostMsg(pResMsg);
	}

}
//�˳�
void ExecuteEngine::OnLogout(ServerMsg*pReqMsg)
{
	ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_OK, 0, NULL);
	if (pResMsg != NULL)
	{
		CTcpServer::GetInstance()->PostMsg(pResMsg);
	}
}
//��ѯ�ʽ�
void ExecuteEngine::OnQueryBalance(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//����Ա��Balance
		int iRequestId = m_TdAdapterCtp->ReqQueryBalance();
		if (iRequestId >= 0)
		{
			//���ͳɹ�
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//����ʧ��
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
//��ѯί��
void ExecuteEngine::OnQueryOrder(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�

	QueryOrderReqT*pQueryOrderReqT = (QueryOrderReqT*)(pReqMsg->pBody);
	int iFlag = pQueryOrderReqT->iFlag;//0-�鵱��ȫ����1-����szClientOrderId��ѯ
	string szClientOrderId = pQueryOrderReqT->szClientOrderId;

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		if (1 == iFlag)
		{
			//û��Ȩ��
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
			//���ͳɹ�
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//����ʧ��
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
		//0-�鵱��ȫ����1-����szClientOrderId��ѯ
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
//��ѯ�ɽ�
void ExecuteEngine::OnQueryTrade(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�

	QueryTradeReqT*pQueryTradeReqT = (QueryTradeReqT*)(pReqMsg->pBody);
	int iFlag = pQueryTradeReqT->iFlag;//0-�鵱��ȫ����1-����szClientOrderId��ѯ
	string szClientOrderId = pQueryTradeReqT->szClientOrderId;

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		if (1 == iFlag)
		{
			//û��Ȩ��
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
			//���ͳɹ�
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//����ʧ��
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
		//0-�鵱��ȫ����1-����szClientOrderId��ѯ
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
//��ѯ�ֲ�
void ExecuteEngine::OnQueryPosition(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		int iRequestId = m_TdAdapterCtp->ReqQueryPosition();
		if (iRequestId >= 0)
		{
			//���ͳɹ�
			AddReqBase(iRequestId, pReqMsg);
		}
		else
		{
			//����ʧ��
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
//����ͨѶ��Ϣ����
void ExecuteEngine::OnAddMsgQueue(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//����Աû��Ȩ�ޣ�����Ա������������
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
//ɾ��ͨѶ��Ϣ����
void ExecuteEngine::OnDelMsgQueue(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//����Աû��Ȩ�ޣ�����Ա������������
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
//���������˺��ʽ�
void ExecuteEngine::OnAddDelMoney(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�

	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		AddDelMoneyReqT*pAddDelMoneyReqT = (AddDelMoneyReqT*)pReqMsg->pBody;
		string szTradeUserId = pAddDelMoneyReqT->szUserId;
		LInfo("�����ʽ�:szTradeUserId=[{0}],iFlag=[{1}],iAmount=[{2}] ", szTradeUserId, pAddDelMoneyReqT->iFlag, pAddDelMoneyReqT->iAmount);

		if (0 == m_RuntimeManagerCtpMap.count(szTradeUserId))
		{
			//�˺Ų�����
			ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_ACCOUNT_NOT_EXIST, 0, NULL);
			if (pResMsg != NULL)
			{
				CTcpServer::GetInstance()->PostMsg(pResMsg);
			}
			return;
		}

		BalanceT BalanceData;
		//������־��0-�����ʽ�1-�����ʽ�
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
		//û��Ȩ��
		ServerMsg*pResMsg = GetResMsg(pReqMsg, RET_NO_RIGHT, 0, NULL);
		if (pResMsg != NULL)
		{
			CTcpServer::GetInstance()->PostMsg(pResMsg);
		}
	}


}
//ί��
void ExecuteEngine::OnSendOrder(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//����Աû��Ȩ�ޣ�����Ա������������
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

		//ctp����û��ExchangeID���˴���ȡExchangeID
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
			//�еĲ���ȡ����������
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
			//��ͷ
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dLongMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dLongMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByVolume;
		}
		else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//��ͷ
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dCloseRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dCloseRatioByVolume;
			OrderData.dCloseTodayRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dCloseTodayRatioByMoney;
			OrderData.dCloseTodayRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dCloseTodayRatioByVolume;

		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction)
		{
			//��ͷ
			OrderData.dMarginRatioByMoney = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByMoney;
			OrderData.dMarginRatioByVolume = m_InstrumentMarginRateMap[szKey]->dShortMarginRatioByVolume;
			OrderData.dRatioByMoney = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByMoney;
			OrderData.dRatioByVolume = m_InstrumentCommissionRateMap[szKey]->dOpenRatioByVolume;

		}
		else if (ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			//��ͷ
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
//����
void ExecuteEngine::OnCancelOrder(ServerMsg*pReqMsg)
{
	int  iRoletype = pReqMsg->iRoletype;
	string szUserId = pReqMsg->szUserId;  //�˺�
	if (ROLE_TYPE_ADMIN == iRoletype)
	{
		//����Աû��Ȩ�ޣ�����Ա������������
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

	//��ؼ��
	iRet = RiskRule(OrderData);
	if (iRet != RET_OK)
	{
		return iRet;
	}

	//�����Գɽ�
	char cSellBuyFlag = '\0';
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		//��
		cSellBuyFlag = TRADE_DIRECTION_BUY;
	}
	else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
	{
		//��
		cSellBuyFlag = TRADE_DIRECTION_SELL;
	}
	MngOneselfTrade(cSellBuyFlag, FLAG_ADD, OrderData.szStockCode, zutil::ConvertDoubleToInt(OrderData.dPrice, PRICE_MULTIPLE), OrderData.iEntrustNum);

	//����order
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
			BalanceData.dCommission = 0 - OrderData.dOrderCommByVolume;//����������
			LDebug("szOrderRef=[{0}],OrderData.dOrderCommByVolume=[{1}]", OrderData.szOrderRef, OrderData.dOrderCommByVolume);

			m_RuntimeManagerCtpMap[OrderData.szUserId]->UpdateBalance(BalanceData);
		}

	}
	else
	{
		OrderData.cStatus = ORDER_STATUS_SEND_FAIL;
		iRet = RET_SEND_JYS_FAIL;
	}

	//����order
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
			BalanceData.dCommission = 0 - pOrderT->dOrderActionCommByVolume;//����������
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
//�����Գɽ�
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
				//�����ڴ����
			}
			else
			{
				if (0 == m_BuyMap[szStockCode].count(iPrice))
				{
					//�����ڴ����
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
				//�����ڴ����
			}
			else
			{
				if (0 == m_SellMap[szStockCode].count(iPrice))
				{
					//�����ڴ����
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

//��ѯ�ʽ���Ӧ
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
//��ѯί����Ӧ
void ExecuteEngine::HandleQueryOrderRsp(TdResponse<OrderT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryOrderRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		OrderT&OrderData = TdResponse.RspResult;
		//��ӡί��
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

		//���͵����һ����¼ʱ��ɾ���洢��������Ϣ
		if (TdResponse.bIsLast)
		{
			DelReqBase(TdResponse.iRequestId);
		}
	}
}
//��ѯ�ɽ���Ӧ
void ExecuteEngine::HandleQueryTradeRsp(TdResponse<TradeT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryTradeRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		TradeT &TradeData = TdResponse.RspResult;

		//��ӡ����
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

		//���͵����һ����¼ʱ��ɾ���洢��������Ϣ
		if (TdResponse.bIsLast)
		{
			DelReqBase(TdResponse.iRequestId);
		}
	}
}
//��ѯ�ֲ���Ӧ
void ExecuteEngine::HandleQueryPositionRsp(TdResponse<PositionT> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryPositionRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		PositionT &PositionData = TdResponse.RspResult;
		//��ӡ�ֲ�
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

		//���͵����һ����¼ʱ��ɾ���洢��������Ϣ
		if (TdResponse.bIsLast)
		{
			DelReqBase(TdResponse.iRequestId);
		}
	}

}

//��ѯ��Լ
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

//��ѯ��Լ��֤������Ӧ
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

		//���������û�и�ֵ������ֵ�ܴ󣬴˴���Ϊ0
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
//��ѯ��Լ����������Ӧ
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
///��ѯ������������Ӧ
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
//��ѯ������Ӧ
void ExecuteEngine::HandleQueryDepthMarketDataRsp(TdResponse<MarketData> &TdResponse)
{
	LDebug("ExecuteEngine HandleQueryDepthMarketDataRsp, err_code= {0} msg={1}", TdResponse.iErrCode, TdResponse.szMsg);

	if (TdResponse.iErrCode == 0)
	{
		string szKey = TdResponse.RspResult.szExchangeId + string("_") + TdResponse.RspResult.szStockCode;
		m_MarketDataMap[szKey] = TdResponse.RspResult;
	}
}
//ί�лر�
void ExecuteEngine::HandleOrderNotify(TdResponse<OrderT> &TdNotify)
{
	LDebug("ExecuteEngine HandleOrderNotify , err_code= {0} msg={1}", TdNotify.iErrCode, TdNotify.szMsg);

	if (TdNotify.iErrCode == 0)
	{
		OrderT OrderData = TdNotify.RspResult;
		m_RuntimeManagerCtpMap[OrderData.szUserId]->UpdateOrder(OrderData);

		if (ORDER_STATUS_INSERT_REFUSE == OrderData.cStatus || ORDER_STATUS_CANCEL == OrderData.cStatus)
		{
			//�������ܾ����߳���
			//�����Գɽ�
			char cSellBuyFlag = '\0';
			if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
			{
				//��
				cSellBuyFlag = TRADE_DIRECTION_BUY;
			}
			else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
			{
				//��
				cSellBuyFlag = TRADE_DIRECTION_SELL;
			}

			shared_ptr<OrderT> pOrderT;
			if (m_RuntimeManagerCtpMap[OrderData.szUserId]->GetOrderByOrderRef(OrderData.szOrderRef, pOrderT))
			{
				//ʹ��ί�м۸񣬼�ȥ��ί�м۸��ϵ�����
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
//�ɽ��ر�
void ExecuteEngine::HandleTradeNotify(TdResponse<TradeT> &TdNotify)
{
	LDebug("ExecuteEngine HandleTradeNotify , err_code= {0} msg={1}", TdNotify.iErrCode, TdNotify.szMsg);

	if (TdNotify.iErrCode == 0)
	{
		TradeT TradeData = TdNotify.RspResult;
		m_RuntimeManagerCtpMap[TradeData.szUserId]->UpdateTrade(TradeData);

		//�����Գɽ�
		char cSellBuyFlag = '\0';
		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
		{
			//��
			cSellBuyFlag = TRADE_DIRECTION_BUY;
		}
		else if (ORDER_ACTION_SELL_CLOSE == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
		{
			//��
			cSellBuyFlag = TRADE_DIRECTION_SELL;
		}
		shared_ptr<OrderT> pOrderT;
		if (m_RuntimeManagerCtpMap[TradeData.szUserId]->GetOrderByOrderRef(TradeData.szOrderRef, pOrderT))
		{
			//ʹ��ί�м۸񣬼�ȥ��ί�м۸��ϵ�����
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
//ί�д���֪ͨ
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
//��������֪ͨ
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

//���
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

//���ֲ�
int ExecuteEngine::RiskPosition(const OrderT&OrderData)
{
	if (false == m_RuntimeManagerCtpMap[OrderData.szUserId]->RiskPosition(OrderData))
	{
		LError("RET_NO_POSITION, szClientOrderId=[{0}]", OrderData.szClientOrderId);
		return RET_NO_POSITION;
	}

	return 0;
}
//����ʽ�
int ExecuteEngine::RiskBalance(const OrderT&OrderData)
{
	if (false == m_RuntimeManagerCtpMap[OrderData.szUserId]->RiskBalance(OrderData))
	{
		LError("RET_NO_BALANCE, szClientOrderId=[{0}]", OrderData.szClientOrderId);
		return RET_NO_BALANCE;
	}

	return 0;
}
//���Ψһ��
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
//����Գɽ�
int ExecuteEngine::RiskOneselfTrade(const OrderT&OrderData)
{
	lock_guard<mutex> lk(m_SellBuyMtx);

	unsigned long long iPrice = zutil::ConvertDoubleToInt(OrderData.dPrice, PRICE_MULTIPLE);
	unsigned long long iEntrustNum = OrderData.iEntrustNum;


	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		//��
		//����Ƿ�������
		if (m_SellMap.count(OrderData.szStockCode) > 0)
		{
			for (auto& PriceNumPair : m_SellMap[OrderData.szStockCode])
			{
				//δ�ɽ��ļ۸������
				unsigned long long iNoTradePrice = PriceNumPair.first;
				unsigned long long iNoTradeEntrustNum = PriceNumPair.second;

				//����С�ڵ�����ļ۸�,��������0
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
		//��
		//����Ƿ������
		if (m_BuyMap.count(OrderData.szStockCode) > 0)
		{
			for (auto& PriceNumPair : m_BuyMap[OrderData.szStockCode])
			{
				//δ�ɽ��ļ۸������
				unsigned long long iNoTradePrice = PriceNumPair.first;
				unsigned long long iNoTradeEntrustNum = PriceNumPair.second;

				//�򣺴��ڵ������ļ۸�,��������0
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
//���ί������
int ExecuteEngine::RiskEntrustNum(const OrderT&OrderData)
{
	if (OrderData.iEntrustNum <= 0)
	{
		return RET_ENTRUST_NUM_ERROR;
	}

	return 0;
}

//OrderRef:3λ����Ա�˺�+ʱ��+3λ���
string ExecuteEngine::GetOrderRef(string&szUserId)
{
	m_iOrderRefNum++;
	if (m_iOrderRefNum > PART_ORDER_REF_MAX)
	{
		m_iOrderRefNum = 0;
	}
	char szBuf[10] = { 0 };
	//����3λ����
	snprintf(szBuf, sizeof(szBuf), "%03d", m_iOrderRefNum);
	string szOrderRefNum = szBuf;
	string szRet = szUserId + zutil::GetStrSecondTime() + szOrderRefNum;

	return szRet;
}
/*
pReqMsg:������Ϣ
iErrorCode:������
iBodyLen�����������ݵĳ���
pBody:���������ݵ��׵�ַ
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
iErrorCode:������
iBodyLen�����������ݵĳ���
pBody:���������ݵ��׵�ַ
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


//�������������Ϣ
int ExecuteEngine::AddReqBase(int iRequestId, ServerMsg*pReqMsg)
{
	lock_guard<mutex> lk(m_RequestIdMtx);

	ServerMsg  MsgData = *pReqMsg;
	MsgData.iBodyLen = 0;
	MsgData.pBody = NULL;
	m_RequestIdMap[iRequestId] = MsgData;

	return 0;
}

//ɾ�����������Ϣ
int ExecuteEngine::DelReqBase(int iRequestId)
{
	lock_guard<mutex> lk(m_RequestIdMtx);

	m_RequestIdMap.erase(iRequestId);

	return 0;
}
//��ȡ���������Ϣ
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
//��ȡ��Լ����
int ExecuteEngine::ReqCtpInstrumentData()
{
	printf("---Start  ReqCtpInstrumentData---\n");

	//������ݱ�
	TradeData::DeleteInstrument();
	TradeData::DeleteInstrumentMarginRate();
	TradeData::DeleteInstrumentCommissionRate();
	TradeData::DeleteInstrumentOrderCommRate();

	//��ѯ��Լ
	m_TdAdapterCtp->ReqQryInstrument();

	LInfo("����30�룬�ȴ����պ�Լ");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 30));

	//�������ݿ��Լ����
	//��Լ
	vector<shared_ptr<InstrumentT>> InstrumentDatas;
	TradeData::GetInstruments(InstrumentDatas);
	for (int i = 0; i < InstrumentDatas.size(); i++)
	{
		string szKey = InstrumentDatas[i]->szExchangeId + string("_") + InstrumentDatas[i]->szInstrumentId;
		m_InstrumentMap[szKey] = InstrumentDatas[i];
		m_InstrumentExchangeMap[InstrumentDatas[i]->szInstrumentId] = InstrumentDatas[i]->szExchangeId;
	}

	//�������з����ٶȿ��ƣ����ܷ���̫��
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

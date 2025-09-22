#ifndef _EXECUTE_ENGINE_H
#define _EXECUTE_ENGINE_H

#include <memory>
#include "struct.h"
#include "message.h"
#include "typedef.h"
#include "runtime_manager_ctp.h"
#include "td_adapter_ctp.h"
#include "thread_group.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class ExecuteEngine :public CThreadGroup
{
public:
	static ExecuteEngine *GetInstance()
	{
		// C++11 style singleton
		static ExecuteEngine execute_engine;
		return &execute_engine;
	}

	~ExecuteEngine();
private:
	// keep singleton save, set constructor and copy construstor private
	ExecuteEngine();
	ExecuteEngine(const ExecuteEngine &other) {}

public:
	int Init();
	int InitErrMsg();
	int Start();
	int Stop();
	int Release();
	virtual int DealMsg(void*pMsg, int iThreadId);
	virtual int PostMsg(void*pMsg);
	int DisPatchMsg(ServerMsg*pServerMsg, int iThreadId);
public:
	//登录
	void OnLogin(ServerMsg*pReqMsg);
	//退出
	void OnLogout(ServerMsg*pReqMsg);
	//查询资金
	void OnQueryBalance(ServerMsg*pReqMsg);
	//查询委托
	void OnQueryOrder(ServerMsg*pReqMsg);
	//查询成交
	void OnQueryTrade(ServerMsg*pReqMsg);
	//查询持仓
	void OnQueryPosition(ServerMsg*pReqMsg);
	//委托
	void OnSendOrder(ServerMsg*pReqMsg);
	//撤单
	void OnCancelOrder(ServerMsg*pReqMsg);
	//增加通讯消息队列
	void OnAddMsgQueue(ServerMsg*pReqMsg);
	//删除通讯消息队列
	void OnDelMsgQueue(ServerMsg*pReqMsg);
	//增减交易账号资金
	void OnAddDelMoney(ServerMsg*pReqMsg);
public:
	int SendOrder(OrderT&OrderData);
	int CancelOldOrder(const string&szUserId, const string& szOrderRef);
	//管理自成交
	int MngOneselfTrade(char cSellBuyFlag, int iAddDelFlag, string&szStockCode, unsigned long long iPrice, int iNum);
public:
	//查询资金响应
	void HandleQueryBalanceRsp(TdResponse<BalanceT> &TdResponse);
	//查询委托响应
	void HandleQueryOrderRsp(TdResponse<OrderT> &TdResponse);
	//查询成交响应
	void HandleQueryTradeRsp(TdResponse<TradeT> &TdResponse);
	//查询持仓响应
	void HandleQueryPositionRsp(TdResponse<PositionT> &TdResponse);
	//查询合约响应
	void HandleQueryInstrumentRsp(TdResponse<InstrumentT> &TdResponse);
	//查询合约保证金率响应
	void HandleQueryInstrumentMarginRateRsp(TdResponse<InstrumentMarginRate> &TdResponse);
	//查询合约手续费率响应
	void HandleQueryInstrumentCommissionRateRsp(TdResponse<InstrumentCommissionRate> &TdResponse);
	///查询报单手续费响应
	void HandleQueryInstrumentOrderCommRateRsp(TdResponse<InstrumentOrderCommRate> &TdResponse);
	//查询行情响应
	void HandleQueryDepthMarketDataRsp(TdResponse<MarketData> &TdResponse);
	//委托回报
	void HandleOrderNotify(TdResponse<OrderT> &TdNotify);
	//成交回报
	void HandleTradeNotify(TdResponse<TradeT> &TdNotify);
	//委托错误通知
	void HandlePlaceOrderErrNotify(TdResponse<PlaceOrderErrNotifyT> &TdNotify);
	//撤单错误通知
	void HandleCancelOrderErrNotify(TdResponse<CancelOrderErrNotifyT> &TdNotify);
public:
	//风控
	int RiskRule(const OrderT&OrderData);
	//检查持仓
	int RiskPosition(const OrderT&OrderData);
	//检查资金
	int RiskBalance(const OrderT&OrderData);
	//检查唯一性
	int RiskIndex(const OrderT&OrderData);
	//检查自成交
	int RiskOneselfTrade(const OrderT&OrderData);
	//检查委托数量
	int RiskEntrustNum(const OrderT&OrderData);

public:
	string GetOrderRef(string&szUserId);
	ServerMsg*GetResMsg(ServerMsg*pReqMsg, int iErrorCode, int iBodyLen, void*pBody, bool bIsLast = true);
	ServerMsg*GetResMsg(int iRequestId, int iErrorCode, int iBodyLen, void*pBody, bool bIsLast = true);
	int AddReqBase(int iRequestId, ServerMsg*pReqMsg);
	int DelReqBase(int iRequestId);
	int GetReqBase(int iRequestId, ServerMsg&ReqData);
public:
	//获取合约数据
	int ReqCtpInstrumentData();

private:
	map<string, shared_ptr<RuntimeManagerCtp>> m_RuntimeManagerCtpMap; // 
	map<int, string > m_ErrMap; //错误码，错误消息
	map<string, User> m_UserMap; //UserId,User
	map<int, ServerMsg> m_RequestIdMap; //RequestId，ServerMsg

	MarketDataInfo m_MarketDataMap; //ExchangeID+InstrumentID,行情数据
	InstrumentExchange m_InstrumentExchangeMap; //InstrumentID,ExchangeID
	InstrumentInfo m_InstrumentMap; //ExchangeID+InstrumentID,合约
	InstrumentMarginRateInfo m_InstrumentMarginRateMap; //ExchangeID+InstrumentID,合约保证金率
	InstrumentCommissionRateInfo m_InstrumentCommissionRateMap; //ExchangeID+InstrumentID,合约手续费率
	InstrumentOrderCommRateInfo m_InstrumentOrderCommRateMap; //ExchangeID+InstrumentID,报单手续费

	map<string, map<unsigned long long, unsigned long long>> m_SellMap; //<InstrumentID,<价格,卖的数量>>   存储未成交的委托，用于自成交风控
	map<string, map<unsigned long long, unsigned long long>> m_BuyMap; //<InstrumentID,<价格,买的数量 >>  存储未成交的委托，用于自成交风控
private:
	TdAdapterCtp *m_TdAdapterCtp;
	int m_iOrderRefNum;  //OrderRef用到的序号
	mutex m_RequestIdMtx; //锁m_RequestIdMap
	mutex m_SellBuyMtx;  //锁m_SellMap,m_BuyMap
};

#endif 

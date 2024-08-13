#ifndef _RUNTIME_MANAGER_H
#define _RUNTIME_MANAGER_H
#include <vector>
#include <map>
#include <list>
#include <mutex>
#include <memory>
#include "struct.h"
#include "const.h"
#include "typedef.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class RuntimeManagerCtp
{
public:
	RuntimeManagerCtp(string &szUserId);
	virtual ~RuntimeManagerCtp();

public:
	//初始化
	void Init();
	//结算持仓
	void SettlePosition(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates);
	//更新order费率信息
	void UpdateOrderRate(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates, InstrumentCommissionRateInfo&CommissionRates, InstrumentOrderCommRateInfo&OrderCommRates);
public:
	//更新资金
	void UpdateBalance(BalanceT &BalanceData);
	//更新委托
	void UpdateOrder(OrderT &OrderData);
	//更新委托
	void UpdateOrder(TradeT &TradeData);
	//更新交易
	void UpdateTrade(TradeT &TradeData);
	//更新平仓盈亏
	void UpdateCloseProfit(TradeT &TradeData);
	//更新持仓
	void UpdatePosition(PositionT &PositionData, bool bIsLock = true);
	//更新持仓
	void UpdatePositionByUnLock(PositionT &PositionData);
	//根据委托更新持仓
	bool UpdatePositionByOrder(OrderT &OrderData);
	//根据交易更新持仓
	bool UpdatePositionByTrade(TradeT &TradeData);
	//更新用于计算手续费的开仓数量
	bool  UpdateCommissionOpenCount(TradeT &TradeData);
	//获取平仓手续费
	double GetCloseCommission(OrderT &OrderData);
	//获取平仓手续费
	double GetCloseCommission(TradeT &TradeData, OrderT &OrderData);

public:
	//查询资金
	BalanceT GetBalance();
	//根据客户端ClientOrderId查询委托
	bool GetOrderByClientOrderId(const string& szClientOrderId, shared_ptr<OrderT>& pOrderT);
	//根据OrderRef查询委托
	bool GetOrderByOrderRef(string& szOrderRef, shared_ptr<OrderT>& pOrderT);
	//查询当天所有委托
	bool GetOrders(vector<shared_ptr<OrderT>>& Orders);
	//根据客户端ClientOrderId查询交易
	bool GetTradesByClientOrderId(string& szClientOrderId, vector<shared_ptr<TradeT>>&vTradeT);
	//根据OrderRef查询交易
	bool GetTradesByOrderRef(string& szOrderRef, vector<shared_ptr<TradeT>>&vTradeT);
	//查询当天所有交易
	bool GetTrades(vector<shared_ptr<TradeT>>& Trades);
	//根据股票代码查询持仓
	bool GetPositionBySecurity(string &szExchangeID, string &szStockCode, vector<shared_ptr<PositionT>>&vPositionT);
	//根据股票代码和持仓方向查询持仓
	bool GetPositionBySecurity(string &szExchangeID, string &szStockCode, char cDirection, shared_ptr<PositionT>&pPosition);
	//查询所有股票的持仓
	bool GetPositions(vector<shared_ptr<PositionT>> &Positions);
	//获取未成交的数量
	int GetNotTradeNum(string& szOrderRef);
public:
	//风险控制:检查资金
	bool RiskBalance(const OrderT&OrderData);
	//风险控制:检查持仓
	bool RiskPosition(const OrderT&OrderData);
public:
	//szClientOrderId是否存在
	bool IsClientOrderIdExist(const string& szClientOrderId);
	//szOrderRef是否存在
	bool IsOrderRefExist(const string& szOrderRef);

public:
	string m_szUserId;
	BalanceT m_Balance; // current balance
	//map<string, string> m_ReqIdOrderRefMap; // szReqId -> szOrderRef
	map<string, string> m_ClientOrderIdOrderRefMap; // szClientOrderId -> szOrderRef

	map<string, shared_ptr<OrderT>> m_OrdersMap; // szOrderRef -> Order
	map<string, shared_ptr<list<string>>> m_OrderTradeMap; // szOrderRef -> list<szTradeID>
	map<string, vector<string>> m_StockCodeTradeMap; //存储开仓数据  szExchangeID_szStockCode_cDirection->vector<szTradeID>  
	map<string, shared_ptr<TradeT>> m_TradesMap; // szTradeID -> Trade
	map<string, shared_ptr<PositionT>> m_PositionsMap; // szExchangeID_szStockCode_cDirection ->Position
	map<string, shared_ptr<OpenCountT>>  m_CommissionOpenCountMap;  //存储用于手续费计算的股票开仓数量。<szExchangeID_szStockCode_cDirection，OpenCountT>

	mutex m_BalanceMtx;  //锁m_Balance
	mutex m_OrderMtx;  //锁m_ReqIdOrderRefMap，m_OrdersMap
	mutex m_TradeMtx;//锁m_OrderTradeMap，m_TradesMap,m_StockCodeTradeMap
	mutex m_PositionMtx;//锁m_PositionsMap
	mutex m_CommissionOpenCountMtx;//锁m_CommissionOpenCountMap
};


#endif // !_RUNTIME_MANAGER_H

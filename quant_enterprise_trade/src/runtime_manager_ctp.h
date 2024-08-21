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
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

class RuntimeManagerCtp
{
public:
	RuntimeManagerCtp(string &szUserId);
	virtual ~RuntimeManagerCtp();

public:
	//��ʼ��
	void Init();
	//����ֲ�
	void SettlePosition(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates);
	//����order������Ϣ
	void UpdateOrderRate(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates, InstrumentCommissionRateInfo&CommissionRates, InstrumentOrderCommRateInfo&OrderCommRates);
public:
	//�����ʽ�
	void UpdateBalance(BalanceT &BalanceData);
	//����ί��
	void UpdateOrder(OrderT &OrderData);
	//����ί��
	void UpdateOrder(TradeT &TradeData);
	//���½���
	void UpdateTrade(TradeT &TradeData);
	//����ƽ��ӯ��
	void UpdateCloseProfit(TradeT &TradeData);
	//���³ֲ�
	void UpdatePosition(PositionT &PositionData, bool bIsLock = true);
	//���³ֲ�
	void UpdatePositionByUnLock(PositionT &PositionData);
	//����ί�и��³ֲ�
	bool UpdatePositionByOrder(OrderT &OrderData);
	//���ݽ��׸��³ֲ�
	bool UpdatePositionByTrade(TradeT &TradeData);
	//�������ڼ��������ѵĿ�������
	bool  UpdateCommissionOpenCount(TradeT &TradeData);
	//��ȡƽ��������
	double GetCloseCommission(OrderT &OrderData);
	//��ȡƽ��������
	double GetCloseCommission(TradeT &TradeData, OrderT &OrderData);

public:
	//��ѯ�ʽ�
	BalanceT GetBalance();
	//���ݿͻ���ClientOrderId��ѯί��
	bool GetOrderByClientOrderId(const string& szClientOrderId, shared_ptr<OrderT>& pOrderT);
	//����OrderRef��ѯί��
	bool GetOrderByOrderRef(string& szOrderRef, shared_ptr<OrderT>& pOrderT);
	//��ѯ��������ί��
	bool GetOrders(vector<shared_ptr<OrderT>>& Orders);
	//���ݿͻ���ClientOrderId��ѯ����
	bool GetTradesByClientOrderId(string& szClientOrderId, vector<shared_ptr<TradeT>>&vTradeT);
	//����OrderRef��ѯ����
	bool GetTradesByOrderRef(string& szOrderRef, vector<shared_ptr<TradeT>>&vTradeT);
	//��ѯ�������н���
	bool GetTrades(vector<shared_ptr<TradeT>>& Trades);
	//���ݹ�Ʊ�����ѯ�ֲ�
	bool GetPositionBySecurity(string &szExchangeID, string &szStockCode, vector<shared_ptr<PositionT>>&vPositionT);
	//���ݹ�Ʊ����ͳֲַ����ѯ�ֲ�
	bool GetPositionBySecurity(string &szExchangeID, string &szStockCode, char cDirection, shared_ptr<PositionT>&pPosition);
	//��ѯ���й�Ʊ�ĳֲ�
	bool GetPositions(vector<shared_ptr<PositionT>> &Positions);
	//��ȡδ�ɽ�������
	int GetNotTradeNum(string& szOrderRef);
public:
	//���տ���:����ʽ�
	bool RiskBalance(const OrderT&OrderData);
	//���տ���:���ֲ�
	bool RiskPosition(const OrderT&OrderData);
public:
	//szClientOrderId�Ƿ����
	bool IsClientOrderIdExist(const string& szClientOrderId);
	//szOrderRef�Ƿ����
	bool IsOrderRefExist(const string& szOrderRef);

public:
	string m_szUserId;
	BalanceT m_Balance; // current balance
	//map<string, string> m_ReqIdOrderRefMap; // szReqId -> szOrderRef
	map<string, string> m_ClientOrderIdOrderRefMap; // szClientOrderId -> szOrderRef

	map<string, shared_ptr<OrderT>> m_OrdersMap; // szOrderRef -> Order
	map<string, shared_ptr<list<string>>> m_OrderTradeMap; // szOrderRef -> list<szTradeID>
	map<string, vector<string>> m_StockCodeTradeMap; //�洢��������  szExchangeID_szStockCode_cDirection->vector<szTradeID>  
	map<string, shared_ptr<TradeT>> m_TradesMap; // szTradeID -> Trade
	map<string, shared_ptr<PositionT>> m_PositionsMap; // szExchangeID_szStockCode_cDirection ->Position
	map<string, shared_ptr<OpenCountT>>  m_CommissionOpenCountMap;  //�洢���������Ѽ���Ĺ�Ʊ����������<szExchangeID_szStockCode_cDirection��OpenCountT>

	mutex m_BalanceMtx;  //��m_Balance
	mutex m_OrderMtx;  //��m_ReqIdOrderRefMap��m_OrdersMap
	mutex m_TradeMtx;//��m_OrderTradeMap��m_TradesMap,m_StockCodeTradeMap
	mutex m_PositionMtx;//��m_PositionsMap
	mutex m_CommissionOpenCountMtx;//��m_CommissionOpenCountMap
};


#endif // !_RUNTIME_MANAGER_H

#ifndef TRADE_DATA_H_
#define TRADE_DATA_H_
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include "sqlite3.h"
#include "struct.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class TradeData
{
public:
	TradeData();
	~TradeData();
public:
	static void Init();
	static void Close();
public:
	static void InsertBalance(BalanceT&BalanceData);
	static void InsertOrder(OrderT&OrderData);
	static void InsertTrade(TradeT&TradeData);
	static void InsertPosition(PositionT&PositionData);
	static void InsertInstrument(InstrumentT&InstrumentData);
	static void InsertInstrumentMarginRate(InstrumentMarginRate&InstrumentMarginRateData);
	static void InsertInstrumentCommissionRate(InstrumentCommissionRate&InstrumentCommissionRateData);
	static void InsertInstrumentOrderCommRate(InstrumentOrderCommRate&InstrumentOrderCommRateData);
	static void InsertCommissionOpenCount(OpenCountT&CommissionOpenCountData);

	static void DeleteInstrument();
	static void DeleteInstrumentMarginRate();
	static void DeleteInstrumentCommissionRate();
	static void DeleteInstrumentOrderCommRate();

	static void UpdateBalance(BalanceT&BalanceData);
	static void UpdateOrder(OrderT&OrderData);
	static void UpdateTrade(TradeT&TradeData);
	static void UpdatePosition(PositionT&PositionData);
	static void UpdateCommissionOpenCount(OpenCountT&CommissionOpenCountData);

	static void GetBalance(const string &szUserId, BalanceT&BalanceData);
	static void GetOrders(const int iStartDate, const int iEndDate, const string &szUserId, vector<shared_ptr<OrderT>>&OrderDatas);
	static void GetTrades(const int iStartDate, const int iEndDate, const string &szUserId, vector<shared_ptr<TradeT>>&TradeDatas);
	static void GetTrades(const int iDate, const string &szUserId, char cAction, vector<shared_ptr<TradeT>>&TradeDatas);
	static void GetPositions(const string &szUserId, const string& szExchangeId, const string& szStockCode, char cDirection, PositionT&PositionData);
	static void GetPositions(const string &szUserId, vector<shared_ptr<PositionT>>&PositionDatas);
	static void GetInstruments(vector<shared_ptr<InstrumentT>>&InstrumentDatas);
	static void GetInstruments(string szPartInstrumentId, vector<shared_ptr<InstrumentT>>&InstrumentDatas);
	static void GetInstruments(string szInstrumentId, InstrumentT&InstrumentData);
	static void GetInstrumentMarginRates(vector<shared_ptr<InstrumentMarginRate>>&InstrumentMarginRateDatas);
	static void GetInstrumentCommissionRates(vector<shared_ptr<InstrumentCommissionRate>>&InstrumentCommissionRateDatas);
	static void GetInstrumentCommissionRates(string szInstrumentId, InstrumentCommissionRate& InstrumentCommissionRateData);
	static void GetInstrumentOrderCommRates(vector<shared_ptr<InstrumentOrderCommRate>>&InstrumentOrderCommRateDatas);
	static void GetInstrumentOrderCommRates(string szInstrumentId, InstrumentOrderCommRate& InstrumentOrderCommRateData);
	static void GetCommissionOpenCount(const int iDate, const string &szUserId, vector<shared_ptr<OpenCountT>>&CommissionOpenCountDatas);
private:
	static sqlite3 *SqliteDb; // db handler
	static  map<string, shared_ptr<InstrumentT>> m_InstrumentMap; //市场+合约代码，合约信息

};




#endif

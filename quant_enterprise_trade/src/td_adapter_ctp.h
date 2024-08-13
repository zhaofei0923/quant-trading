#ifndef _TD_ADAPTER_CTP_H
#define _TD_ADAPTER_CTP_H
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include "const.h"
#include "struct.h"
#include "ThostFtdcTraderApi.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class TdAdapterCtp : public CThostFtdcTraderSpi
{
public:
	TdAdapterCtp();
	virtual ~TdAdapterCtp();

public:
	void Connect(CtpConfig &config);
	void LogOut();
	bool IsLogin() const;
public:
	//查询资金
	int ReqQueryBalance();
	//查询委托
	int ReqQueryOrder();
	//查询成交
	int ReqQueryTrade();
	//查询持仓
	int ReqQueryPosition();
	//查询合约
	int ReqQryInstrument();
	///请求查询合约保证金率
	int ReqQryInstrumentMarginRate(string szInstrumentId);
	///请求查询合约手续费率
	int ReqQryInstrumentCommissionRate(string szInstrumentId);
	///请求查询报单手续费
	int ReqQryInstrumentOrderCommRate(string szInstrumentId);
	//请求查询经纪公司交易参数
	int ReqQryBrokerTradingParams();
	//请求查询结算信息确认
	int ReqQrySettlementInfoConfirm();
	///请求查询投资者结算结果
	int ReqQrySettlementInfo();
	///投资者结算结果确认
	int ReqSettlementInfoConfirm();
	//请求查询行情
	int ReqQryDepthMarketData(string szInstrumentId = "", string szExchangeId = "");
	//委托
	TdResponse<string> ReqPlaceOrder(const string& szOrderRef, const string &szStockCode, const char &cAction, const double dPrice, const int iQuantity);
	//撤单
	TdResponse<string> ReqCancelOrder(int iFrontID, int iSessionID, const string& szOrderRef, const string& szInstrumentID);
private:
	virtual void OnFrontConnected();
	virtual void OnFrontDisConnected(int nReason);
	///客户端认证响应
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///用户口令更新请求响应
	virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//登录响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//退出响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//查询资金响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//查询委托响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//查询持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询合约保证金率响应
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询合约手续费率响应
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询报单手续费响应
	virtual void OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询经纪公司交易参数响应
	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询结算信息确认响应
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询投资者结算结果响应
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询行情响应
	virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
private:
	//委托回报
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	//成交回报
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///报单录入错误回报
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
	///报单操作错误回报
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

private:
	BalanceT ConvertBalance(const CThostFtdcTradingAccountField *pAccount);
	OrderT ConvertOrder(const CThostFtdcOrderField *pOrder);
	TradeT ConvertTrade(const CThostFtdcTradeField *pTrade);
	PositionT ConvertPosition(const CThostFtdcInvestorPositionField *pPos);
	int GetRequestId();
private:
	CThostFtdcTraderApi *m_pTraderApi;
	CtpConfig m_Config;
	int  m_iRequestId;
	bool m_IsLogin;

	mutex m_RequestIdMtx; //锁 m_iRequestId

};



#endif // !_TD_ADAPTER_CTP_H
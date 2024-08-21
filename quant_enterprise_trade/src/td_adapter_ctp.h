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
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//��ѯ�ʽ�
	int ReqQueryBalance();
	//��ѯί��
	int ReqQueryOrder();
	//��ѯ�ɽ�
	int ReqQueryTrade();
	//��ѯ�ֲ�
	int ReqQueryPosition();
	//��ѯ��Լ
	int ReqQryInstrument();
	///�����ѯ��Լ��֤����
	int ReqQryInstrumentMarginRate(string szInstrumentId);
	///�����ѯ��Լ��������
	int ReqQryInstrumentCommissionRate(string szInstrumentId);
	///�����ѯ����������
	int ReqQryInstrumentOrderCommRate(string szInstrumentId);
	//�����ѯ���͹�˾���ײ���
	int ReqQryBrokerTradingParams();
	//�����ѯ������Ϣȷ��
	int ReqQrySettlementInfoConfirm();
	///�����ѯͶ���߽�����
	int ReqQrySettlementInfo();
	///Ͷ���߽�����ȷ��
	int ReqSettlementInfoConfirm();
	//�����ѯ����
	int ReqQryDepthMarketData(string szInstrumentId = "", string szExchangeId = "");
	//ί��
	TdResponse<string> ReqPlaceOrder(const string& szOrderRef, const string &szStockCode, const char &cAction, const double dPrice, const int iQuantity);
	//����
	TdResponse<string> ReqCancelOrder(int iFrontID, int iSessionID, const string& szOrderRef, const string& szInstrumentID);
private:
	virtual void OnFrontConnected();
	virtual void OnFrontDisConnected(int nReason);
	///�ͻ�����֤��Ӧ
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�û��������������Ӧ
	virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//��¼��Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//�˳���Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//��ѯ�ʽ���Ӧ
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//��ѯί����Ӧ
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//��ѯ�ɽ���Ӧ
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//��ѯ�ֲ���Ӧ
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ��Լ��֤������Ӧ
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ��Լ����������Ӧ
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ������������Ӧ
	virtual void OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ���͹�˾���ײ�����Ӧ
	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ������Ϣȷ����Ӧ
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯͶ���߽�������Ӧ
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///Ͷ���߽�����ȷ����Ӧ
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ������Ӧ
	virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
private:
	//ί�лر�
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	//�ɽ��ر�
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	///����¼��������Ӧ
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///��������������Ӧ
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///����¼�����ر�
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
	///������������ر�
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

	mutex m_RequestIdMtx; //�� m_iRequestId

};



#endif // !_TD_ADAPTER_CTP_H
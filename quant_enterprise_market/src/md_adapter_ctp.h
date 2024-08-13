#ifndef _MD_ADAPTER_CTP_H
#define _MD_ADAPTER_CTP_H
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include "const.h"
#include "struct.h"
#include "message.h"
#include "ThostFtdcMdApi.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class MdAdapterCtp : public CThostFtdcMdSpi
{
public:
	MdAdapterCtp();
	virtual ~MdAdapterCtp();

public:
	void Connect(CtpConfig &ConfigData);
	void Logout();
	void ReqSubscribeMarketdata(const vector<string> &vInstrumentId);

private:
	int GetRequestId();
private:
	CtpConfig m_CtpConfig;
	CThostFtdcMdApi *m_pQuoteApi;
	int  m_iRequestId; // user custom req_id
	mutex m_RequestIdMtx;
	bool m_bIsLogin;

private:
	// CTP callbacks
	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

private:
	//IsValidTick
	bool IsValidTick(string sTickTime);

};


#endif
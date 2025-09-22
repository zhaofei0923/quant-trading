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
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//��¼
	void OnLogin(ServerMsg*pReqMsg);
	//�˳�
	void OnLogout(ServerMsg*pReqMsg);
	//��ѯ�ʽ�
	void OnQueryBalance(ServerMsg*pReqMsg);
	//��ѯί��
	void OnQueryOrder(ServerMsg*pReqMsg);
	//��ѯ�ɽ�
	void OnQueryTrade(ServerMsg*pReqMsg);
	//��ѯ�ֲ�
	void OnQueryPosition(ServerMsg*pReqMsg);
	//ί��
	void OnSendOrder(ServerMsg*pReqMsg);
	//����
	void OnCancelOrder(ServerMsg*pReqMsg);
	//����ͨѶ��Ϣ����
	void OnAddMsgQueue(ServerMsg*pReqMsg);
	//ɾ��ͨѶ��Ϣ����
	void OnDelMsgQueue(ServerMsg*pReqMsg);
	//���������˺��ʽ�
	void OnAddDelMoney(ServerMsg*pReqMsg);
public:
	int SendOrder(OrderT&OrderData);
	int CancelOldOrder(const string&szUserId, const string& szOrderRef);
	//�����Գɽ�
	int MngOneselfTrade(char cSellBuyFlag, int iAddDelFlag, string&szStockCode, unsigned long long iPrice, int iNum);
public:
	//��ѯ�ʽ���Ӧ
	void HandleQueryBalanceRsp(TdResponse<BalanceT> &TdResponse);
	//��ѯί����Ӧ
	void HandleQueryOrderRsp(TdResponse<OrderT> &TdResponse);
	//��ѯ�ɽ���Ӧ
	void HandleQueryTradeRsp(TdResponse<TradeT> &TdResponse);
	//��ѯ�ֲ���Ӧ
	void HandleQueryPositionRsp(TdResponse<PositionT> &TdResponse);
	//��ѯ��Լ��Ӧ
	void HandleQueryInstrumentRsp(TdResponse<InstrumentT> &TdResponse);
	//��ѯ��Լ��֤������Ӧ
	void HandleQueryInstrumentMarginRateRsp(TdResponse<InstrumentMarginRate> &TdResponse);
	//��ѯ��Լ����������Ӧ
	void HandleQueryInstrumentCommissionRateRsp(TdResponse<InstrumentCommissionRate> &TdResponse);
	///��ѯ������������Ӧ
	void HandleQueryInstrumentOrderCommRateRsp(TdResponse<InstrumentOrderCommRate> &TdResponse);
	//��ѯ������Ӧ
	void HandleQueryDepthMarketDataRsp(TdResponse<MarketData> &TdResponse);
	//ί�лر�
	void HandleOrderNotify(TdResponse<OrderT> &TdNotify);
	//�ɽ��ر�
	void HandleTradeNotify(TdResponse<TradeT> &TdNotify);
	//ί�д���֪ͨ
	void HandlePlaceOrderErrNotify(TdResponse<PlaceOrderErrNotifyT> &TdNotify);
	//��������֪ͨ
	void HandleCancelOrderErrNotify(TdResponse<CancelOrderErrNotifyT> &TdNotify);
public:
	//���
	int RiskRule(const OrderT&OrderData);
	//���ֲ�
	int RiskPosition(const OrderT&OrderData);
	//����ʽ�
	int RiskBalance(const OrderT&OrderData);
	//���Ψһ��
	int RiskIndex(const OrderT&OrderData);
	//����Գɽ�
	int RiskOneselfTrade(const OrderT&OrderData);
	//���ί������
	int RiskEntrustNum(const OrderT&OrderData);

public:
	string GetOrderRef(string&szUserId);
	ServerMsg*GetResMsg(ServerMsg*pReqMsg, int iErrorCode, int iBodyLen, void*pBody, bool bIsLast = true);
	ServerMsg*GetResMsg(int iRequestId, int iErrorCode, int iBodyLen, void*pBody, bool bIsLast = true);
	int AddReqBase(int iRequestId, ServerMsg*pReqMsg);
	int DelReqBase(int iRequestId);
	int GetReqBase(int iRequestId, ServerMsg&ReqData);
public:
	//��ȡ��Լ����
	int ReqCtpInstrumentData();

private:
	map<string, shared_ptr<RuntimeManagerCtp>> m_RuntimeManagerCtpMap; // 
	map<int, string > m_ErrMap; //�����룬������Ϣ
	map<string, User> m_UserMap; //UserId,User
	map<int, ServerMsg> m_RequestIdMap; //RequestId��ServerMsg

	MarketDataInfo m_MarketDataMap; //ExchangeID+InstrumentID,��������
	InstrumentExchange m_InstrumentExchangeMap; //InstrumentID,ExchangeID
	InstrumentInfo m_InstrumentMap; //ExchangeID+InstrumentID,��Լ
	InstrumentMarginRateInfo m_InstrumentMarginRateMap; //ExchangeID+InstrumentID,��Լ��֤����
	InstrumentCommissionRateInfo m_InstrumentCommissionRateMap; //ExchangeID+InstrumentID,��Լ��������
	InstrumentOrderCommRateInfo m_InstrumentOrderCommRateMap; //ExchangeID+InstrumentID,����������

	map<string, map<unsigned long long, unsigned long long>> m_SellMap; //<InstrumentID,<�۸�,��������>>   �洢δ�ɽ���ί�У������Գɽ����
	map<string, map<unsigned long long, unsigned long long>> m_BuyMap; //<InstrumentID,<�۸�,������� >>  �洢δ�ɽ���ί�У������Գɽ����
private:
	TdAdapterCtp *m_TdAdapterCtp;
	int m_iOrderRefNum;  //OrderRef�õ������
	mutex m_RequestIdMtx; //��m_RequestIdMap
	mutex m_SellBuyMtx;  //��m_SellMap,m_BuyMap
};

#endif 

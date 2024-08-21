#include <string.h>
#include "helper_time.h" 
#include "print.h" 
#include "runtime_manager_ctp.h"
#include "sim_log.h"
#include "trade_data.h"
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

RuntimeManagerCtp::RuntimeManagerCtp(string &szUserId) : m_szUserId(szUserId)
{
	LInfo("����RuntimeManager,  m_szUserId=[{0}]", m_szUserId);
}

RuntimeManagerCtp::~RuntimeManagerCtp()
{
}
//��ʼ��
void RuntimeManagerCtp::Init()
{
	m_OrdersMap.clear();
	m_TradesMap.clear();
	m_PositionsMap.clear();
	m_ClientOrderIdOrderRefMap.clear();
	m_OrderTradeMap.clear();
	m_StockCodeTradeMap.clear();

	//��ȡ��������
	int iNowDate = zutil::GetToday();



	//�����ݱ��������
	//�����ʽ�
	LDebug("�����ݱ�����ʽ�,m_szUserId=[{0}]", m_szUserId);
	TradeData::GetBalance(m_szUserId, m_Balance);
	//��ӡ�ʽ�
	PrintData::PrintBalance(m_Balance);


	//���ص���ί��
	LDebug("�����ݱ���ص���ί��,m_szUserId=[{0}]", m_szUserId);
	vector<shared_ptr<OrderT>> OrderDatas;
	TradeData::GetOrders(iNowDate, iNowDate, m_szUserId, OrderDatas);
	for (int i = 0; i < OrderDatas.size(); i++)
	{
		shared_ptr<OrderT> pOrder = OrderDatas[i];
		PrintData::PrintOrder(*pOrder);
		m_ClientOrderIdOrderRefMap[pOrder->szClientOrderId] = pOrder->szOrderRef;
		m_OrdersMap[pOrder->szOrderRef] = pOrder;
	}

	//���ص���ɽ�
	LDebug("�����ݱ���ص���ɽ�,m_szUserId=[{0}]", m_szUserId);
	vector<shared_ptr<TradeT>> TradeDatas;
	TradeData::GetTrades(iNowDate, iNowDate, m_szUserId, TradeDatas);
	for (int i = 0; i < TradeDatas.size(); i++)
	{
		shared_ptr<TradeT> pTrade = TradeDatas[i];
		PrintData::PrintTrade(*pTrade);
		m_TradesMap[pTrade->szTradeID] = pTrade;

		//һ��ί���ж���ɽ�
		shared_ptr<list<string>> pTradeList = nullptr;
		if (m_OrderTradeMap.count(pTrade->szOrderRef) > 0)
		{
			pTradeList = m_OrderTradeMap[pTrade->szOrderRef];
		}
		else
		{
			pTradeList = make_shared<list<string>>();
			m_OrderTradeMap[pTrade->szOrderRef] = pTradeList;
		}
		pTradeList->push_back(pTrade->szTradeID);

		//��¼TradeId��m_StockCodeTradeMap
		char cDirection = POSITION_DIRECTION_NET;
		if (ORDER_ACTION_BUY_OPEN == pTrade->cAction || ORDER_ACTION_SELL_CLOSE == pTrade->cAction)
		{
			cDirection = POSITION_DIRECTION_LONG;
			LDebug("��ͷ  ���򿪲֣���ƽ��");
		}
		else if (ORDER_ACTION_SELL_OPEN == pTrade->cAction || ORDER_ACTION_BUY_CLOSE == pTrade->cAction)
		{
			cDirection = POSITION_DIRECTION_SHORT;
			LDebug("��ͷ  �������֣���ƽ��");
		}
		string szSecurityDirectionKey = pTrade->szExchangeID + '_' + pTrade->szStockCode + '_' + cDirection;
		LDebug("�ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

		if (ORDER_ACTION_BUY_OPEN == pTrade->cAction || ORDER_ACTION_SELL_OPEN == pTrade->cAction)
		{
			LDebug("���ֵ�����,szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

			//���ֵ�����
			if (m_StockCodeTradeMap.count(szSecurityDirectionKey) > 0)
			{
				m_StockCodeTradeMap[szSecurityDirectionKey].push_back(pTrade->szTradeID);
			}
			else
			{
				vector<string> vTradeIds;
				m_StockCodeTradeMap[szSecurityDirectionKey] = vTradeIds;
				m_StockCodeTradeMap[szSecurityDirectionKey].push_back(pTrade->szTradeID);
			}

		}

	}


	//���ص��������ѿ�������
	LDebug("�����ݱ���ص��������ѿ�������,m_szUserId=[{0}]", m_szUserId);
	vector<shared_ptr<OpenCountT>> CommissionOpenCountDatas;
	TradeData::GetCommissionOpenCount(iNowDate, m_szUserId, CommissionOpenCountDatas);
	for (int i = 0; i < CommissionOpenCountDatas.size(); i++)
	{
		shared_ptr<OpenCountT> pOpenCountT = CommissionOpenCountDatas[i];
		string szSecurityDirectionKey = pOpenCountT->szExchangeID + '_' + pOpenCountT->szStockCode + '_' + pOpenCountT->cDirection;
		m_CommissionOpenCountMap[szSecurityDirectionKey] = pOpenCountT;
	}

}
//����ֲ�
void RuntimeManagerCtp::SettlePosition(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates)
{
	//ϵͳ������ʱ�������һ�������ս��н���

	//��ȡ�ϴν�������
	BalanceT BalanceData;
	TradeData::GetBalance(m_szUserId, BalanceData);
	if ("" == BalanceData.szUserId)
	{
		LError("���ݱ�td_balanceû�д���m_szUserId=[{0}]��¼", m_szUserId);
		return;
	}
	int iLastSettleDate = BalanceData.iSettleDate;
	LInfo("�ϴν�������iLastSettleDate=[{0}]", iLastSettleDate);

	//��ȡ�ϴν��������еĿ��ֳɽ�����
	vector<shared_ptr<TradeT>> LongTradeDatas; //��ͷ
	TradeData::GetTrades(iLastSettleDate, m_szUserId, ORDER_ACTION_BUY_OPEN, LongTradeDatas);
	vector<shared_ptr<TradeT>> ShortTradeDatas;//��ͷ
	TradeData::GetTrades(iLastSettleDate, m_szUserId, ORDER_ACTION_SELL_OPEN, ShortTradeDatas);

	//��ȡ�ֲ�����
	vector<shared_ptr<PositionT>> PositionDatas;
	TradeData::GetPositions(m_szUserId, PositionDatas);

	//�ж��Ƿ��Ѿ�����ֲ�ӯ�������ձ�֤��
	if (zutil::GetToday() == iLastSettleDate)
	{
		LInfo("m_szUserId=[{0}]�Ѿ�����ֲ�ӯ�������ձ�֤��,ֱ�Ӽ��سֲ�����", m_szUserId);

		//�Ѿ�����ֲ�ӯ�������ձ�֤��,ֱ�Ӽ��سֲ�����
		for (int i = 0; i < PositionDatas.size(); i++)
		{
			shared_ptr<PositionT> pPosition = PositionDatas[i];
			PrintData::PrintPosition(*pPosition);
			string szSecurityDirectionKey = pPosition->szExchangeID + '_' + pPosition->szStockCode + '_' + pPosition->cDirection;
			m_PositionsMap[szSecurityDirectionKey] = pPosition;
		}
	}
	else
	{
		LInfo("m_szUserId=[{0}]û�н���ֲ�ӯ�������ձ�֤�𣬿�ʼ��������iLastSettleDate=[{1}]����", m_szUserId, iLastSettleDate);

		/*
		����δƽ�ֵĿ����������ڵ���ֲ�����
		����δƽ�ֵĿ��ֽ����ڵ���ֲֿ��ֽ��
		*/
		map<string, double> LongOpenAmount;//��Լ,���   ��ͷ����δƽ�ֵĿ��ֽ����ܺ�,����ͷ����ֲֿ��ֽ��
		map<string, double> ShortOpenAmount;//��Լ,���  ��ͷ����δƽ�ֵĿ��ֽ����ܺ�,����ͷ����ֲֿ��ֽ��


		//�����ͷ����δƽ�ֵĿ��ֽ����ܺ�,����ͷ����ֲֿ��ֽ��
		LInfo("�����ͷ����δƽ�ֵĿ��ֽ����ܺ�,����ͷ����ֲֿ��ֽ��");
		for (int i = 0; i < LongTradeDatas.size(); i++)
		{
			string szKey = LongTradeDatas[i]->szExchangeID + string("_") + LongTradeDatas[i]->szStockCode;

			bool bFlag = true;
			if (0 == MarketDatas.count(szKey))
			{
				LError("MarketDatas:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == MarginRates.count(szKey))
			{
				LError("MarginRates:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == Instruments.count(szKey))
			{
				LError("Instruments:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}

			if (bFlag)
			{
				//��Լ����
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				//����δƽ�ֵĿ�������iCount=�ɽ�����-����ƽ������
				int iCount = LongTradeDatas[i]->iTradeNum - LongTradeDatas[i]->iCloseNum;

				//����˺�Լ�ĵ���δƽ�ֵĿ��ֽ����ܺ�
				double dAmount = iCount*LongTradeDatas[i]->dPrice*iVolumeMultiple;
				if (LongOpenAmount.count(szKey) > 0)
				{
					LongOpenAmount[szKey] = LongOpenAmount[szKey] + dAmount;
				}
				else
				{
					LongOpenAmount[szKey] = dAmount;
				}

				LDebug("szKey=[{0}],iVolumeMultiple=[{1}],iTradeNum=[{2}],iCloseNum=[{3}],", szKey, iVolumeMultiple, LongTradeDatas[i]->iTradeNum, LongTradeDatas[i]->iCloseNum);
				LDebug("szKey=[{0}],iCount=[{1}],dAmount=[{2}],dTotalAmount=[{3}],", szKey, iCount, dAmount, LongOpenAmount[szKey]);

			}
		}

		//�����ͷ����δƽ�ֵĿ��ֽ����ܺ�,����ͷ����ֲֿ��ֽ��
		LInfo("�����ͷ����δƽ�ֵĿ��ֽ����ܺ�,����ͷ����ֲֿ��ֽ��");
		for (int i = 0; i < ShortTradeDatas.size(); i++)
		{
			string szKey = ShortTradeDatas[i]->szExchangeID + string("_") + ShortTradeDatas[i]->szStockCode;

			bool bFlag = true;
			if (0 == MarketDatas.count(szKey))
			{
				LError("MarketDatas:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == MarginRates.count(szKey))
			{
				LError("MarginRates:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == Instruments.count(szKey))
			{
				LError("Instruments:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}

			if (bFlag)
			{
				//��Լ����
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				//����δƽ�ֵĿ�������iCount=�ɽ�����-����ƽ������
				int iCount = ShortTradeDatas[i]->iTradeNum - ShortTradeDatas[i]->iCloseNum;

				//����˺�Լ�ĵ���δƽ�ֵĿ��ֽ����ܺ�
				double dAmount = iCount*ShortTradeDatas[i]->dPrice*iVolumeMultiple;
				if (ShortOpenAmount.count(szKey) > 0)
				{
					ShortOpenAmount[szKey] = ShortOpenAmount[szKey] + dAmount;
				}
				else
				{
					ShortOpenAmount[szKey] = dAmount;
				}

				LDebug("szKey=[{0}],iVolumeMultiple=[{1}],iTradeNum=[{2}],iCloseNum=[{3}],", szKey, iVolumeMultiple, ShortTradeDatas[i]->iTradeNum, ShortTradeDatas[i]->iCloseNum);
				LDebug("szKey=[{0}],iCount=[{1}],dAmount=[{2}],dTotalAmount=[{3}],", szKey, iCount, dAmount, ShortOpenAmount[szKey]);

			}
		}


		/*
		����ӯ��������
		������ԼΪ����
		����ӯ����ƽ��ӯ�����ֲ�ӯ��
		ƽ��ӯ����ƽ��ʷ��ӯ����ƽ���ղ�ӯ��
		ƽ��ʷ��ӯ����(����ƽ�ּۣ���һ�����ս����)������ƽ��������Լ������(��һ�����ս���ۣ�����ƽ�ּ�)������ƽ��������Լ����
		ƽ���ղ�ӯ����(��������ƽ�ּۣ��������뿪�ּ�)������ƽ��������Լ������(�����������ּۣ���������ƽ�ּ�)������ƽ��������Լ����
		�ֲ�ӯ������ʷ�ֲ�ӯ�������տ��ֲֳ�ӯ��
		��ʷ�ֲ�ӯ����(��һ�ս���ۣ����ս����)��������ʷ�ֲ�������Լ������(���ս���ۣ���һ�ս����)��������ʷ�ֲ�������Լ����
		���տ��ֲֳ�ӯ����(�������ּۣ����ս����)����������������Լ������(���ս���ۣ����뿪�ּ�)�����뿪��������Լ����

		*/

		//����ֲ�ӯ��
		LInfo("����ֲ�ӯ��");

		//��ʷ�ֲ�ӯ��
		double dHisPositionProfit = 0;
		//���տ��ֲֳ�ӯ����
		//�����Ƕ���һ�ս��н��㣬���Խ�������iLastSettleDate���н��㣬��������ĵ�����ָ��һ�գ���iLastSettleDate
		double dTodayPositionProfit = 0;
		for (int i = 0; i < PositionDatas.size(); i++)
		{
			int  iYdPosition = PositionDatas[i]->iYdPosition;
			int  iTodayPosition = PositionDatas[i]->iTodayPosition;

			string szStockCode = PositionDatas[i]->szStockCode;
			string szExchangeId = PositionDatas[i]->szExchangeID;
			string szKey = szExchangeId + string("_") + szStockCode;

			LDebug("szKey=[{0}],iYdPosition=[{1}],iTodayPosition=[{2}]", szKey, iYdPosition, iTodayPosition);

			bool bFlag = true;
			if (0 == MarketDatas.count(szKey))
			{
				LError("MarketDatas:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == MarginRates.count(szKey))
			{
				LError("MarginRates:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == Instruments.count(szKey))
			{
				LError("Instruments:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}

			if (bFlag)
			{
				//���ս���ۣ���iLastSettleDate�����
				double dPreSettlementPrice = MarketDatas[szKey].iPreSettlePrice / PRICE_MULTIPLE;
				//���ս���۵���һ�����ս����,��iLastSettleDate����һ�յĽ����
				double dPrePreSettlementPrice = PositionDatas[i]->dSettlementPrice;
				//��Լ����
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				LDebug("dPrePreSettlementPrice=[{0}],dPreSettlementPrice=[{1}],iVolumeMultiple=[{2}]", dPrePreSettlementPrice, dPreSettlementPrice, iVolumeMultiple);

				if (POSITION_DIRECTION_LONG == PositionDatas[i]->cDirection)//��ͷ  ���򿪲֣���ƽ��
				{
					//������ʷ�ֲ�ӯ��
					//��ͷ��ʷ�ֲ�ӯ��:(���ս���ۣ���һ�ս����)��������ʷ�ֲ�������Լ����
					double dHisValue = (dPreSettlementPrice - dPrePreSettlementPrice)*iYdPosition*iVolumeMultiple;
					dHisPositionProfit = dHisPositionProfit + dHisValue;
					LDebug("dHisValue=[{0}],dHisPositionProfit=[{1}]", dHisValue, dHisPositionProfit);

					//���㵱�տ��ֲֳ�ӯ��  
					//��ͷ���տ��ֲֳ�ӯ��:(���ս���ۣ����뿪�ּ�)�����뿪��������Լ����
					double dTodayValue = iTodayPosition*dPreSettlementPrice*iVolumeMultiple - LongOpenAmount[szKey];
					dTodayPositionProfit = dTodayPositionProfit + dTodayValue;
					LDebug("LongOpenAmount[szKey]=[{0}],dTodayValue=[{1}],dTodayPositionProfit=[{2}]", LongOpenAmount[szKey], dTodayValue, dTodayPositionProfit);


				}
				else if (POSITION_DIRECTION_SHORT == PositionDatas[i]->cDirection)//��ͷ  �������֣���ƽ��
				{
					//������ʷ�ֲ�ӯ��
					//��ͷ��ʷ�ֲ�ӯ��:(��һ�ս���ۣ����ս����)��������ʷ�ֲ�������Լ����
					double dHisValue = (dPrePreSettlementPrice - dPreSettlementPrice)*iYdPosition*iVolumeMultiple;
					dHisPositionProfit = dHisPositionProfit + dHisValue;
					LDebug("dHisValue=[{0}],dHisPositionProfit=[{1}]", dHisValue, dHisPositionProfit);

					//���㵱�տ��ֲֳ�ӯ��
					//��ͷ���տ��ֲֳ�ӯ��:(�������ּۣ����ս����)����������������Լ����
					double dTodayValue = ShortOpenAmount[szKey] - iTodayPosition*dPreSettlementPrice*iVolumeMultiple;
					dTodayPositionProfit = dTodayPositionProfit + dTodayValue;
					LDebug("ShortOpenAmount[szKey]=[{0}],dTodayValue=[{1}],dTodayPositionProfit=[{2}]", ShortOpenAmount[szKey], dTodayValue, dTodayPositionProfit);

				}


			}
		}

		//����ӯ����Balance
		m_Balance.dAddMoney = m_Balance.dAddMoney + dHisPositionProfit + dTodayPositionProfit;
		LDebug("dHisPositionProfit=[{0}],dTodayPositionProfit=[{1}],dTotal=[{2}]", dHisPositionProfit, dTodayPositionProfit, dHisPositionProfit + dTodayPositionProfit);


		//����ֱֲ�֤��
		//�������ս���ۣ���iLastSettleDate����ۼ���ռ�õı�֤��
		LInfo("����ֱֲ�֤��");
		for (int i = 0; i < PositionDatas.size(); i++)
		{
			shared_ptr<PositionT> pPosition = PositionDatas[i];
			double	dUseMargin = pPosition->dUseMargin; //ռ�õı�֤��
			//�ֲܳ�����=�������+�������
			int  iPosition = pPosition->iYdPosition + pPosition->iTodayPosition;

			string szStockCode = pPosition->szStockCode;
			string szExchangeId = pPosition->szExchangeID;
			string szKey = szExchangeId + string("_") + szStockCode;

			LDebug("szKey=[{0}],iPosition=[{1}]", szKey, iPosition);

			bool bFlag = true;
			if (0 == MarketDatas.count(szKey))
			{
				LError("MarketDatas:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == MarginRates.count(szKey))
			{
				LError("MarginRates:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}
			if (0 == Instruments.count(szKey))
			{
				LError("Instruments:szKey=[{0}]  is not exist", szKey);
				bFlag = false;
			}

			if (bFlag)
			{
				//�ȴ�Balance�м�ȥռ�õı�֤��
				m_Balance.dUseMargin = m_Balance.dUseMargin - dUseMargin;

				//��ȡ��֤�����
				double dMarginRatioByVolume = 0;
				double dMarginRatioByMoney = 0;
				if (POSITION_DIRECTION_LONG == pPosition->cDirection)
				{
					dMarginRatioByVolume = MarginRates[szKey]->dLongMarginRatioByVolume;
					dMarginRatioByMoney = MarginRates[szKey]->dLongMarginRatioByMoney;
				}
				else if (POSITION_DIRECTION_SHORT == pPosition->cDirection)
				{
					dMarginRatioByVolume = MarginRates[szKey]->dShortMarginRatioByVolume;
					dMarginRatioByMoney = MarginRates[szKey]->dShortMarginRatioByMoney;
				}
				LDebug("dMarginRatioByVolume=[{0}],dMarginRatioByMoney=[{1}]", dMarginRatioByVolume, dMarginRatioByMoney);

				//��Լ����
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				//���ս����,��iLastSettleDate�����
				double dPreSettlementPrice = MarketDatas[szKey].iPreSettlePrice / PRICE_MULTIPLE;
				LDebug("iVolumeMultiple=[{0}],dPreSettlementPrice=[{1}]", iVolumeMultiple, dPreSettlementPrice);

				//�������ս���۸����ı�֤��
				double	dNewUseMargin = iPosition*dMarginRatioByVolume + dMarginRatioByMoney*dPreSettlementPrice*iPosition*iVolumeMultiple;
				pPosition->dUseMargin = dNewUseMargin;
				LDebug("OldUseMargin=[{0}],NewUseMargin=[{1}]", dUseMargin, dNewUseMargin);

				//��ԭ����۸��µ��ϴν����
				pPosition->dPreSettlementPrice = pPosition->dSettlementPrice;
				//�����ս����,��iLastSettleDate����۸��µ����ν����
				pPosition->dSettlementPrice = dPreSettlementPrice;
				LDebug("dPreSettlementPrice=[{0}],dSettlementPrice=[{1}]", pPosition->dPreSettlementPrice, pPosition->dSettlementPrice);

				//�����µı�֤��Balance
				m_Balance.dUseMargin = m_Balance.dUseMargin + dNewUseMargin;
			}

			//����ֱֲ仯
			//���ճֲ�����=���ճֲ�����+���ճֲ�����
			pPosition->iYdPosition = pPosition->iYdPosition + pPosition->iTodayPosition;
			//���ճֲ�������Ϊ0	
			pPosition->iTodayPosition = 0;
			pPosition->iPosition = pPosition->iYdPosition; ///�ֲܳ�	

			string szSecurityDirectionKey = pPosition->szExchangeID + '_' + pPosition->szStockCode + '_' + pPosition->cDirection;
			m_PositionsMap[szSecurityDirectionKey] = pPosition;

			//�������ݿ�
			TradeData::UpdatePosition(*pPosition);
		}
	}


	//����m_Balance
	//��ǰ��֤���ܶ�=ռ�õı�֤��+����ı�֤��
	m_Balance.dCurrMargin = m_Balance.dUseMargin + m_Balance.dFrozenMargin;
	//ʹ�õ����ʽ�=ռ�õı�֤��+����ı�֤��+�����������+������
	double dTotalUseCash = m_Balance.dUseMargin + m_Balance.dFrozenMargin + m_Balance.dFrozenCommission + m_Balance.dCommission;
	//���ʽ�=�ڳ����ʽ�+�������ӵ��ʽ�
	double dTotalCash = m_Balance.dStartMoney + m_Balance.dAddMoney;
	m_Balance.dAvailable = dTotalCash - dTotalUseCash; ///�����ʽ�
	m_Balance.iSettleDate = zutil::GetToday();
	m_Balance.iModifyDate = zutil::GetToday();
	m_Balance.iModifyTime = zutil::GetNowSedondTime();

	//�������ݿ�
	TradeData::UpdateBalance(m_Balance);

}

//����order������Ϣ
void RuntimeManagerCtp::UpdateOrderRate(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates, InstrumentCommissionRateInfo&CommissionRates, InstrumentOrderCommRateInfo&OrderCommRates)
{
	for (auto&OrderPair : m_OrdersMap)
	{
		shared_ptr<OrderT> pOrder = OrderPair.second;
		string szKey = pOrder->szExchangeID + string("_") + pOrder->szStockCode;

		if (0 == Instruments.count(szKey))
		{
			LError("Instruments:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);
			continue;
		}
		if (0 == MarketDatas.count(szKey))
		{
			LError("MarketDatas:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);
			continue;
		}
		if (0 == MarginRates.count(szKey))
		{
			LInfo("MarginRates:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);
			MarginRates[szKey] = make_shared<InstrumentMarginRate>();
		}
		if (0 == CommissionRates.count(szKey))
		{
			LInfo("CommissionRates:ExchangeId and StockCode  is  not exist, szKey=[{0}]", szKey);
			CommissionRates[szKey] = make_shared<InstrumentCommissionRate>();
		}

		pOrder->iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
		pOrder->dPreSettlementPrice = MarketDatas[szKey].iPreSettlePrice / PRICE_MULTIPLE;


		if (0 == OrderCommRates.count(szKey))
		{
			//�еĲ���ȡ����������
			pOrder->dOrderCommByVolume = 0;
			pOrder->dOrderActionCommByVolume = 0;
		}
		else
		{
			pOrder->dOrderCommByVolume = OrderCommRates[szKey]->dOrderCommByVolume;
			pOrder->dOrderActionCommByVolume = OrderCommRates[szKey]->dOrderActionCommByVolume;
		}

		if (ORDER_ACTION_BUY_OPEN == pOrder->cAction)
		{
			//��ͷ
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dLongMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dLongMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dOpenRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dOpenRatioByVolume;
		}
		else if (ORDER_ACTION_SELL_CLOSE == pOrder->cAction)
		{
			//��ͷ
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dShortMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dShortMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dCloseRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dCloseRatioByVolume;
			pOrder->dCloseTodayRatioByMoney = CommissionRates[szKey]->dCloseTodayRatioByMoney;
			pOrder->dCloseTodayRatioByVolume = CommissionRates[szKey]->dCloseTodayRatioByVolume;

		}
		else if (ORDER_ACTION_SELL_OPEN == pOrder->cAction)
		{
			//��ͷ
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dShortMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dShortMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dOpenRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dOpenRatioByVolume;

		}
		else if (ORDER_ACTION_BUY_CLOSE == pOrder->cAction)
		{
			//��ͷ
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dLongMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dLongMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dCloseRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dCloseRatioByVolume;
			pOrder->dCloseTodayRatioByMoney = CommissionRates[szKey]->dCloseTodayRatioByMoney;
			pOrder->dCloseTodayRatioByVolume = CommissionRates[szKey]->dCloseTodayRatioByVolume;

		}

		LDebug("---------------����order������Ϣ--------------------");

		//��ӡί��
		PrintData::PrintOrder(*pOrder);
	}

}
//�����ʽ�
void RuntimeManagerCtp::UpdateBalance(BalanceT &BalanceData)
{
	lock_guard<mutex> lk(m_BalanceMtx);

	LDebug("RuntimeManagerCtp  UpdateBalance,  m_szUserId=[{0}]", m_szUserId);

	//��ӡ�ʽ�
	PrintData::PrintBalance(BalanceData);

	LDebug("---------before update------------ m_szUserId=[{0}]", m_szUserId);
	PrintData::PrintBalance(m_Balance);

	m_Balance.dUseMargin = m_Balance.dUseMargin + BalanceData.dUseMargin;///ռ�õı�֤��	
	m_Balance.dFrozenMargin = m_Balance.dFrozenMargin + BalanceData.dFrozenMargin; ///����ı�֤��	
	m_Balance.dFrozenCash = m_Balance.dFrozenCash + BalanceData.dFrozenCash; ///������ʽ�
	m_Balance.dFrozenCommission = m_Balance.dFrozenCommission + BalanceData.dFrozenCommission; ///�����������	
	//��ǰ��֤���ܶ�=ռ�õı�֤��+����ı�֤��
	m_Balance.dCurrMargin = m_Balance.dUseMargin + m_Balance.dFrozenMargin; ///��ǰ��֤���ܶ�	
	m_Balance.dCommission = m_Balance.dCommission + BalanceData.dCommission; ///������	
	m_Balance.dAddMoney = m_Balance.dAddMoney + BalanceData.dAddMoney;//�������ӵ��ʽ����ƽ��ӯ��

	//ʹ�õ����ʽ�=ռ�õı�֤��+����ı�֤��+�����������+������
	double dTotalUseCash = m_Balance.dUseMargin + m_Balance.dFrozenMargin + m_Balance.dFrozenCommission + m_Balance.dCommission;
	//���ʽ�=�ڳ����ʽ�+�������ӵ��ʽ�
	double dTotalCash = m_Balance.dStartMoney + m_Balance.dAddMoney;
	m_Balance.dAvailable = dTotalCash - dTotalUseCash; ///�����ʽ�

	m_Balance.iModifyDate = zutil::GetToday(); //�޸�����
	m_Balance.iModifyTime = zutil::GetNowSedondTime(); //�޸�ʱ��


	LDebug("---------after update------------ m_szUserId=[{0}]", m_szUserId);
	PrintData::PrintBalance(m_Balance);

	BalanceT OldBalanceData;
	TradeData::GetBalance(m_szUserId, OldBalanceData);
	if ("" == OldBalanceData.szUserId)
	{
		LError("���ݱ�td_balanceû�д���m_szUserId=[{0}]��¼", m_szUserId);
	}
	else
	{
		TradeData::UpdateBalance(m_Balance);
	}
}
//����ί��
void RuntimeManagerCtp::UpdateOrder(OrderT &OrderData)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  UpdateOrder,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, OrderData.szOrderRef);

	//��ӡί��
	PrintData::PrintOrder(OrderData);

	if (ORDER_STATUS_UNKNOWN == OrderData.cAction)
	{
		LError("OrderData.cAction is ORDER_STATUS_UNKNOWN");
		return;
	}

	if (OrderData.szClientOrderId != ""&&OrderData.szOrderRef != "")
	{
		if (0 == m_ClientOrderIdOrderRefMap.count(OrderData.szClientOrderId))
		{
			m_ClientOrderIdOrderRefMap[OrderData.szClientOrderId] = OrderData.szOrderRef;
			LDebug("m_ClientOrderIdOrderRefMap������Ӧ��ϵ,szClientOrderId=[{0}],szOrderRef=[{1}]", OrderData.szClientOrderId, OrderData.szOrderRef);
		}
	}

	if (m_OrdersMap.find(OrderData.szOrderRef) == m_OrdersMap.end())
	{
		m_OrdersMap[OrderData.szOrderRef] = make_shared<OrderT>(OrderData);
		LDebug("m_OrdersMap������Ӧ��ϵ,szOrderRef=[{0}]", OrderData.szOrderRef);

		//�������ݿ�
		TradeData::InsertOrder(OrderData);


	}
	else
	{
		shared_ptr<OrderT> pOrder = m_OrdersMap[OrderData.szOrderRef];

		char cLastStatus = pOrder->cStatus;
		char cNowStatus = OrderData.cStatus;

		//�ַ������֣�ʹ���ַ��Ƚ�
		if (cNowStatus >= cLastStatus)
		{
			if (cNowStatus == ORDER_STATUS_PART_TRADE || cNowStatus == ORDER_STATUS_ALL_TRADE)
			{
				//�ɽ�״̬�ͳɽ������ɳɽ��ر�����
				LDebug("do not  update m_OrdersMap,szOrderRef=[{0}]  cLastStatus= {1} cNowStatus={2}", OrderData.szOrderRef, cLastStatus, cNowStatus);
				return;
			}
			else
			{
				LDebug("update m_OrdersMap,szOrderRef=[{0}]  cLastStatus= {1} cNowStatus={2}", OrderData.szOrderRef, cLastStatus, cNowStatus);

			}


			pOrder->cStatus = cNowStatus;
			pOrder->iModifyDate = OrderData.iModifyDate;
			pOrder->iModifyTime = OrderData.iModifyTime;

			if (0 == pOrder->szJysInsertDate.length() && OrderData.szJysInsertDate.length() != 0)
			{
				pOrder->szJysInsertDate = OrderData.szJysInsertDate;
				LDebug("update m_OrdersMap,szJysInsertDate=[{0}] ", pOrder->szJysInsertDate);

			}

			if (0 == pOrder->szJysInsertTime.length() && OrderData.szJysInsertTime.length() != 0)
			{
				pOrder->szJysInsertTime = OrderData.szJysInsertTime;
				LDebug("update m_OrdersMap,szJysInsertTime=[{0}] ", pOrder->szJysInsertTime);

			}

			if (0 == pOrder->szExchangeID.length() && OrderData.szExchangeID.length() != 0)
			{
				pOrder->szExchangeID = OrderData.szExchangeID;
				LDebug("update m_OrdersMap,szExchangeID=[{0}] ", pOrder->szExchangeID);

			}

			if (0 == pOrder->szOrderSysID.length() && OrderData.szOrderSysID.length() != 0)
			{
				pOrder->szOrderSysID = OrderData.szOrderSysID;
				LDebug("update m_OrdersMap,szOrderSysID=[{0}] ", pOrder->szOrderSysID);
			}

			if (0 == pOrder->iFrontID && 0 == pOrder->iSessionID)
			{
				pOrder->iFrontID = OrderData.iFrontID;
				pOrder->iSessionID = OrderData.iSessionID;
				LDebug("update m_OrdersMap,iFrontID=[{0}]  iSessionID= {1} ", pOrder->iFrontID, pOrder->iSessionID);

			}

			//������ͬ״̬�����³ֲ�
			if (cNowStatus > cLastStatus)
			{
				UpdatePositionByOrder(*pOrder);
			}


		}

		//�������ݿ�
		TradeData::UpdateOrder(*pOrder);


		LDebug("---------after update----------------------");
		//��ӡί��
		PrintData::PrintOrder(*pOrder);

	}

}
//���½���
void RuntimeManagerCtp::UpdateTrade(TradeT &TradeData)
{
	lock_guard<mutex> lk(m_TradeMtx);

	LDebug("RuntimeManagerCtp  UpdateTrade,  m_szUserId=[{0}]", m_szUserId);

	//��ӡ����
	PrintData::PrintTrade(TradeData);

	shared_ptr<TradeT> pTrade = make_shared<TradeT>(TradeData);
	m_TradesMap[TradeData.szTradeID] = pTrade;

	//һ��ί���ж���ɽ�
	shared_ptr<list<string>> pTradeList = nullptr;
	if (m_OrderTradeMap.count(TradeData.szOrderRef) > 0)
	{
		pTradeList = m_OrderTradeMap[TradeData.szOrderRef];
	}
	else
	{
		pTradeList = make_shared<list<string>>();
		m_OrderTradeMap[TradeData.szOrderRef] = pTradeList;
	}
	pTradeList->push_back(TradeData.szTradeID);


	//��¼TradeId��m_StockCodeTradeMap
	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("�ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		//�洢��������
		if (m_StockCodeTradeMap.count(szSecurityDirectionKey) > 0)
		{
			m_StockCodeTradeMap[szSecurityDirectionKey].push_back(TradeData.szTradeID);
		}
		else
		{
			vector<string> vTradeIds;
			m_StockCodeTradeMap[szSecurityDirectionKey] = vTradeIds;
			m_StockCodeTradeMap[szSecurityDirectionKey].push_back(TradeData.szTradeID);
		}

	}


	UpdateCloseProfit(TradeData);
	UpdatePositionByTrade(TradeData);
	UpdateOrder(TradeData);
	UpdateCommissionOpenCount(TradeData);

	TradeData::InsertTrade(TradeData);

}
//����ί��
void RuntimeManagerCtp::UpdateOrder(TradeT &TradeData)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("UpdateOrder  TradeData.szOrderRef=[{0}]", TradeData.szOrderRef);

	if (0 == m_OrdersMap[TradeData.szOrderRef])
	{
		LError("old Order is not exist  szOrderRef=[{0}]", TradeData.szOrderRef);
		return;
	}

	//ί�лر�״̬ORDER_STATUS_SUBMIT,ORDER_STATUS_INSERT_REFUSE,ORDER_STATUS_CANCEL���Ա�֤�������������ֲ�
	//�ɽ��ر��еĳɽ��������������ӣ����Ա�֤�������������ֲ�
	//���ݳɽ��ر�����ί�еĳɽ�����
	shared_ptr<OrderT> pOrder = m_OrdersMap[TradeData.szOrderRef];
	pOrder->iTradeNum = pOrder->iTradeNum + TradeData.iTradeNum;
	if (pOrder->iTradeNum == pOrder->iEntrustNum)
	{
		//ȫ���ɽ�
		pOrder->cStatus = ORDER_STATUS_ALL_TRADE;
	}
	else
	{
		//���ֳɽ�
		pOrder->cStatus = ORDER_STATUS_PART_TRADE;
	}

	TradeData::UpdateOrder(*pOrder);
	UpdatePositionByOrder(*pOrder);

}
//����ƽ��ӯ��
void RuntimeManagerCtp::UpdateCloseProfit(TradeT &TradeData)
{
	LDebug("UpdateCloseProfit  TradeData.szOrderRef=[{0}]", TradeData.szOrderRef);

	//ֻ��ƽ�ֲż���ӯ��
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		return;
	}

	double  dPreSettlementPrice = 0; //������
	int iVolumeMultiple = 0;//��Լ��������
	//��ȡorder��Ϣ
	{
		lock_guard<mutex> lk(m_OrderMtx);
		if (0 == m_OrdersMap[TradeData.szOrderRef])
		{
			LError("old Order is not exist  szOrderRef=[{0}]", TradeData.szOrderRef);
			return;
		}
		shared_ptr<OrderT> pOrder = m_OrdersMap[TradeData.szOrderRef];
		dPreSettlementPrice = pOrder->dPreSettlementPrice;
		iVolumeMultiple = pOrder->iVolumeMultiple;
	}


	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("�ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);


	int iCloseYdNum = 0; //ƽ�������
	int iCloseTodayNum = 0;//ƽ�������

	//���ݳֲ����ݽ��м���
	{
		lock_guard<mutex> lk(m_PositionMtx);

		//���ݲ�ѯ��ԭ�ֲ��������м���
		if (0 == m_PositionsMap.count(szSecurityDirectionKey))
		{
			LError("�ֲ�����Ϊ0,szOrderRef=[{0}]  ", TradeData.szOrderRef);
			return;
		}
		else
		{
			shared_ptr<PositionT> pPosition = m_PositionsMap[szSecurityDirectionKey];
			//iValue=�������-�ɽ�����
			int iValue = pPosition->iYdPosition - TradeData.iTradeNum;
			if (iValue >= 0)  //ƽ���
			{
				iCloseTodayNum = 0;
				iCloseYdNum = TradeData.iTradeNum;

			}
			else  //��ƽ���,��ƽ���
			{
				iCloseTodayNum = 0 - iValue;
				iCloseYdNum = pPosition->iYdPosition;

			}

			LDebug("iCloseYdNum=[{0}],iCloseTodayNum=[{1}]", iCloseYdNum, iCloseTodayNum);
		}

	}


	/*
	����ӯ��������
	������ԼΪ����
	����ӯ����ƽ��ӯ�����ֲ�ӯ��
	ƽ��ӯ����ƽ��ʷ��ӯ����ƽ���ղ�ӯ��
	ƽ��ʷ��ӯ����(����ƽ�ּۣ���һ�����ս����)������ƽ��������Լ������(��һ�����ս���ۣ�����ƽ�ּ�)������ƽ��������Լ����
	ƽ���ղ�ӯ����(��������ƽ�ּۣ��������뿪�ּ�)������ƽ��������Լ������(�����������ּۣ���������ƽ�ּ�)������ƽ��������Լ����
	�ֲ�ӯ������ʷ�ֲ�ӯ�������տ��ֲֳ�ӯ��
	��ʷ�ֲ�ӯ����(��һ�ս���ۣ����ս����)��������ʷ�ֲ�������Լ������(���ս���ۣ���һ�ս����)��������ʷ�ֲ�������Լ����
	���տ��ֲֳ�ӯ����(�������ּۣ����ս����)����������������Լ������(���ս���ۣ����뿪�ּ�)�����뿪��������Լ����

	*/

	/*
	ƽ���ղ�ӯ���ļ����ѵ�
	������
	�򿪲֣�
	�ֱ���100Ԫ����1�ֺ�ԼA����110Ԫ����1�ֺ�ԼA����120Ԫ����1�ֺ�ԼA
	��ƽ�֣�
	��105Ԫ����2�ֺ�ԼA
	���⣺ƽ��ʱ��ƽ���ֵ���2�֣�

	���������
	�����ȿ��֣���ƽ�ֵĹ��򡣼�ƽ����ƽ100Ԫ�����1�ֺ�110Ԫ�����1�֡�
	ƽ��ӯ��Ϊƽ���ܽ���ȥ�����ܽ���: ƽ��ӯ��=105*2-(100*1+110*1)
	*/

	double dCloseHisProfit = 0; //ƽ��ʷ��ӯ��
	double dCloseTodayProfit = 0; //ƽ���ղ�ӯ��
	double dCloseTodayAmount = iCloseTodayNum*TradeData.dPrice*iVolumeMultiple; //ƽ���ղ��ܽ��

	double dOpenTodayAmount = 0; //�����ܽ��
	//��ȡszSecurityDirectionKey������TradeId
	vector<string>& vTradeIds = m_StockCodeTradeMap[szSecurityDirectionKey];

	//ѭ����������TradeId,���㿪���ܽ��
	for (int i = 0; i < vTradeIds.size(); i++)
	{
		if (iCloseTodayNum <= 0)
		{
			//ƽ������
			break;
		}

		//����TradeId���ҳɽ���ϢTradeT
		string szTradeId = vTradeIds[i];
		shared_ptr<TradeT> pTrade = m_TradesMap[szTradeId];

		//�˱ʳɽ��Ŀ�ƽ������=�ɽ�����-����ƽ������
		int iAvailable = pTrade->iTradeNum - pTrade->iCloseNum;

		//�˱ʳɽ��Ŀ�ƽ����������0������ƽ��
		if (iAvailable > 0)
		{
			//iValue=ƽ�������-�˱ʳɽ��Ŀ�ƽ������
			int iValue = iCloseTodayNum - iAvailable;
			if (iValue <= 0) //��iCloseTodayNumȫ��ƽ��
			{
				//���㿪���ܽ��
				dOpenTodayAmount = dOpenTodayAmount + iCloseTodayNum*pTrade->dPrice*iVolumeMultiple;
				LDebug("iCloseTodayNum=[{0}],dPrice=[{1}],iVolumeMultiple=[{2}],dOpenTodayAmount=[{3}]", iCloseTodayNum, pTrade->dPrice, iVolumeMultiple, dOpenTodayAmount);

				//���´˳ɽ��ĵ���ƽ������
				pTrade->iCloseNum = pTrade->iCloseNum + iCloseTodayNum;

				//����ƽ�������
				iCloseTodayNum = iCloseTodayNum - iCloseTodayNum;
			}
			else //ƽiCloseTodayNum��һ����
			{
				//���㿪���ܽ��		
				dOpenTodayAmount = dOpenTodayAmount + iAvailable*pTrade->dPrice*iVolumeMultiple;
				LDebug("iCloseTodayNum=[{0}],dPrice=[{1}],iVolumeMultiple=[{2}],dOpenTodayAmount=[{3}]", iCloseTodayNum, pTrade->dPrice, iVolumeMultiple, dOpenTodayAmount);

				//���´˳ɽ��ĵ���ƽ������
				pTrade->iCloseNum = pTrade->iCloseNum + iAvailable;

				//����ƽ�������
				iCloseTodayNum = iCloseTodayNum - iAvailable;
			}

			//�������ݿ�
			TradeData::UpdateTrade(*pTrade);
		}
	}

	if (POSITION_DIRECTION_LONG == cDirection)
	{
		//ƽ��ʷ��ӯ��
		//��ͷƽ��ʷ��ӯ����(����ƽ�ּۣ���һ�����ս����)������ƽ��������Լ����
		dCloseHisProfit = iCloseYdNum*TradeData.dPrice*iVolumeMultiple - iCloseYdNum*dPreSettlementPrice*iVolumeMultiple;
		LDebug("LONG:iCloseYdNum=[{0}],dPrice=[{1}],dPreSettlementPrice=[{2}],iVolumeMultiple=[{3}],dCloseHisProfit=[{4}]", iCloseYdNum, TradeData.dPrice, dPreSettlementPrice, iVolumeMultiple, dCloseHisProfit);

		//ƽ���ղ�ӯ��
		//��ͷƽ���ղ�ӯ����(��������ƽ�ּۣ��������뿪�ּ�)������ƽ��������Լ����
		dCloseTodayProfit = dCloseTodayAmount - dOpenTodayAmount;
		LDebug("LONG:dCloseTodayAmount=[{0}],dOpenTodayAmount=[{1}],dCloseTodayProfit=[{2}]", dCloseTodayAmount, dOpenTodayAmount, dCloseTodayProfit);

	}
	else if (POSITION_DIRECTION_SHORT == cDirection)
	{
		//ƽ��ʷ��ӯ��
		//��ͷƽ��ʷ��ӯ����(��һ�����ս���ۣ�����ƽ�ּ�)������ƽ��������Լ����
		dCloseHisProfit = iCloseYdNum*dPreSettlementPrice*iVolumeMultiple - iCloseYdNum*TradeData.dPrice*iVolumeMultiple;
		LDebug("SHORT:iCloseYdNum=[{0}],dPrice=[{1}],dPreSettlementPrice=[{2}],iVolumeMultiple=[{3}],dCloseHisProfit=[{4}]", iCloseYdNum, TradeData.dPrice, dPreSettlementPrice, iVolumeMultiple, dCloseHisProfit);

		//ƽ���ղ�ӯ��
		//��ͷƽ���ղ�ӯ����(�����������ּۣ���������ƽ�ּ�)������ƽ��������Լ����
		dCloseTodayProfit = dOpenTodayAmount - dCloseTodayAmount;
		LDebug("SHORT:dCloseTodayAmount=[{0}],dOpenTodayAmount=[{1}],dCloseTodayProfit=[{2}]", dCloseTodayAmount, dOpenTodayAmount, dCloseTodayProfit);

	}

	//ƽ��ӯ����ƽ��ʷ��ӯ����ƽ���ղ�ӯ��
	double dTotalProfit = dCloseTodayProfit + dCloseHisProfit;
	LDebug("dCloseTodayProfit=[{0}],dCloseHisProfit=[{1}],dTotalProfit=[{2}]", dCloseTodayProfit, dCloseHisProfit, dTotalProfit);

	//����Balance
	BalanceT  BalanceData;
	BalanceData.szUserId = m_szUserId;
	BalanceData.dAddMoney = dTotalProfit;
	BalanceData.iModifyDate = zutil::GetToday();
	BalanceData.iModifyTime = zutil::GetNowSedondTime();

	UpdateBalance(BalanceData);

}
//����ί�и��³ֲ�
bool RuntimeManagerCtp::UpdatePositionByOrder(OrderT& OrderData)
{
	LDebug("UpdatePositionByOrder,szOrderRef=[{0}]  ", OrderData.szOrderRef);

	PositionT PositionData;
	char cNowStatus = OrderData.cStatus;

	if (ORDER_STATUS_SUBMIT == cNowStatus)
	{
		//��״̬Ϊ���ύʱ�������ֲ�,���㱣֤��������ѡ�
		//�ֲֲ�������Ϊ���ᡣPositionData.cOperType = POSITION_OPER_FROZEN;

		char cDirection = POSITION_DIRECTION_NET;
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_LONG;
			LDebug("��ͷ  ���򿪲֣���ƽ��");
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_SHORT;
			LDebug("��ͷ  �������֣���ƽ��");
		}

		string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;
		LDebug("UpdatePositionByOrder�����ֲ֣��ֲַ���szSecurityDirectionKey=[{0}]  ״̬Ϊ���ύ,�ֲֲ�������Ϊ����", szSecurityDirectionKey);

		PositionData.iInitDate = zutil::GetToday();
		PositionData.iInitTime = zutil::GetNowSedondTime();
		PositionData.iModifyDate = zutil::GetToday();
		PositionData.iModifyTime = zutil::GetNowSedondTime();
		PositionData.szUserId = OrderData.szUserId;
		PositionData.szExchangeID = OrderData.szExchangeID;
		PositionData.szStockCode = OrderData.szStockCode;
		PositionData.cDirection = cDirection;
		PositionData.cHedgeFlag = HF_Speculation;
		PositionData.cOperType = POSITION_OPER_FROZEN;//����
		PositionData.cAction = OrderData.cAction;
		PositionData.dSettlementPrice = OrderData.dPreSettlementPrice;
		PositionData.dMarginRateByMoney = OrderData.dMarginRatioByMoney;
		PositionData.dMarginRateByVolume = OrderData.dMarginRatioByVolume;

		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			//��
			//���������Ϊί������
			//����Ľ��Ϊί������*ί�м۸�*��Լ����
			PositionData.iLongFrozen = OrderData.iEntrustNum;
			PositionData.dLongFrozenAmount = OrderData.iEntrustNum*OrderData.dPrice*OrderData.iVolumeMultiple;

			//��iLongFrozen��dLongFrozenAmount���µ�order
			OrderData.iLongShortFrozen = PositionData.iLongFrozen;
			OrderData.dLongShortFrozenAmount = PositionData.dLongFrozenAmount;
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//��
			//���������Ϊί������
			//����Ľ��Ϊί������*ί�м۸�*��Լ����
			PositionData.iShortFrozen = OrderData.iEntrustNum;
			PositionData.dShortFrozenAmount = OrderData.iEntrustNum*OrderData.dPrice*OrderData.iVolumeMultiple;

			//��iShortFrozen��dShortFrozenAmount���µ�order
			OrderData.iLongShortFrozen = PositionData.iShortFrozen;
			OrderData.dLongShortFrozenAmount = PositionData.dShortFrozenAmount;
		}

		//���㱣֤���������
		//��֤�� = ��������֤���*����+����֤����*�۸�*����*��Լ������
		//���㱣֤��ʹ�õļ۸�����������ۣ���ֿ���ʹ�������ۣ����¼ۣ��ɽ����ۣ����ּۡ�simnow���ʹ�������ۡ�
		//Ϊ�˺�ctp�ķ��滷��simnow����һ�£�����������ֺͽ�ֵı�֤��۸�ͳһʹ�����ս����
		//����ͻ��ı�֤�𣬶������ͳһʹ�������ۼ��㣬���ڽ�֣����ڻ���˾��̨�ǿ������õġ��ͻ�Ҳ����ͨ��ReqQryBrokerTradingParams�������ѯ���͹�˾���ײ�������ѯ�õ����õ����͡�
		//���õ�����:�����ۣ����¼ۣ��ɽ����ۣ����ּ�
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
		{
			//����,�����ǽ��

			//ʹ�����ս������Ϊ���㱣֤��ļ۸�
			double dTodayPrice = OrderData.dPreSettlementPrice;
			//���㱣֤��
			PositionData.dFrozenMargin = OrderData.iEntrustNum*OrderData.dMarginRatioByVolume + OrderData.dMarginRatioByMoney*dTodayPrice*OrderData.iEntrustNum*OrderData.iVolumeMultiple;
			//���������ѡ������� = ������������*����+��������*�۸�*����*��Լ������
			PositionData.dFrozenCommission = OrderData.iEntrustNum*OrderData.dRatioByVolume + OrderData.dRatioByMoney*OrderData.dPrice*OrderData.iEntrustNum*OrderData.iVolumeMultiple;

			PositionData.iYdPosition = 0;
			PositionData.iTodayPosition = 0;
			PositionData.iPosition = 0;

			//���³ֲ�
			UpdatePosition(PositionData);

			//�ѿ��ֶ���ı�֤��������Ѹ��µ�order
			OrderData.dFrozenMargin = PositionData.dFrozenMargin;
			OrderData.dFrozenCommission = PositionData.dFrozenCommission;

		}
		else if (ORDER_ACTION_BUY_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//ƽ��,ƽ����ƽ��ֺ�ƽ���

			PositionData.dFrozenCommission = GetCloseCommission(OrderData);
			PositionData.dFrozenMargin = 0;
			PositionData.iYdPosition = 0;
			PositionData.iTodayPosition = 0;
			PositionData.iPosition = 0;

			//���³ֲ�
			UpdatePosition(PositionData);

			//��ƽ�ֶ���������Ѹ��µ�order
			OrderData.dFrozenCommission = PositionData.dFrozenCommission;
		}
	}
	else if (ORDER_STATUS_INSERT_REFUSE == cNowStatus || ORDER_STATUS_CANCEL == cNowStatus || (ORDER_STATUS_ALL_TRADE == cNowStatus&&OrderData.iEntrustNum == OrderData.iTradeNum))
	{
		//��������״̬���������ܾ�,����,ȫ���ɽ�����ʱorder��ʣ�ಿ��ȫ���ⶳ�����³ֲ�
		//�������ܾ�:�ⶳ����Ĳ���
		//����:����ʱ����ȫ��δ�ɽ��������Ѿ����ֳɽ�����δ�ɽ��Ľⶳ
		//ȫ���ɽ����Ѷඳ��Ĳ��ֽ��нⶳ�����ʹ�����ս���ۼ��㱣֤�𲻻�����ඳ�ᣬ���ʹ�������۸���㱣֤������ܲ����ඳ��
		//�ֲֲ�������Ϊʣ�ಿ�ֽⶳ����PositionData.cOperType = POSITION_OPER_REMAIN_UN_FROZEN;

		char cDirection = POSITION_DIRECTION_NET;
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_LONG;
			LDebug("��ͷ  ���򿪲֣���ƽ��");
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_SHORT;
			LDebug("��ͷ  �������֣���ƽ��");
		}

		string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;
		LDebug("UpdatePositionByOrder�����ֲ֣��ֲַ���szSecurityDirectionKey=[{0}] ״̬Ϊ����״̬���ֲֲ�������Ϊʣ�ಿ�ֽⶳ", szSecurityDirectionKey);

		PositionData.iInitDate = zutil::GetToday();
		PositionData.iInitTime = zutil::GetNowSedondTime();
		PositionData.iModifyDate = zutil::GetToday();
		PositionData.iModifyTime = zutil::GetNowSedondTime();
		PositionData.szUserId = OrderData.szUserId;
		PositionData.szExchangeID = OrderData.szExchangeID;
		PositionData.szStockCode = OrderData.szStockCode;
		PositionData.cDirection = cDirection;
		PositionData.cHedgeFlag = HF_Speculation;
		PositionData.cOperType = POSITION_OPER_REMAIN_UN_FROZEN;//ʣ�ಿ�ֽⶳ
		PositionData.cAction = OrderData.cAction;
		PositionData.dSettlementPrice = OrderData.dPreSettlementPrice;
		PositionData.dMarginRateByMoney = OrderData.dMarginRatioByMoney;
		PositionData.dMarginRateByVolume = OrderData.dMarginRatioByVolume;

		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			//��
			//���������Ϊorderʣ�������
			//����Ľ��Ϊorderʣ��Ľ��
			PositionData.iLongFrozen = OrderData.iLongShortFrozen;
			PositionData.dLongFrozenAmount = OrderData.dLongShortFrozenAmount;
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//��
			//���������Ϊorderʣ�������
			//����Ľ��Ϊorderʣ��Ľ��
			PositionData.iShortFrozen = OrderData.iLongShortFrozen;
			PositionData.dShortFrozenAmount = OrderData.dLongShortFrozenAmount;

		}

		//����ı�֤��Ϊorderʣ��ı�֤��
		//�����������Ϊorderʣ���������
		PositionData.dFrozenMargin = OrderData.dFrozenMargin;
		PositionData.dFrozenCommission = OrderData.dFrozenCommission;

		//���³ֲ�
		UpdatePosition(PositionData);

	}
	else
	{
		return false;
	}


	return true;
}
//���ݽ��׸��³ֲ�
bool RuntimeManagerCtp::UpdatePositionByTrade(TradeT &TradeData)
{
	//���гɽ�ʱ�������ֲ֣����㱣֤���������
	//�ֲֲ�������Ϊ���ֽⶳ����PositionData.cOperType = POSITION_OPER_PART_UN_FROZEN;

	PositionT PositionData;
	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("UpdatePositionByTrade �����ֲ֣��ֲַ���szSecurityDirectionKey=[{0}] ״̬Ϊ�ɽ����ֲֲ�������Ϊ���ֽⶳ", szSecurityDirectionKey);


	double dOrderPrice = 0;
	int iVolumeMultiple = 0;//��Լ��������
	double	dMarginRatioByMoney = 0;///��֤����	
	double	dMarginRatioByVolume = 0;///��֤���	
	double	dRatioByMoney = 0;///��ƽ�ֺ�Լ��������	
	double	dRatioByVolume = 0;///��ƽ�ֺ�Լ������							   
	double	dCloseTodayRatioByMoney = 0;///ƽ����������	
	double	dCloseTodayRatioByVolume = 0;///ƽ��������
	double  dPreSettlementPrice = 0; //������

	//��ȡorder��Ϣ
	{
		lock_guard<mutex> lk(m_OrderMtx);
		if (0 == m_OrdersMap[TradeData.szOrderRef])
		{
			LError("old Order is not exist  szOrderRef=[{0}]", TradeData.szOrderRef);
			return false;
		}
		shared_ptr<OrderT> pOrder = m_OrdersMap[TradeData.szOrderRef];
		dOrderPrice = pOrder->dPrice;
		iVolumeMultiple = pOrder->iVolumeMultiple;
		dMarginRatioByVolume = pOrder->dMarginRatioByVolume;
		dMarginRatioByMoney = pOrder->dMarginRatioByMoney;
		dRatioByMoney = pOrder->dRatioByMoney;
		dRatioByVolume = pOrder->dRatioByVolume;
		dPreSettlementPrice = pOrder->dPreSettlementPrice;
		dCloseTodayRatioByMoney = pOrder->dCloseTodayRatioByMoney;
		dCloseTodayRatioByVolume = pOrder->dCloseTodayRatioByVolume;

	}

	PositionData.iInitDate = zutil::GetToday();
	PositionData.iInitTime = zutil::GetNowSedondTime();
	PositionData.iModifyDate = zutil::GetToday();
	PositionData.iModifyTime = zutil::GetNowSedondTime();
	PositionData.szUserId = TradeData.szUserId;
	PositionData.szExchangeID = TradeData.szExchangeID;
	PositionData.szStockCode = TradeData.szStockCode;
	PositionData.cDirection = cDirection;
	PositionData.cHedgeFlag = HF_Speculation;
	PositionData.cAction = TradeData.cAction;
	PositionData.cOperType = POSITION_OPER_PART_UN_FROZEN;//���ֽⶳ
	PositionData.dSettlementPrice = dPreSettlementPrice;
	PositionData.dMarginRateByMoney = dMarginRatioByMoney;
	PositionData.dMarginRateByVolume = dMarginRatioByVolume;

	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		//��
		//���������Ϊ�ɽ�����
		PositionData.iLongFrozen = TradeData.iTradeNum;
		//�˴�ʹ��ί�м۸���Ϊ�����ʱ��ʹ�õļ۸���ί�м۸�
		//�ɽ��۸�һ����ί�м۸����
		//����Ľ��Ϊ�ɽ�����*ί�м۸�*��Լ����
		PositionData.dLongFrozenAmount = TradeData.iTradeNum*dOrderPrice*iVolumeMultiple;
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		//��
		//���������Ϊ�ɽ�����
		PositionData.iShortFrozen = TradeData.iTradeNum;
		//�˴�ʹ��ί�м۸���Ϊ�����ʱ��ʹ�õļ۸���ί�м۸�
		//�ɽ��۸�һ����ί�м۸����
		//����Ľ��Ϊ�ɽ�����*ί�м۸�*��Լ����
		PositionData.dShortFrozenAmount = TradeData.iTradeNum*dOrderPrice*iVolumeMultiple;

	}

	//���㱣֤���������
	//��֤�� = ��������֤���*����+����֤����*�۸�*����*��Լ������
	//���㱣֤��ʹ�õļ۸�����������ۣ���ֿ���ʹ�������ۣ����¼ۣ��ɽ����ۣ����ּۡ�simnow���ʹ�������ۡ�
	//Ϊ�˺�ctp�ķ��滷��simnow����һ�£�����������ֺͽ�ֵı�֤��۸�ͳһʹ�����ս����
	//����ͻ��ı�֤�𣬶������ͳһʹ�������ۼ��㣬���ڽ�֣����ڻ���˾��̨�ǿ������õġ��ͻ�Ҳ����ͨ��ReqQryBrokerTradingParams�������ѯ���͹�˾���ײ�������ѯ�õ����õ����͡�
	//���õ�����:�����ۣ����¼ۣ��ɽ����ۣ����ּ�
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		//����,�����ǽ��

		//��ּ��㱣֤��ʹ�õļ۸�ʹ�����ս������Ϊ��ּ��㱣֤��ʹ�õļ۸�
		double dTodayPrice = dPreSettlementPrice;
		//���㱣֤��
		PositionData.dFrozenMargin = TradeData.iTradeNum*dMarginRatioByVolume + dMarginRatioByMoney*dTodayPrice*TradeData.iTradeNum*iVolumeMultiple;
		//����������,������ = ������������*����+��������*�۸�*����*��Լ������
		PositionData.dFrozenCommission = TradeData.iTradeNum*dRatioByVolume + dRatioByMoney*TradeData.dPrice*TradeData.iTradeNum*iVolumeMultiple;

		PositionData.iYdPosition = 0;
		PositionData.iTodayPosition = TradeData.iTradeNum;
		PositionData.iPosition = TradeData.iTradeNum;

		//���³ֲ�
		UpdatePosition(PositionData);
	}
	else if (ORDER_ACTION_BUY_CLOSE == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		lock_guard<mutex> lk(m_PositionMtx);

		//ƽ��,ƽ����ƽ��ֺ�ƽ���

		//��ּ��㱣֤��ʹ�õļ۸�ʹ�����ս������Ϊ��ּ��㱣֤��ʹ�õļ۸�
		double dTodayPrice = dPreSettlementPrice;
		//��ּ��㱣֤��ʹ�õļ۸�ʹ�����ս������Ϊ��ּ��㱣֤��ʹ�õļ۸�
		double dYdPrice = dPreSettlementPrice;

		//���ݲ�ѯ��ԭ�ֲ��������м���
		if (0 == m_PositionsMap.count(szSecurityDirectionKey))
		{

			LError("�ֲ�����Ϊ0,szOrderRef=[{0}]  ", TradeData.szOrderRef);
			return false;
		}
		else
		{

			shared_ptr<PositionT> pPosition = m_PositionsMap[szSecurityDirectionKey];

			//���㱣֤��:�����ָ��ƽ��֣�����ƽ��֣���ƽ���
			{
				int iYdPosition = pPosition->iYdPosition;
				//iValue=�������-�ɽ�����
				int iValue = pPosition->iYdPosition - TradeData.iTradeNum;
				if (iValue >= 0)  //�ɽ�����С�ڵ������������ƽ���
				{
					//ʹ�óɽ����������ս���ۼ��㱣֤��
					PositionData.dFrozenMargin = TradeData.iTradeNum*dMarginRatioByVolume + dMarginRatioByMoney*dYdPrice*TradeData.iTradeNum*iVolumeMultiple;

					PositionData.iYdPosition = TradeData.iTradeNum;
					PositionData.iTodayPosition = 0;
					PositionData.iPosition = TradeData.iTradeNum;
				}
				else  //�ɽ��������������������ƽ���,��ƽ���
				{
					//�������
					int iTodayPosition = 0 - iValue;

					//��ƽ���
					//ʹ����������������ۼ��㱣֤��
					PositionData.dFrozenMargin = iYdPosition*dMarginRatioByVolume + dMarginRatioByMoney*dYdPrice*iYdPosition*iVolumeMultiple;

					//��ƽ���
					//ʹ�ý�������������ۼ��㱣֤��
					PositionData.dFrozenMargin = PositionData.dFrozenMargin + iTodayPosition*dMarginRatioByVolume + dMarginRatioByMoney*dTodayPrice*iTodayPosition*iVolumeMultiple;

					PositionData.iYdPosition = iYdPosition;
					PositionData.iTodayPosition = iTodayPosition;
					PositionData.iPosition = TradeData.iTradeNum;
				}
			}

			//����������
			OrderT OrderData;
			OrderData.dPrice = dOrderPrice;
			OrderData.iVolumeMultiple = iVolumeMultiple;
			OrderData.dMarginRatioByVolume = dMarginRatioByVolume;
			OrderData.dMarginRatioByMoney = dMarginRatioByMoney;
			OrderData.dRatioByMoney = dRatioByMoney;
			OrderData.dRatioByVolume = dRatioByVolume;
			OrderData.dPreSettlementPrice = dPreSettlementPrice;
			OrderData.dCloseTodayRatioByMoney = dCloseTodayRatioByMoney;
			OrderData.dCloseTodayRatioByVolume = dCloseTodayRatioByVolume;

			PositionData.dFrozenCommission = GetCloseCommission(TradeData, OrderData);

		}

		//���³ֲ�
		UpdatePosition(PositionData, false);
	}


	//����order����ȥ�ɽ��Ĳ���
	{
		lock_guard<mutex> lk(m_OrderMtx);
		shared_ptr<OrderT> pOrder = m_OrdersMap[TradeData.szOrderRef];

		//��ȥiLongFrozen��dLongFrozenAmount��iShortFrozen��dShortFrozenAmount
		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
		{
			//��
			pOrder->iLongShortFrozen = pOrder->iLongShortFrozen - PositionData.iLongFrozen;
			pOrder->dLongShortFrozenAmount = pOrder->dLongShortFrozenAmount - PositionData.dLongFrozenAmount;

		}
		else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
		{
			//��
			pOrder->iLongShortFrozen = pOrder->iLongShortFrozen - PositionData.iShortFrozen;
			pOrder->dLongShortFrozenAmount = pOrder->dLongShortFrozenAmount - PositionData.dShortFrozenAmount;


		}

		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
		{
			//����,��ȥ��֤��
			pOrder->dFrozenMargin = pOrder->dFrozenMargin - PositionData.dFrozenMargin;
		}

		//��ȥ������
		pOrder->dFrozenCommission = pOrder->dFrozenCommission - PositionData.dFrozenCommission;
	}

	return true;
}
//���³ֲ�
void RuntimeManagerCtp::UpdatePosition(PositionT &PositionData, bool bIsLock)
{

	LDebug("RuntimeManagerCtp  UpdatePosition,  m_szUserId=[{0}],cOperType=[{1}],cAction=[{1}]", m_szUserId, PositionData.cOperType, PositionData.cAction);
	if (bIsLock)
	{
		lock_guard<mutex> lk(m_PositionMtx);
		UpdatePositionByUnLock(PositionData);
	}
	else
	{
		UpdatePositionByUnLock(PositionData);
	}
}
//���³ֲ�
void RuntimeManagerCtp::UpdatePositionByUnLock(PositionT &PositionData)
{
	//��ӡ�ֲ�
	PrintData::PrintPosition(PositionData);

	BalanceT  BalanceData;
	BalanceData.szUserId = m_szUserId;
	string szSecurityDirectionKey = PositionData.szExchangeID + '_' + PositionData.szStockCode + '_' + PositionData.cDirection;

	if (m_PositionsMap.find(szSecurityDirectionKey) == m_PositionsMap.end())
	{
		LDebug("m_PositionsMap�����ֲ֣��ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		shared_ptr<PositionT> pPosition = make_shared<PositionT>(PositionData);
		m_PositionsMap[szSecurityDirectionKey] = pPosition;

		//�˴���ί�д����ĳֲ�	
		if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
		{
			//����	
			//����Balance
			BalanceData.dFrozenMargin = PositionData.dFrozenMargin; ///����ı�֤��	
			BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///�����������	
		}
		else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
		{
			//ƽ��
			//����Balance
			BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///�����������	
		}

		PositionT TmpPositionData;
		TradeData::GetPositions(m_szUserId, PositionData.szExchangeID, PositionData.szStockCode, PositionData.cDirection, TmpPositionData);
		if ("" == TmpPositionData.szUserId)
		{
			TradeData::InsertPosition(PositionData);
			LDebug("TradeData::InsertPosition(PositionData)���ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		}
		else
		{
			TradeData::UpdatePosition(PositionData);
			LDebug("TradeData::UpdatePosition(PositionData)���ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		}
	}
	else
	{
		LDebug("m_PositionsMap���³ֲ֣��ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		shared_ptr<PositionT> pPosition = m_PositionsMap[szSecurityDirectionKey];

		LDebug("-----------��ӡ����ǰ�ĳֲ�����--------");
		PrintData::PrintPosition(*pPosition);


		if (POSITION_OPER_FROZEN == PositionData.cOperType)//����
		{
			//����������
			pPosition->dFrozenCommission = pPosition->dFrozenCommission + PositionData.dFrozenCommission;

			//����
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
			{
				//���ֶ��ᱣ֤��
				pPosition->dFrozenMargin = pPosition->dFrozenMargin + PositionData.dFrozenMargin;

				//����Balance
				BalanceData.dFrozenMargin = PositionData.dFrozenMargin; ///����ı�֤��	
				BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///�����������	
			}
			else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//ƽ��
				//����Balance
				BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///�����������	
			}

			//�����ն��������ͽ��
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_BUY_CLOSE == PositionData.cAction)
			{
				//��
				pPosition->iLongFrozen = pPosition->iLongFrozen + PositionData.iLongFrozen;
				pPosition->dLongFrozenAmount = pPosition->dLongFrozenAmount + PositionData.dLongFrozenAmount;

			}
			else if (ORDER_ACTION_SELL_OPEN == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//��
				pPosition->iShortFrozen = pPosition->iShortFrozen + PositionData.iShortFrozen;
				pPosition->dShortFrozenAmount = pPosition->dShortFrozenAmount + PositionData.dShortFrozenAmount;
			}

			LDebug("���Ӷ��ᣬ�ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		}
		else if (POSITION_OPER_PART_UN_FROZEN == PositionData.cOperType)//���ֽⶳ
		{
			//�Ѷ�����������ƶ���ռ����
			pPosition->dFrozenCommission = pPosition->dFrozenCommission - PositionData.dFrozenCommission;
			pPosition->dCommission = pPosition->dCommission + PositionData.dFrozenCommission;

			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
			{
				//����
				//�Ѷ���ı�֤���ƶ���ռ����
				pPosition->dFrozenMargin = pPosition->dFrozenMargin - PositionData.dFrozenMargin;
				pPosition->dUseMargin = pPosition->dUseMargin + PositionData.dFrozenMargin;

				//���ӳֲ�����
				pPosition->iTodayPosition = pPosition->iTodayPosition + PositionData.iTodayPosition;
				pPosition->iPosition = pPosition->iPosition + PositionData.iPosition;

				//����Balance
				BalanceData.dFrozenMargin = 0 - PositionData.dFrozenMargin; ///����ı�֤��	
				BalanceData.dUseMargin = PositionData.dFrozenMargin;///ռ�õı�֤��	
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///�����������					
				BalanceData.dCommission = PositionData.dFrozenCommission; ///������	
			}
			else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//ƽ��
				//��ռ�õı�֤���ͷ�
				pPosition->dUseMargin = pPosition->dUseMargin - PositionData.dFrozenMargin;


				//���ٳֲ�����
				pPosition->iTodayPosition = pPosition->iTodayPosition - PositionData.iTodayPosition;
				pPosition->iYdPosition = pPosition->iYdPosition - PositionData.iYdPosition;
				pPosition->iPosition = pPosition->iPosition - PositionData.iPosition;

				//����Balance
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///�����������
				BalanceData.dCommission = PositionData.dFrozenCommission; ///������	
				BalanceData.dUseMargin = 0 - PositionData.dFrozenMargin;///ռ�õı�֤��	

			}

			//�����ն��������ͽ��
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_BUY_CLOSE == PositionData.cAction)
			{
				//��
				pPosition->iLongFrozen = pPosition->iLongFrozen - PositionData.iLongFrozen;
				pPosition->dLongFrozenAmount = pPosition->dLongFrozenAmount - PositionData.dLongFrozenAmount;
			}
			else if (ORDER_ACTION_SELL_OPEN == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//��
				pPosition->iShortFrozen = pPosition->iShortFrozen - PositionData.iShortFrozen;
				pPosition->dShortFrozenAmount = pPosition->dShortFrozenAmount - PositionData.dShortFrozenAmount;
			}

			LDebug("���ֽⶳ���ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

		}
		else if (POSITION_OPER_REMAIN_UN_FROZEN == PositionData.cOperType) //ʣ�ಿ�ֽⶳ
		{
			//��������״̬���������ܾ�,����,ȫ���ɽ�����ʱorder��ʣ�ಿ��ȫ���ⶳ�����³ֲ�
			//�������ܾ�:�ⶳ����Ĳ���
			//����:����ʱ����ȫ��δ�ɽ��������Ѿ����ֳɽ�����δ�ɽ��Ľⶳ
			//ȫ���ɽ����Ѷඳ��Ĳ��ֽ��нⶳ�����ʹ�����ս���ۼ��㱣֤�𲻻�����ඳ�ᣬ���ʹ�������۸���㱣֤������ܲ����ඳ��


			//����ඳ���������
			pPosition->dFrozenCommission = pPosition->dFrozenCommission - PositionData.dFrozenCommission;

			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
			{
				//����
				//����ඳ��ı�֤��
				pPosition->dFrozenMargin = pPosition->dFrozenMargin - PositionData.dFrozenMargin;

				//����Balance
				BalanceData.dFrozenMargin = 0 - PositionData.dFrozenMargin; ///����ı�֤��	
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///�����������					
			}
			else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//ƽ��

				//����Balance
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///�����������

			}

			//�����ն��������ͽ��
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_BUY_CLOSE == PositionData.cAction)
			{
				//��
				pPosition->iLongFrozen = pPosition->iLongFrozen - PositionData.iLongFrozen;
				pPosition->dLongFrozenAmount = pPosition->dLongFrozenAmount - PositionData.dLongFrozenAmount;
			}
			else if (ORDER_ACTION_SELL_OPEN == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//��
				pPosition->iShortFrozen = pPosition->iShortFrozen - PositionData.iShortFrozen;
				pPosition->dShortFrozenAmount = pPosition->dShortFrozenAmount - PositionData.dShortFrozenAmount;
			}

			LDebug("ʣ�ಿ�ֽⶳ���ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

		}
		else
		{
			LError("cOperType error��cOperType=[{0}]", PositionData.cOperType);

		}

		pPosition->iModifyDate = PositionData.iModifyDate;
		pPosition->iModifyTime = PositionData.iModifyTime;

		//��ӡ�ֲ�
		LDebug("-----------�ֲ��Ѿ����£���ӡ���º�ĳֲ�����--------");
		PrintData::PrintPosition(*pPosition);

		//�������ݿ�
		TradeData::UpdatePosition(*pPosition);

	}

	//����Balance
	BalanceData.iModifyDate = PositionData.iModifyDate; //�޸�����
	BalanceData.iModifyTime = PositionData.iModifyTime; //�޸�ʱ��
	UpdateBalance(BalanceData);

}

//��ѯ�ʽ�
BalanceT RuntimeManagerCtp::GetBalance()
{
	lock_guard<mutex> lk(m_BalanceMtx);

	LDebug("RuntimeManagerCtp  GetBalance,  m_szUserId=[{0}]", m_szUserId);

	return m_Balance;
}

//����OrderRef��ѯί��
bool RuntimeManagerCtp::GetOrderByOrderRef(string& szOrderRef, shared_ptr<OrderT>& pOrderT)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetOrderByOrderRef,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, szOrderRef);

	if (m_OrdersMap.count(szOrderRef) > 0)
	{
		pOrderT = m_OrdersMap[szOrderRef];

		//��ӡί��
		PrintData::PrintOrder(*pOrderT);

		return true;
	}
	else
	{
		return false;
	}
	return true;
}
//���ݿͻ���ClientOrderId��ѯί��
bool RuntimeManagerCtp::GetOrderByClientOrderId(const string& szClientOrderId, shared_ptr<OrderT>& pOrderT)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetOrderByClientOrderId,  m_szUserId=[{0}],szClientOrderId=[{1}]", m_szUserId, szClientOrderId);

	string szOrderRef;
	if (m_ClientOrderIdOrderRefMap.count(szClientOrderId) > 0)
	{
		szOrderRef = m_ClientOrderIdOrderRefMap[szClientOrderId];
		LDebug("RuntimeManagerCtp  GetOrderByClientOrderId,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, szOrderRef);

	}
	else
	{
		return false;
	}

	if (m_OrdersMap.count(szOrderRef) > 0)
	{
		pOrderT = m_OrdersMap[szOrderRef];

		//��ӡί��
		PrintData::PrintOrder(*pOrderT);
		return true;
	}
	else
	{
		return false;
	}
	return true;
}
//��ѯ��������ί��
bool RuntimeManagerCtp::GetOrders(vector<shared_ptr<OrderT>>& Orders)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetOrders,  m_szUserId=[{0}]", m_szUserId);

	if (m_OrdersMap.size() > 0)
	{
		for (auto &OrderPair : m_OrdersMap)
		{
			Orders.push_back(OrderPair.second);

			//��ӡί��
			PrintData::PrintOrder(*(OrderPair.second));
		}

		return true;
	}
	else
	{
		return false;
	}
	return true;
}

//��ѯ�������н���
bool  RuntimeManagerCtp::GetTrades(vector<shared_ptr<TradeT>>& Trades)
{
	lock_guard<mutex> lk(m_TradeMtx);

	LDebug("RuntimeManagerCtp  GetTrades,  m_szUserId=[{0}]", m_szUserId);

	if (m_TradesMap.size() > 0)
	{
		for (auto &TradePair : m_TradesMap)
		{
			Trades.push_back(TradePair.second);

			//��ӡ����
			PrintData::PrintTrade(*(TradePair.second));
		}
		return true;
	}
	else
	{
		return false;
	}

	return true;
}
//����OrderRef��ѯ����
bool RuntimeManagerCtp::GetTradesByOrderRef(string& szOrderRef, vector<shared_ptr<TradeT>>&vTradeT)
{
	lock_guard<mutex> lk(m_TradeMtx);

	LDebug("RuntimeManagerCtp  GetTradesByOrderRef,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, szOrderRef);

	shared_ptr<list<string>> pTradeList;
	if (m_OrderTradeMap.count(szOrderRef) > 0)
	{
		pTradeList = m_OrderTradeMap[szOrderRef];
	}
	else
	{
		return false;
	}

	list<string>  ::iterator iter;
	for (iter = pTradeList->begin(); iter != pTradeList->end(); ++iter)
	{
		string szTradeId = *iter;
		shared_ptr<TradeT> pTrade = m_TradesMap[szTradeId];
		vTradeT.push_back(pTrade);

		//��ӡ����
		PrintData::PrintTrade(*pTrade);
	}

	return true;
}
//���ݿͻ���ClientOrderId��ѯ����
bool RuntimeManagerCtp::GetTradesByClientOrderId(string& szClientOrderId, vector<shared_ptr<TradeT>>&vTradeT)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetTradesByClientOrderId,  m_szUserId=[{0}],szClientOrderId=[{1}]", m_szUserId, szClientOrderId);

	if (m_ClientOrderIdOrderRefMap.count(szClientOrderId) > 0)
	{
		string szOrderRef = m_ClientOrderIdOrderRefMap[szClientOrderId];
		bool bRet = GetTradesByOrderRef(szOrderRef, vTradeT);
		return bRet;
	}
	else
	{
		return false;
	}

	return true;
}
//���ݹ�Ʊ�����ѯ�ֲ�
bool RuntimeManagerCtp::GetPositionBySecurity(string &szExchangeID, string &szStockCode, vector<shared_ptr<PositionT>>&vPositionT)
{
	lock_guard<mutex> lk(m_PositionMtx);

	LDebug("RuntimeManagerCtp  GetPositionBySecurity,  m_szUserId=[{0}],szExchangeID=[{1}],szStockCode=[{2}]", m_szUserId, szExchangeID, szStockCode);

	int iCount = 0;
	char cDirection = POSITION_DIRECTION_LONG;
	string szSecurityDirectionKey = szExchangeID + '_' + szStockCode + '_' + cDirection;

	if (m_PositionsMap.count(szSecurityDirectionKey) > 0)
	{
		shared_ptr<PositionT> pPositionT = m_PositionsMap[szSecurityDirectionKey];
		vPositionT.push_back(pPositionT);
		iCount++;

		//��ӡ�ֲ�
		PrintData::PrintPosition(*pPositionT);
	}

	cDirection = POSITION_DIRECTION_SHORT;
	szSecurityDirectionKey = szExchangeID + '_' + szStockCode + '_' + cDirection;
	if (m_PositionsMap.count(szSecurityDirectionKey) > 0)
	{
		shared_ptr<PositionT> pPositionT = m_PositionsMap[szSecurityDirectionKey];
		vPositionT.push_back(pPositionT);
		iCount++;

		//��ӡ�ֲ�
		PrintData::PrintPosition(*pPositionT);
	}

	if (0 == iCount)
	{
		return false;
	}

	return true;
}
//���ݹ�Ʊ����ͳֲַ����ѯ�ֲ�
bool RuntimeManagerCtp::GetPositionBySecurity(string &szExchangeID, string &szStockCode, char cDirection, shared_ptr<PositionT>&pPosition)
{
	lock_guard<mutex> lk(m_PositionMtx);

	LDebug("RuntimeManagerCtp  GetPositionBySecurity,  m_szUserId=[{0}],szExchangeID=[{1}],szStockCode=[{2}],cDirection=[{3}]", m_szUserId, szExchangeID, szStockCode, cDirection);

	int iCount = 0;
	string szSecurityDirectionKey = szExchangeID + '_' + szStockCode + '_' + cDirection;

	if (m_PositionsMap.count(szSecurityDirectionKey) > 0)
	{
		pPosition = m_PositionsMap[szSecurityDirectionKey];
		iCount++;

		//��ӡ�ֲ�
		PrintData::PrintPosition(*pPosition);
	}

	if (0 == iCount)
	{
		return false;
	}

	return true;
}
//��ѯ���й�Ʊ�ĳֲ�
bool RuntimeManagerCtp::GetPositions(vector<shared_ptr<PositionT>>& Positions)
{
	lock_guard<mutex> lk(m_PositionMtx);

	LDebug("RuntimeManagerCtp  GetPositions,  m_szUserId=[{0}]", m_szUserId);

	if (m_PositionsMap.size() > 0)
	{
		for (auto &PositionPair : m_PositionsMap)
		{
			Positions.push_back(PositionPair.second);

			//��ӡ�ֲ�
			PrintData::PrintPosition(*(PositionPair.second));
		}
		return true;
	}
	else
	{
		return false;
	}

	return true;
}
//��ȡδ�ɽ�������
int  RuntimeManagerCtp::GetNotTradeNum(string& szOrderRef)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetNotTradeNum,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, szOrderRef);

	int iNum = 0;
	if (m_OrdersMap.count(szOrderRef) > 0)
	{
		iNum = m_OrdersMap[szOrderRef]->iEntrustNum - m_OrdersMap[szOrderRef]->iTradeNum;
	}

	return iNum;
}
//���տ���:����ʽ�
bool RuntimeManagerCtp::RiskBalance(const OrderT&OrderData)
{
	lock_guard<mutex> lk(m_BalanceMtx);

	//���ּ���ʽ�
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
	{
		double dAmount = OrderData.iEntrustNum*OrderData.dPrice;

		//���ڸ��������ʹ�����ʽ���٣����Դ˴�ʹ��RISK_BALANCE_RATE
		double dAvailable = m_Balance.dAvailable*RISK_BALANCE_RATE;

		//���������ʽ�
		if ((dAmount - dAvailable) > 0)
		{
			return false;
		}

	}

	return true;
}
//���տ���:���ֲ�
bool RuntimeManagerCtp::RiskPosition(const OrderT&OrderData)
{
	lock_guard<mutex> lk(m_PositionMtx);

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}
	string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;

	/*
	��ͷ�ֲ��¿�ƽ���� = �ֲ����� - ��ͷ����������
	��ͷ�ֲ��¿�ƽ���� = �ֲ����� - ��ͷ����������
	*/
	if (POSITION_DIRECTION_LONG == cDirection)
	{
		//ƽ�ּ��ֲ�
		if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			if (m_PositionsMap.count(szSecurityDirectionKey) > 0)
			{
				shared_ptr<PositionT> pPositionT = m_PositionsMap[szSecurityDirectionKey];
				int iAvailable = pPositionT->iPosition - pPositionT->iShortFrozen;
				if (OrderData.iEntrustNum > iAvailable)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	else if (POSITION_DIRECTION_SHORT == cDirection)
	{
		//ƽ�ּ��ֲ�
		if (ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			if (m_PositionsMap.count(szSecurityDirectionKey) > 0)
			{
				shared_ptr<PositionT> pPositionT = m_PositionsMap[szSecurityDirectionKey];
				int iAvailable = pPositionT->iPosition - pPositionT->iLongFrozen;
				if (OrderData.iEntrustNum > iAvailable)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}
//szClientOrderId�Ƿ����
bool RuntimeManagerCtp::IsClientOrderIdExist(const string& szClientOrderId)
{
	lock_guard<mutex> lk(m_OrderMtx);
	LDebug("RuntimeManagerCtp  IsClientOrderIdExist,  m_szUserId=[{0}],szClientOrderId=[{1}]", m_szUserId, szClientOrderId);

	if (m_ClientOrderIdOrderRefMap.count(szClientOrderId) > 0)
	{
		return true;
	}
	else
	{
		return false;
	}

	return true;
}
//szOrderRef�Ƿ����
bool RuntimeManagerCtp::IsOrderRefExist(const string& szOrderRef)
{
	lock_guard<mutex> lk(m_OrderMtx);
	LDebug("RuntimeManagerCtp  IsOrderRefExist,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, szOrderRef);

	if (m_OrdersMap.count(szOrderRef) > 0)
	{
		return true;
	}
	else
	{
		return false;
	}

	return true;
}

//�������ڼ��������ѵĿ�������
bool  RuntimeManagerCtp::UpdateCommissionOpenCount(TradeT &TradeData)
{
	lock_guard<mutex> lk(m_CommissionOpenCountMtx);

	LDebug("RuntimeManagerCtp  UpdateCommissionOpenCount,  m_szUserId=[{0}]", m_szUserId);

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}

	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("�ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		//�洢��������
		if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
		{
			shared_ptr<OpenCountT>  pOpenCountT = m_CommissionOpenCountMap[szSecurityDirectionKey];
			int iOldNum = pOpenCountT->iOpenCount;
			int iNewNum = iOldNum + TradeData.iTradeNum;
			pOpenCountT->iOpenCount = iNewNum;
			LDebug("iOldNum=[{0}], iNewNum=[{1}]", iOldNum, iNewNum);

			TradeData::UpdateCommissionOpenCount(*pOpenCountT);
		}
		else
		{
			shared_ptr<OpenCountT>  pOpenCountT = make_shared<OpenCountT>();
			pOpenCountT->iInitDate = zutil::GetToday();
			pOpenCountT->szExchangeID = TradeData.szExchangeID;
			pOpenCountT->szStockCode = TradeData.szStockCode;
			pOpenCountT->cDirection = cDirection;
			pOpenCountT->szUserId = TradeData.szUserId;
			pOpenCountT->iOpenCount = TradeData.iTradeNum;

			m_CommissionOpenCountMap[szSecurityDirectionKey] = pOpenCountT;
			LDebug("iNewNum=[{0}]", m_CommissionOpenCountMap[szSecurityDirectionKey]->iOpenCount);

			TradeData::InsertCommissionOpenCount(*pOpenCountT);
		}

	}
	else if (ORDER_ACTION_BUY_CLOSE == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		//ƽ��
		if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
		{
			shared_ptr<OpenCountT>  pOpenCountT = m_CommissionOpenCountMap[szSecurityDirectionKey];
			int iOldNum = pOpenCountT->iOpenCount;
			int iNewNum = iOldNum - TradeData.iTradeNum;
			LDebug("iOldNum=[{0}], iNewNum=[{1}]", iOldNum, iNewNum);

			//�ɽ��������ܴ��ڿ�������
			if (iNewNum < 0)
			{
				iNewNum = 0;
			}
			pOpenCountT->iOpenCount = iNewNum;

			TradeData::UpdateCommissionOpenCount(*pOpenCountT);
		}

	}

	return true;
}
//��ȡƽ��������
double RuntimeManagerCtp::GetCloseCommission(OrderT &OrderData)
{
	lock_guard<mutex> lk(m_CommissionOpenCountMtx);

	LDebug("RuntimeManagerCtp  GetCloseCommission,  m_szUserId=[{0}]", m_szUserId);

	//ƽ��,ƽ����ƽ��ֺ�ƽ���
	//�����������
	double dFrozenCommission = 0;

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}

	string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;
	LDebug("�ֲַ���szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);


	//�������
	int iTodayPosition = 0;
	if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
	{
		iTodayPosition = m_CommissionOpenCountMap[szSecurityDirectionKey]->iOpenCount;
	}
	LDebug("iTodayPosition=[{0}] ", iTodayPosition);

	// ������: DCE ,CZCE,CFFEX ��ƽ��ֺ�ƽ���,����������Ĭ��ֻƽ���
	if (OrderData.szExchangeID == "DCE" || OrderData.szExchangeID == "CZCE" || OrderData.szExchangeID == "CFFEX")
	{
		//������:��ƽ��ֺ�ƽ���

		//iValue=�������-ί������
		int iValue = iTodayPosition - OrderData.iEntrustNum;
		if (iValue >= 0) //ί������С�ڵ��ڽ��������ֻƽ���
		{
			//������ = ������������*����+��������*�۸�*����*��Լ������
			//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ�������ѡ�
			dFrozenCommission = OrderData.iEntrustNum*OrderData.dCloseTodayRatioByVolume + OrderData.dCloseTodayRatioByMoney*OrderData.dPrice*OrderData.iEntrustNum*OrderData.iVolumeMultiple;

		}
		else //ί���������ڽ����������ƽ��֣���ƽ���
		{

			//�������
			int iYdPosition = 0 - iValue;

			//������ = ������������*����+��������*�۸�*����*��Լ������
			//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ�������ѡ� 
			double dTodayFrozenCommission = iTodayPosition*OrderData.dCloseTodayRatioByVolume + OrderData.dCloseTodayRatioByMoney*OrderData.dPrice*iTodayPosition*OrderData.iVolumeMultiple;
			//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ��������
			double dYdFrozenCommission = iYdPosition*OrderData.dRatioByVolume + OrderData.dRatioByMoney*OrderData.dPrice*iYdPosition*OrderData.iVolumeMultiple;
			//������=ƽ���������+ƽ���������
			dFrozenCommission = dTodayFrozenCommission + dYdFrozenCommission;
		}

	}
	else
	{
		//������:����������Ĭ��ֻƽ���

		//�������
		int iYdPosition = OrderData.iEntrustNum;

		//������ = ������������*����+��������*�۸�*����*��Լ������
		//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ��������
		double dYdFrozenCommission = iYdPosition*OrderData.dRatioByVolume + OrderData.dRatioByMoney*OrderData.dPrice*iYdPosition*OrderData.iVolumeMultiple;
		//������=ƽ���������
		dFrozenCommission = dYdFrozenCommission;

	}

	return dFrozenCommission;
}
//��ȡƽ��������
double RuntimeManagerCtp::GetCloseCommission(TradeT &TradeData, OrderT &OrderData)
{
	lock_guard<mutex> lk(m_CommissionOpenCountMtx);

	LDebug("RuntimeManagerCtp  GetCloseCommission,  m_szUserId=[{0}]", m_szUserId);

	//��ȡ������Ϣ
	double dOrderPrice = OrderData.dPrice;
	int iVolumeMultiple = OrderData.iVolumeMultiple;//��Լ��������
	double	dMarginRatioByMoney = OrderData.dMarginRatioByMoney;///��֤����	
	double	dMarginRatioByVolume = OrderData.dMarginRatioByVolume;///��֤���	
	double	dRatioByMoney = OrderData.dRatioByMoney;///��ƽ�ֺ�Լ��������	
	double	dRatioByVolume = OrderData.dRatioByVolume;///��ƽ�ֺ�Լ������							   
	double	dCloseTodayRatioByMoney = OrderData.dCloseTodayRatioByMoney;///ƽ����������	
	double	dCloseTodayRatioByVolume = OrderData.dCloseTodayRatioByVolume;///ƽ��������
	double  dPreSettlementPrice = OrderData.dPreSettlementPrice; //������


	//�����������
	double dFrozenCommission = 0;

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("��ͷ  ���򿪲֣���ƽ��");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("��ͷ  �������֣���ƽ��");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("�ֲַ���szSecurityDirectionKey=[{0}] ", szSecurityDirectionKey);

	//�������
	int  iTodayPosition = 0;
	if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
	{
		iTodayPosition = m_CommissionOpenCountMap[szSecurityDirectionKey]->iOpenCount;
	}
	LDebug("iTodayPosition=[{0}] ", iTodayPosition);

	// ������: DCE ,CZCE,CFFEX ��ƽ��ֺ�ƽ���,����������Ĭ��ֻƽ���
	if (TradeData.szExchangeID == "DCE" || TradeData.szExchangeID == "CZCE" || TradeData.szExchangeID == "CFFEX")
	{
		//������:��ƽ��ֺ�ƽ���

		//iValue=�������-ί������
		int iValue = iTodayPosition - TradeData.iTradeNum;
		if (iValue >= 0) //ί������С�ڵ��ڽ��������ֻƽ���
		{
			//������ = ������������*����+��������*�۸�*����*��Լ������
			//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ�������ѡ�
			dFrozenCommission = TradeData.iTradeNum*dCloseTodayRatioByVolume + dCloseTodayRatioByMoney*TradeData.dPrice*TradeData.iTradeNum*iVolumeMultiple;

		}
		else //ί���������ڽ����������ƽ��֣���ƽ���
		{

			//�������
			int iYdPosition = 0 - iValue;

			//������ = ������������*����+��������*�۸�*����*��Լ������
			//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ�������ѡ� 
			double dTodayFrozenCommission = iTodayPosition*dCloseTodayRatioByVolume + dCloseTodayRatioByMoney*TradeData.dPrice*iTodayPosition*iVolumeMultiple;
			//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ��������
			double dYdFrozenCommission = iYdPosition*dRatioByVolume + dRatioByMoney*TradeData.dPrice*iYdPosition*iVolumeMultiple;
			//������=ƽ���������+ƽ���������
			dFrozenCommission = dTodayFrozenCommission + dYdFrozenCommission;
		}

	}
	else
	{
		//������:����������Ĭ��ֻƽ���

		//�������
		int iYdPosition = TradeData.iTradeNum;

		//������ = ������������*����+��������*�۸�*����*��Լ������
		//����ƽ��������ѣ�ƽ��ַ���ʹ��ƽ��������
		double dYdFrozenCommission = iYdPosition*dRatioByVolume + dRatioByMoney*TradeData.dPrice*iYdPosition*iVolumeMultiple;
		//������=ƽ���������
		dFrozenCommission = dYdFrozenCommission;

	}

	return dFrozenCommission;
}
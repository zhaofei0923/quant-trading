#include <string.h>
#include "helper_time.h" 
#include "print.h" 
#include "runtime_manager_ctp.h"
#include "sim_log.h"
#include "trade_data.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

RuntimeManagerCtp::RuntimeManagerCtp(string &szUserId) : m_szUserId(szUserId)
{
	LInfo("创建RuntimeManager,  m_szUserId=[{0}]", m_szUserId);
}

RuntimeManagerCtp::~RuntimeManagerCtp()
{
}
//初始化
void RuntimeManagerCtp::Init()
{
	m_OrdersMap.clear();
	m_TradesMap.clear();
	m_PositionsMap.clear();
	m_ClientOrderIdOrderRefMap.clear();
	m_OrderTradeMap.clear();
	m_StockCodeTradeMap.clear();

	//获取当天日期
	int iNowDate = zutil::GetToday();



	//从数据表加载数据
	//加载资金
	LDebug("从数据表加载资金,m_szUserId=[{0}]", m_szUserId);
	TradeData::GetBalance(m_szUserId, m_Balance);
	//打印资金
	PrintData::PrintBalance(m_Balance);


	//加载当天委托
	LDebug("从数据表加载当天委托,m_szUserId=[{0}]", m_szUserId);
	vector<shared_ptr<OrderT>> OrderDatas;
	TradeData::GetOrders(iNowDate, iNowDate, m_szUserId, OrderDatas);
	for (int i = 0; i < OrderDatas.size(); i++)
	{
		shared_ptr<OrderT> pOrder = OrderDatas[i];
		PrintData::PrintOrder(*pOrder);
		m_ClientOrderIdOrderRefMap[pOrder->szClientOrderId] = pOrder->szOrderRef;
		m_OrdersMap[pOrder->szOrderRef] = pOrder;
	}

	//加载当天成交
	LDebug("从数据表加载当天成交,m_szUserId=[{0}]", m_szUserId);
	vector<shared_ptr<TradeT>> TradeDatas;
	TradeData::GetTrades(iNowDate, iNowDate, m_szUserId, TradeDatas);
	for (int i = 0; i < TradeDatas.size(); i++)
	{
		shared_ptr<TradeT> pTrade = TradeDatas[i];
		PrintData::PrintTrade(*pTrade);
		m_TradesMap[pTrade->szTradeID] = pTrade;

		//一个委托有多个成交
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

		//记录TradeId到m_StockCodeTradeMap
		char cDirection = POSITION_DIRECTION_NET;
		if (ORDER_ACTION_BUY_OPEN == pTrade->cAction || ORDER_ACTION_SELL_CLOSE == pTrade->cAction)
		{
			cDirection = POSITION_DIRECTION_LONG;
			LDebug("多头  即买开仓，卖平仓");
		}
		else if (ORDER_ACTION_SELL_OPEN == pTrade->cAction || ORDER_ACTION_BUY_CLOSE == pTrade->cAction)
		{
			cDirection = POSITION_DIRECTION_SHORT;
			LDebug("空头  即卖开仓，买平仓");
		}
		string szSecurityDirectionKey = pTrade->szExchangeID + '_' + pTrade->szStockCode + '_' + cDirection;
		LDebug("持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

		if (ORDER_ACTION_BUY_OPEN == pTrade->cAction || ORDER_ACTION_SELL_OPEN == pTrade->cAction)
		{
			LDebug("开仓的数据,szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

			//开仓的数据
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


	//加载当天手续费开仓数量
	LDebug("从数据表加载当天手续费开仓数量,m_szUserId=[{0}]", m_szUserId);
	vector<shared_ptr<OpenCountT>> CommissionOpenCountDatas;
	TradeData::GetCommissionOpenCount(iNowDate, m_szUserId, CommissionOpenCountDatas);
	for (int i = 0; i < CommissionOpenCountDatas.size(); i++)
	{
		shared_ptr<OpenCountT> pOpenCountT = CommissionOpenCountDatas[i];
		string szSecurityDirectionKey = pOpenCountT->szExchangeID + '_' + pOpenCountT->szStockCode + '_' + pOpenCountT->cDirection;
		m_CommissionOpenCountMap[szSecurityDirectionKey] = pOpenCountT;
	}

}
//结算持仓
void RuntimeManagerCtp::SettlePosition(MarketDataInfo&MarketDatas, InstrumentInfo&Instruments, InstrumentMarginRateInfo&MarginRates)
{
	//系统在启动时，会对上一个交易日进行结算

	//获取上次结算日期
	BalanceT BalanceData;
	TradeData::GetBalance(m_szUserId, BalanceData);
	if ("" == BalanceData.szUserId)
	{
		LError("数据表td_balance没有创建m_szUserId=[{0}]记录", m_szUserId);
		return;
	}
	int iLastSettleDate = BalanceData.iSettleDate;
	LInfo("上次结算日期iLastSettleDate=[{0}]", iLastSettleDate);

	//获取上次结算日期中的开仓成交数据
	vector<shared_ptr<TradeT>> LongTradeDatas; //多头
	TradeData::GetTrades(iLastSettleDate, m_szUserId, ORDER_ACTION_BUY_OPEN, LongTradeDatas);
	vector<shared_ptr<TradeT>> ShortTradeDatas;//空头
	TradeData::GetTrades(iLastSettleDate, m_szUserId, ORDER_ACTION_SELL_OPEN, ShortTradeDatas);

	//获取持仓数据
	vector<shared_ptr<PositionT>> PositionDatas;
	TradeData::GetPositions(m_szUserId, PositionDatas);

	//判断是否已经结算持仓盈亏和昨日保证金
	if (zutil::GetToday() == iLastSettleDate)
	{
		LInfo("m_szUserId=[{0}]已经结算持仓盈亏和昨日保证金,直接加载持仓数据", m_szUserId);

		//已经结算持仓盈亏和昨日保证金,直接加载持仓数据
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
		LInfo("m_szUserId=[{0}]没有结算持仓盈亏和昨日保证金，开始结算日期iLastSettleDate=[{1}]数据", m_szUserId, iLastSettleDate);

		/*
		当天未平仓的开仓数量等于当天持仓数量
		当天未平仓的开仓金额等于当天持仓开仓金额
		*/
		map<string, double> LongOpenAmount;//合约,金额   多头当天未平仓的开仓金额的总和,即多头当天持仓开仓金额
		map<string, double> ShortOpenAmount;//合约,金额  空头当天未平仓的开仓金额的总和,即空头当天持仓开仓金额


		//计算多头当天未平仓的开仓金额的总和,即多头当天持仓开仓金额
		LInfo("计算多头当天未平仓的开仓金额的总和,即多头当天持仓开仓金额");
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
				//合约乘数
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				//当天未平仓的开仓数量iCount=成交数量-当天平仓数量
				int iCount = LongTradeDatas[i]->iTradeNum - LongTradeDatas[i]->iCloseNum;

				//计算此合约的当天未平仓的开仓金额的总和
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

		//计算空头当天未平仓的开仓金额的总和,即空头当天持仓开仓金额
		LInfo("计算空头当天未平仓的开仓金额的总和,即空头当天持仓开仓金额");
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
				//合约乘数
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				//当天未平仓的开仓数量iCount=成交数量-当天平仓数量
				int iCount = ShortTradeDatas[i]->iTradeNum - ShortTradeDatas[i]->iCloseNum;

				//计算此合约的当天未平仓的开仓金额的总和
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
		计算盈亏方法：
		单个合约为例：
		当日盈亏＝平仓盈亏＋持仓盈亏
		平仓盈亏＝平历史仓盈亏＋平当日仓盈亏
		平历史仓盈亏＝(卖出平仓价－上一交易日结算价)×卖出平仓量×合约乘数＋(上一交易日结算价－买入平仓价)×买入平仓量×合约乘数
		平当日仓盈亏＝(当日卖出平仓价－当日买入开仓价)×卖出平仓量×合约乘数＋(当日卖出开仓价－当日买入平仓价)×买入平仓量×合约乘数
		持仓盈亏＝历史持仓盈亏＋当日开仓持仓盈亏
		历史持仓盈亏＝(上一日结算价－当日结算价)×卖出历史持仓量×合约乘数＋(当日结算价－上一日结算价)×买入历史持仓量×合约乘数
		当日开仓持仓盈亏＝(卖出开仓价－当日结算价)×卖出开仓量×合约乘数＋(当日结算价－买入开仓价)×买入开仓量×合约乘数

		*/

		//计算持仓盈亏
		LInfo("计算持仓盈亏");

		//历史持仓盈亏
		double dHisPositionProfit = 0;
		//当日开仓持仓盈亏。
		//由于是对上一日进行结算，即对结算日期iLastSettleDate进行结算，所以这里的当日是指上一日，即iLastSettleDate
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
				//昨日结算价，即iLastSettleDate结算价
				double dPreSettlementPrice = MarketDatas[szKey].iPreSettlePrice / PRICE_MULTIPLE;
				//昨日结算价的上一交易日结算价,即iLastSettleDate的上一日的结算价
				double dPrePreSettlementPrice = PositionDatas[i]->dSettlementPrice;
				//合约乘数
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				LDebug("dPrePreSettlementPrice=[{0}],dPreSettlementPrice=[{1}],iVolumeMultiple=[{2}]", dPrePreSettlementPrice, dPreSettlementPrice, iVolumeMultiple);

				if (POSITION_DIRECTION_LONG == PositionDatas[i]->cDirection)//多头  即买开仓，卖平仓
				{
					//计算历史持仓盈亏
					//多头历史持仓盈亏:(当日结算价－上一日结算价)×买入历史持仓量×合约乘数
					double dHisValue = (dPreSettlementPrice - dPrePreSettlementPrice)*iYdPosition*iVolumeMultiple;
					dHisPositionProfit = dHisPositionProfit + dHisValue;
					LDebug("dHisValue=[{0}],dHisPositionProfit=[{1}]", dHisValue, dHisPositionProfit);

					//计算当日开仓持仓盈亏  
					//多头当日开仓持仓盈亏:(当日结算价－买入开仓价)×买入开仓量×合约乘数
					double dTodayValue = iTodayPosition*dPreSettlementPrice*iVolumeMultiple - LongOpenAmount[szKey];
					dTodayPositionProfit = dTodayPositionProfit + dTodayValue;
					LDebug("LongOpenAmount[szKey]=[{0}],dTodayValue=[{1}],dTodayPositionProfit=[{2}]", LongOpenAmount[szKey], dTodayValue, dTodayPositionProfit);


				}
				else if (POSITION_DIRECTION_SHORT == PositionDatas[i]->cDirection)//空头  即卖开仓，买平仓
				{
					//计算历史持仓盈亏
					//空头历史持仓盈亏:(上一日结算价－当日结算价)×卖出历史持仓量×合约乘数
					double dHisValue = (dPrePreSettlementPrice - dPreSettlementPrice)*iYdPosition*iVolumeMultiple;
					dHisPositionProfit = dHisPositionProfit + dHisValue;
					LDebug("dHisValue=[{0}],dHisPositionProfit=[{1}]", dHisValue, dHisPositionProfit);

					//计算当日开仓持仓盈亏
					//空头当日开仓持仓盈亏:(卖出开仓价－当日结算价)×卖出开仓量×合约乘数
					double dTodayValue = ShortOpenAmount[szKey] - iTodayPosition*dPreSettlementPrice*iVolumeMultiple;
					dTodayPositionProfit = dTodayPositionProfit + dTodayValue;
					LDebug("ShortOpenAmount[szKey]=[{0}],dTodayValue=[{1}],dTodayPositionProfit=[{2}]", ShortOpenAmount[szKey], dTodayValue, dTodayPositionProfit);

				}


			}
		}

		//更新盈亏到Balance
		m_Balance.dAddMoney = m_Balance.dAddMoney + dHisPositionProfit + dTodayPositionProfit;
		LDebug("dHisPositionProfit=[{0}],dTodayPositionProfit=[{1}],dTotal=[{2}]", dHisPositionProfit, dTodayPositionProfit, dHisPositionProfit + dTodayPositionProfit);


		//计算持仓保证金
		//根据昨日结算价，即iLastSettleDate结算价计算占用的保证金
		LInfo("计算持仓保证金");
		for (int i = 0; i < PositionDatas.size(); i++)
		{
			shared_ptr<PositionT> pPosition = PositionDatas[i];
			double	dUseMargin = pPosition->dUseMargin; //占用的保证金
			//总持仓数量=昨仓数量+今仓数量
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
				//先从Balance中减去占用的保证金
				m_Balance.dUseMargin = m_Balance.dUseMargin - dUseMargin;

				//获取保证金费率
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

				//合约乘数
				int iVolumeMultiple = Instruments[szKey]->iVolumeMultiple;
				//昨日结算价,即iLastSettleDate结算价
				double dPreSettlementPrice = MarketDatas[szKey].iPreSettlePrice / PRICE_MULTIPLE;
				LDebug("iVolumeMultiple=[{0}],dPreSettlementPrice=[{1}]", iVolumeMultiple, dPreSettlementPrice);

				//根据昨日结算价格计算的保证金
				double	dNewUseMargin = iPosition*dMarginRatioByVolume + dMarginRatioByMoney*dPreSettlementPrice*iPosition*iVolumeMultiple;
				pPosition->dUseMargin = dNewUseMargin;
				LDebug("OldUseMargin=[{0}],NewUseMargin=[{1}]", dUseMargin, dNewUseMargin);

				//把原结算价更新到上次结算价
				pPosition->dPreSettlementPrice = pPosition->dSettlementPrice;
				//把昨日结算价,即iLastSettleDate结算价更新到本次结算价
				pPosition->dSettlementPrice = dPreSettlementPrice;
				LDebug("dPreSettlementPrice=[{0}],dSettlementPrice=[{1}]", pPosition->dPreSettlementPrice, pPosition->dSettlementPrice);

				//更新新的保证金到Balance
				m_Balance.dUseMargin = m_Balance.dUseMargin + dNewUseMargin;
			}

			//计算持仓变化
			//昨日持仓数量=昨日持仓数量+今日持仓数量
			pPosition->iYdPosition = pPosition->iYdPosition + pPosition->iTodayPosition;
			//今日持仓数量置为0	
			pPosition->iTodayPosition = 0;
			pPosition->iPosition = pPosition->iYdPosition; ///总持仓	

			string szSecurityDirectionKey = pPosition->szExchangeID + '_' + pPosition->szStockCode + '_' + pPosition->cDirection;
			m_PositionsMap[szSecurityDirectionKey] = pPosition;

			//更新数据库
			TradeData::UpdatePosition(*pPosition);
		}
	}


	//更新m_Balance
	//当前保证金总额=占用的保证金+冻结的保证金
	m_Balance.dCurrMargin = m_Balance.dUseMargin + m_Balance.dFrozenMargin;
	//使用的总资金=占用的保证金+冻结的保证金+冻结的手续费+手续费
	double dTotalUseCash = m_Balance.dUseMargin + m_Balance.dFrozenMargin + m_Balance.dFrozenCommission + m_Balance.dCommission;
	//总资金=期初的资金+调仓增加的资金
	double dTotalCash = m_Balance.dStartMoney + m_Balance.dAddMoney;
	m_Balance.dAvailable = dTotalCash - dTotalUseCash; ///可用资金
	m_Balance.iSettleDate = zutil::GetToday();
	m_Balance.iModifyDate = zutil::GetToday();
	m_Balance.iModifyTime = zutil::GetNowSedondTime();

	//更新数据库
	TradeData::UpdateBalance(m_Balance);

}

//更新order费率信息
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
			//有的不收取报单手续费
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
			//多头
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dLongMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dLongMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dOpenRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dOpenRatioByVolume;
		}
		else if (ORDER_ACTION_SELL_CLOSE == pOrder->cAction)
		{
			//多头
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dShortMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dShortMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dCloseRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dCloseRatioByVolume;
			pOrder->dCloseTodayRatioByMoney = CommissionRates[szKey]->dCloseTodayRatioByMoney;
			pOrder->dCloseTodayRatioByVolume = CommissionRates[szKey]->dCloseTodayRatioByVolume;

		}
		else if (ORDER_ACTION_SELL_OPEN == pOrder->cAction)
		{
			//空头
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dShortMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dShortMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dOpenRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dOpenRatioByVolume;

		}
		else if (ORDER_ACTION_BUY_CLOSE == pOrder->cAction)
		{
			//空头
			pOrder->dMarginRatioByMoney = MarginRates[szKey]->dLongMarginRatioByMoney;
			pOrder->dMarginRatioByVolume = MarginRates[szKey]->dLongMarginRatioByVolume;
			pOrder->dRatioByMoney = CommissionRates[szKey]->dCloseRatioByMoney;
			pOrder->dRatioByVolume = CommissionRates[szKey]->dCloseRatioByVolume;
			pOrder->dCloseTodayRatioByMoney = CommissionRates[szKey]->dCloseTodayRatioByMoney;
			pOrder->dCloseTodayRatioByVolume = CommissionRates[szKey]->dCloseTodayRatioByVolume;

		}

		LDebug("---------------更新order费率信息--------------------");

		//打印委托
		PrintData::PrintOrder(*pOrder);
	}

}
//更新资金
void RuntimeManagerCtp::UpdateBalance(BalanceT &BalanceData)
{
	lock_guard<mutex> lk(m_BalanceMtx);

	LDebug("RuntimeManagerCtp  UpdateBalance,  m_szUserId=[{0}]", m_szUserId);

	//打印资金
	PrintData::PrintBalance(BalanceData);

	LDebug("---------before update------------ m_szUserId=[{0}]", m_szUserId);
	PrintData::PrintBalance(m_Balance);

	m_Balance.dUseMargin = m_Balance.dUseMargin + BalanceData.dUseMargin;///占用的保证金	
	m_Balance.dFrozenMargin = m_Balance.dFrozenMargin + BalanceData.dFrozenMargin; ///冻结的保证金	
	m_Balance.dFrozenCash = m_Balance.dFrozenCash + BalanceData.dFrozenCash; ///冻结的资金
	m_Balance.dFrozenCommission = m_Balance.dFrozenCommission + BalanceData.dFrozenCommission; ///冻结的手续费	
	//当前保证金总额=占用的保证金+冻结的保证金
	m_Balance.dCurrMargin = m_Balance.dUseMargin + m_Balance.dFrozenMargin; ///当前保证金总额	
	m_Balance.dCommission = m_Balance.dCommission + BalanceData.dCommission; ///手续费	
	m_Balance.dAddMoney = m_Balance.dAddMoney + BalanceData.dAddMoney;//调仓增加的资金或者平仓盈亏

	//使用的总资金=占用的保证金+冻结的保证金+冻结的手续费+手续费
	double dTotalUseCash = m_Balance.dUseMargin + m_Balance.dFrozenMargin + m_Balance.dFrozenCommission + m_Balance.dCommission;
	//总资金=期初的资金+调仓增加的资金
	double dTotalCash = m_Balance.dStartMoney + m_Balance.dAddMoney;
	m_Balance.dAvailable = dTotalCash - dTotalUseCash; ///可用资金

	m_Balance.iModifyDate = zutil::GetToday(); //修改日期
	m_Balance.iModifyTime = zutil::GetNowSedondTime(); //修改时间


	LDebug("---------after update------------ m_szUserId=[{0}]", m_szUserId);
	PrintData::PrintBalance(m_Balance);

	BalanceT OldBalanceData;
	TradeData::GetBalance(m_szUserId, OldBalanceData);
	if ("" == OldBalanceData.szUserId)
	{
		LError("数据表td_balance没有创建m_szUserId=[{0}]记录", m_szUserId);
	}
	else
	{
		TradeData::UpdateBalance(m_Balance);
	}
}
//更新委托
void RuntimeManagerCtp::UpdateOrder(OrderT &OrderData)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  UpdateOrder,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, OrderData.szOrderRef);

	//打印委托
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
			LDebug("m_ClientOrderIdOrderRefMap创建对应关系,szClientOrderId=[{0}],szOrderRef=[{1}]", OrderData.szClientOrderId, OrderData.szOrderRef);
		}
	}

	if (m_OrdersMap.find(OrderData.szOrderRef) == m_OrdersMap.end())
	{
		m_OrdersMap[OrderData.szOrderRef] = make_shared<OrderT>(OrderData);
		LDebug("m_OrdersMap创建对应关系,szOrderRef=[{0}]", OrderData.szOrderRef);

		//更新数据库
		TradeData::InsertOrder(OrderData);


	}
	else
	{
		shared_ptr<OrderT> pOrder = m_OrdersMap[OrderData.szOrderRef];

		char cLastStatus = pOrder->cStatus;
		char cNowStatus = OrderData.cStatus;

		//字符是数字，使用字符比较
		if (cNowStatus >= cLastStatus)
		{
			if (cNowStatus == ORDER_STATUS_PART_TRADE || cNowStatus == ORDER_STATUS_ALL_TRADE)
			{
				//成交状态和成交数量由成交回报更新
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

			//过滤相同状态，更新持仓
			if (cNowStatus > cLastStatus)
			{
				UpdatePositionByOrder(*pOrder);
			}


		}

		//更新数据库
		TradeData::UpdateOrder(*pOrder);


		LDebug("---------after update----------------------");
		//打印委托
		PrintData::PrintOrder(*pOrder);

	}

}
//更新交易
void RuntimeManagerCtp::UpdateTrade(TradeT &TradeData)
{
	lock_guard<mutex> lk(m_TradeMtx);

	LDebug("RuntimeManagerCtp  UpdateTrade,  m_szUserId=[{0}]", m_szUserId);

	//打印交易
	PrintData::PrintTrade(TradeData);

	shared_ptr<TradeT> pTrade = make_shared<TradeT>(TradeData);
	m_TradesMap[TradeData.szTradeID] = pTrade;

	//一个委托有多个成交
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


	//记录TradeId到m_StockCodeTradeMap
	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		//存储开仓数据
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
//更新委托
void RuntimeManagerCtp::UpdateOrder(TradeT &TradeData)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("UpdateOrder  TradeData.szOrderRef=[{0}]", TradeData.szOrderRef);

	if (0 == m_OrdersMap[TradeData.szOrderRef])
	{
		LError("old Order is not exist  szOrderRef=[{0}]", TradeData.szOrderRef);
		return;
	}

	//委托回报状态ORDER_STATUS_SUBMIT,ORDER_STATUS_INSERT_REFUSE,ORDER_STATUS_CANCEL可以保证连续，方便计算持仓
	//成交回报中的成交数量的连续增加，可以保证连续，方便计算持仓
	//根据成交回报更新委托的成交数量
	shared_ptr<OrderT> pOrder = m_OrdersMap[TradeData.szOrderRef];
	pOrder->iTradeNum = pOrder->iTradeNum + TradeData.iTradeNum;
	if (pOrder->iTradeNum == pOrder->iEntrustNum)
	{
		//全部成交
		pOrder->cStatus = ORDER_STATUS_ALL_TRADE;
	}
	else
	{
		//部分成交
		pOrder->cStatus = ORDER_STATUS_PART_TRADE;
	}

	TradeData::UpdateOrder(*pOrder);
	UpdatePositionByOrder(*pOrder);

}
//更新平仓盈亏
void RuntimeManagerCtp::UpdateCloseProfit(TradeT &TradeData)
{
	LDebug("UpdateCloseProfit  TradeData.szOrderRef=[{0}]", TradeData.szOrderRef);

	//只有平仓才计算盈亏
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		return;
	}

	double  dPreSettlementPrice = 0; //昨结算价
	int iVolumeMultiple = 0;//合约数量乘数
	//获取order信息
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
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);


	int iCloseYdNum = 0; //平昨仓数量
	int iCloseTodayNum = 0;//平今仓数量

	//根据持仓数据进行计算
	{
		lock_guard<mutex> lk(m_PositionMtx);

		//根据查询的原持仓数量进行计算
		if (0 == m_PositionsMap.count(szSecurityDirectionKey))
		{
			LError("持仓数量为0,szOrderRef=[{0}]  ", TradeData.szOrderRef);
			return;
		}
		else
		{
			shared_ptr<PositionT> pPosition = m_PositionsMap[szSecurityDirectionKey];
			//iValue=昨仓数量-成交数量
			int iValue = pPosition->iYdPosition - TradeData.iTradeNum;
			if (iValue >= 0)  //平昨仓
			{
				iCloseTodayNum = 0;
				iCloseYdNum = TradeData.iTradeNum;

			}
			else  //先平昨仓,再平今仓
			{
				iCloseTodayNum = 0 - iValue;
				iCloseYdNum = pPosition->iYdPosition;

			}

			LDebug("iCloseYdNum=[{0}],iCloseTodayNum=[{1}]", iCloseYdNum, iCloseTodayNum);
		}

	}


	/*
	计算盈亏方法：
	单个合约为例：
	当日盈亏＝平仓盈亏＋持仓盈亏
	平仓盈亏＝平历史仓盈亏＋平当日仓盈亏
	平历史仓盈亏＝(卖出平仓价－上一交易日结算价)×卖出平仓量×合约乘数＋(上一交易日结算价－买入平仓价)×买入平仓量×合约乘数
	平当日仓盈亏＝(当日卖出平仓价－当日买入开仓价)×卖出平仓量×合约乘数＋(当日卖出开仓价－当日买入平仓价)×买入平仓量×合约乘数
	持仓盈亏＝历史持仓盈亏＋当日开仓持仓盈亏
	历史持仓盈亏＝(上一日结算价－当日结算价)×卖出历史持仓量×合约乘数＋(当日结算价－上一日结算价)×买入历史持仓量×合约乘数
	当日开仓持仓盈亏＝(卖出开仓价－当日结算价)×卖出开仓量×合约乘数＋(当日结算价－买入开仓价)×买入开仓量×合约乘数

	*/

	/*
	平当日仓盈亏的计算难点
	举例：
	买开仓：
	分别以100元买入1手合约A，以110元买入1手合约A，以120元买入1手合约A
	卖平仓：
	以105元卖出2手合约A
	问题：平仓时是平开仓的哪2手？

	解决方法：
	采用先开仓，先平仓的规则。即平仓是平100元买入的1手和110元买入的1手。
	平仓盈亏为平仓总金额减去开仓总金额，即: 平仓盈亏=105*2-(100*1+110*1)
	*/

	double dCloseHisProfit = 0; //平历史仓盈亏
	double dCloseTodayProfit = 0; //平当日仓盈亏
	double dCloseTodayAmount = iCloseTodayNum*TradeData.dPrice*iVolumeMultiple; //平当日仓总金额

	double dOpenTodayAmount = 0; //开仓总金额
	//获取szSecurityDirectionKey的所有TradeId
	vector<string>& vTradeIds = m_StockCodeTradeMap[szSecurityDirectionKey];

	//循环遍历所有TradeId,计算开仓总金额
	for (int i = 0; i < vTradeIds.size(); i++)
	{
		if (iCloseTodayNum <= 0)
		{
			//平今仓完成
			break;
		}

		//根据TradeId查找成交信息TradeT
		string szTradeId = vTradeIds[i];
		shared_ptr<TradeT> pTrade = m_TradesMap[szTradeId];

		//此笔成交的可平仓数量=成交数量-当天平仓数量
		int iAvailable = pTrade->iTradeNum - pTrade->iCloseNum;

		//此笔成交的可平仓数量大于0，可以平仓
		if (iAvailable > 0)
		{
			//iValue=平今仓数量-此笔成交的可平仓数量
			int iValue = iCloseTodayNum - iAvailable;
			if (iValue <= 0) //把iCloseTodayNum全部平仓
			{
				//计算开仓总金额
				dOpenTodayAmount = dOpenTodayAmount + iCloseTodayNum*pTrade->dPrice*iVolumeMultiple;
				LDebug("iCloseTodayNum=[{0}],dPrice=[{1}],iVolumeMultiple=[{2}],dOpenTodayAmount=[{3}]", iCloseTodayNum, pTrade->dPrice, iVolumeMultiple, dOpenTodayAmount);

				//更新此成交的当天平仓数量
				pTrade->iCloseNum = pTrade->iCloseNum + iCloseTodayNum;

				//更新平今仓数量
				iCloseTodayNum = iCloseTodayNum - iCloseTodayNum;
			}
			else //平iCloseTodayNum中一部分
			{
				//计算开仓总金额		
				dOpenTodayAmount = dOpenTodayAmount + iAvailable*pTrade->dPrice*iVolumeMultiple;
				LDebug("iCloseTodayNum=[{0}],dPrice=[{1}],iVolumeMultiple=[{2}],dOpenTodayAmount=[{3}]", iCloseTodayNum, pTrade->dPrice, iVolumeMultiple, dOpenTodayAmount);

				//更新此成交的当天平仓数量
				pTrade->iCloseNum = pTrade->iCloseNum + iAvailable;

				//更新平今仓数量
				iCloseTodayNum = iCloseTodayNum - iAvailable;
			}

			//更新数据库
			TradeData::UpdateTrade(*pTrade);
		}
	}

	if (POSITION_DIRECTION_LONG == cDirection)
	{
		//平历史仓盈亏
		//多头平历史仓盈亏：(卖出平仓价－上一交易日结算价)×卖出平仓量×合约乘数
		dCloseHisProfit = iCloseYdNum*TradeData.dPrice*iVolumeMultiple - iCloseYdNum*dPreSettlementPrice*iVolumeMultiple;
		LDebug("LONG:iCloseYdNum=[{0}],dPrice=[{1}],dPreSettlementPrice=[{2}],iVolumeMultiple=[{3}],dCloseHisProfit=[{4}]", iCloseYdNum, TradeData.dPrice, dPreSettlementPrice, iVolumeMultiple, dCloseHisProfit);

		//平当日仓盈亏
		//多头平当日仓盈亏：(当日卖出平仓价－当日买入开仓价)×卖出平仓量×合约乘数
		dCloseTodayProfit = dCloseTodayAmount - dOpenTodayAmount;
		LDebug("LONG:dCloseTodayAmount=[{0}],dOpenTodayAmount=[{1}],dCloseTodayProfit=[{2}]", dCloseTodayAmount, dOpenTodayAmount, dCloseTodayProfit);

	}
	else if (POSITION_DIRECTION_SHORT == cDirection)
	{
		//平历史仓盈亏
		//空头平历史仓盈亏：(上一交易日结算价－买入平仓价)×买入平仓量×合约乘数
		dCloseHisProfit = iCloseYdNum*dPreSettlementPrice*iVolumeMultiple - iCloseYdNum*TradeData.dPrice*iVolumeMultiple;
		LDebug("SHORT:iCloseYdNum=[{0}],dPrice=[{1}],dPreSettlementPrice=[{2}],iVolumeMultiple=[{3}],dCloseHisProfit=[{4}]", iCloseYdNum, TradeData.dPrice, dPreSettlementPrice, iVolumeMultiple, dCloseHisProfit);

		//平当日仓盈亏
		//空头平当日仓盈亏：(当日卖出开仓价－当日买入平仓价)×买入平仓量×合约乘数
		dCloseTodayProfit = dOpenTodayAmount - dCloseTodayAmount;
		LDebug("SHORT:dCloseTodayAmount=[{0}],dOpenTodayAmount=[{1}],dCloseTodayProfit=[{2}]", dCloseTodayAmount, dOpenTodayAmount, dCloseTodayProfit);

	}

	//平仓盈亏＝平历史仓盈亏＋平当日仓盈亏
	double dTotalProfit = dCloseTodayProfit + dCloseHisProfit;
	LDebug("dCloseTodayProfit=[{0}],dCloseHisProfit=[{1}],dTotalProfit=[{2}]", dCloseTodayProfit, dCloseHisProfit, dTotalProfit);

	//更新Balance
	BalanceT  BalanceData;
	BalanceData.szUserId = m_szUserId;
	BalanceData.dAddMoney = dTotalProfit;
	BalanceData.iModifyDate = zutil::GetToday();
	BalanceData.iModifyTime = zutil::GetNowSedondTime();

	UpdateBalance(BalanceData);

}
//根据委托更新持仓
bool RuntimeManagerCtp::UpdatePositionByOrder(OrderT& OrderData)
{
	LDebug("UpdatePositionByOrder,szOrderRef=[{0}]  ", OrderData.szOrderRef);

	PositionT PositionData;
	char cNowStatus = OrderData.cStatus;

	if (ORDER_STATUS_SUBMIT == cNowStatus)
	{
		//当状态为已提交时，创建持仓,计算保证金和手续费。
		//持仓操作类型为冻结。PositionData.cOperType = POSITION_OPER_FROZEN;

		char cDirection = POSITION_DIRECTION_NET;
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_LONG;
			LDebug("多头  即买开仓，卖平仓");
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_SHORT;
			LDebug("空头  即卖开仓，买平仓");
		}

		string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;
		LDebug("UpdatePositionByOrder创建持仓，持仓方向szSecurityDirectionKey=[{0}]  状态为已提交,持仓操作类型为冻结", szSecurityDirectionKey);

		PositionData.iInitDate = zutil::GetToday();
		PositionData.iInitTime = zutil::GetNowSedondTime();
		PositionData.iModifyDate = zutil::GetToday();
		PositionData.iModifyTime = zutil::GetNowSedondTime();
		PositionData.szUserId = OrderData.szUserId;
		PositionData.szExchangeID = OrderData.szExchangeID;
		PositionData.szStockCode = OrderData.szStockCode;
		PositionData.cDirection = cDirection;
		PositionData.cHedgeFlag = HF_Speculation;
		PositionData.cOperType = POSITION_OPER_FROZEN;//冻结
		PositionData.cAction = OrderData.cAction;
		PositionData.dSettlementPrice = OrderData.dPreSettlementPrice;
		PositionData.dMarginRateByMoney = OrderData.dMarginRatioByMoney;
		PositionData.dMarginRateByVolume = OrderData.dMarginRatioByVolume;

		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			//买
			//冻结的数量为委托数量
			//冻结的金额为委托数量*委托价格*合约乘数
			PositionData.iLongFrozen = OrderData.iEntrustNum;
			PositionData.dLongFrozenAmount = OrderData.iEntrustNum*OrderData.dPrice*OrderData.iVolumeMultiple;

			//把iLongFrozen和dLongFrozenAmount更新到order
			OrderData.iLongShortFrozen = PositionData.iLongFrozen;
			OrderData.dLongShortFrozenAmount = PositionData.dLongFrozenAmount;
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//卖
			//冻结的数量为委托数量
			//冻结的金额为委托数量*委托价格*合约乘数
			PositionData.iShortFrozen = OrderData.iEntrustNum;
			PositionData.dShortFrozenAmount = OrderData.iEntrustNum*OrderData.dPrice*OrderData.iVolumeMultiple;

			//把iShortFrozen和dShortFrozenAmount更新到order
			OrderData.iLongShortFrozen = PositionData.iShortFrozen;
			OrderData.dLongShortFrozenAmount = PositionData.dShortFrozenAmount;
		}

		//计算保证金和手续费
		//保证金 = 按手数保证金费*手数+按金额保证金率*价格*手数*合约乘数。
		//计算保证金使用的价格：昨仓用昨结算价，今仓可以使用昨结算价，最新价，成交均价，开仓价。simnow今仓使用昨结算价。
		//为了和ctp的仿真环境simnow保持一致，本代码中昨仓和今仓的保证金价格统一使用昨日结算价
		//计算客户的保证金，对于昨仓统一使用昨结算价计算，对于今仓，在期货公司后台是可以设置的。客户也可以通过ReqQryBrokerTradingParams（请求查询经纪公司交易参数）查询得到设置的类型。
		//设置的类型:昨结算价，最新价，成交均价，开仓价
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
		{
			//开仓,开仓是今仓

			//使用昨日结算价作为计算保证金的价格
			double dTodayPrice = OrderData.dPreSettlementPrice;
			//计算保证金
			PositionData.dFrozenMargin = OrderData.iEntrustNum*OrderData.dMarginRatioByVolume + OrderData.dMarginRatioByMoney*dTodayPrice*OrderData.iEntrustNum*OrderData.iVolumeMultiple;
			//计算手续费。手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
			PositionData.dFrozenCommission = OrderData.iEntrustNum*OrderData.dRatioByVolume + OrderData.dRatioByMoney*OrderData.dPrice*OrderData.iEntrustNum*OrderData.iVolumeMultiple;

			PositionData.iYdPosition = 0;
			PositionData.iTodayPosition = 0;
			PositionData.iPosition = 0;

			//更新持仓
			UpdatePosition(PositionData);

			//把开仓冻结的保证金和手续费更新到order
			OrderData.dFrozenMargin = PositionData.dFrozenMargin;
			OrderData.dFrozenCommission = PositionData.dFrozenCommission;

		}
		else if (ORDER_ACTION_BUY_CLOSE == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//平仓,平仓有平昨仓和平今仓

			PositionData.dFrozenCommission = GetCloseCommission(OrderData);
			PositionData.dFrozenMargin = 0;
			PositionData.iYdPosition = 0;
			PositionData.iTodayPosition = 0;
			PositionData.iPosition = 0;

			//更新持仓
			UpdatePosition(PositionData);

			//把平仓冻结的手续费更新到order
			OrderData.dFrozenCommission = PositionData.dFrozenCommission;
		}
	}
	else if (ORDER_STATUS_INSERT_REFUSE == cNowStatus || ORDER_STATUS_CANCEL == cNowStatus || (ORDER_STATUS_ALL_TRADE == cNowStatus&&OrderData.iEntrustNum == OrderData.iTradeNum))
	{
		//交易最终状态：报单被拒绝,撤单,全部成交。此时order的剩余部分全部解冻。更新持仓
		//报单被拒绝:解冻冻结的部分
		//撤单:撤单时可能全部未成交，或者已经部分成交。把未成交的解冻
		//全部成交：把多冻结的部分进行解冻。如果使用昨日结算价计算保证金不会产生多冻结，如果使用其他价格计算保证金则可能产生多冻结
		//持仓操作类型为剩余部分解冻，即PositionData.cOperType = POSITION_OPER_REMAIN_UN_FROZEN;

		char cDirection = POSITION_DIRECTION_NET;
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_LONG;
			LDebug("多头  即买开仓，卖平仓");
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			cDirection = POSITION_DIRECTION_SHORT;
			LDebug("空头  即卖开仓，买平仓");
		}

		string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;
		LDebug("UpdatePositionByOrder创建持仓，持仓方向szSecurityDirectionKey=[{0}] 状态为最终状态，持仓操作类型为剩余部分解冻", szSecurityDirectionKey);

		PositionData.iInitDate = zutil::GetToday();
		PositionData.iInitTime = zutil::GetNowSedondTime();
		PositionData.iModifyDate = zutil::GetToday();
		PositionData.iModifyTime = zutil::GetNowSedondTime();
		PositionData.szUserId = OrderData.szUserId;
		PositionData.szExchangeID = OrderData.szExchangeID;
		PositionData.szStockCode = OrderData.szStockCode;
		PositionData.cDirection = cDirection;
		PositionData.cHedgeFlag = HF_Speculation;
		PositionData.cOperType = POSITION_OPER_REMAIN_UN_FROZEN;//剩余部分解冻
		PositionData.cAction = OrderData.cAction;
		PositionData.dSettlementPrice = OrderData.dPreSettlementPrice;
		PositionData.dMarginRateByMoney = OrderData.dMarginRatioByMoney;
		PositionData.dMarginRateByVolume = OrderData.dMarginRatioByVolume;

		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			//买
			//冻结的数量为order剩余的数量
			//冻结的金额为order剩余的金额
			PositionData.iLongFrozen = OrderData.iLongShortFrozen;
			PositionData.dLongFrozenAmount = OrderData.dLongShortFrozenAmount;
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			//卖
			//冻结的数量为order剩余的数量
			//冻结的金额为order剩余的金额
			PositionData.iShortFrozen = OrderData.iLongShortFrozen;
			PositionData.dShortFrozenAmount = OrderData.dLongShortFrozenAmount;

		}

		//冻结的保证金为order剩余的保证金
		//冻结的手续费为order剩余的手续费
		PositionData.dFrozenMargin = OrderData.dFrozenMargin;
		PositionData.dFrozenCommission = OrderData.dFrozenCommission;

		//更新持仓
		UpdatePosition(PositionData);

	}
	else
	{
		return false;
	}


	return true;
}
//根据交易更新持仓
bool RuntimeManagerCtp::UpdatePositionByTrade(TradeT &TradeData)
{
	//当有成交时，创建持仓，计算保证金和手续费
	//持仓操作类型为部分解冻，即PositionData.cOperType = POSITION_OPER_PART_UN_FROZEN;

	PositionT PositionData;
	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("UpdatePositionByTrade 创建持仓，持仓方向szSecurityDirectionKey=[{0}] 状态为成交，持仓操作类型为部分解冻", szSecurityDirectionKey);


	double dOrderPrice = 0;
	int iVolumeMultiple = 0;//合约数量乘数
	double	dMarginRatioByMoney = 0;///保证金率	
	double	dMarginRatioByVolume = 0;///保证金费	
	double	dRatioByMoney = 0;///开平仓合约手续费率	
	double	dRatioByVolume = 0;///开平仓合约手续费							   
	double	dCloseTodayRatioByMoney = 0;///平今手续费率	
	double	dCloseTodayRatioByVolume = 0;///平今手续费
	double  dPreSettlementPrice = 0; //昨结算价

	//获取order信息
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
	PositionData.cOperType = POSITION_OPER_PART_UN_FROZEN;//部分解冻
	PositionData.dSettlementPrice = dPreSettlementPrice;
	PositionData.dMarginRateByMoney = dMarginRatioByMoney;
	PositionData.dMarginRateByVolume = dMarginRatioByVolume;

	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		//买
		//冻结的数量为成交数量
		PositionData.iLongFrozen = TradeData.iTradeNum;
		//此处使用委托价格，因为冻结的时候使用的价格是委托价格
		//成交价格不一定和委托价格相等
		//冻结的金额为成交数量*委托价格*合约乘数
		PositionData.dLongFrozenAmount = TradeData.iTradeNum*dOrderPrice*iVolumeMultiple;
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		//卖
		//冻结的数量为成交数量
		PositionData.iShortFrozen = TradeData.iTradeNum;
		//此处使用委托价格，因为冻结的时候使用的价格是委托价格
		//成交价格不一定和委托价格相等
		//冻结的金额为成交数量*委托价格*合约乘数
		PositionData.dShortFrozenAmount = TradeData.iTradeNum*dOrderPrice*iVolumeMultiple;

	}

	//计算保证金和手续费
	//保证金 = 按手数保证金费*手数+按金额保证金率*价格*手数*合约乘数。
	//计算保证金使用的价格：昨仓用昨结算价，今仓可以使用昨结算价，最新价，成交均价，开仓价。simnow今仓使用昨结算价。
	//为了和ctp的仿真环境simnow保持一致，本代码中昨仓和今仓的保证金价格统一使用昨日结算价
	//计算客户的保证金，对于昨仓统一使用昨结算价计算，对于今仓，在期货公司后台是可以设置的。客户也可以通过ReqQryBrokerTradingParams（请求查询经纪公司交易参数）查询得到设置的类型。
	//设置的类型:昨结算价，最新价，成交均价，开仓价
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		//开仓,开仓是今仓

		//今仓计算保证金使用的价格。使用昨日结算价作为今仓计算保证金使用的价格
		double dTodayPrice = dPreSettlementPrice;
		//计算保证金
		PositionData.dFrozenMargin = TradeData.iTradeNum*dMarginRatioByVolume + dMarginRatioByMoney*dTodayPrice*TradeData.iTradeNum*iVolumeMultiple;
		//计算手续费,手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
		PositionData.dFrozenCommission = TradeData.iTradeNum*dRatioByVolume + dRatioByMoney*TradeData.dPrice*TradeData.iTradeNum*iVolumeMultiple;

		PositionData.iYdPosition = 0;
		PositionData.iTodayPosition = TradeData.iTradeNum;
		PositionData.iPosition = TradeData.iTradeNum;

		//更新持仓
		UpdatePosition(PositionData);
	}
	else if (ORDER_ACTION_BUY_CLOSE == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		lock_guard<mutex> lk(m_PositionMtx);

		//平仓,平仓有平昨仓和平今仓

		//今仓计算保证金使用的价格。使用昨日结算价作为今仓计算保证金使用的价格
		double dTodayPrice = dPreSettlementPrice;
		//昨仓计算保证金使用的价格。使用昨日结算价作为昨仓计算保证金使用的价格
		double dYdPrice = dPreSettlementPrice;

		//根据查询的原持仓数量进行计算
		if (0 == m_PositionsMap.count(szSecurityDirectionKey))
		{

			LError("持仓数量为0,szOrderRef=[{0}]  ", TradeData.szOrderRef);
			return false;
		}
		else
		{

			shared_ptr<PositionT> pPosition = m_PositionsMap[szSecurityDirectionKey];

			//计算保证金:如果不指定平今仓，则先平昨仓，再平今仓
			{
				int iYdPosition = pPosition->iYdPosition;
				//iValue=昨仓数量-成交数量
				int iValue = pPosition->iYdPosition - TradeData.iTradeNum;
				if (iValue >= 0)  //成交数量小于等于昨仓数量，平昨仓
				{
					//使用成交数量和昨日结算价计算保证金
					PositionData.dFrozenMargin = TradeData.iTradeNum*dMarginRatioByVolume + dMarginRatioByMoney*dYdPrice*TradeData.iTradeNum*iVolumeMultiple;

					PositionData.iYdPosition = TradeData.iTradeNum;
					PositionData.iTodayPosition = 0;
					PositionData.iPosition = TradeData.iTradeNum;
				}
				else  //成交数量大于昨仓数量，先平昨仓,再平今仓
				{
					//今仓数量
					int iTodayPosition = 0 - iValue;

					//先平昨仓
					//使用昨仓数量和昨结算价计算保证金
					PositionData.dFrozenMargin = iYdPosition*dMarginRatioByVolume + dMarginRatioByMoney*dYdPrice*iYdPosition*iVolumeMultiple;

					//再平今仓
					//使用今仓数量和昨结算价计算保证金
					PositionData.dFrozenMargin = PositionData.dFrozenMargin + iTodayPosition*dMarginRatioByVolume + dMarginRatioByMoney*dTodayPrice*iTodayPosition*iVolumeMultiple;

					PositionData.iYdPosition = iYdPosition;
					PositionData.iTodayPosition = iTodayPosition;
					PositionData.iPosition = TradeData.iTradeNum;
				}
			}

			//计算手续费
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

		//更新持仓
		UpdatePosition(PositionData, false);
	}


	//更新order，减去成交的部分
	{
		lock_guard<mutex> lk(m_OrderMtx);
		shared_ptr<OrderT> pOrder = m_OrdersMap[TradeData.szOrderRef];

		//减去iLongFrozen，dLongFrozenAmount，iShortFrozen，dShortFrozenAmount
		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
		{
			//买
			pOrder->iLongShortFrozen = pOrder->iLongShortFrozen - PositionData.iLongFrozen;
			pOrder->dLongShortFrozenAmount = pOrder->dLongShortFrozenAmount - PositionData.dLongFrozenAmount;

		}
		else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
		{
			//卖
			pOrder->iLongShortFrozen = pOrder->iLongShortFrozen - PositionData.iShortFrozen;
			pOrder->dLongShortFrozenAmount = pOrder->dLongShortFrozenAmount - PositionData.dShortFrozenAmount;


		}

		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
		{
			//开仓,减去保证金
			pOrder->dFrozenMargin = pOrder->dFrozenMargin - PositionData.dFrozenMargin;
		}

		//减去手续费
		pOrder->dFrozenCommission = pOrder->dFrozenCommission - PositionData.dFrozenCommission;
	}

	return true;
}
//更新持仓
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
//更新持仓
void RuntimeManagerCtp::UpdatePositionByUnLock(PositionT &PositionData)
{
	//打印持仓
	PrintData::PrintPosition(PositionData);

	BalanceT  BalanceData;
	BalanceData.szUserId = m_szUserId;
	string szSecurityDirectionKey = PositionData.szExchangeID + '_' + PositionData.szStockCode + '_' + PositionData.cDirection;

	if (m_PositionsMap.find(szSecurityDirectionKey) == m_PositionsMap.end())
	{
		LDebug("m_PositionsMap创建持仓，持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		shared_ptr<PositionT> pPosition = make_shared<PositionT>(PositionData);
		m_PositionsMap[szSecurityDirectionKey] = pPosition;

		//此处是委托创建的持仓	
		if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
		{
			//开仓	
			//计算Balance
			BalanceData.dFrozenMargin = PositionData.dFrozenMargin; ///冻结的保证金	
			BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///冻结的手续费	
		}
		else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
		{
			//平仓
			//计算Balance
			BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///冻结的手续费	
		}

		PositionT TmpPositionData;
		TradeData::GetPositions(m_szUserId, PositionData.szExchangeID, PositionData.szStockCode, PositionData.cDirection, TmpPositionData);
		if ("" == TmpPositionData.szUserId)
		{
			TradeData::InsertPosition(PositionData);
			LDebug("TradeData::InsertPosition(PositionData)，持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		}
		else
		{
			TradeData::UpdatePosition(PositionData);
			LDebug("TradeData::UpdatePosition(PositionData)，持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		}
	}
	else
	{
		LDebug("m_PositionsMap更新持仓，持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		shared_ptr<PositionT> pPosition = m_PositionsMap[szSecurityDirectionKey];

		LDebug("-----------打印更新前的持仓数据--------");
		PrintData::PrintPosition(*pPosition);


		if (POSITION_OPER_FROZEN == PositionData.cOperType)//冻结
		{
			//冻结手续费
			pPosition->dFrozenCommission = pPosition->dFrozenCommission + PositionData.dFrozenCommission;

			//开仓
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
			{
				//开仓冻结保证金
				pPosition->dFrozenMargin = pPosition->dFrozenMargin + PositionData.dFrozenMargin;

				//计算Balance
				BalanceData.dFrozenMargin = PositionData.dFrozenMargin; ///冻结的保证金	
				BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///冻结的手续费	
			}
			else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//平仓
				//计算Balance
				BalanceData.dFrozenCommission = PositionData.dFrozenCommission; ///冻结的手续费	
			}

			//计算多空冻结数量和金额
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_BUY_CLOSE == PositionData.cAction)
			{
				//买
				pPosition->iLongFrozen = pPosition->iLongFrozen + PositionData.iLongFrozen;
				pPosition->dLongFrozenAmount = pPosition->dLongFrozenAmount + PositionData.dLongFrozenAmount;

			}
			else if (ORDER_ACTION_SELL_OPEN == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//卖
				pPosition->iShortFrozen = pPosition->iShortFrozen + PositionData.iShortFrozen;
				pPosition->dShortFrozenAmount = pPosition->dShortFrozenAmount + PositionData.dShortFrozenAmount;
			}

			LDebug("增加冻结，持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);
		}
		else if (POSITION_OPER_PART_UN_FROZEN == PositionData.cOperType)//部分解冻
		{
			//把冻结的手续费移动到占用中
			pPosition->dFrozenCommission = pPosition->dFrozenCommission - PositionData.dFrozenCommission;
			pPosition->dCommission = pPosition->dCommission + PositionData.dFrozenCommission;

			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
			{
				//开仓
				//把冻结的保证金移动到占用中
				pPosition->dFrozenMargin = pPosition->dFrozenMargin - PositionData.dFrozenMargin;
				pPosition->dUseMargin = pPosition->dUseMargin + PositionData.dFrozenMargin;

				//增加持仓数量
				pPosition->iTodayPosition = pPosition->iTodayPosition + PositionData.iTodayPosition;
				pPosition->iPosition = pPosition->iPosition + PositionData.iPosition;

				//计算Balance
				BalanceData.dFrozenMargin = 0 - PositionData.dFrozenMargin; ///冻结的保证金	
				BalanceData.dUseMargin = PositionData.dFrozenMargin;///占用的保证金	
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///冻结的手续费					
				BalanceData.dCommission = PositionData.dFrozenCommission; ///手续费	
			}
			else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//平仓
				//把占用的保证金释放
				pPosition->dUseMargin = pPosition->dUseMargin - PositionData.dFrozenMargin;


				//减少持仓数量
				pPosition->iTodayPosition = pPosition->iTodayPosition - PositionData.iTodayPosition;
				pPosition->iYdPosition = pPosition->iYdPosition - PositionData.iYdPosition;
				pPosition->iPosition = pPosition->iPosition - PositionData.iPosition;

				//计算Balance
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///冻结的手续费
				BalanceData.dCommission = PositionData.dFrozenCommission; ///手续费	
				BalanceData.dUseMargin = 0 - PositionData.dFrozenMargin;///占用的保证金	

			}

			//计算多空冻结数量和金额
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_BUY_CLOSE == PositionData.cAction)
			{
				//买
				pPosition->iLongFrozen = pPosition->iLongFrozen - PositionData.iLongFrozen;
				pPosition->dLongFrozenAmount = pPosition->dLongFrozenAmount - PositionData.dLongFrozenAmount;
			}
			else if (ORDER_ACTION_SELL_OPEN == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//卖
				pPosition->iShortFrozen = pPosition->iShortFrozen - PositionData.iShortFrozen;
				pPosition->dShortFrozenAmount = pPosition->dShortFrozenAmount - PositionData.dShortFrozenAmount;
			}

			LDebug("部分解冻，持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

		}
		else if (POSITION_OPER_REMAIN_UN_FROZEN == PositionData.cOperType) //剩余部分解冻
		{
			//交易最终状态：报单被拒绝,撤单,全部成交。此时order的剩余部分全部解冻。更新持仓
			//报单被拒绝:解冻冻结的部分
			//撤单:撤单时可能全部未成交，或者已经部分成交。把未成交的解冻
			//全部成交：把多冻结的部分进行解冻。如果使用昨日结算价计算保证金不会产生多冻结，如果使用其他价格计算保证金则可能产生多冻结


			//清除多冻结的手续费
			pPosition->dFrozenCommission = pPosition->dFrozenCommission - PositionData.dFrozenCommission;

			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_SELL_OPEN == PositionData.cAction)
			{
				//开仓
				//清除多冻结的保证金
				pPosition->dFrozenMargin = pPosition->dFrozenMargin - PositionData.dFrozenMargin;

				//计算Balance
				BalanceData.dFrozenMargin = 0 - PositionData.dFrozenMargin; ///冻结的保证金	
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///冻结的手续费					
			}
			else if (ORDER_ACTION_BUY_CLOSE == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//平仓

				//计算Balance
				BalanceData.dFrozenCommission = 0 - PositionData.dFrozenCommission; ///冻结的手续费

			}

			//计算多空冻结数量和金额
			if (ORDER_ACTION_BUY_OPEN == PositionData.cAction || ORDER_ACTION_BUY_CLOSE == PositionData.cAction)
			{
				//买
				pPosition->iLongFrozen = pPosition->iLongFrozen - PositionData.iLongFrozen;
				pPosition->dLongFrozenAmount = pPosition->dLongFrozenAmount - PositionData.dLongFrozenAmount;
			}
			else if (ORDER_ACTION_SELL_OPEN == PositionData.cAction || ORDER_ACTION_SELL_CLOSE == PositionData.cAction)
			{
				//卖
				pPosition->iShortFrozen = pPosition->iShortFrozen - PositionData.iShortFrozen;
				pPosition->dShortFrozenAmount = pPosition->dShortFrozenAmount - PositionData.dShortFrozenAmount;
			}

			LDebug("剩余部分解冻。持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

		}
		else
		{
			LError("cOperType error，cOperType=[{0}]", PositionData.cOperType);

		}

		pPosition->iModifyDate = PositionData.iModifyDate;
		pPosition->iModifyTime = PositionData.iModifyTime;

		//打印持仓
		LDebug("-----------持仓已经更新，打印更新后的持仓数据--------");
		PrintData::PrintPosition(*pPosition);

		//更新数据库
		TradeData::UpdatePosition(*pPosition);

	}

	//更新Balance
	BalanceData.iModifyDate = PositionData.iModifyDate; //修改日期
	BalanceData.iModifyTime = PositionData.iModifyTime; //修改时间
	UpdateBalance(BalanceData);

}

//查询资金
BalanceT RuntimeManagerCtp::GetBalance()
{
	lock_guard<mutex> lk(m_BalanceMtx);

	LDebug("RuntimeManagerCtp  GetBalance,  m_szUserId=[{0}]", m_szUserId);

	return m_Balance;
}

//根据OrderRef查询委托
bool RuntimeManagerCtp::GetOrderByOrderRef(string& szOrderRef, shared_ptr<OrderT>& pOrderT)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetOrderByOrderRef,  m_szUserId=[{0}],szOrderRef=[{1}]", m_szUserId, szOrderRef);

	if (m_OrdersMap.count(szOrderRef) > 0)
	{
		pOrderT = m_OrdersMap[szOrderRef];

		//打印委托
		PrintData::PrintOrder(*pOrderT);

		return true;
	}
	else
	{
		return false;
	}
	return true;
}
//根据客户端ClientOrderId查询委托
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

		//打印委托
		PrintData::PrintOrder(*pOrderT);
		return true;
	}
	else
	{
		return false;
	}
	return true;
}
//查询当天所有委托
bool RuntimeManagerCtp::GetOrders(vector<shared_ptr<OrderT>>& Orders)
{
	lock_guard<mutex> lk(m_OrderMtx);

	LDebug("RuntimeManagerCtp  GetOrders,  m_szUserId=[{0}]", m_szUserId);

	if (m_OrdersMap.size() > 0)
	{
		for (auto &OrderPair : m_OrdersMap)
		{
			Orders.push_back(OrderPair.second);

			//打印委托
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

//查询当天所有交易
bool  RuntimeManagerCtp::GetTrades(vector<shared_ptr<TradeT>>& Trades)
{
	lock_guard<mutex> lk(m_TradeMtx);

	LDebug("RuntimeManagerCtp  GetTrades,  m_szUserId=[{0}]", m_szUserId);

	if (m_TradesMap.size() > 0)
	{
		for (auto &TradePair : m_TradesMap)
		{
			Trades.push_back(TradePair.second);

			//打印交易
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
//根据OrderRef查询交易
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

		//打印交易
		PrintData::PrintTrade(*pTrade);
	}

	return true;
}
//根据客户端ClientOrderId查询交易
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
//根据股票代码查询持仓
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

		//打印持仓
		PrintData::PrintPosition(*pPositionT);
	}

	cDirection = POSITION_DIRECTION_SHORT;
	szSecurityDirectionKey = szExchangeID + '_' + szStockCode + '_' + cDirection;
	if (m_PositionsMap.count(szSecurityDirectionKey) > 0)
	{
		shared_ptr<PositionT> pPositionT = m_PositionsMap[szSecurityDirectionKey];
		vPositionT.push_back(pPositionT);
		iCount++;

		//打印持仓
		PrintData::PrintPosition(*pPositionT);
	}

	if (0 == iCount)
	{
		return false;
	}

	return true;
}
//根据股票代码和持仓方向查询持仓
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

		//打印持仓
		PrintData::PrintPosition(*pPosition);
	}

	if (0 == iCount)
	{
		return false;
	}

	return true;
}
//查询所有股票的持仓
bool RuntimeManagerCtp::GetPositions(vector<shared_ptr<PositionT>>& Positions)
{
	lock_guard<mutex> lk(m_PositionMtx);

	LDebug("RuntimeManagerCtp  GetPositions,  m_szUserId=[{0}]", m_szUserId);

	if (m_PositionsMap.size() > 0)
	{
		for (auto &PositionPair : m_PositionsMap)
		{
			Positions.push_back(PositionPair.second);

			//打印持仓
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
//获取未成交的数量
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
//风险控制:检查资金
bool RuntimeManagerCtp::RiskBalance(const OrderT&OrderData)
{
	lock_guard<mutex> lk(m_BalanceMtx);

	//开仓检查资金
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_OPEN == OrderData.cAction)
	{
		double dAmount = OrderData.iEntrustNum*OrderData.dPrice;

		//由于浮动亏损会使可用资金减少，所以此处使用RISK_BALANCE_RATE
		double dAvailable = m_Balance.dAvailable*RISK_BALANCE_RATE;

		//超过可用资金
		if ((dAmount - dAvailable) > 0)
		{
			return false;
		}

	}

	return true;
}
//风险控制:检查持仓
bool RuntimeManagerCtp::RiskPosition(const OrderT&OrderData)
{
	lock_guard<mutex> lk(m_PositionMtx);

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}
	string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;

	/*
	多头持仓下可平数量 = 持仓数量 - 空头冻结数量；
	空头持仓下可平数量 = 持仓数量 - 多头冻结数量；
	*/
	if (POSITION_DIRECTION_LONG == cDirection)
	{
		//平仓检查持仓
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
		//平仓检查持仓
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
//szClientOrderId是否存在
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
//szOrderRef是否存在
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

//更新用于计算手续费的开仓数量
bool  RuntimeManagerCtp::UpdateCommissionOpenCount(TradeT &TradeData)
{
	lock_guard<mutex> lk(m_CommissionOpenCountMtx);

	LDebug("RuntimeManagerCtp  UpdateCommissionOpenCount,  m_szUserId=[{0}]", m_szUserId);

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}

	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);

	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_OPEN == TradeData.cAction)
	{
		//存储开仓数据
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
		//平仓
		if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
		{
			shared_ptr<OpenCountT>  pOpenCountT = m_CommissionOpenCountMap[szSecurityDirectionKey];
			int iOldNum = pOpenCountT->iOpenCount;
			int iNewNum = iOldNum - TradeData.iTradeNum;
			LDebug("iOldNum=[{0}], iNewNum=[{1}]", iOldNum, iNewNum);

			//成交数量可能大于开仓数量
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
//获取平仓手续费
double RuntimeManagerCtp::GetCloseCommission(OrderT &OrderData)
{
	lock_guard<mutex> lk(m_CommissionOpenCountMtx);

	LDebug("RuntimeManagerCtp  GetCloseCommission,  m_szUserId=[{0}]", m_szUserId);

	//平仓,平仓有平昨仓和平今仓
	//冻结的手续费
	double dFrozenCommission = 0;

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == OrderData.cAction || ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction || ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}

	string szSecurityDirectionKey = OrderData.szExchangeID + '_' + OrderData.szStockCode + '_' + cDirection;
	LDebug("持仓方向szSecurityDirectionKey=[{0}]", szSecurityDirectionKey);


	//今仓数量
	int iTodayPosition = 0;
	if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
	{
		iTodayPosition = m_CommissionOpenCountMap[szSecurityDirectionKey]->iOpenCount;
	}
	LDebug("iTodayPosition=[{0}] ", iTodayPosition);

	// 手续费: DCE ,CZCE,CFFEX 先平今仓后平昨仓,其他交易所默认只平昨仓
	if (OrderData.szExchangeID == "DCE" || OrderData.szExchangeID == "CZCE" || OrderData.szExchangeID == "CFFEX")
	{
		//手续费:先平今仓后平昨仓

		//iValue=今仓数量-委托数量
		int iValue = iTodayPosition - OrderData.iEntrustNum;
		if (iValue >= 0) //委托数量小于等于今仓数量，只平今仓
		{
			//手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
			//计算平今仓手续费，平今仓费率使用平今手续费。
			dFrozenCommission = OrderData.iEntrustNum*OrderData.dCloseTodayRatioByVolume + OrderData.dCloseTodayRatioByMoney*OrderData.dPrice*OrderData.iEntrustNum*OrderData.iVolumeMultiple;

		}
		else //委托数量大于今仓数量，先平今仓，再平昨仓
		{

			//昨仓数量
			int iYdPosition = 0 - iValue;

			//手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
			//计算平今仓手续费，平今仓费率使用平今手续费。 
			double dTodayFrozenCommission = iTodayPosition*OrderData.dCloseTodayRatioByVolume + OrderData.dCloseTodayRatioByMoney*OrderData.dPrice*iTodayPosition*OrderData.iVolumeMultiple;
			//计算平昨仓手续费，平昨仓费率使用平仓手续费
			double dYdFrozenCommission = iYdPosition*OrderData.dRatioByVolume + OrderData.dRatioByMoney*OrderData.dPrice*iYdPosition*OrderData.iVolumeMultiple;
			//手续费=平昨仓手续费+平今仓手续费
			dFrozenCommission = dTodayFrozenCommission + dYdFrozenCommission;
		}

	}
	else
	{
		//手续费:其他交易所默认只平昨仓

		//昨仓数量
		int iYdPosition = OrderData.iEntrustNum;

		//手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
		//计算平昨仓手续费，平昨仓费率使用平仓手续费
		double dYdFrozenCommission = iYdPosition*OrderData.dRatioByVolume + OrderData.dRatioByMoney*OrderData.dPrice*iYdPosition*OrderData.iVolumeMultiple;
		//手续费=平昨仓手续费
		dFrozenCommission = dYdFrozenCommission;

	}

	return dFrozenCommission;
}
//获取平仓手续费
double RuntimeManagerCtp::GetCloseCommission(TradeT &TradeData, OrderT &OrderData)
{
	lock_guard<mutex> lk(m_CommissionOpenCountMtx);

	LDebug("RuntimeManagerCtp  GetCloseCommission,  m_szUserId=[{0}]", m_szUserId);

	//获取费率信息
	double dOrderPrice = OrderData.dPrice;
	int iVolumeMultiple = OrderData.iVolumeMultiple;//合约数量乘数
	double	dMarginRatioByMoney = OrderData.dMarginRatioByMoney;///保证金率	
	double	dMarginRatioByVolume = OrderData.dMarginRatioByVolume;///保证金费	
	double	dRatioByMoney = OrderData.dRatioByMoney;///开平仓合约手续费率	
	double	dRatioByVolume = OrderData.dRatioByVolume;///开平仓合约手续费							   
	double	dCloseTodayRatioByMoney = OrderData.dCloseTodayRatioByMoney;///平今手续费率	
	double	dCloseTodayRatioByVolume = OrderData.dCloseTodayRatioByVolume;///平今手续费
	double  dPreSettlementPrice = OrderData.dPreSettlementPrice; //昨结算价


	//冻结的手续费
	double dFrozenCommission = 0;

	char cDirection = POSITION_DIRECTION_NET;
	if (ORDER_ACTION_BUY_OPEN == TradeData.cAction || ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_LONG;
		LDebug("多头  即买开仓，卖平仓");
	}
	else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction || ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
	{
		cDirection = POSITION_DIRECTION_SHORT;
		LDebug("空头  即卖开仓，买平仓");
	}
	string szSecurityDirectionKey = TradeData.szExchangeID + '_' + TradeData.szStockCode + '_' + cDirection;
	LDebug("持仓方向szSecurityDirectionKey=[{0}] ", szSecurityDirectionKey);

	//今仓数量
	int  iTodayPosition = 0;
	if (m_CommissionOpenCountMap.count(szSecurityDirectionKey) > 0)
	{
		iTodayPosition = m_CommissionOpenCountMap[szSecurityDirectionKey]->iOpenCount;
	}
	LDebug("iTodayPosition=[{0}] ", iTodayPosition);

	// 手续费: DCE ,CZCE,CFFEX 先平今仓后平昨仓,其他交易所默认只平昨仓
	if (TradeData.szExchangeID == "DCE" || TradeData.szExchangeID == "CZCE" || TradeData.szExchangeID == "CFFEX")
	{
		//手续费:先平今仓后平昨仓

		//iValue=今仓数量-委托数量
		int iValue = iTodayPosition - TradeData.iTradeNum;
		if (iValue >= 0) //委托数量小于等于今仓数量，只平今仓
		{
			//手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
			//计算平今仓手续费，平今仓费率使用平今手续费。
			dFrozenCommission = TradeData.iTradeNum*dCloseTodayRatioByVolume + dCloseTodayRatioByMoney*TradeData.dPrice*TradeData.iTradeNum*iVolumeMultiple;

		}
		else //委托数量大于今仓数量，先平今仓，再平昨仓
		{

			//昨仓数量
			int iYdPosition = 0 - iValue;

			//手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
			//计算平今仓手续费，平今仓费率使用平今手续费。 
			double dTodayFrozenCommission = iTodayPosition*dCloseTodayRatioByVolume + dCloseTodayRatioByMoney*TradeData.dPrice*iTodayPosition*iVolumeMultiple;
			//计算平昨仓手续费，平昨仓费率使用平仓手续费
			double dYdFrozenCommission = iYdPosition*dRatioByVolume + dRatioByMoney*TradeData.dPrice*iYdPosition*iVolumeMultiple;
			//手续费=平昨仓手续费+平今仓手续费
			dFrozenCommission = dTodayFrozenCommission + dYdFrozenCommission;
		}

	}
	else
	{
		//手续费:其他交易所默认只平昨仓

		//昨仓数量
		int iYdPosition = TradeData.iTradeNum;

		//手续费 = 按手数手续费*手数+手续费率*价格*手数*合约乘数。
		//计算平昨仓手续费，平昨仓费率使用平仓手续费
		double dYdFrozenCommission = iYdPosition*dRatioByVolume + dRatioByMoney*TradeData.dPrice*iYdPosition*iVolumeMultiple;
		//手续费=平昨仓手续费
		dFrozenCommission = dYdFrozenCommission;

	}

	return dFrozenCommission;
}
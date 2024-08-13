#ifndef _PRINT_H
#define _PRINT_H
#include "config.h"
#include "const.h"
#include "struct.h"
#include "sim_log.h"
#include "ThostFtdcTraderApi.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

namespace PrintData
{
	using namespace std;

	inline void PrintConfigData(ConfigData&ConfigDatas)
	{
		LDebug("--------ConfigData------------");
		LInfo("LogCfgData.iLoglevel: {0}", ConfigDatas.LogCfgData.iLoglevel);
		LInfo("CtpCfgData.szTradeIp: {0}", ConfigDatas.CtpCfgData.szTradeIp);
		LInfo("CtpCfgData.iTradePort: {0}", ConfigDatas.CtpCfgData.iTradePort);
		LInfo("CtpCfgData.szMarketIp: {0}", ConfigDatas.CtpCfgData.szMarketIp);
		LInfo("CtpCfgData.iMarketPort: {0}", ConfigDatas.CtpCfgData.iMarketPort);
		LInfo("CtpCfgData.szBrokerId: {0}", ConfigDatas.CtpCfgData.szBrokerId);
		LInfo("CtpCfgData.szUser: {0}", ConfigDatas.CtpCfgData.szUserId);
		LInfo("CtpCfgData.szPassword: {0}", ConfigDatas.CtpCfgData.szPassword);
		LInfo("CtpCfgData.szAuthCode: {0}", ConfigDatas.CtpCfgData.szAuthCode);
		LInfo("CtpCfgData.szAppID: {0}", ConfigDatas.CtpCfgData.szAppID);
		LInfo("TcpCfgData.iPort: {0}", ConfigDatas.TcpCfgData.iPort);
		LInfo("szDataBase: {0}", ConfigDatas.szDataBase);

		for (int i = 0; i < ConfigDatas.AdminUsers.size(); i++)
		{
			User&UserInfo = ConfigDatas.AdminUsers[i];
			LInfo("UserInfo.iRoletype: {0}", UserInfo.iRoletype);
			LInfo("UserInfo.szUserId: {0}", UserInfo.szUserId);
			LInfo("UserInfo.szPassword: {0}", UserInfo.szPassword);

		}

		for (int i = 0; i < ConfigDatas.TradeUsers.size(); i++)
		{
			User&UserInfo = ConfigDatas.TradeUsers[i];
			LInfo("UserInfo.iRoletype: {0}", UserInfo.iRoletype);
			LInfo("UserInfo.szUserId: {0}", UserInfo.szUserId);
			LInfo("UserInfo.szPassword: {0}", UserInfo.szPassword);
		}
	}

	inline void PrintBalance(BalanceT&BalanceData)
	{
		LDebug("--------BalanceData------------");
		LDebug("BalanceData.szUserId=[{0}]", BalanceData.szUserId);
		LDebug("BalanceData.dUseMargin=[{0}]", BalanceData.dUseMargin);
		LDebug("BalanceData.dFrozenMargin=[{0}]", BalanceData.dFrozenMargin);
		LDebug("BalanceData.dFrozenCash=[{0}]", BalanceData.dFrozenCash);
		LDebug("BalanceData.dFrozenCommission=[{0}]", BalanceData.dFrozenCommission);
		LDebug("BalanceData.dCurrMargin=[{0}]", BalanceData.dCurrMargin);
		LDebug("BalanceData.dCommission=[{0}]", BalanceData.dCommission);
		LDebug("BalanceData.dAvailable=[{0}]", BalanceData.dAvailable);
		LDebug("BalanceData.dStartMoney=[{0}]", BalanceData.dStartMoney);
		LDebug("BalanceData.dAddMoney=[{0}]", BalanceData.dAddMoney);
		LDebug("BalanceData.iModifyDate=[{0}]", BalanceData.iModifyDate);
		LDebug("BalanceData.iModifyTime=[{0}]", BalanceData.iModifyTime);
	}
	inline void PrintOrder(OrderT&OrderData)
	{
		LDebug("--------OrderData------------");
		LDebug("OrderData.iInitDate=[{0}]", OrderData.iInitDate);
		LDebug("OrderData.iInitTime=[{0}]", OrderData.iInitTime);
		LDebug("OrderData.iModifyDate=[{0}]", OrderData.iModifyDate);
		LDebug("OrderData.iModifyTime=[{0}]", OrderData.iModifyTime);
		LDebug("OrderData.szUserId=[{0}]", OrderData.szUserId);
		LDebug("OrderData.szJysInsertDate=[{0}]", OrderData.szJysInsertDate);
		LDebug("OrderData.szJysInsertTime=[{0}]", OrderData.szJysInsertTime);
		LDebug("OrderData.szExchangeID=[{0}]", OrderData.szExchangeID);
		LDebug("OrderData.szStockCode=[{0}]", OrderData.szStockCode);
		LDebug("OrderData.szOrderRef=[{0}]", OrderData.szOrderRef);
		LDebug("OrderData.szOrderSysID=[{0}]", OrderData.szOrderSysID);
		LDebug("OrderData.iFrontID=[{0}]", OrderData.iFrontID);
		LDebug("OrderData.iSessionID=[{0}]", OrderData.iSessionID);
		LDebug("OrderData.cAction=[{0}]", OrderData.cAction);
		if (ORDER_ACTION_BUY_OPEN == OrderData.cAction)
		{
			LDebug("买进开仓");
		}
		else if (ORDER_ACTION_SELL_CLOSE == OrderData.cAction)
		{
			LDebug("卖出平仓");
		}
		else if (ORDER_ACTION_SELL_OPEN == OrderData.cAction)
		{
			LDebug("卖出开仓");
		}
		else if (ORDER_ACTION_BUY_CLOSE == OrderData.cAction)
		{
			LDebug("买进平仓");
		}
		else if (ORDER_ACTION_BUY_UNKNOWN == OrderData.cAction)
		{
			LDebug("未知");
		}
		LDebug("OrderData.dPrice=[{0}]", OrderData.dPrice);
		LDebug("OrderData.iEntrustNum=[{0}]", OrderData.iEntrustNum);
		LDebug("OrderData.iTradeNum=[{0}]", OrderData.iTradeNum);
		LDebug("OrderData.cStatus=[{0}]", OrderData.cStatus);
		if (ORDER_STATUS_NO_SEND == OrderData.cStatus)
		{
			LDebug("未发送");
		}
		else if (ORDER_STATUS_SEND == OrderData.cStatus)
		{
			LDebug("已发送");
		}
		else if (ORDER_STATUS_SUBMIT == OrderData.cStatus)
		{
			LDebug("已提交");
		}
		else if (ORDER_STATUS_ACCEPT == OrderData.cStatus)
		{
			LDebug("已接受");
		}
		else if (ORDER_STATUS_PART_TRADE == OrderData.cStatus)
		{
			LDebug("部分成交");
		}
		else if (ORDER_STATUS_ALL_TRADE == OrderData.cStatus)
		{
			LDebug("全部成交");
		}
		else if (ORDER_STATUS_CANCEL == OrderData.cStatus)
		{
			LDebug("已撤单");
		}
		else if (ORDER_STATUS_SEND_FAIL == OrderData.cStatus)
		{
			LDebug("发送失败");
		}
		else if (ORDER_STATUS_CANCEL_REFUSE == OrderData.cStatus)
		{
			LDebug("撤单被拒绝");
		}
		else if (ORDER_STATUS_INSERT_REFUSE == OrderData.cStatus)
		{
			LDebug("报单被拒绝");
		}
		else if (ORDER_STATUS_UNKNOWN == OrderData.cStatus)
		{
			LDebug("未知");
		}

		LDebug("OrderData.dFrozenMargin=[{0}]", OrderData.dFrozenMargin);
		LDebug("OrderData.dFrozenCommission=[{0}]", OrderData.dFrozenCommission);
		LDebug("OrderData.iVolumeMultiple=[{0}]", OrderData.iVolumeMultiple);
		LDebug("OrderData.dMarginRatioByMoney=[{0}]", OrderData.dMarginRatioByMoney);
		LDebug("OrderData.dMarginRatioByVolume=[{0}]", OrderData.dMarginRatioByVolume);
		LDebug("OrderData.dRatioByMoney=[{0}]", OrderData.dRatioByMoney);
		LDebug("OrderData.dRatioByVolume=[{0}]", OrderData.dRatioByVolume);
		LDebug("OrderData.dCloseTodayRatioByMoney=[{0}]", OrderData.dCloseTodayRatioByMoney);
		LDebug("OrderData.dCloseTodayRatioByVolume=[{0}]", OrderData.dCloseTodayRatioByVolume);
		LDebug("OrderData.dOrderCommByVolume=[{0}]", OrderData.dOrderCommByVolume);
		LDebug("OrderData.dOrderActionCommByVolume=[{0}]", OrderData.dOrderActionCommByVolume);
		LDebug("OrderData.dPreSettlementPrice=[{0}]", OrderData.dPreSettlementPrice);

	}

	inline void PrintTrade(TradeT&TradeData)
	{
		LDebug("--------Tradedata------------");
		LDebug("TradeData.iInitDate=[{0}]", TradeData.iInitDate);
		LDebug("TradeData.iInitTime=[{0}]", TradeData.iInitTime);
		LDebug("TradeData.szUserId=[{0}]", TradeData.szUserId);
		LDebug("TradeData.szJysTradeDate=[{0}]", TradeData.szJysTradeDate);
		LDebug("TradeData.szJysTradeTime=[{0}]", TradeData.szJysTradeTime);
		LDebug("TradeData.szExchangeID=[{0}]", TradeData.szExchangeID);
		LDebug("TradeData.szStockCode=[{0}]", TradeData.szStockCode);
		LDebug("TradeData.szTradeID=[{0}]", TradeData.szTradeID);
		LDebug("TradeData.szOrderRef=[{0}]", TradeData.szOrderRef);
		LDebug("TradeData.szOrderSysID=[{0}]", TradeData.szOrderSysID);
		LDebug("TradeData.cAction=[{0}]", TradeData.cAction);
		if (ORDER_ACTION_BUY_OPEN == TradeData.cAction)
		{
			LDebug("买进开仓");
		}
		else if (ORDER_ACTION_SELL_CLOSE == TradeData.cAction)
		{
			LDebug("卖出平仓");
		}
		else if (ORDER_ACTION_SELL_OPEN == TradeData.cAction)
		{
			LDebug("卖出开仓");
		}
		else if (ORDER_ACTION_BUY_CLOSE == TradeData.cAction)
		{
			LDebug("买进平仓");
		}
		else if (ORDER_ACTION_BUY_UNKNOWN == TradeData.cAction)
		{
			LDebug("未知");
		}
		LDebug("TradeData.dPrice=[{0}]", TradeData.dPrice);
		LDebug("TradeData.iTradeNum=[{0}]", TradeData.iTradeNum);
	}
	inline void PrintPosition(PositionT&PositionData)
	{
		LDebug("--------PositionData------------");
		LDebug("PositionData.iInitDate=[{0}]", PositionData.iInitDate);
		LDebug("PositionData.iInitTime=[{0}]", PositionData.iInitTime);
		LDebug("PositionData.iModifyDate=[{0}]", PositionData.iModifyDate);
		LDebug("PositionData.iModifyTime=[{0}]", PositionData.iModifyTime);
		LDebug("PositionData.szUserId=[{0}]", PositionData.szUserId);
		LDebug("PositionData.szExchangeID=[{0}]", PositionData.szExchangeID);
		LDebug("PositionData.szStockCode=[{0}]", PositionData.szStockCode);
		LDebug("PositionData.cDirection=[{0}]", PositionData.cDirection);
		if (POSITION_DIRECTION_NET == PositionData.cDirection)
		{
			LDebug("净");
		}
		else if (POSITION_DIRECTION_LONG == PositionData.cDirection)
		{
			LDebug("多头");
		}
		else if (POSITION_DIRECTION_SHORT == PositionData.cDirection)
		{
			LDebug("空头");
		}
		LDebug("PositionData.cHedgeFlag=[{0}]", PositionData.cHedgeFlag);
		LDebug("PositionData.iYdPosition=[{0}]", PositionData.iYdPosition);
		LDebug("PositionData.iTodayPosition=[{0}]", PositionData.iTodayPosition);
		LDebug("PositionData.iPosition=[{0}]", PositionData.iPosition);
		LDebug("PositionData.iLongFrozen=[{0}]", PositionData.iLongFrozen);
		LDebug("PositionData.iShortFrozen=[{0}]", PositionData.iShortFrozen);
		LDebug("PositionData.dLongFrozenAmount=[{0}]", PositionData.dLongFrozenAmount);
		LDebug("PositionData.dShortFrozenAmount=[{0}]", PositionData.dShortFrozenAmount);
		LDebug("PositionData.dUseMargin=[{0}]", PositionData.dUseMargin);
		LDebug("PositionData.dFrozenMargin=[{0}]", PositionData.dFrozenMargin);
		LDebug("PositionData.dFrozenCash=[{0}]", PositionData.dFrozenCash);
		LDebug("PositionData.dFrozenCommission=[{0}]", PositionData.dFrozenCommission);
		LDebug("PositionData.dCommission=[{0}]", PositionData.dCommission);
		LDebug("PositionData.dPreSettlementPrice=[{0}]", PositionData.dPreSettlementPrice);
		LDebug("PositionData.dSettlementPrice=[{0}]", PositionData.dSettlementPrice);
		LDebug("PositionData.dMarginRateByMoney=[{0}]", PositionData.dMarginRateByMoney);
		LDebug("PositionData.dMarginRateByVolume=[{0}]", PositionData.dMarginRateByVolume);
		LDebug("PositionData.cOperType=[{0}]", PositionData.cOperType);
		LDebug("PositionData.cAction=[{0}]", PositionData.cAction);


	}


}; // namespace PrintData

#endif

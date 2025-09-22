#include "trade_data.h"
#include "sim_log.h"
#include "config.h"
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

map<string, shared_ptr<InstrumentT>> TradeData::m_InstrumentMap;
sqlite3 *TradeData::SqliteDb = nullptr;

TradeData::TradeData()
{
}

TradeData::	~TradeData()
{

}
void TradeData::Init()
{
	// connect db
	string szDbPath = ConfigData::GetInstance()->szDataBase;
	int iRet = sqlite3_open(szDbPath.c_str(), &SqliteDb);
	if (iRet != SQLITE_OK)
	{
		LError("failed to open sqlite3 db, error=[{0}]", sqlite3_errmsg(SqliteDb));
	}

}
void TradeData::Close()
{
	sqlite3_close(SqliteDb);
}


void TradeData::InsertBalance(BalanceT&BalanceData)
{
	string szSql = "insert into td_balance(user_id,td_type,use_margin,\
        frozen_margin,frozen_cash,frozen_commission,curr_margin,commission,available,\
        start_money,add_money,settle_date,modify_date,modify_time) "
		+ string("values (") + "'" + BalanceData.szUserId + "'," + string("0,")
		+ to_string(BalanceData.dUseMargin) + "," + to_string(BalanceData.dFrozenMargin) + ","
		+ to_string(BalanceData.dFrozenCash) + "," + to_string(BalanceData.dFrozenCommission) + ","
		+ to_string(BalanceData.dCurrMargin) + "," + to_string(BalanceData.dCommission) + ","
		+ to_string(BalanceData.dAvailable) + "," + to_string(BalanceData.dStartMoney) + ","
		+ to_string(BalanceData.dAddMoney) + "," + to_string(BalanceData.iSettleDate) + ","
		+ to_string(BalanceData.iModifyDate) + "," + to_string(BalanceData.iModifyTime) + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::InsertOrder(OrderT&OrderData)
{
	string szSql = "insert into td_order(init_date,init_time,modify_date,modify_time,td_type,\
        jys_insert_date,jys_insert_time,user_id,exchange_id,stock_code,client_order_id,order_ref,\
        order_sys_id,front_id,session_id,action,price,entrust_num,trade_num,status) "
		+ string("values (") + to_string(OrderData.iInitDate) + "," + to_string(OrderData.iInitTime) + ","
		+ to_string(OrderData.iModifyDate) + "," + to_string(OrderData.iModifyTime) + "," + string("0,")
		+ "'" + OrderData.szJysInsertDate + "'," + "'" + OrderData.szJysInsertTime + "',"
		+ "'" + OrderData.szUserId + "'," + "'" + OrderData.szExchangeID + "',"
		+ "'" + OrderData.szStockCode + "',"
		+ "'" + OrderData.szClientOrderId + "'," + "'" + OrderData.szOrderRef + "',"
		+ "'" + OrderData.szOrderSysID + "'," + to_string(OrderData.iFrontID) + ","
		+ to_string(OrderData.iSessionID) + "," + "'" + OrderData.cAction + "',"
		+ to_string(OrderData.dPrice) + "," + to_string(OrderData.iEntrustNum) + ","
		+ to_string(OrderData.iTradeNum) + "," + "'" + OrderData.cStatus + "'" + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::InsertTrade(TradeT&TradeData)
{
	string szSql = "insert into td_trade(init_date,init_time,td_type,\
        jys_trade_date,jys_trade_time,user_id,exchange_id,stock_code,order_ref,\
        order_sys_id,trade_id,action,price,trade_num,close_num) "
		+ string("values (") + to_string(TradeData.iInitDate) + "," + to_string(TradeData.iInitTime) + "," + string("0,")
		+ "'" + TradeData.szJysTradeDate + "'," + "'" + TradeData.szJysTradeTime + "',"
		+ "'" + TradeData.szUserId + "'," + "'" + TradeData.szExchangeID + "',"
		+ "'" + TradeData.szStockCode + "'," + "'" + TradeData.szOrderRef + "',"
		+ "'" + TradeData.szOrderSysID + "'," + "'" + TradeData.szTradeID + "',"
		+ "'" + TradeData.cAction + "'," + to_string(TradeData.dPrice) + ","
		+ to_string(TradeData.iTradeNum) + ",0)";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::InsertPosition(PositionT&PositionData)
{
	string szSql = "insert into td_position(init_date,init_time,modify_date,modify_time,td_type,\
        user_id,exchange_id,stock_code,direction,hedge_flag,yd_position,today_position,position,long_frozen,short_frozen,\
        long_frozen_amount,short_frozen_amount,use_margin,frozen_margin,frozen_cash,frozen_commission,commission,\
        pre_settlement_price,settlement_price,margin_rate_by_money,margin_rate_by_volume) "
		+ string("values (") + to_string(PositionData.iInitDate) + "," + to_string(PositionData.iInitTime) + ","
		+ to_string(PositionData.iModifyDate) + "," + to_string(PositionData.iModifyTime) + "," + string("0,")
		+ "'" + PositionData.szUserId + "'," + "'" + PositionData.szExchangeID + "',"
		+ "'" + PositionData.szStockCode + "',"
		+ "'" + PositionData.cDirection + "'," + "'" + PositionData.cHedgeFlag + "',"
		+ to_string(PositionData.iYdPosition) + "," + to_string(PositionData.iTodayPosition) + "," + to_string(PositionData.iPosition) + ","
		+ to_string(PositionData.iLongFrozen) + "," + to_string(PositionData.iShortFrozen) + "," + to_string(PositionData.dLongFrozenAmount) + ","
		+ to_string(PositionData.dShortFrozenAmount) + "," + to_string(PositionData.dUseMargin) + "," + to_string(PositionData.dFrozenMargin) + ","
		+ to_string(PositionData.dFrozenCash) + "," + to_string(PositionData.dFrozenCommission) + "," + to_string(PositionData.dCommission) + ","
		+ to_string(PositionData.dPreSettlementPrice) + "," + to_string(PositionData.dSettlementPrice) + "," + to_string(PositionData.dMarginRateByMoney) + ","
		+ to_string(PositionData.dMarginRateByVolume) + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::InsertInstrument(InstrumentT&InstrumentData)
{
	string szSql = "insert into td_instrument (instrument_id,exchange_id,instrument_name,product_id,  \
        product_class,delivery_year,delivery_month,max_market_order_volume,min_market_order_volume, \
	    max_limit_order_volume,min_limit_order_volume,volume_multiple,price_tick,expire_date,\
	    is_trading,position_type,position_date_type,long_margin_ratio,short_margin_ratio,\
        max_margin_side_algorithm,strike_price,options_type,underlying_multiple)"
		+ string("values (") + "'" + InstrumentData.szInstrumentId + "'," + "'" + InstrumentData.szExchangeId + "',"
		+ "'" + InstrumentData.szInstrumentName + "'," + "'" + InstrumentData.szProductId + "'," + "'" + InstrumentData.szProductClass + "',"
		+ to_string(InstrumentData.iDeliveryYear) + "," + to_string(InstrumentData.iDeliveryMonth) + ","
		+ to_string(InstrumentData.iMaxMarketOrderVolume) + "," + to_string(InstrumentData.iMinMarketOrderVolume) + ","
		+ to_string(InstrumentData.iMaxLimitOrderVolume) + "," + to_string(InstrumentData.iMinLimitOrderVolume) + ","
		+ to_string(InstrumentData.iVolumeMultiple) + "," + to_string(InstrumentData.dPriceTick) + ","
		+ "'" + InstrumentData.szExpireDate + "'," + to_string(InstrumentData.iIsTrading) + ","
		+ "'" + InstrumentData.szPositionType + "'," + "'" + InstrumentData.szPositionDateType + "',"
		+ to_string(InstrumentData.dLongMarginRatio) + "," + to_string(InstrumentData.dShortMarginRatio) + ","
		+ "'" + InstrumentData.szMaxMarginSideAlgorithm + "'," + to_string(InstrumentData.dStrikePrice) + ","
		+ "'" + InstrumentData.cOptionsType + "'," + to_string(InstrumentData.dUnderlyingMultiple) + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::InsertInstrumentMarginRate(InstrumentMarginRate&InstrumentMarginRateData)
{
	string szSql = "insert into td_instrument_margin_rate(instrument_id,exchange_id,hedge_flag,long_margin_ratio_by_money,long_margin_ratio_by_volume,\
        short_margin_ratio_by_money,short_margin_ratio_by_volume,is_relative) "
		+ string("values ('") + InstrumentMarginRateData.szInstrumentId + "'," + "'" + InstrumentMarginRateData.szExchangeId + "',"
		+ "'" + InstrumentMarginRateData.cHedgeFlag + "',"
		+ to_string(InstrumentMarginRateData.dLongMarginRatioByMoney) + "," + to_string(InstrumentMarginRateData.dLongMarginRatioByVolume) + "," + to_string(InstrumentMarginRateData.dShortMarginRatioByMoney) + ","
		+ to_string(InstrumentMarginRateData.dShortMarginRatioByVolume) + "," + to_string(InstrumentMarginRateData.iIsRelative) + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::InsertInstrumentCommissionRate(InstrumentCommissionRate&InstrumentCommissionRateData)
{
	string szSql = "insert into td_instrument_commission_rate(instrument_id,exchange_id,open_ratio_by_money,open_ratio_by_volume,close_ratio_by_money,\
        close_ratio_by_volume,close_today_ratio_by_money,close_today_ratio_by_volume,biz_type) "
		+ string("values ('") + InstrumentCommissionRateData.szInstrumentId + "'," + "'" + InstrumentCommissionRateData.szExchangeId + "',"
		+ to_string(InstrumentCommissionRateData.dOpenRatioByMoney) + "," + to_string(InstrumentCommissionRateData.dOpenRatioByVolume) + "," + to_string(InstrumentCommissionRateData.dCloseRatioByMoney) + ","
		+ to_string(InstrumentCommissionRateData.dCloseRatioByVolume) + "," + to_string(InstrumentCommissionRateData.dCloseTodayRatioByMoney) + "," + to_string(InstrumentCommissionRateData.dCloseTodayRatioByVolume) + ","
		+ "'" + InstrumentCommissionRateData.cBizType + "'" + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::InsertInstrumentOrderCommRate(InstrumentOrderCommRate&InstrumentOrderCommRateData)
{
	string szSql = "insert into td_instrument_order_comm_rate(instrument_id,exchange_id,hedge_flag,order_comm_by_volume,order_action_comm_by_volume) "
		+ string("values ('") + InstrumentOrderCommRateData.szInstrumentId + "'," + "'" + InstrumentOrderCommRateData.szExchangeId + "'," + "'" + InstrumentOrderCommRateData.cHedgeFlag + "',"
		+ to_string(InstrumentOrderCommRateData.dOrderCommByVolume) + "," + to_string(InstrumentOrderCommRateData.dOrderActionCommByVolume) + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::InsertCommissionOpenCount(OpenCountT&CommissionOpenCountData)
{
	string szSql = "insert into td_commission_open_count(init_date,td_type,\
        user_id,exchange_id,stock_code,direction,open_count) "
		+ string("values (") + to_string(CommissionOpenCountData.iInitDate) + "," + string("0,")
		+ "'" + CommissionOpenCountData.szUserId + "'," + "'" + CommissionOpenCountData.szExchangeID + "',"
		+ "'" + CommissionOpenCountData.szStockCode + "'," + "'" + CommissionOpenCountData.cDirection + "',"
		+ to_string(CommissionOpenCountData.iOpenCount) + ")";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::DeleteInstrument()
{
	string szSql = "delete from td_instrument ";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::DeleteInstrumentMarginRate()
{
	string szSql = "delete from td_instrument_margin_rate ";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::DeleteInstrumentCommissionRate()
{
	string szSql = "delete from td_instrument_commission_rate ";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::DeleteInstrumentOrderCommRate()
{
	string szSql = "delete from td_instrument_order_comm_rate ";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::UpdateBalance(BalanceT&BalanceData)
{
	string szSql = "update td_balance  set  "
		+ string("modify_date=") + to_string(BalanceData.iModifyDate) + ","
		+ "modify_time=" + to_string(BalanceData.iModifyTime) + ","
		+ "settle_date=" + to_string(BalanceData.iSettleDate) + ","
		+ "use_margin=" + to_string(BalanceData.dUseMargin) + ","
		+ "frozen_margin=" + to_string(BalanceData.dFrozenMargin) + ","
		+ "frozen_cash=" + to_string(BalanceData.dFrozenCash) + ","
		+ "frozen_commission=" + to_string(BalanceData.dFrozenCommission) + ","
		+ "curr_margin=" + to_string(BalanceData.dCurrMargin) + ","
		+ "commission=" + to_string(BalanceData.dCommission) + ","
		+ "available=" + to_string(BalanceData.dAvailable) + ","
		+ "add_money=" + to_string(BalanceData.dAddMoney)
		+ "   where user_id='" + BalanceData.szUserId + "'";


	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}

void TradeData::UpdateOrder(OrderT&OrderData)
{
	string szSql = "update td_order  set   "
		+ string("modify_date=") + to_string(OrderData.iModifyDate) + ","
		+ "modify_time=" + to_string(OrderData.iModifyTime) + ","
		+ "jys_insert_date='" + OrderData.szJysInsertDate + "',"
		+ "jys_insert_time='" + OrderData.szJysInsertTime + "',"
		+ "exchange_id='" + OrderData.szExchangeID + "',"
		+ "order_sys_id='" + OrderData.szOrderSysID + "',"
		+ "front_id=" + to_string(OrderData.iFrontID) + ","
		+ "session_id=" + to_string(OrderData.iSessionID) + ","
		+ "trade_num=" + to_string(OrderData.iTradeNum) + ","
		+ "status='" + OrderData.cStatus + "'"
		+ "   where init_date=" + to_string(OrderData.iInitDate) + "  and  "
		+ "order_ref='" + OrderData.szOrderRef + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::UpdateTrade(TradeT&TradeData)
{
	string szSql = "update td_trade set close_num=" + to_string(TradeData.iCloseNum)
		+ "   where init_date=" + to_string(TradeData.iInitDate) + "  and  "
		+ "order_ref='" + TradeData.szOrderRef + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::UpdatePosition(PositionT&PositionData)
{
	string szSql = "update td_position  set   "
		+ string("modify_date=") + to_string(PositionData.iModifyDate) + ","
		+ "modify_time=" + to_string(PositionData.iModifyTime) + ","
		+ "yd_position=" + to_string(PositionData.iYdPosition) + ","
		+ "today_position=" + to_string(PositionData.iTodayPosition) + ","
		+ "position=" + to_string(PositionData.iPosition) + ","
		+ "long_frozen=" + to_string(PositionData.iLongFrozen) + ","
		+ "short_frozen=" + to_string(PositionData.iShortFrozen) + ","
		+ "long_frozen_amount=" + to_string(PositionData.dLongFrozenAmount) + ","
		+ "short_frozen_amount=" + to_string(PositionData.dShortFrozenAmount) + ","
		+ "use_margin=" + to_string(PositionData.dUseMargin) + ","
		+ "frozen_margin=" + to_string(PositionData.dFrozenMargin) + ","
		+ "frozen_cash=" + to_string(PositionData.dFrozenCash) + ","
		+ "frozen_commission=" + to_string(PositionData.dFrozenCommission) + ","
		+ "commission=" + to_string(PositionData.dCommission) + ","
		+ "pre_settlement_price=" + to_string(PositionData.dPreSettlementPrice) + ","
		+ "settlement_price=" + to_string(PositionData.dSettlementPrice) + ","
		+ "margin_rate_by_money=" + to_string(PositionData.dMarginRateByMoney) + ","
		+ "margin_rate_by_volume=" + to_string(PositionData.dMarginRateByVolume)
		+ "   where user_id='" + PositionData.szUserId + "'  and  exchange_id='" + PositionData.szExchangeID
		+ "'  and  stock_code='" + PositionData.szStockCode + "'  and  direction='" + PositionData.cDirection + "'";


	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::UpdateCommissionOpenCount(OpenCountT&CommissionOpenCountData)
{
	string szSql = "update td_commission_open_count  set  "
		+ string("open_count=") + to_string(CommissionOpenCountData.iOpenCount)
		+ "   where user_id='" + CommissionOpenCountData.szUserId + "'  and  exchange_id='" + CommissionOpenCountData.szExchangeID
		+ "'  and  stock_code='" + CommissionOpenCountData.szStockCode + "'  and  direction='" + CommissionOpenCountData.cDirection
		+ "'  and  init_date=" + to_string(CommissionOpenCountData.iInitDate);

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);
	if (iRet != SQLITE_OK)
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_step(pStatement);
	sqlite3_finalize(pStatement);
}
void TradeData::GetBalance(const string &szUserId, BalanceT&BalanceData)
{
	string szSql = "select user_id,td_type,use_margin,frozen_margin,frozen_cash,\
        frozen_commission,curr_margin,commission,available,start_money,\
        add_money,settle_date,modify_date,modify_time from td_balance "
		+ string("where") + " user_id='" + szUserId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			BalanceData.szUserId = (const char *)sqlite3_column_text(pStatement, 0); //�˺�	
			BalanceData.iTdType = sqlite3_column_int(pStatement, 1);///��������	
			BalanceData.dUseMargin = sqlite3_column_double(pStatement, 2);///ռ�õı�֤��	
			BalanceData.dFrozenMargin = sqlite3_column_double(pStatement, 3); ///����ı�֤��	
			BalanceData.dFrozenCash = sqlite3_column_double(pStatement, 4); ///������ʽ�
			BalanceData.dFrozenCommission = sqlite3_column_double(pStatement, 5); ///�����������	
			BalanceData.dCurrMargin = sqlite3_column_double(pStatement, 6); ///��ǰ��֤���ܶ�	
			BalanceData.dCommission = sqlite3_column_double(pStatement, 7); ///������	
			BalanceData.dAvailable = sqlite3_column_double(pStatement, 8); ///�����ʽ�
			BalanceData.dStartMoney = sqlite3_column_double(pStatement, 9); ///�ڳ��ı�֤��
			BalanceData.dAddMoney = sqlite3_column_double(pStatement, 10); ///�������ӵı�֤��
			BalanceData.iSettleDate = sqlite3_column_int(pStatement, 11); //��������
			BalanceData.iModifyDate = sqlite3_column_int(pStatement, 12); //�޸�����
			BalanceData.iModifyTime = sqlite3_column_int(pStatement, 13); //�޸�ʱ��

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

void TradeData::GetOrders(const int iStartDate, const int iEndDate, const string &szUserId, vector<shared_ptr<OrderT>>&OrderDatas)
{
	string szSql = "select init_date,init_time,modify_date,modify_time,td_type,\
        jys_insert_date,jys_insert_time,user_id,exchange_id,stock_code,client_order_id,order_ref,\
        order_sys_id,front_id,session_id,action,price,entrust_num,trade_num,status from td_order "
		+ string("  where") + " init_date>=" + to_string(iStartDate) + "  and  init_date<=" + to_string(iEndDate)
		+ "  and  user_id='" + szUserId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			OrderT OrderData;
			OrderData.iInitDate = sqlite3_column_int(pStatement, 0); //��������
			OrderData.iInitTime = sqlite3_column_int(pStatement, 1); //����ʱ��
			OrderData.iModifyDate = sqlite3_column_int(pStatement, 2); //�޸�����
			OrderData.iModifyTime = sqlite3_column_int(pStatement, 3); //�޸�ʱ��
			OrderData.iTdType = sqlite3_column_int(pStatement, 4);
			OrderData.szJysInsertDate = (const char *)sqlite3_column_text(pStatement, 5); //��������������
			OrderData.szJysInsertTime = (const char *)sqlite3_column_text(pStatement, 6); //������ί��ʱ��
			OrderData.szUserId = (const char *)sqlite3_column_text(pStatement, 7); //�˺�
			OrderData.szExchangeID = (const char *)sqlite3_column_text(pStatement, 8); //�г�
			OrderData.szStockCode = (const char *)sqlite3_column_text(pStatement, 9);  //����
			OrderData.szClientOrderId = (const char *)sqlite3_column_text(pStatement, 10); //�ͻ���ί��id��Ψһ����Ҫ�ظ�	
			OrderData.szOrderRef = (const char *)sqlite3_column_text(pStatement, 11);  //��������  12λ�ַ�
			OrderData.szOrderSysID = (const char *)sqlite3_column_text(pStatement, 12); // �������,�൱��ί�б��
			OrderData.iFrontID = sqlite3_column_int(pStatement, 13);  //ǰ�ñ��
			OrderData.iSessionID = sqlite3_column_int(pStatement, 14); //�Ự���
			OrderData.cAction = ((const char *)sqlite3_column_text(pStatement, 15))[0];  //��������
			OrderData.dPrice = sqlite3_column_double(pStatement, 16);  //�۸�
			OrderData.iEntrustNum = sqlite3_column_int(pStatement, 17); //ί������
			OrderData.iTradeNum = sqlite3_column_int(pStatement, 18); //�ɽ�����
			OrderData.cStatus = ((const char *)sqlite3_column_text(pStatement, 19))[0];//����״̬

			OrderDatas.push_back(make_shared<OrderT>(OrderData));

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_finalize(pStatement);

}

void TradeData::GetTrades(const int iStartDate, const int iEndDate, const string &szUserId, vector<shared_ptr<TradeT>>&TradeDatas)
{
	string szSql = "select init_date,init_time,td_type,\
        jys_trade_date,jys_trade_time,user_id,exchange_id,stock_code,order_ref,\
        order_sys_id,trade_id,action,price,trade_num,close_num from td_trade "
		+ string("  where") + " init_date>=" + to_string(iStartDate) + " and  init_date<=" + to_string(iEndDate)
		+ " and  user_id='" + szUserId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			TradeT TradeData;

			TradeData.iInitDate = sqlite3_column_int(pStatement, 0); //�������� yyyymmdd
			TradeData.iInitTime = sqlite3_column_int(pStatement, 1); //����ʱ�� HHMMSS
			TradeData.iTdType = sqlite3_column_int(pStatement, 2);
			TradeData.szJysTradeDate = (const char *)sqlite3_column_text(pStatement, 3);//�������ɽ�����
			TradeData.szJysTradeTime = (const char *)sqlite3_column_text(pStatement, 4);//�������ɽ�ʱ��
			TradeData.szUserId = (const char *)sqlite3_column_text(pStatement, 5); //�˺�
			TradeData.szExchangeID = (const char *)sqlite3_column_text(pStatement, 6); //�г�
			TradeData.szStockCode = (const char *)sqlite3_column_text(pStatement, 7);  //����
			TradeData.szOrderRef = (const char *)sqlite3_column_text(pStatement, 8);  //��������
			TradeData.szOrderSysID = (const char *)sqlite3_column_text(pStatement, 9); // �������,�൱��ί�б��
			TradeData.szTradeID = (const char *)sqlite3_column_text(pStatement, 10); //�ɽ����
			TradeData.cAction = ((const char *)sqlite3_column_text(pStatement, 11))[0];  //��������
			TradeData.dPrice = sqlite3_column_double(pStatement, 12);  //�۸�
			TradeData.iTradeNum = sqlite3_column_int(pStatement, 13); //�ɽ�����
			TradeData.iCloseNum = sqlite3_column_int(pStatement, 14); //����ƽ������

			TradeDatas.push_back(make_shared<TradeT>(TradeData));

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

void TradeData::GetTrades(const int iDate, const string &szUserId, char cAction, vector<shared_ptr<TradeT>>&TradeDatas)
{
	string szSql = "select init_date,init_time,td_type,\
        jys_trade_date,jys_trade_time,user_id,exchange_id,stock_code,order_ref,\
        order_sys_id,trade_id,action,price,trade_num,close_num from td_trade "
		+ string("  where") + " init_date=" + to_string(iDate)
		+ " and  action='" + cAction + "'"
		+ " and  user_id='" + szUserId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			TradeT TradeData;

			TradeData.iInitDate = sqlite3_column_int(pStatement, 0); //�������� yyyymmdd
			TradeData.iInitTime = sqlite3_column_int(pStatement, 1); //����ʱ�� HHMMSS
			TradeData.iTdType = sqlite3_column_int(pStatement, 2);
			TradeData.szJysTradeDate = (const char *)sqlite3_column_text(pStatement, 3);//�������ɽ�����
			TradeData.szJysTradeTime = (const char *)sqlite3_column_text(pStatement, 4);//�������ɽ�ʱ��
			TradeData.szUserId = (const char *)sqlite3_column_text(pStatement, 5); //�˺�
			TradeData.szExchangeID = (const char *)sqlite3_column_text(pStatement, 6); //�г�
			TradeData.szStockCode = (const char *)sqlite3_column_text(pStatement, 7);  //����
			TradeData.szOrderRef = (const char *)sqlite3_column_text(pStatement, 8);  //��������
			TradeData.szOrderSysID = (const char *)sqlite3_column_text(pStatement, 9); // �������,�൱��ί�б��
			TradeData.szTradeID = (const char *)sqlite3_column_text(pStatement, 10); //�ɽ����
			TradeData.cAction = ((const char *)sqlite3_column_text(pStatement, 11))[0];  //��������
			TradeData.dPrice = sqlite3_column_double(pStatement, 12);  //�۸�
			TradeData.iTradeNum = sqlite3_column_int(pStatement, 13); //�ɽ�����
			TradeData.iCloseNum = sqlite3_column_int(pStatement, 14); //����ƽ������

			TradeDatas.push_back(make_shared<TradeT>(TradeData));

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}
void TradeData::GetPositions(const string &szUserId, const string &szExchangeId, const string &szStockCode, char cDirection, PositionT&PositionData)
{
	string szSql = "select init_date,init_time,modify_date,modify_time,td_type,\
        user_id,exchange_id,stock_code,direction,hedge_flag,yd_position,today_position,position,long_frozen,short_frozen,\
        long_frozen_amount,short_frozen_amount,use_margin,frozen_margin,frozen_cash,frozen_commission,commission,\
        pre_settlement_price,settlement_price,margin_rate_by_money,margin_rate_by_volume  from td_position "
		+ string("  where") + " user_id='" + szUserId + "' and  exchange_id='" + szExchangeId + "'  and  stock_code='" + szStockCode + "' and direction='" + cDirection + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			PositionData.iInitDate = sqlite3_column_int(pStatement, 0); //���� yyyymmdd
			PositionData.iInitTime = sqlite3_column_int(pStatement, 1); //����ʱ�� HHMMSS
			PositionData.iModifyDate = sqlite3_column_int(pStatement, 2); //�޸�����
			PositionData.iModifyTime = sqlite3_column_int(pStatement, 3); //�޸�ʱ��
			PositionData.iTdType = sqlite3_column_int(pStatement, 4);
			PositionData.szUserId = (const char *)sqlite3_column_text(pStatement, 5); //�˺�
			PositionData.szExchangeID = (const char *)sqlite3_column_text(pStatement, 6); //�г�
			PositionData.szStockCode = (const char *)sqlite3_column_text(pStatement, 7);  //����
			PositionData.cDirection = ((const char *)sqlite3_column_text(pStatement, 8))[0]; //�ֲַ���				  
			PositionData.cHedgeFlag = ((const char *)sqlite3_column_text(pStatement, 9))[0];///Ͷ���ױ���־				 
			PositionData.iYdPosition = sqlite3_column_int(pStatement, 10);///���ճֲ�
			PositionData.iTodayPosition = sqlite3_column_int(pStatement, 11); ///���ճֲ�	
			PositionData.iPosition = sqlite3_column_int(pStatement, 12); ///�ֲܳ�	
			PositionData.iLongFrozen = sqlite3_column_int(pStatement, 13); ///��ͷ����
			PositionData.iShortFrozen = sqlite3_column_int(pStatement, 14); ///��ͷ����					  
			PositionData.dLongFrozenAmount = sqlite3_column_double(pStatement, 15);///��ͷ������
			PositionData.dShortFrozenAmount = sqlite3_column_double(pStatement, 16);///��ͷ������
			PositionData.dUseMargin = sqlite3_column_double(pStatement, 17); ///ռ�õı�֤��
			PositionData.dFrozenMargin = sqlite3_column_double(pStatement, 18); ///����ı�֤��	
			PositionData.dFrozenCash = sqlite3_column_double(pStatement, 19); ///������ʽ�
			PositionData.dFrozenCommission = sqlite3_column_double(pStatement, 20);///�����������	
			PositionData.dCommission = sqlite3_column_double(pStatement, 21); ///������
			PositionData.dPreSettlementPrice = sqlite3_column_double(pStatement, 22); ///�ϴν����	
			PositionData.dSettlementPrice = sqlite3_column_double(pStatement, 23); ///���ν����							  
			PositionData.dMarginRateByMoney = sqlite3_column_double(pStatement, 24); ///��֤����	
			PositionData.dMarginRateByVolume = sqlite3_column_double(pStatement, 25); ///��֤����(������)

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

void TradeData::GetPositions(const string &szUserId, vector<shared_ptr<PositionT>>&PositionDatas)
{
	string szSql = "select init_date,init_time,modify_date,modify_time,td_type,\
        user_id,exchange_id,stock_code,direction,hedge_flag,yd_position,today_position,position,long_frozen,short_frozen,\
        long_frozen_amount,short_frozen_amount,use_margin,frozen_margin,frozen_cash,frozen_commission,commission,\
        pre_settlement_price,settlement_price,margin_rate_by_money,margin_rate_by_volume  from td_position "
		+ string("where") + " user_id='" + szUserId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			PositionT PositionData;

			PositionData.iInitDate = sqlite3_column_int(pStatement, 0); //���� yyyymmdd
			PositionData.iInitTime = sqlite3_column_int(pStatement, 1); //����ʱ�� HHMMSS
			PositionData.iModifyDate = sqlite3_column_int(pStatement, 2); //�޸�����
			PositionData.iModifyTime = sqlite3_column_int(pStatement, 3); //�޸�ʱ��
			PositionData.iTdType = sqlite3_column_int(pStatement, 4);
			PositionData.szUserId = (const char *)sqlite3_column_text(pStatement, 5); //�˺�
			PositionData.szExchangeID = (const char *)sqlite3_column_text(pStatement, 6); //�г�
			PositionData.szStockCode = (const char *)sqlite3_column_text(pStatement, 7);  //����
			PositionData.cDirection = ((const char *)sqlite3_column_text(pStatement, 8))[0]; //�ֲַ���				  
			PositionData.cHedgeFlag = ((const char *)sqlite3_column_text(pStatement, 9))[0];///Ͷ���ױ���־				 
			PositionData.iYdPosition = sqlite3_column_int(pStatement, 10);///���ճֲ�
			PositionData.iTodayPosition = sqlite3_column_int(pStatement, 11); ///���ճֲ�	
			PositionData.iPosition = sqlite3_column_int(pStatement, 12); ///�ֲܳ�	
			PositionData.iLongFrozen = sqlite3_column_int(pStatement, 13); ///��ͷ����
			PositionData.iShortFrozen = sqlite3_column_int(pStatement, 14); ///��ͷ����					  
			PositionData.dLongFrozenAmount = sqlite3_column_double(pStatement, 15);///��ͷ������
			PositionData.dShortFrozenAmount = sqlite3_column_double(pStatement, 16);///��ͷ������
			PositionData.dUseMargin = sqlite3_column_double(pStatement, 17); ///ռ�õı�֤��
			PositionData.dFrozenMargin = sqlite3_column_double(pStatement, 18); ///����ı�֤��	
			PositionData.dFrozenCash = sqlite3_column_double(pStatement, 19); ///������ʽ�
			PositionData.dFrozenCommission = sqlite3_column_double(pStatement, 20);///�����������	
			PositionData.dCommission = sqlite3_column_double(pStatement, 21); ///������
			PositionData.dPreSettlementPrice = sqlite3_column_double(pStatement, 22); ///�ϴν����	
			PositionData.dSettlementPrice = sqlite3_column_double(pStatement, 23); ///���ν����							  
			PositionData.dMarginRateByMoney = sqlite3_column_double(pStatement, 24); ///��֤����	
			PositionData.dMarginRateByVolume = sqlite3_column_double(pStatement, 25); ///��֤����(������)

			PositionDatas.push_back(make_shared<PositionT>(PositionData));

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

void TradeData::GetInstruments(vector<shared_ptr<InstrumentT>>&InstrumentDatas)
{
	string szSql = "select instrument_id,exchange_id,instrument_name,product_id,  \
        product_class,delivery_year,delivery_month,max_market_order_volume,min_market_order_volume, \
	    max_limit_order_volume,min_limit_order_volume,volume_multiple,price_tick,expire_date,\
	    is_trading,position_type,position_date_type,long_margin_ratio,short_margin_ratio,\
        max_margin_side_algorithm,strike_price,options_type,underlying_multiple  from td_instrument";
	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentT   InstrumentData;
			InstrumentData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����
			InstrumentData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������
			InstrumentData.szInstrumentName = (const char *)sqlite3_column_text(pStatement, 2);//��Լ����
			InstrumentData.szProductId = (const char *)sqlite3_column_text(pStatement, 3);//��Ʒ����
			InstrumentData.szProductClass = (const char *)sqlite3_column_text(pStatement, 4); //��Ʒ����
			InstrumentData.iDeliveryYear = sqlite3_column_int(pStatement, 5); //�������
			InstrumentData.iDeliveryMonth = sqlite3_column_int(pStatement, 6); //������
			InstrumentData.iMaxMarketOrderVolume = sqlite3_column_int(pStatement, 7);//�м۵�����µ���
			InstrumentData.iMinMarketOrderVolume = sqlite3_column_int(pStatement, 8);//�м۵���С�µ���
			InstrumentData.iMaxLimitOrderVolume = sqlite3_column_int(pStatement, 9);//�޼۵�����µ���
			InstrumentData.iMinLimitOrderVolume = sqlite3_column_int(pStatement, 10);//�޼۵���С�µ���
			InstrumentData.iVolumeMultiple = sqlite3_column_int(pStatement, 11);//��Լ��������
			InstrumentData.dPriceTick = sqlite3_column_double(pStatement, 12);//��С�䶯��λ
			InstrumentData.szExpireDate = (const char *)sqlite3_column_text(pStatement, 13);//������
			InstrumentData.iIsTrading = sqlite3_column_int(pStatement, 14);//��ǰ�Ƿ���
			InstrumentData.szPositionType = (const char *)sqlite3_column_text(pStatement, 15);//�ֲ����� 1-���ֲ� 2-�ۺϳֲ�
			InstrumentData.szPositionDateType = (const char *)sqlite3_column_text(pStatement, 16);//�ֲ��������� 1-ʹ����ʷ�ֲ� 2-��ʹ����ʷ�ֲ�
			InstrumentData.dLongMarginRatio = sqlite3_column_double(pStatement, 17);//��ͷ��֤����
			InstrumentData.dShortMarginRatio = sqlite3_column_double(pStatement, 18);//��ͷ��֤����
			InstrumentData.szMaxMarginSideAlgorithm = (const char *)sqlite3_column_text(pStatement, 19);//�Ƿ�ʹ�ô��߱�֤���㷨
			InstrumentData.dStrikePrice = sqlite3_column_double(pStatement, 20);//ִ�м�
			InstrumentData.cOptionsType = ((const char *)sqlite3_column_text(pStatement, 21))[0];//��Ȩ����
			InstrumentData.dUnderlyingMultiple = sqlite3_column_double(pStatement, 22);//��Լ������Ʒ����

			InstrumentDatas.push_back(make_shared<InstrumentT>(InstrumentData));
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_finalize(pStatement);

}

void TradeData::GetInstruments(string szPartInstrumentId, vector<shared_ptr<InstrumentT>>&InstrumentDatas)
{
	string szSql = "select instrument_id,exchange_id,instrument_name,product_id,  \
        product_class,delivery_year,delivery_month,max_market_order_volume,min_market_order_volume, \
	    max_limit_order_volume,min_limit_order_volume,volume_multiple,price_tick,expire_date,\
	    is_trading,position_type,position_date_type,long_margin_ratio,short_margin_ratio,\
        max_margin_side_algorithm,strike_price,options_type,underlying_multiple  \
		from td_instrument ";

	//szSql = szSql + "  where instrument_id like '" + szPartInstrumentId + "%'";
	szSql = szSql + "  where substr(instrument_id ,1, " + to_string(szPartInstrumentId.length()) + ")='" + szPartInstrumentId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentT   InstrumentData;
			InstrumentData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����
			InstrumentData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������
			InstrumentData.szInstrumentName = (const char *)sqlite3_column_text(pStatement, 2);//��Լ����
			InstrumentData.szProductId = (const char *)sqlite3_column_text(pStatement, 3);//��Ʒ����
			InstrumentData.szProductClass = (const char *)sqlite3_column_text(pStatement, 4); //��Ʒ����
			InstrumentData.iDeliveryYear = sqlite3_column_int(pStatement, 5); //�������
			InstrumentData.iDeliveryMonth = sqlite3_column_int(pStatement, 6); //������
			InstrumentData.iMaxMarketOrderVolume = sqlite3_column_int(pStatement, 7);//�м۵�����µ���
			InstrumentData.iMinMarketOrderVolume = sqlite3_column_int(pStatement, 8);//�м۵���С�µ���
			InstrumentData.iMaxLimitOrderVolume = sqlite3_column_int(pStatement, 9);//�޼۵�����µ���
			InstrumentData.iMinLimitOrderVolume = sqlite3_column_int(pStatement, 10);//�޼۵���С�µ���
			InstrumentData.iVolumeMultiple = sqlite3_column_int(pStatement, 11);//��Լ��������
			InstrumentData.dPriceTick = sqlite3_column_double(pStatement, 12);//��С�䶯��λ
			InstrumentData.szExpireDate = (const char *)sqlite3_column_text(pStatement, 13);//������
			InstrumentData.iIsTrading = sqlite3_column_int(pStatement, 14);//��ǰ�Ƿ���
			InstrumentData.szPositionType = (const char *)sqlite3_column_text(pStatement, 15);//�ֲ����� 1-���ֲ� 2-�ۺϳֲ�
			InstrumentData.szPositionDateType = (const char *)sqlite3_column_text(pStatement, 16);//�ֲ��������� 1-ʹ����ʷ�ֲ� 2-��ʹ����ʷ�ֲ�
			InstrumentData.dLongMarginRatio = sqlite3_column_double(pStatement, 17);//��ͷ��֤����
			InstrumentData.dShortMarginRatio = sqlite3_column_double(pStatement, 18);//��ͷ��֤����
			InstrumentData.szMaxMarginSideAlgorithm = (const char *)sqlite3_column_text(pStatement, 19);//�Ƿ�ʹ�ô��߱�֤���㷨
			InstrumentData.dStrikePrice = sqlite3_column_double(pStatement, 20);//ִ�м�
			InstrumentData.cOptionsType = ((const char *)sqlite3_column_text(pStatement, 21))[0];//��Ȩ����
			InstrumentData.dUnderlyingMultiple = sqlite3_column_double(pStatement, 22);//��Լ������Ʒ����

			InstrumentDatas.push_back(make_shared<InstrumentT>(InstrumentData));
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_finalize(pStatement);

}
void TradeData::GetInstruments(string szInstrumentId, InstrumentT&InstrumentData)
{
	string szSql = "select instrument_id,exchange_id,instrument_name,product_id,  \
        product_class,delivery_year,delivery_month,max_market_order_volume,min_market_order_volume, \
	    max_limit_order_volume,min_limit_order_volume,volume_multiple,price_tick,expire_date,\
	    is_trading,position_type,position_date_type,long_margin_ratio,short_margin_ratio,\
        max_margin_side_algorithm,strike_price,options_type,underlying_multiple  from td_instrument";

	szSql = szSql + "  where instrument_id = '" + szInstrumentId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����
			InstrumentData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������
			InstrumentData.szInstrumentName = (const char *)sqlite3_column_text(pStatement, 2);//��Լ����
			InstrumentData.szProductId = (const char *)sqlite3_column_text(pStatement, 3);//��Ʒ����
			InstrumentData.szProductClass = (const char *)sqlite3_column_text(pStatement, 4); //��Ʒ����
			InstrumentData.iDeliveryYear = sqlite3_column_int(pStatement, 5); //�������
			InstrumentData.iDeliveryMonth = sqlite3_column_int(pStatement, 6); //������
			InstrumentData.iMaxMarketOrderVolume = sqlite3_column_int(pStatement, 7);//�м۵�����µ���
			InstrumentData.iMinMarketOrderVolume = sqlite3_column_int(pStatement, 8);//�м۵���С�µ���
			InstrumentData.iMaxLimitOrderVolume = sqlite3_column_int(pStatement, 9);//�޼۵�����µ���
			InstrumentData.iMinLimitOrderVolume = sqlite3_column_int(pStatement, 10);//�޼۵���С�µ���
			InstrumentData.iVolumeMultiple = sqlite3_column_int(pStatement, 11);//��Լ��������
			InstrumentData.dPriceTick = sqlite3_column_double(pStatement, 12);//��С�䶯��λ
			InstrumentData.szExpireDate = (const char *)sqlite3_column_text(pStatement, 13);//������
			InstrumentData.iIsTrading = sqlite3_column_int(pStatement, 14);//��ǰ�Ƿ���
			InstrumentData.szPositionType = (const char *)sqlite3_column_text(pStatement, 15);//�ֲ����� 1-���ֲ� 2-�ۺϳֲ�
			InstrumentData.szPositionDateType = (const char *)sqlite3_column_text(pStatement, 16);//�ֲ��������� 1-ʹ����ʷ�ֲ� 2-��ʹ����ʷ�ֲ�
			InstrumentData.dLongMarginRatio = sqlite3_column_double(pStatement, 17);//��ͷ��֤����
			InstrumentData.dShortMarginRatio = sqlite3_column_double(pStatement, 18);//��ͷ��֤����
			InstrumentData.szMaxMarginSideAlgorithm = (const char *)sqlite3_column_text(pStatement, 19);//�Ƿ�ʹ�ô��߱�֤���㷨
			InstrumentData.dStrikePrice = sqlite3_column_double(pStatement, 20);//ִ�м�
			InstrumentData.cOptionsType = ((const char *)sqlite3_column_text(pStatement, 21))[0];//��Ȩ����
			InstrumentData.dUnderlyingMultiple = sqlite3_column_double(pStatement, 22);//��Լ������Ʒ����

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_finalize(pStatement);

}
void TradeData::GetInstrumentMarginRates(vector<shared_ptr<InstrumentMarginRate>>&InstrumentMarginRateDatas)
{
	string szSql = "select instrument_id,exchange_id,hedge_flag,long_margin_ratio_by_money,long_margin_ratio_by_volume,\
        short_margin_ratio_by_money,short_margin_ratio_by_volume,is_relative from td_instrument_margin_rate";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentMarginRate InstrumentMarginRateData;

			InstrumentMarginRateData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0);   //��Լ����	
			InstrumentMarginRateData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1);   //����������	
			InstrumentMarginRateData.cHedgeFlag = ((const char *)sqlite3_column_text(pStatement, 2))[0];  ///Ͷ���ױ���־	
			InstrumentMarginRateData.dLongMarginRatioByMoney = sqlite3_column_double(pStatement, 3);  ///��ͷ��֤����	
			InstrumentMarginRateData.dLongMarginRatioByVolume = sqlite3_column_double(pStatement, 4);  ///��ͷ��֤���	
			InstrumentMarginRateData.dShortMarginRatioByMoney = sqlite3_column_double(pStatement, 5);  ///��ͷ��֤����	
			InstrumentMarginRateData.dShortMarginRatioByVolume = sqlite3_column_double(pStatement, 6);  ///��ͷ��֤���	
			InstrumentMarginRateData.iIsRelative = sqlite3_column_int(pStatement, 7);  ///�Ƿ���Խ�������ȡ

			InstrumentMarginRateDatas.push_back(make_shared<InstrumentMarginRate>(InstrumentMarginRateData));
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}
void TradeData::GetInstrumentCommissionRates(vector<shared_ptr<InstrumentCommissionRate>>&InstrumentCommissionRateDatas)
{
	string szSql = "select instrument_id,exchange_id,open_ratio_by_money,open_ratio_by_volume,close_ratio_by_money,\
        close_ratio_by_volume,close_today_ratio_by_money,close_today_ratio_by_volume,biz_type from td_instrument_commission_rate";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentCommissionRate InstrumentCommissionRateData;

			InstrumentCommissionRateData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����	
			InstrumentCommissionRateData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������	
			InstrumentCommissionRateData.dOpenRatioByMoney = sqlite3_column_double(pStatement, 2); ///������������	
			InstrumentCommissionRateData.dOpenRatioByVolume = sqlite3_column_double(pStatement, 3); ///����������	
			InstrumentCommissionRateData.dCloseRatioByMoney = sqlite3_column_double(pStatement, 4); ///ƽ����������	
			InstrumentCommissionRateData.dCloseRatioByVolume = sqlite3_column_double(pStatement, 5); ///ƽ��������	
			InstrumentCommissionRateData.dCloseTodayRatioByMoney = sqlite3_column_double(pStatement, 6);  ///ƽ����������	
			InstrumentCommissionRateData.dCloseTodayRatioByVolume = sqlite3_column_double(pStatement, 7); ///ƽ��������	
			InstrumentCommissionRateData.cBizType = sqlite3_column_int(pStatement, 8); ///ҵ������

			InstrumentCommissionRateDatas.push_back(make_shared<InstrumentCommissionRate>(InstrumentCommissionRateData));
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}
void TradeData::GetInstrumentCommissionRates(string szInstrumentId, InstrumentCommissionRate& InstrumentCommissionRateData)
{
	string szSql = "select instrument_id,exchange_id,open_ratio_by_money,open_ratio_by_volume,close_ratio_by_money,\
        close_ratio_by_volume,close_today_ratio_by_money,close_today_ratio_by_volume,biz_type \
        from td_instrument_commission_rate  where instrument_id='" + szInstrumentId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentCommissionRateData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����	
			InstrumentCommissionRateData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������	
			InstrumentCommissionRateData.dOpenRatioByMoney = sqlite3_column_double(pStatement, 2); ///������������	
			InstrumentCommissionRateData.dOpenRatioByVolume = sqlite3_column_double(pStatement, 3); ///����������	
			InstrumentCommissionRateData.dCloseRatioByMoney = sqlite3_column_double(pStatement, 4); ///ƽ����������	
			InstrumentCommissionRateData.dCloseRatioByVolume = sqlite3_column_double(pStatement, 5); ///ƽ��������	
			InstrumentCommissionRateData.dCloseTodayRatioByMoney = sqlite3_column_double(pStatement, 6);  ///ƽ����������	
			InstrumentCommissionRateData.dCloseTodayRatioByVolume = sqlite3_column_double(pStatement, 7); ///ƽ��������	
			InstrumentCommissionRateData.cBizType = sqlite3_column_int(pStatement, 8); ///ҵ������

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}
void TradeData::GetInstrumentOrderCommRates(vector<shared_ptr<InstrumentOrderCommRate>>&InstrumentOrderCommRateDatas)
{
	string szSql = "select instrument_id,exchange_id,hedge_flag,order_comm_by_volume,order_action_comm_by_volume  from td_instrument_order_comm_rate";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentOrderCommRate InstrumentOrderCommRateData;

			InstrumentOrderCommRateData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����	
			InstrumentOrderCommRateData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������	
			InstrumentOrderCommRateData.cHedgeFlag = ((const char *)sqlite3_column_text(pStatement, 2))[0]; ///Ͷ���ױ���־	
			InstrumentOrderCommRateData.dOrderCommByVolume = sqlite3_column_double(pStatement, 3); ///����������	
			InstrumentOrderCommRateData.dOrderActionCommByVolume = sqlite3_column_double(pStatement, 4); ///����������

			InstrumentOrderCommRateDatas.push_back(make_shared<InstrumentOrderCommRate>(InstrumentOrderCommRateData));
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

void TradeData::GetInstrumentOrderCommRates(string szInstrumentId, InstrumentOrderCommRate& InstrumentOrderCommRateData)
{
	string szSql = "select instrument_id,exchange_id,hedge_flag,order_comm_by_volume,order_action_comm_by_volume  \
        from td_instrument_order_comm_rate where instrument_id='" + szInstrumentId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			InstrumentOrderCommRateData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //��Լ����	
			InstrumentOrderCommRateData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //����������	
			InstrumentOrderCommRateData.cHedgeFlag = ((const char *)sqlite3_column_text(pStatement, 2))[0]; ///Ͷ���ױ���־	
			InstrumentOrderCommRateData.dOrderCommByVolume = sqlite3_column_double(pStatement, 3); ///����������	
			InstrumentOrderCommRateData.dOrderActionCommByVolume = sqlite3_column_double(pStatement, 4); ///����������
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

void TradeData::GetCommissionOpenCount(const int iDate, const string &szUserId, vector<shared_ptr<OpenCountT>>&CommissionOpenCountDatas)
{
	string szSql = "select init_date,td_type,user_id,exchange_id,stock_code,direction,open_count from td_commission_open_count  "
		+ string("  where") + " init_date=" + to_string(iDate)
		+ " and  user_id='" + szUserId + "'";

	sqlite3_stmt *pStatement = nullptr;
	int iRet = sqlite3_prepare_v2(SqliteDb, szSql.c_str(), -1, &pStatement, NULL);

	if (SQLITE_OK == iRet)
	{
		// get results
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			OpenCountT CommissionOpenCountData;

			CommissionOpenCountData.iInitDate = sqlite3_column_int(pStatement, 0);
			CommissionOpenCountData.iTdType = sqlite3_column_int(pStatement, 1);
			CommissionOpenCountData.szUserId = (const char *)sqlite3_column_text(pStatement, 2);
			CommissionOpenCountData.szExchangeID = (const char *)sqlite3_column_text(pStatement, 3);
			CommissionOpenCountData.szStockCode = (const char *)sqlite3_column_text(pStatement, 4);
			CommissionOpenCountData.cDirection = ((const char *)sqlite3_column_text(pStatement, 5))[0];
			CommissionOpenCountData.iOpenCount = sqlite3_column_int(pStatement, 6);

			CommissionOpenCountDatas.push_back(make_shared<OpenCountT>(CommissionOpenCountData));

		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}
	sqlite3_finalize(pStatement);

}

#include "trade_data.h"
#include "sim_log.h"
#include "config.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
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
			InstrumentData.szInstrumentId = (const char *)sqlite3_column_text(pStatement, 0); //合约代码
			InstrumentData.szExchangeId = (const char *)sqlite3_column_text(pStatement, 1); //交易所代码
			InstrumentData.szInstrumentName = (const char *)sqlite3_column_text(pStatement, 2);//合约名称
			InstrumentData.szProductId = (const char *)sqlite3_column_text(pStatement, 3);//产品代码
			InstrumentData.szProductClass = (const char *)sqlite3_column_text(pStatement, 4); //产品类型
			InstrumentData.iDeliveryYear = sqlite3_column_int(pStatement, 5); //交割年份
			InstrumentData.iDeliveryMonth = sqlite3_column_int(pStatement, 6); //交割月
			InstrumentData.iMaxMarketOrderVolume = sqlite3_column_int(pStatement, 7);//市价单最大下单量
			InstrumentData.iMinMarketOrderVolume = sqlite3_column_int(pStatement, 8);//市价单最小下单量
			InstrumentData.iMaxLimitOrderVolume = sqlite3_column_int(pStatement, 9);//限价单最大下单量
			InstrumentData.iMinLimitOrderVolume = sqlite3_column_int(pStatement, 10);//限价单最小下单量
			InstrumentData.iVolumeMultiple = sqlite3_column_int(pStatement, 11);//合约数量乘数
			InstrumentData.dPriceTick = sqlite3_column_double(pStatement, 12);//最小变动价位
			InstrumentData.szExpireDate = (const char *)sqlite3_column_text(pStatement, 13);//到期日
			InstrumentData.iIsTrading = sqlite3_column_int(pStatement, 14);//当前是否交易
			InstrumentData.szPositionType = (const char *)sqlite3_column_text(pStatement, 15);//持仓类型 1-净持仓 2-综合持仓
			InstrumentData.szPositionDateType = (const char *)sqlite3_column_text(pStatement, 16);//持仓日期类型 1-使用历史持仓 2-不使用历史持仓
			InstrumentData.dLongMarginRatio = sqlite3_column_double(pStatement, 17);//多头保证金率
			InstrumentData.dShortMarginRatio = sqlite3_column_double(pStatement, 18);//空头保证金率
			InstrumentData.szMaxMarginSideAlgorithm = (const char *)sqlite3_column_text(pStatement, 19);//是否使用大额单边保证金算法
			InstrumentData.dStrikePrice = sqlite3_column_double(pStatement, 20);//执行价
			InstrumentData.cOptionsType = ((const char *)sqlite3_column_text(pStatement, 21))[0];//期权类型
			InstrumentData.dUnderlyingMultiple = sqlite3_column_double(pStatement, 22);//合约基础商品乘数

			InstrumentDatas.push_back(make_shared<InstrumentT>(InstrumentData));
		}
	}
	else
	{
		LError("failed to exec sql, sql=[{0}],error=[{1}]", szSql, sqlite3_errmsg(SqliteDb));
	}

	sqlite3_finalize(pStatement);

}

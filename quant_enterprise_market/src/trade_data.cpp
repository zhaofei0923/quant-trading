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

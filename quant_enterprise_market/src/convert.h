#ifndef _CONVERT_H
#define _CONVERT_H
#include <string>
#include <string.h>
#include "sim_log.h"
#include "const.h"
#include "helper_tools.h"
#include "message.h"
#include "struct.h"
#include "ThostFtdcMdApi.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

inline  int ConvertMarketData(const CThostFtdcDepthMarketDataField *pDepthMarketData, MarketData*pMarketData)
{

	memset(pMarketData, 0, sizeof(MarketData));

	sprintf(pMarketData->szTradingDay, "%s", pDepthMarketData->TradingDay);
	sprintf(pMarketData->szActionDay, "%s", pDepthMarketData->ActionDay);
	sprintf(pMarketData->szStockCode, "%s", pDepthMarketData->InstrumentID);
	sprintf(pMarketData->szExchangeId, "%s", pDepthMarketData->ExchangeID);
	sprintf(pMarketData->szUpdateTime, "%s", pDepthMarketData->UpdateTime);
	pMarketData->iUpdateMillisec = pDepthMarketData->UpdateMillisec;

	pMarketData->iLastPrice = zutil::ConvertDoubleToInt(pDepthMarketData->LastPrice, PRICE_MULTIPLE);
	pMarketData->iPreSettlePrice = zutil::ConvertDoubleToInt(pDepthMarketData->PreSettlementPrice, PRICE_MULTIPLE);
	pMarketData->iPreClosePrice = zutil::ConvertDoubleToInt(pDepthMarketData->PreClosePrice, PRICE_MULTIPLE);
	pMarketData->iPreOpenInterest = pDepthMarketData->PreOpenInterest;
	pMarketData->iOpenPrice = zutil::ConvertDoubleToInt(pDepthMarketData->OpenPrice, PRICE_MULTIPLE);
	pMarketData->iLowPrice = zutil::ConvertDoubleToInt(pDepthMarketData->LowestPrice, PRICE_MULTIPLE);
	pMarketData->iHighPrice = zutil::ConvertDoubleToInt(pDepthMarketData->HighestPrice, PRICE_MULTIPLE);
	pMarketData->iVolume = pDepthMarketData->Volume;
	pMarketData->iTurnOver = zutil::ConvertDoubleToInt(pDepthMarketData->Turnover, PRICE_MULTIPLE);
	pMarketData->iOpenInterest = pDepthMarketData->OpenInterest;
	pMarketData->iClosePrice = zutil::ConvertDoubleToInt(pDepthMarketData->ClosePrice, PRICE_MULTIPLE);
	pMarketData->iSettlePrice = zutil::ConvertDoubleToInt(pDepthMarketData->SettlementPrice, PRICE_MULTIPLE);
	pMarketData->iUpperLimitPrice = zutil::ConvertDoubleToInt(pDepthMarketData->UpperLimitPrice, PRICE_MULTIPLE);
	pMarketData->iLowerLimitPrice = zutil::ConvertDoubleToInt(pDepthMarketData->LowerLimitPrice, PRICE_MULTIPLE);
	pMarketData->iPreDelta = zutil::ConvertDoubleToInt(pDepthMarketData->PreDelta, PRICE_MULTIPLE);
	pMarketData->iCurrDelta = zutil::ConvertDoubleToInt(pDepthMarketData->CurrDelta, PRICE_MULTIPLE);
	pMarketData->iAveragePrice = zutil::ConvertDoubleToInt(pDepthMarketData->AveragePrice, PRICE_MULTIPLE);

	pMarketData->iSellPrice1 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice1, PRICE_MULTIPLE);
	pMarketData->iSellPrice2 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice2, PRICE_MULTIPLE);
	pMarketData->iSellPrice3 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice3, PRICE_MULTIPLE);
	pMarketData->iSellPrice4 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice4, PRICE_MULTIPLE);
	pMarketData->iSellPrice5 = zutil::ConvertDoubleToInt(pDepthMarketData->AskPrice5, PRICE_MULTIPLE);
	pMarketData->iBuyPrice1 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice1, PRICE_MULTIPLE);
	pMarketData->iBuyPrice2 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice2, PRICE_MULTIPLE);
	pMarketData->iBuyPrice3 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice3, PRICE_MULTIPLE);
	pMarketData->iBuyPrice4 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice4, PRICE_MULTIPLE);
	pMarketData->iBuyPrice5 = zutil::ConvertDoubleToInt(pDepthMarketData->BidPrice5, PRICE_MULTIPLE);
	pMarketData->iSellQuantity1 = pDepthMarketData->AskVolume1;
	pMarketData->iSellQuantity2 = pDepthMarketData->AskVolume2;
	pMarketData->iSellQuantity3 = pDepthMarketData->AskVolume3;
	pMarketData->iSellQuantity4 = pDepthMarketData->AskVolume4;
	pMarketData->iSellQuantity5 = pDepthMarketData->AskVolume5;
	pMarketData->iBuyQuantity1 = pDepthMarketData->BidVolume1;
	pMarketData->iBuyQuantity2 = pDepthMarketData->BidVolume2;
	pMarketData->iBuyQuantity3 = pDepthMarketData->BidVolume3;
	pMarketData->iBuyQuantity4 = pDepthMarketData->BidVolume4;
	pMarketData->iBuyQuantity5 = pDepthMarketData->BidVolume5;

	return 0;
}

#endif

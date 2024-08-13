#include "callback.h"
#include "sim_log.h"
#include "strategy.h"
#include "mem_pool_mng.h"

//-----------------------------------CallBack---------------------------------------------
CallBack::CallBack()
{
	m_iTradeReadKey = 0;
	m_iTradeWriteKey = 0;
	m_iMarketKey = 0;
	m_iMarketClientNum = 0;
}
CallBack::~CallBack()
{

}

void CallBack::OnReceive(Head*pHead, char*pBody, int iBodyLength)
{
	// printf("----------------CallBack::OnReceive----------------------\n");
	// printf("iFunctionId=%d\n", pHead->iFunctionId);
	// printf("iErrorCode=%d\n", pHead->iErrorCode);
	// printf("szErrorInfo=%s\n", pHead->szErrorInfo);
	// printf("pHead->iBodyLength=%d\n", pHead->iBodyLength);
	// printf("iBodyLength=%d\n", iBodyLength);
	// printf("bIsLast=%d\n", pHead->bIsLast);
	// printf("iReqId=%d\n", pHead->iReqId);

	if (pHead->iErrorCode != 0 || 0 == iBodyLength)
	{
		return;
	}

	if (FUNC_QUERY_BALANCE == pHead->iFunctionId)
	{
		QueryBalanceResT*pQueryBalanceResT = (QueryBalanceResT*)pBody;
		printf("----------------FUNC_QUERY_BALANCE----------------------\n");
		printf("iUseMargin=%lld\n", pQueryBalanceResT->iUseMargin);
		printf("iFrozenMargin=%lld\n", pQueryBalanceResT->iFrozenMargin);
		printf("iFrozenCash=%lld\n", pQueryBalanceResT->iFrozenCash);
		printf("iFrozenCommission=%lld\n", pQueryBalanceResT->iFrozenCommission);
		printf("iCurrMargin=%lld\n", pQueryBalanceResT->iCurrMargin);
		printf("iCommission=%lld\n", pQueryBalanceResT->iCommission);
		std::cout << "iAvailable=" << pQueryBalanceResT->iAvailable << endl;

	}
	else if (FUNC_QUERY_ORDER == pHead->iFunctionId)
	{
		printf("----------------FUNC_QUERY_ORDER----------------------\n");
		QueryOrderResT*pQueryOrderResT = (QueryOrderResT*)pBody;
		printf("iInitDate=%d\n", pQueryOrderResT->OrderData.iInitDate);
		printf("iInitTime=%d\n", pQueryOrderResT->OrderData.iInitTime);
		printf("szJysInsertDate=%s\n", pQueryOrderResT->OrderData.szJysInsertDate);
		printf("szJysInsertTime=%s\n", pQueryOrderResT->OrderData.szJysInsertTime);
		printf("szUserId=%s\n", pQueryOrderResT->OrderData.szUserId);
		printf("szExchangeID=%s\n", pQueryOrderResT->OrderData.szExchangeID);
		printf("szStockCode=%s\n", pQueryOrderResT->OrderData.szStockCode);
		printf("szOrderRef=%s\n", pQueryOrderResT->OrderData.szOrderRef);
		printf("szOrderSysID=%s\n", pQueryOrderResT->OrderData.szOrderSysID);
		printf("cAction=%c\n", pQueryOrderResT->OrderData.cAction);
		printf("iPrice=%lld\n", pQueryOrderResT->OrderData.iPrice);
		printf("iEntrustNum=%d\n", pQueryOrderResT->OrderData.iEntrustNum);
		printf("iTradeNum=%d\n", pQueryOrderResT->OrderData.iTradeNum);
		printf("cStatus=%c\n", pQueryOrderResT->OrderData.cStatus);

	}
	else if (FUNC_QUERY_TRADE == pHead->iFunctionId)
	{
		printf("----------------FUNC_QUERY_TRADE----------------------\n");
		QueryTradeResT*pQueryTradeResT = (QueryTradeResT*)pBody;
		printf("iInitDate=%d\n", pQueryTradeResT->TradeData.iInitDate);
		printf("iInitTime=%d\n", pQueryTradeResT->TradeData.iInitTime);
		printf("szJysTradeDate=%s\n", pQueryTradeResT->TradeData.szJysTradeDate);
		printf("szJysTradeTime=%s\n", pQueryTradeResT->TradeData.szJysTradeTime);
		printf("szUserId=%s\n", pQueryTradeResT->TradeData.szUserId);
		printf("szExchangeID=%s\n", pQueryTradeResT->TradeData.szExchangeID);
		printf("szStockCode=%s\n", pQueryTradeResT->TradeData.szStockCode);
		printf("szOrderRef=%s\n", pQueryTradeResT->TradeData.szOrderRef);
		printf("szOrderSysID=%s\n", pQueryTradeResT->TradeData.szOrderSysID);
		printf("szTradeID=%s\n", pQueryTradeResT->TradeData.szTradeID);
		printf("cAction=%c\n", pQueryTradeResT->TradeData.cAction);
		printf("iPrice=%lld\n", pQueryTradeResT->TradeData.iPrice);
		printf("iTradeNum=%d\n", pQueryTradeResT->TradeData.iTradeNum);
	}
	else if (FUNC_QUERY_POSITION == pHead->iFunctionId)
	{
		printf("----------------FUNC_QUERY_POSITION----------------------\n");
		QueryPositionResT*pQueryPositionResT = (QueryPositionResT*)pBody;

		printf("iInitDate=%d\n", pQueryPositionResT->PositionData.iInitDate);
		printf("szUserId=%s\n", pQueryPositionResT->PositionData.szUserId);
		printf("szExchangeID=%s\n", pQueryPositionResT->PositionData.szExchangeID);
		printf("szStockCode=%s\n", pQueryPositionResT->PositionData.szStockCode);
		printf("cDirection=%c\n", pQueryPositionResT->PositionData.cDirection);
		printf("cHedgeFlag=%c\n", pQueryPositionResT->PositionData.cHedgeFlag);
		printf("iYdPosition=%d\n", pQueryPositionResT->PositionData.iYdPosition);
		printf("iTodayPosition=%d\n", pQueryPositionResT->PositionData.iTodayPosition);
		printf("iPosition=%d\n", pQueryPositionResT->PositionData.iPosition);
		printf("iLongFrozen=%lld\n", pQueryPositionResT->PositionData.iLongFrozen);
		printf("iShortFrozen=%lld\n", pQueryPositionResT->PositionData.iShortFrozen);
		printf("iUseMargin=%lld\n", pQueryPositionResT->PositionData.iUseMargin);
		printf("iFrozenMargin=%lld\n", pQueryPositionResT->PositionData.iFrozenMargin);
		printf("iFrozenCash=%lld\n", pQueryPositionResT->PositionData.iFrozenCash);
		printf("iFrozenCommission=%lld\n", pQueryPositionResT->PositionData.iFrozenCommission);
		printf("iCommission=%lld\n", pQueryPositionResT->PositionData.iCommission);
		printf("iPreSettlementPrice=%lld\n", pQueryPositionResT->PositionData.iPreSettlementPrice);
		printf("iSettlementPrice=%lld\n", pQueryPositionResT->PositionData.iSettlementPrice);

	}
	else if (FUNC_PLACE_ORDER == pHead->iFunctionId)
	{
		printf("----------------FUNC_PLACE_ORDER----------------------\n");
		PlaceOrderResT*pPlaceOrderResT = (PlaceOrderResT*)pBody;
		printf("szOrderRef=%s\n", pPlaceOrderResT->szOrderRef);

	}
	else if (FUNC_CANCEL_ORDER == pHead->iFunctionId)
	{
		printf("----------------FUNC_CANCEL_ORDER----------------------\n");

	}
	else if (FUNC_ADD_MSG_QUEUE == pHead->iFunctionId)
	{
		printf("----------------FUNC_ADD_MSG_QUEUE----------------------\n");
		AddMsgQueueResT*pAddMsgQueueResT = (AddMsgQueueResT*)pBody;
		printf("iReadKey=%d\n", pAddMsgQueueResT->iReadKey);
		printf("iWriteKey=%d\n", pAddMsgQueueResT->iWriteKey);
		//客户端和服务端的读写key相反
		m_iTradeWriteKey = pAddMsgQueueResT->iReadKey;
		m_iTradeReadKey = pAddMsgQueueResT->iWriteKey;

	}
	else if (FUNC_DEL_MSG_QUEUE == pHead->iFunctionId)
	{
		printf("----------------FUNC_DEL_MSG_QUEUE----------------------\n");

	}
	else if (FUNC_ORDER_NOTIFY == pHead->iFunctionId)
	{
		printf("----------------FUNC_ORDER_NOTIFY----------------------\n");
		OrderNotifyT*pOrderNotifyT = (OrderNotifyT*)pBody;

		OrderDetail*pOrderDetail = &(pOrderNotifyT->OrderData);
		printf("iInitDate=%d\n", pOrderDetail->iInitDate);
		printf("iInitTime=%d\n", pOrderDetail->iInitTime);
		printf("szJysInsertDate=%s\n", pOrderDetail->szJysInsertDate);
		printf("szJysInsertTime=%s\n", pOrderDetail->szJysInsertTime);
		printf("szUserId=%s\n", pOrderDetail->szUserId);
		printf("szExchangeID=%s\n", pOrderDetail->szExchangeID);
		printf("szStockCode=%s\n", pOrderDetail->szStockCode);
		printf("szOrderRef=%s\n", pOrderDetail->szOrderRef);
		printf("szOrderSysID=%s\n", pOrderDetail->szOrderSysID);
		printf("cAction=%c\n", pOrderDetail->cAction);
		printf("iPrice=%lld\n", pOrderDetail->iPrice);
		printf("iEntrustNum=%d\n", pOrderDetail->iEntrustNum);
		printf("iTradeNum=%d\n", pOrderDetail->iTradeNum);
		printf("cStatus=%c\n", pOrderDetail->cStatus);
	}
	else if (FUNC_TRADE_NOTIFY == pHead->iFunctionId)
	{
		printf("----------------FUNC_TRADE_NOTIFY----------------------\n");
		TradeNotifyT*pTradeNotifyT = (TradeNotifyT*)pBody;

		TradeDetail*pTradeDetail = &(pTradeNotifyT->TradeData);
		printf("iInitDate=%d\n", pTradeDetail->iInitDate);
		printf("iInitTime=%d\n", pTradeDetail->iInitTime);
		printf("szJysTradeDate=%s\n", pTradeDetail->szJysTradeDate);
		printf("szJysTradeTime=%s\n", pTradeDetail->szJysTradeTime);
		printf("szUserId=%s\n", pTradeDetail->szUserId);
		printf("szExchangeID=%s\n", pTradeDetail->szExchangeID);
		printf("szStockCode=%s\n", pTradeDetail->szStockCode);
		printf("szOrderRef=%s\n", pTradeDetail->szOrderRef);
		printf("szOrderSysID=%s\n", pTradeDetail->szOrderSysID);
		printf("szTradeID=%s\n", pTradeDetail->szTradeID);
		printf("cAction=%c\n", pTradeDetail->cAction);
		printf("iPrice=%lld\n", pTradeDetail->iPrice);
		printf("iTradeNum=%d\n", pTradeDetail->iTradeNum);
	}
	else if (FUNC_PLACE_ORDER_ERR_NOTIFY == pHead->iFunctionId)
	{
		printf("----------------FUNC_PLACE_ORDER_ERR_NOTIFY----------------------\n");
		PlaceOrderErrNotifyT*pPlaceOrderErrNotifyT = (PlaceOrderErrNotifyT*)pBody;
		printf("szUserId=%s\n", pPlaceOrderErrNotifyT->szUserId);
		printf("szClientOrderId=%s\n", pPlaceOrderErrNotifyT->szClientOrderId);
		printf("szOrderRef=%s\n", pPlaceOrderErrNotifyT->szOrderRef);
		printf("iErrorCode=%d\n", pPlaceOrderErrNotifyT->iErrorCode);
		printf("szErrorInfo=%s\n", pPlaceOrderErrNotifyT->szErrorInfo);
	}
	else if (FUNC_CANCEL_ORDER_ERR_NOTIFY == pHead->iFunctionId)
	{
		printf("----------------FUNC_CANCEL_ORDER_ERR_NOTIFY----------------------\n");
		CancelOrderErrNotifyT*pCancelOrderErrNotifyT = (CancelOrderErrNotifyT*)pBody;
		printf("szUserId=%s\n", pCancelOrderErrNotifyT->szUserId);
		printf("szClientOrderId=%s\n", pCancelOrderErrNotifyT->szClientOrderId);
		printf("szOrderRef=%s\n", pCancelOrderErrNotifyT->szOrderRef);
		printf("iErrorCode=%d\n", pCancelOrderErrNotifyT->iErrorCode);
		printf("szErrorInfo=%s\n", pCancelOrderErrNotifyT->szErrorInfo);
	}
	else if (FUNC_SUB_QUOTE == pHead->iFunctionId)
	{
		printf("----------------FUNC_SUB_QUOTE----------------------\n");
		SubQuoteResT*pSubQuoteResT = (SubQuoteResT*)pBody;
		printf("iClientNum=%d\n", pSubQuoteResT->iClientNum);
		printf("iDataKey=%d\n", pSubQuoteResT->iDataKey);

		m_iMarketClientNum = pSubQuoteResT->iClientNum;
		m_iMarketKey = pSubQuoteResT->iDataKey;


	}
	else if (FUNC_CANCEL_SUB_QUOTE == pHead->iFunctionId)
	{
		printf("----------------FUNC_CANCEL_SUB_QUOTE----------------------\n");

	}
	else if (FUNC_MARKET_QUOTE == pHead->iFunctionId)
	{
		//printf("----------------FUNC_MARKET_QUOTE----------------------\n");
		MarketData*pMarketData = (MarketData*)pBody;
		//printf("szExchangeId=%s\n", pMarketData->szExchangeId);
		// printf("szStockCode=%s\n", pMarketData->szStockCode);
		// printf("iLastPrice=%d\n", pMarketData->iLastPrice);
		//printf("iPreSettlePrice=%d\n", pMarketData->iPreSettlePrice);
		//printf("iPreClosePrice=%d\n", pMarketData->iPreClosePrice);

		/*
		LDebug("szExchangeId=[{0}]", pMarketData->szExchangeId);
		LDebug("szStockCode=[{0}]", pMarketData->szStockCode);
		LDebug("szStockName=[{0}]", pMarketData->szStockName);
		LDebug("szTime=[{0}]", pMarketData->szTime);
		LDebug("iLastPrice=[{0}]", pMarketData->iLastPrice);
		LDebug("iPreSettlePrice=[{0}]", pMarketData->iPreSettlePrice);
		LDebug("iPreClosePrice=[{0}]", pMarketData->iPreClosePrice);
		LDebug("iOpenPrice=[{0}]", pMarketData->iOpenPrice);
		LDebug("iLowPrice=[{0}]", pMarketData->iLowPrice);
		LDebug("iHighPrice=[{0}]", pMarketData->iHighPrice);
		LDebug("iClosePrice=[{0}]", pMarketData->iClosePrice);
		LDebug("iVolume=[{0}]", pMarketData->iVolume);
		LDebug("iTurnOver=[{0}]", pMarketData->iTurnOver);
		LDebug("iSellPrice1=[{0}]", pMarketData->iSellPrice1);
		LDebug("iSellPrice2=[{0}]", pMarketData->iSellPrice2);
		LDebug("iSellPrice3=[{0}]", pMarketData->iSellPrice3);
		LDebug("iSellPrice4=[{0}]", pMarketData->iSellPrice4);
		LDebug("iSellPrice5=[{0}]", pMarketData->iSellPrice5);
		LDebug("iBuyPrice1=[{0}]", pMarketData->iBuyPrice1);
		LDebug("iBuyPrice2=[{0}]", pMarketData->iBuyPrice2);
		LDebug("iBuyPrice3=[{0}]", pMarketData->iBuyPrice3);
		LDebug("iBuyPrice4=[{0}]", pMarketData->iBuyPrice4);
		LDebug("iBuyPrice5=[{0}]", pMarketData->iBuyPrice5);
		LDebug("iSellQquantity1=[{0}]", pMarketData->iSellQuantity1);
		LDebug("iSellQquantity2=[{0}]", pMarketData->iSellQuantity2);
		LDebug("iSellQquantity3=[{0}]", pMarketData->iSellQuantity3);
		LDebug("iSellQquantity4=[{0}]", pMarketData->iSellQuantity4);
		LDebug("iSellQquantity5=[{0}]", pMarketData->iSellQuantity5);
		LDebug("iBuyQuantity1=[{0}]", pMarketData->iBuyQuantity1);
		LDebug("iBuyQuantity2=[{0}]", pMarketData->iBuyQuantity2);
		LDebug("iBuyQuantity3=[{0}]", pMarketData->iBuyQuantity3);
		LDebug("iBuyQuantity4=[{0}]", pMarketData->iBuyQuantity4);
		LDebug("iBuyQuantity5=[{0}]", pMarketData->iBuyQuantity5);
		*/

		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();

		//发送到CStrategy
		{
			char*pData = pMemPoolMng->GetMaxMemBlock(sizeof(MarketData));
			if (NULL == pData)
			{
				LError("pData is NULL!");
				return;
			}
			memcpy(pData, pMarketData, sizeof(MarketData));


			char*pMemBlock = pMemPoolMng->GetMaxMemBlock(sizeof(ServerMsg));
			if (NULL == pMemBlock)
			{
				LError("pMemBlock is NULL!");
				return;
			}
			ServerMsg*pServerMsg = (ServerMsg*)pMemBlock;
			pServerMsg->iFunctionId = FUNC_MARKET_QUOTE;
			pServerMsg->pBody = pData;
			pServerMsg->iBodyLen = sizeof(MarketData);

			CStrategy::GetInstance()->PostMsg(pServerMsg);
		}
	}
	else if (FUNC_ADD_DEL_MONEY == pHead->iFunctionId)
	{
		printf("----------------FUNC_ADD_DEL_MONEY----------------------\n");

	}
}

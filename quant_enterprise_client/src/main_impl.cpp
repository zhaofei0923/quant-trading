#include "main_impl.h"
#include "config.h"
#include "helper_tools.h"
#include "sim_log.h"
#include <chrono>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

int MainImpl::Init()
{

	ConfigData*pConfigData = ConfigData::GetInstance();
	if (false == pConfigData->LoadConfig())
	{
		return -1;
	}

	SimLog::Instance().InitSimLog(pConfigData->iLogLevel);

	//交易
	m_pTrade = new CTcpClient(pConfigData->szTradeIp, pConfigData->iTradePort);
	CallBack*pTdTcpCall = new CallBack();
	m_pTrade->SetCallBack(pTdTcpCall);
	m_pTrade->Init();

	//交易
	CallBack*pTdMqCall = new CallBack();
	m_pCMsgQueueTd = CMsgQueueTd::GetInstance();
	m_pCMsgQueueTd->SetCallBack(pTdMqCall);


	//行情
	m_pMarket = new CTcpClient(pConfigData->szMarketIp, pConfigData->iMarketPort);
	CallBack*pMdTcpCall = new CallBack();
	m_pMarket->SetCallBack(pMdTcpCall);
	m_pMarket->Init();

	//行情
	CallBack*pMdMqCall = new CallBack();
	m_pCMsgQueueMd = CMsgQueueMd::GetInstance();
	m_pCMsgQueueMd->SetCallBack(pMdMqCall);


	m_bIsInit = true;
	return 0;
}
int MainImpl::Start()
{
	if (false == m_bIsInit)
	{
		return -1;
	}


	if (m_pTrade->Connect() != 0)
	{
		return -1;
	}

	if (m_pTrade->Start() != 0)
	{
		return -1;
	}


	if (m_pMarket->Connect() != 0)
	{
		return -1;
	}

	if (m_pMarket->Start() != 0)
	{
		return -1;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
	printf("\n");
	printf("\n");
	printf("function list:\n");
	printf("1:AdminDemo()\n");
	printf("2:TraderQueryDemo()\n");
	printf("3:TraderQueryDemo2()\n");
	printf("4:TraderEntrustDemo()\n");
	printf("5:MarketDemo()\n");
	printf("please select function,please input num:");

	int iNum = 0;
	scanf("%d",&iNum);
	if (1 == iNum)
	{
		AdminDemo();
	}
	else if (2 == iNum)
	{
		TraderQueryDemo();
	}
	else if (3 == iNum)
	{
		TraderQueryDemo2();
	}
	else if (4 == iNum)
	{
		TraderEntrustDemo();
	}
	else if (5 == iNum)
	{
		MarketDemo();
	}


	return 0;
}
int MainImpl::Stop()
{
	return 0;
}
int MainImpl::Release()
{
	return 0;
}
//管理员接口测试
void MainImpl::AdminDemo()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 100;
	HeadBuf.iRoletype = ROLE_TYPE_ADMIN;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);


	printf("------press Enter key to login------\n");
	getchar();

	//登录
	LoginReqT LoginData;

	sprintf(LoginData.szUserId, pConfigData->szAdmin.c_str());
	sprintf(LoginData.szPassword, pConfigData->szAdminPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query balance------\n");
	getchar();

	//查询资金
	HeadBuf.iFunctionId = FUNC_QUERY_BALANCE;
	HeadBuf.iReqId = 101;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query order------\n");
	getchar();

	//查询委托
	HeadBuf.iFunctionId = FUNC_QUERY_ORDER;
	HeadBuf.iReqId = 102;
	HeadBuf.iBodyLength = sizeof(QueryOrderReqT);

	QueryOrderReqT  QueryOrderReqData;
	QueryOrderReqData.iFlag = 0;

	m_pTrade->SendMsg(&HeadBuf, &QueryOrderReqData, sizeof(QueryOrderReqData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query trade------\n");
	getchar();

	//查询成交
	HeadBuf.iFunctionId = FUNC_QUERY_TRADE;
	HeadBuf.iReqId = 103;
	HeadBuf.iBodyLength = sizeof(QueryTradeReqT);

	QueryTradeReqT  QueryTradeReqData;
	QueryTradeReqData.iFlag = 0;

	m_pTrade->SendMsg(&HeadBuf, &QueryTradeReqData, sizeof(QueryTradeReqData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query position------\n");
	getchar();

	//查询持仓
	HeadBuf.iFunctionId = FUNC_QUERY_POSITION;
	HeadBuf.iReqId = 104;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to add del money------\n");
	getchar();


	//为交易员账户增加或者减少资金
	HeadBuf.iFunctionId = FUNC_ADD_DEL_MONEY;
	HeadBuf.iReqId = 105;
	HeadBuf.iBodyLength = sizeof(AddDelMoneyReqT);

	AddDelMoneyReqT  AddDelMoneyReqData;
	snprintf(AddDelMoneyReqData.szUserId, sizeof(AddDelMoneyReqData.szUserId), "102");
	AddDelMoneyReqData.iFlag = 1;
	//10元
	int iAddDelMoney = 10;
	//价格类型转换为整数进行传输，交易服务端收到价格类型后会转换为double类型
	AddDelMoneyReqData.iAmount = iAddDelMoney * PRICE_MULTIPLE;

	m_pTrade->SendMsg(&HeadBuf, &AddDelMoneyReqData, sizeof(AddDelMoneyReqData));

}
//交易员查询接口测试
void MainImpl::TraderQueryDemo()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 200;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);


	printf("------press Enter key to login------\n");
	getchar();

	//登录
	LoginReqT LoginData;

	sprintf(LoginData.szUserId, pConfigData->szTrader.c_str());
	sprintf(LoginData.szPassword, pConfigData->szTraderPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query balance------\n");
	getchar();

	//查询资金
	HeadBuf.iFunctionId = FUNC_QUERY_BALANCE;
	HeadBuf.iReqId = 201;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query order------\n");
	getchar();

	//查询委托
	HeadBuf.iFunctionId = FUNC_QUERY_ORDER;
	HeadBuf.iReqId = 202;
	HeadBuf.iBodyLength = sizeof(QueryOrderReqT);

	QueryOrderReqT  QueryOrderReqData;
	QueryOrderReqData.iFlag = 0;

	m_pTrade->SendMsg(&HeadBuf, &QueryOrderReqData, sizeof(QueryOrderReqData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query trade------\n");
	getchar();

	//查询成交
	HeadBuf.iFunctionId = FUNC_QUERY_TRADE;
	HeadBuf.iReqId = 203;
	HeadBuf.iBodyLength = sizeof(QueryTradeReqT);

	QueryTradeReqT  QueryTradeReqData;
	QueryTradeReqData.iFlag = 0;

	m_pTrade->SendMsg(&HeadBuf, &QueryTradeReqData, sizeof(QueryTradeReqData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query position------\n");
	getchar();

	//查询持仓
	HeadBuf.iFunctionId = FUNC_QUERY_POSITION;
	HeadBuf.iReqId = 204;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);

}
//交易员查询接口测试
void MainImpl::TraderQueryDemo2()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 200;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);


	printf("------press Enter key to login------\n");
	getchar();

	//登录
	LoginReqT LoginData;

	sprintf(LoginData.szUserId, pConfigData->szTrader.c_str());
	sprintf(LoginData.szPassword, pConfigData->szTraderPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query order------\n");
	getchar();

	//根据szClientOrderId查询委托
	char szClientOrderId[100] = { 0 };
	printf("please input szClientOrderId,szClientOrderId=");
	scanf("%s", szClientOrderId);


	HeadBuf.iFunctionId = FUNC_QUERY_ORDER;
	HeadBuf.iReqId = 202;
	HeadBuf.iBodyLength = sizeof(QueryOrderReqT);

	QueryOrderReqT  QueryOrderReqData;
	QueryOrderReqData.iFlag = 1;
	snprintf(QueryOrderReqData.szClientOrderId, sizeof(QueryOrderReqData.szClientOrderId), "%s", szClientOrderId);

	m_pTrade->SendMsg(&HeadBuf, &QueryOrderReqData, sizeof(QueryOrderReqData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to query trade------\n");
	getchar();
	getchar();

	//根据szClientOrderId查询成交
	HeadBuf.iFunctionId = FUNC_QUERY_TRADE;
	HeadBuf.iReqId = 203;
	HeadBuf.iBodyLength = sizeof(QueryTradeReqT);

	QueryTradeReqT  QueryTradeReqData;
	QueryTradeReqData.iFlag = 1;
	snprintf(QueryTradeReqData.szClientOrderId, sizeof(QueryTradeReqData.szClientOrderId), "%s", szClientOrderId);

	m_pTrade->SendMsg(&HeadBuf, &QueryTradeReqData, sizeof(QueryTradeReqData));


}
//交易员委托报单和撤单测试
void MainImpl::TraderEntrustDemo()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 300;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);


	printf("------press Enter key to login------\n");
	getchar();

	//登录
	LoginReqT LoginData;

	sprintf(LoginData.szUserId, pConfigData->szTrader.c_str());
	sprintf(LoginData.szPassword, pConfigData->szTraderPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to add msg queue------\n");
	getchar();

	//增加共享内存通讯队列
	HeadBuf.iFunctionId = FUNC_ADD_MSG_QUEUE;
	HeadBuf.iReqId = 301;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	//启动共享内存通讯
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
	int iReadKey = 0;
	int	iWriteKey = 0;
	m_pTrade->GetTradeKey(iReadKey, iWriteKey);
	if (iReadKey != 0 && iWriteKey != 0)
	{
		m_pCMsgQueueTd->Init(iReadKey, iWriteKey);
		m_pCMsgQueueTd->Start();
	}
	else
	{
		printf("------start share memory fail-----\n");
		return;
	}


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to place order-----\n");
	getchar();


	//发送委托单
	char szStockCode[20] = { 0 };
	double dPrice = 0;
	char cAction = '\0';
	int  iDirection = 0;
	int  iEntrustNum = 0;

	printf("please input:\n");
	printf("szStockCode=");
	scanf("%s", szStockCode);

	printf("\n");
	printf("*************direction list**********\n");
	printf("iDirection: 0 - ORDER_ACTION_BUY_OPEN\n");
	printf("iDirection: 1 - ORDER_ACTION_SELL_CLOSE\n");
	printf("iDirection: 2 - ORDER_ACTION_SELL_OPEN\n");
	printf("iDirection: 3 - ORDER_ACTION_BUY_CLOSE\n");
	printf("*************************************\n");
	printf("\niDirection=");
	scanf("%d", &iDirection);
	if (0 == iDirection)
	{
		cAction = ORDER_ACTION_BUY_OPEN;
	}
	else if (1 == iDirection)
	{
		cAction = ORDER_ACTION_SELL_CLOSE;
	}
	else if (2 == iDirection)
	{
		cAction = ORDER_ACTION_SELL_OPEN;
	}
	else if (3 == iDirection)
	{
		cAction = ORDER_ACTION_BUY_CLOSE;
	}

	printf("dPrice=");
	scanf("%lf", &dPrice);
	printf("iEntrustNum=");
	scanf("%d", &iEntrustNum);


	printf("\n");
	printf("*************show input data**********\n");
	printf("szStockCode=%s\n", szStockCode);
	printf("dPrice=%lf\n", dPrice);
	printf("iEntrustNum=%d\n", iEntrustNum);
	printf("iDirection=%d\n", iDirection);
	printf("cAction=%c\n", cAction);

	HeadBuf.iFunctionId = FUNC_PLACE_ORDER;
	HeadBuf.iReqId = 302;
	HeadBuf.iBodyLength = sizeof(PlaceOrderReqT);

	string szSeconds = to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	string szClientOrderId = szSeconds;
	PlaceOrderReqT PlaceOrderReqData;
	snprintf(PlaceOrderReqData.szClientOrderId, sizeof(PlaceOrderReqData.szClientOrderId), "%s", szClientOrderId.c_str());
	snprintf(PlaceOrderReqData.szStockCode, sizeof(PlaceOrderReqData.szStockCode), "%s", szStockCode);
	//由于期货行情中的市场代码ExchangeID没有值，所以此处不需要填写szExchangeID。在交易服务端会根据szStockCode查询szExchangeID。
	//snprintf(PlaceOrderReqData.szExchangeID, sizeof(PlaceOrderReqData.szExchangeID), "DCE");
	PlaceOrderReqData.cAction = cAction;
	//价格转换为整数进行传输，交易服务端收到价格后会转换为double类型
	PlaceOrderReqData.iPrice = zutil::ConvertDoubleToInt(dPrice, PRICE_MULTIPLE);
	PlaceOrderReqData.iEntrustNum = iEntrustNum;

	m_pCMsgQueueTd->WriteQueue(&HeadBuf, &PlaceOrderReqData, sizeof(PlaceOrderReqData));



	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to cancel order-----\n");
	getchar();
	getchar();

	//撤单
	HeadBuf.iFunctionId = FUNC_CANCEL_ORDER;
	HeadBuf.iReqId = 303;
	HeadBuf.iBodyLength = sizeof(CancelOrderReqT);

	CancelOrderReqT CancelOrderReqData;
	snprintf(CancelOrderReqData.szClientOrderId, sizeof(CancelOrderReqData.szClientOrderId), "%s", szClientOrderId.c_str());

	m_pCMsgQueueTd->WriteQueue(&HeadBuf, &CancelOrderReqData, sizeof(CancelOrderReqData));


	//删除共享内存通讯队列
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));
	printf("------press Enter key to delete msg queue-----\n");
	getchar();

	printf("------stop  local  msg queue-----\n");
	//发送删除共享内存通讯队列报文之前先停止本地共享内存通讯线程
	m_pCMsgQueueTd->Stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));

	HeadBuf.iFunctionId = FUNC_DEL_MSG_QUEUE;
	HeadBuf.iReqId = 304;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);

}
//行情测试
void MainImpl::MarketDemo()
{
	printf("-------MarketDemo----Start------\n");

	//组装订阅行情的报文
	vector<string> vStockCodes;
	vStockCodes.push_back("p2405");

	vector<SubItem> vSubStockCodes;
	for (int i = 0; i < vStockCodes.size(); i++)
	{
		SubItem Item;
		snprintf(Item.szStockCode, sizeof(Item.szStockCode), "%s", vStockCodes[i].c_str());
		vSubStockCodes.push_back(Item);
	}

	int iNum = vSubStockCodes.size();
	int iLength = sizeof(iNum) + iNum * sizeof(SubItem);

	char*pBuf = new char[iLength];
	memcpy(&pBuf[0], &iNum, sizeof(iNum));
	memcpy(&pBuf[sizeof(iNum)], &vSubStockCodes[0], iNum * sizeof(SubItem));

	//订阅行情

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_SUB_QUOTE;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 500;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = iLength;

	printf("-------MarketDemo-----SendMsg---FUNC_SUB_QUOTE--\n");
	m_pMarket->SendMsg(&HeadBuf, pBuf, iLength);
	delete[] pBuf;

	//启动共享内存通讯
	int iMarketKey = 0;
	int	iClientNum = 0;

	//由于是异步通讯，此处采用循环查询
	unsigned long long  i = 0;
	while (1)
	{
		i++;

		if (i > 1000000)
		{
			printf("------not receive res msg!   start share memory fail-----\n");
			break;
		}

		m_pMarket->GetMarketKey(iMarketKey, iClientNum);
		if (iMarketKey != 0 && iClientNum != 0)
		{
			m_pCMsgQueueMd->Init(iMarketKey);
			m_pCMsgQueueMd->Start();
			break;
		}

		//休眠1微秒
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	//休眠20秒，20秒后取消订阅行情
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 20));

	//取消订阅行情
	HeadBuf.iFunctionId = FUNC_CANCEL_SUB_QUOTE;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 501;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(CancelSubQuoteReqT);

	CancelSubQuoteReqT   CancelSubQuoteData;
	CancelSubQuoteData.iClientNum = iClientNum;

	printf("-------MarketDemo-----SendMsg---FUNC_CANCEL_SUB_QUOTE--\n");
	m_pMarket->SendMsg(&HeadBuf, &CancelSubQuoteData, sizeof(CancelSubQuoteData));


	printf("-------MarketDemo----End------\n");
}
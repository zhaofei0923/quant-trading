#include "main_impl.h"
#include "config.h"
#include "sim_log.h"
#include <chrono>


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


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
	AdminOper();

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

void MainImpl::AdminOper()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 100;
	HeadBuf.iRoletype = ROLE_TYPE_ADMIN;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);


	printf("------press any key to login------\n");

	LoginReqT LoginData;

	sprintf(LoginData.szUserId, pConfigData->szAdmin.c_str());
	sprintf(LoginData.szPassword, pConfigData->szAdminPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));

	int iFlag = 0;
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
		printf("\n\n");
		printf("please select!\n");
		printf("1:query balance\n");
		printf("2:query order\n");
		printf("3:query trade\n");
		printf("4:query position\n");
		printf("5:add del money\n");
		printf("please input num:");
		scanf("%d", &iFlag);

		if (1 == iFlag)
		{
			printf("------query balance------\n");

			HeadBuf.iFunctionId = FUNC_QUERY_BALANCE;
			HeadBuf.iReqId = 101;
			HeadBuf.iBodyLength = 0;

			m_pTrade->SendMsg(&HeadBuf, NULL, 0);

		}
		else if (2 == iFlag)
		{
			printf("------query order------\n");

			HeadBuf.iFunctionId = FUNC_QUERY_ORDER;
			HeadBuf.iReqId = 102;
			HeadBuf.iBodyLength = sizeof(QueryOrderReqT);

			QueryOrderReqT  QueryOrderReqData;
			QueryOrderReqData.iFlag = 0;

			m_pTrade->SendMsg(&HeadBuf, &QueryOrderReqData, sizeof(QueryOrderReqData));

		}
		else if (3 == iFlag)
		{
			printf("------query trade------\n");

			HeadBuf.iFunctionId = FUNC_QUERY_TRADE;
			HeadBuf.iReqId = 103;
			HeadBuf.iBodyLength = sizeof(QueryTradeReqT);

			QueryTradeReqT  QueryTradeReqData;
			QueryTradeReqData.iFlag = 0;

			m_pTrade->SendMsg(&HeadBuf, &QueryTradeReqData, sizeof(QueryTradeReqData));

		}
		else if (4 == iFlag)
		{
			printf("------query position------\n");

			HeadBuf.iFunctionId = FUNC_QUERY_POSITION;
			HeadBuf.iReqId = 104;
			HeadBuf.iBodyLength = 0;

			m_pTrade->SendMsg(&HeadBuf, NULL, 0);
		}
		else if (5 == iFlag)
		{
			printf("------press any key to add del money------\n");
			getchar();

			char szUserId[100] = { 0 };
			printf("\n please input user_id:");
			scanf("%s", szUserId);
			int iFlag = 0; //操作标志：0 - 增加资金，1 - 减少资金
			printf("\n please select add or del,0-add 1-del:");
			scanf("%d", &iFlag);
			printf("\n please input amount,amount type should be int :");
			int iAmount = 0;
			scanf("%d", &iAmount);
			printf("\n");

			HeadBuf.iFunctionId = FUNC_ADD_DEL_MONEY;
			HeadBuf.iReqId = 105;
			HeadBuf.iBodyLength = sizeof(AddDelMoneyReqT);

			AddDelMoneyReqT  AddDelMoneyReqData;
			snprintf(AddDelMoneyReqData.szUserId, sizeof(AddDelMoneyReqData.szUserId), "%s", szUserId);
			AddDelMoneyReqData.iFlag = iFlag;
			AddDelMoneyReqData.iAmount = iAmount * PRICE_MULTIPLE;

			std::cout << "-------------Msg info----------------------" << endl;
			std::cout << "szUserId=" << AddDelMoneyReqData.szUserId << endl;
			std::cout << "iFlag=" << AddDelMoneyReqData.iFlag << endl;
			std::cout << "iAmount=" << AddDelMoneyReqData.iAmount << endl;

			m_pTrade->SendMsg(&HeadBuf, &AddDelMoneyReqData, sizeof(AddDelMoneyReqData));
		}
	}
}

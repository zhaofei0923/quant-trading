#include "main_impl.h"
#include "config.h"
#include "sim_log.h"
#include "mem_pool_mng.h"
#include "strategy.h"
#include <chrono>

/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

int MainImpl::Init()
{

	ConfigData*pConfigData = ConfigData::GetInstance();
	if (false == pConfigData->LoadConfig())
	{
		return -1;
	}

	SimLog::Instance().InitSimLog(pConfigData->iLogLevel);

	//�ڴ��
	MemPoolMng::GetInstance()->Init();

	//����
	CStrategy::GetInstance()->Init();

	//����
	m_pTrade = new CTcpClient(pConfigData->szTradeIp, pConfigData->iTradePort);
	CallBack*pTdTcpCall = new CallBack();
	m_pTrade->SetCallBack(pTdTcpCall);
	m_pTrade->Init();

	//����
	CallBack*pTdMqCall = new CallBack();
	m_pCMsgQueueTd = CMsgQueueTd::GetInstance();
	m_pCMsgQueueTd->SetCallBack(pTdMqCall);


	//����
	m_pMarket = new CTcpClient(pConfigData->szMarketIp, pConfigData->iMarketPort);
	CallBack*pMdTcpCall = new CallBack();
	m_pMarket->SetCallBack(pMdTcpCall);
	m_pMarket->Init();

	//����
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

	//����
	CStrategy::GetInstance()->Start();

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

	//����Ա��¼
	if (false == TraderLogin())
	{
		LError("TraderLogin  Fail");
		return -1;
	}

	//��������
	vector<string> vStockCodes;
	string szStockCode = "cu2408";
	vStockCodes.push_back(szStockCode);
	//szStockCode = "cu2402";
	//vStockCodes.push_back(szStockCode);

	if (false == SubMarket(vStockCodes))
	{
		LError("SubMarket  Fail");
		return -1;
	}

	return 0;
}
int MainImpl::Stop()
{
	TraderLogout();
	//����
	CStrategy::GetInstance()->Stop();
	return 0;
}
int MainImpl::Release()
{
	//����
	CStrategy::GetInstance()->Release();
	//�ڴ��
	MemPoolMng::GetInstance()->Release();
	return 0;
}

//����Ա��¼
bool MainImpl::TraderLogin()
{
	ConfigData*pConfigData = ConfigData::GetInstance();

	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_LOGIN;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 300;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = sizeof(LoginReqT);

	printf("------ login------\n");

	LoginReqT LoginData;

	sprintf(LoginData.szUserId, "%s", pConfigData->szTrader.c_str());
	sprintf(LoginData.szPassword, "%s", pConfigData->szTraderPassword.c_str());

	m_pTrade->SendMsg(&HeadBuf, &LoginData, sizeof(LoginData));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
	printf("------add msg queue------\n");

	HeadBuf.iFunctionId = FUNC_ADD_MSG_QUEUE;
	HeadBuf.iReqId = 301;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	//���������ڴ�ͨѶ
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
		return false;
	}

	return true;

}
//����Ա�˳�
void MainImpl::TraderLogout()
{
	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_DEL_MSG_QUEUE;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 300;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = 0;


	printf("------stop  local  msg queue-----\n");
	//����ɾ�������ڴ�ͨѶ���б���֮ǰ��ֹͣ���ع����ڴ�ͨѶ�߳�
	m_pCMsgQueueTd->Stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


	printf("------logout------\n");

	HeadBuf.iFunctionId = FUNC_DEL_MSG_QUEUE;
	HeadBuf.iReqId = 301;
	HeadBuf.iBodyLength = 0;

	m_pTrade->SendMsg(&HeadBuf, NULL, 0);


}
//��������
bool MainImpl::SubMarket(const vector<string>&vStockCodes)
{
	printf("-------SubMarket----Start------\n");
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


	Head HeadBuf;
	HeadBuf.iFunctionId = FUNC_SUB_QUOTE;
	HeadBuf.iMsgtype = MSG_TYPE_REQ;
	HeadBuf.iReqId = 500;
	HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	HeadBuf.iErrorCode = 0;
	HeadBuf.iBodyLength = iLength;

	printf("-------SubMarket-----SendMsg---FUNC_SUB_QUOTE--\n");
	m_pMarket->SendMsg(&HeadBuf, pBuf, iLength);
	delete[] pBuf;

	//���������ڴ�ͨѶ
	int iMarketKey = 0;
	int	iClientNum = 0;

	//�������첽ͨѶ���˴�����ѭ����ѯ
	unsigned long long  i = 0;
	while (1)
	{
		i++;

		if (i > 1000000)
		{
			printf("------not receive res msg!   start share memory fail-----\n");
			return false;
		}

		m_pMarket->GetMarketKey(iMarketKey, iClientNum);
		if (iMarketKey != 0 && iClientNum != 0)
		{
			m_pCMsgQueueMd->Init(iMarketKey);
			m_pCMsgQueueMd->Start();
			return true;
		}

		//����1΢��
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	printf("-------SubMarket----End------\n");
	return true;
}
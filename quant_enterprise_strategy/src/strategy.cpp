#include "strategy.h"
#include "sim_log.h"
#include "msg_queue_td.h"
#include "mem_pool_mng.h"
#include <chrono>
#include "helper_tools.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

CStrategy::CStrategy()
{
	m_iCount = 0;

	m_IsSendOrder = false;
	m_iBarCount = 1;
	m_iSubCount = 30;
	m_bIsNewBar = false;

	m_iStrategy = 1;

	//Kama均线策略
	m_iKamaCount = 10;
	m_iDirection = 10;
	m_dKamaFast = 2.0 / (2.0 + 1);
	m_dKamaSlow = 2.0 / (30.0 + 1);

	m_iFilterCount = 10;
	m_dFilter = 0.15;
}

CStrategy::~CStrategy()
{
	m_BarStream.close();
	m_mBar.clear();
	m_mDequeMinBar.clear();
	m_mmDequeSubBar.clear();
}

int CStrategy::Init()
{
	ReadBarData();
	CreateFile();
	SetThreadCount(1);
	return 0;
}
int CStrategy::Start()
{
	LInfo("Start CStrategy ");

	RunThreads();

	return 0;
}
int CStrategy::Stop()
{
	StopThreads();
	return 0;
}
int CStrategy::Release()
{
	return 0;
}

// 封装下单接口（写入本地交易共享内存队列）
bool CStrategy::PlaceOrder(const string &stockCode, char action, double price, int entrustNum, string *outClientOrderId)
{
    Head head;
    head.iFunctionId = FUNC_PLACE_ORDER;
    head.iMsgtype = MSG_TYPE_REQ;
    head.iReqId = m_iCount++;
    head.iRoletype = ROLE_TYPE_TRADER;
    head.iErrorCode = 0;
    head.iBodyLength = sizeof(PlaceOrderReqT);

    // 生成唯一 ClientOrderId（毫秒时间戳）
    long long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    std::string clientOrderId = std::to_string(now_ms);
    if (outClientOrderId)
    {
        *outClientOrderId = clientOrderId;
    }

    PlaceOrderReqT req;
    snprintf(req.szClientOrderId, sizeof(req.szClientOrderId), "%s", clientOrderId.c_str());
    snprintf(req.szStockCode, sizeof(req.szStockCode), "%s", stockCode.c_str());
    // 如需显式指定交易所，可填写 req.szExchangeID
    req.cAction = action;
    req.iPrice = zutil::ConvertDoubleToInt(price, PRICE_MULTIPLE);
    req.iEntrustNum = entrustNum;

    CMsgQueueTd *mq = CMsgQueueTd::GetInstance();
    int ret = mq->WriteQueue(&head, &req, sizeof(req));
    return (ret == 0);
}
int CStrategy::DealMsg(void *pMsg, int iThreadId)
{
	if (pMsg != NULL)
	{
		ServerMsg *pServerMsg = (ServerMsg *)pMsg;
		ExecuteStrategy(pServerMsg);
		// MakeTradeSignal(pServerMsg);

		MemPoolMng *pMemPoolMng = MemPoolMng::GetInstance();
		char *pBody = pServerMsg->pBody;
		if (pBody != NULL)
		{
			pMemPoolMng->DeleteMaxMemBlock(pBody);
		}

		pMemPoolMng->DeleteMaxMemBlock((char *)pServerMsg);
	}

	return 0;
}
// 生成交易信号
void CStrategy::MakeTradeSignal(ServerMsg *pServerMsg)
{
	// printf("----------------MakeTradeSignal----------------------\n");
	// MarketData*pMarketData = (MarketData*)pServerMsg->pBody;
	// printf("szExchangeId=%s\n", pMarketData->szExchangeId);
	// printf("slasttime=%s\n", pMarketData->szTime);
	// printf("szStockCode=%s\n", pMarketData->szStockCode);
	// printf("iLastPrice=%d\n", pMarketData->iLastPrice);
	// printf("iPreSettlePrice=%d\n", pMarketData->iPreSettlePrice);
	// printf("iPreClosePrice=%d\n", pMarketData->iPreClosePrice);

	// Head HeadBuf;
	// HeadBuf.iFunctionId = FUNC_PLACE_ORDER;
	// HeadBuf.iMsgtype = MSG_TYPE_REQ;
	// HeadBuf.iReqId = m_iCount++;
	// HeadBuf.iRoletype = ROLE_TYPE_TRADER;
	// HeadBuf.iErrorCode = 0;
	// HeadBuf.iBodyLength = sizeof(PlaceOrderReqT);

	// //如果没有发送委托单，则发送
	// if (false == m_IsSendOrder)
	// {
	// 	PlaceOrderReqT OrderReq;
	// 	if (AverageLine(pMarketData, OrderReq))
	// 	{

	// 		printf("----------------Send Trade Req----------------------\n");
	// 		std::cout << "szClientOrderId=" << OrderReq.szClientOrderId << endl;
	// 		std::cout << "szStockCode=" << OrderReq.szStockCode << endl;
	// 		std::cout << "cAction=" << OrderReq.cAction << endl;
	// 		std::cout << "iPrice=" << OrderReq.iPrice << endl;
	// 		std::cout << "iEntrustNum=" << OrderReq.iEntrustNum << endl;

	// 		CMsgQueueTd*pCMsgQueueTd = CMsgQueueTd::GetInstance();
	// 		pCMsgQueueTd->WriteQueue(&HeadBuf, &OrderReq, sizeof(OrderReq));

	// 		//发送委托单标志改为true,即已发送
	// 		m_IsSendOrder = true;

	// 	}

	// }
}
// 平均线策略
// 返回值：true-交易，false-不交易
bool CStrategy::AverageLine(MarketData *pMarketData, PlaceOrderReqT &OrderReq)
{
	// m_LongQuote.push_back(*pMarketData);
	// //保留最新20个快照
	// if (m_LongQuote.size() > 20)
	// {
	// 	m_LongQuote.pop_front();
	// }

	// m_ShortQuote.push_back(*pMarketData);
	// //保留最新5个快照
	// if (m_ShortQuote.size() > 5)
	// {
	// 	m_ShortQuote.pop_front();
	// }

	// printf("----AverageLine----m_LongQuote.size()=[%d]-----\n", m_LongQuote.size());

	// //快照数量没有达到20
	// if (m_LongQuote.size() < 20)
	// {
	// 	return false;
	// }

	// //取20个快照的均价
	// double dPrice20 = 0;
	// list<MarketData>::iterator iter;
	// for (iter = m_LongQuote.begin(); iter != m_LongQuote.end(); ++iter)
	// {
	// 	double dPrice = iter->iLastPrice / PRICE_MULTIPLE;
	// 	dPrice20 = dPrice20 + dPrice;
	// }
	// double dAverPrice20 = dPrice20 / 20;
	// LDebug("dAverPrice20=[{0}]", dAverPrice20);

	// //取5个快照的均价
	// double dPrice5 = 0;
	// for (iter = m_ShortQuote.begin(); iter != m_ShortQuote.end(); ++iter)
	// {
	// 	double dPrice = iter->iLastPrice / PRICE_MULTIPLE;
	// 	dPrice5 = dPrice5 + dPrice;
	// }
	// double dAverPrice5 = dPrice5 / 5;
	// LDebug("dAverPrice5=[{0}]", dAverPrice5);

	// //5个快照均线大于20个快照均线,开始购买
	// if (dAverPrice5 > dAverPrice20)
	// {
	// 	//此处以秒数作为szClientOrderId，可以修改为时分秒，只要唯一就可以
	// 	string szSeconds = to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	// 	string szClientOrderId = szSeconds;
	// 	snprintf(OrderReq.szClientOrderId, sizeof(OrderReq.szClientOrderId), "%s", szClientOrderId.c_str());
	// 	snprintf(OrderReq.szStockCode, sizeof(OrderReq.szStockCode), "%s", pMarketData->szStockCode);
	// 	//ctp行情没有ExchangeID，此处不填
	// 	//snprintf(OrderReq.szExchangeID, sizeof(OrderReq.szExchangeID), "DCE");
	// 	OrderReq.cAction = ORDER_ACTION_BUY_OPEN;
	// 	//ctp行情价格已经乘以PRICE_MULTIPLE
	// 	OrderReq.iPrice = (pMarketData->iBuyPrice1/ PRICE_MULTIPLE+10)* PRICE_MULTIPLE;
	// 	OrderReq.iEntrustNum = 1;

	// 	std::cout << "----------AverageLine---------------" << endl;
	// 	std::cout << "szClientOrderId=" << OrderReq.szClientOrderId << endl;
	// 	std::cout << "szStockCode=" << OrderReq.szStockCode << endl;
	// 	std::cout << "cAction=" << OrderReq.cAction << endl;
	// 	std::cout << "iPrice=" << OrderReq.iPrice << endl;
	// 	std::cout << "iEntrustNum=" << OrderReq.iEntrustNum << endl;

	// 	return true;
	// }
	// else
	// {
	// 	return false;
	// }

	return false;
}

/// 生成订阅的Bar

bool CStrategy::IsValidTick(string sTickTime)
{
	// 8:59:00-11:31:00,13:29:00-15:01:00,20:59:00-02:30:00
	// iHour>0 && iHour<23 , iMinute>0 && iMinute<59 , iSecond>0 && iSecond<59
	// 早盘8：59分会有一个tick数据此时tick有效但不做处理，11：30分之后会有数据需要处理
	// 夜盘结束时间到到11：30部分品种到2：30，夜盘结束后不会有数据

	int iHour = atoi(sTickTime.substr(0, 2).c_str());
	int iMinute = atoi(sTickTime.substr(3, 2).c_str());
	int iSecond = atoi(sTickTime.substr(6, 2).c_str());

	if ((iHour == 8 && iMinute == 59) || iHour == 9 || iHour == 10 || (iHour == 11 && iMinute <= 30))
	{
		return true;
	}
	else if ((iHour == 13 && iMinute >= 29) || iHour == 14 || (iHour == 15 && iMinute == 0))
	{
		return true;
	}
	else if ((iHour == 20 && iMinute == 59) || iHour == 21 || iHour == 22 || iHour == 23 || iHour == 0 || iHour == 1 || (iHour == 2 && iMinute <= 30))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CStrategy::CalculateTimeDiff(string sTime1, string sTime2)
{
	int iHour1 = atoi(sTime1.substr(0, 2).c_str());
	int iMinute1 = atoi(sTime1.substr(3, 2).c_str());

	int iHour2 = atoi(sTime2.substr(0, 2).c_str());
	int iMinute2 = atoi(sTime2.substr(3, 2).c_str());

	int iTime1 = iHour1 * 3600 + iMinute1 * 60;
	int iTime2 = iHour2 * 3600 + iMinute2 * 60;

	return iTime2 - iTime1;
}

bool CStrategy::IsSubBar(string sbartime, int icount)
{
	int iMinute = stoi(sbartime.substr(3, 2));

	if (iMinute % icount == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CStrategy::ExecuteStrategy(ServerMsg *pServerMsg)
{
	MarketData *pMarketData = (MarketData *)pServerMsg->pBody;

	string sInstrumentID = pMarketData->szStockCode;

	CalculateBar(pMarketData);

	SelectStrategy(m_iStrategy,sInstrumentID);



}

void CStrategy::CalculateBar(MarketData *pMarketData)
{

	string sUpdateTime = pMarketData->szUpdateTime;
	string sInstrumentID = pMarketData->szStockCode;

	if (m_mBar.count(sInstrumentID) > 0)
	{

		shared_ptr<Bar> bar = m_mBar[sInstrumentID];

		if (CalculateTimeDiff(bar->sTime, sUpdateTime) == 0)
		{
			bar->dHigh = max(bar->dHigh, double(pMarketData->iLastPrice / PRICE_MULTIPLE));
			bar->dLow = min(bar->dLow, double(pMarketData->iLastPrice / PRICE_MULTIPLE));
			bar->dClose = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
			bar->dOpenInterest = double(pMarketData->iOpenInterest);
		}
		else if (CalculateTimeDiff(bar->sTime, sUpdateTime) == 60)
		{
			// 将bar数据存储到vector容器中
			bar->sTime = sUpdateTime.substr(0, 6) + "00";
			bar->dVolume = double(pMarketData->iVolume) - bar->dVolume;

			shared_ptr<Bar> newBar;

			while (m_mDequeMinBar[sInstrumentID].size() > m_iBarCount)
			{
				m_mDequeMinBar[sInstrumentID].pop_front();
			}

			if (m_mDequeMinBar[sInstrumentID].size() == m_iBarCount)
			{
				newBar = m_mDequeMinBar[sInstrumentID].front();
				m_mDequeMinBar[sInstrumentID].pop_front();
			}
			else
			{
				newBar = make_shared<Bar>();
			}

			// 将mBar中的数据赋值给newBar
			newBar->sDate = bar->sDate;
			newBar->sTime = bar->sTime;
			newBar->sInstrumentID = bar->sInstrumentID;
			newBar->dOpen = bar->dOpen;
			newBar->dHigh = bar->dHigh;
			newBar->dLow = bar->dLow;
			newBar->dClose = bar->dClose;
			newBar->dVolume = bar->dVolume;
			newBar->dOpenInterest = bar->dOpenInterest;

			m_mDequeMinBar[sInstrumentID].push_back(newBar);

			cout << " New Bar in mDequeMinBar: " << newBar->sInstrumentID << " Time is: " << newBar->sTime << endl;

			if (IsSubBar(newBar->sTime, m_iBarCount))
			{
				CalculateSubBar(newBar->sInstrumentID);
			}

			// 更新mbar中的数据
			bar->dVolume = double(pMarketData->iVolume);
			bar->dOpenInterest = double(pMarketData->iOpenInterest);
			bar->dClose = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
			bar->dHigh = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
			bar->dLow = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
			bar->dOpen = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
		}
		else if (CalculateTimeDiff(bar->sTime, sUpdateTime) > 60)
		{
			// 两个tick之间的时间差大于60秒，说明有tick数据丢失，需要重新计算。按照新的tick计算bar

			bar->dVolume = double(pMarketData->iVolume) - bar->dVolume;

			shared_ptr<Bar> newBar;

			while (m_mDequeMinBar[sInstrumentID].size() > m_iBarCount)
			{
				m_mDequeMinBar[sInstrumentID].pop_front();
			}

			if (m_mDequeMinBar[sInstrumentID].size() == m_iBarCount)
			{
				newBar = m_mDequeMinBar[sInstrumentID].front();
				m_mDequeMinBar[sInstrumentID].pop_front();
			}
			else
			{
				newBar = make_shared<Bar>();
			}

			// 将mBar中的数据赋值给newBar
			newBar->sDate = bar->sDate;
			// bar->cTime的分钟数加1
			int iHour = stoi(bar->sTime.substr(0, 2));
			int iMinute = stoi(bar->sTime.substr(3, 2));
			int iSecond = stoi(bar->sTime.substr(6, 2));
			if ((iHour == 10 && iMinute == 15) || (iHour == 11 && iMinute == 30) || (iHour == 15 && iMinute == 0) || (CalculateTimeDiff(bar->sTime, sUpdateTime) > 3600))
			{
				bar->dVolume = double(pMarketData->iVolume);
				bar->dOpenInterest = double(pMarketData->iOpenInterest);
				bar->dClose = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->dHigh = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->dLow = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->dOpen = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->sTime = sUpdateTime.substr(0, 6) + "00";
			}
			else
			{
				iMinute += 1;

				if (iMinute == 60)
				{
					iHour += 1;
					iMinute = 0;
					if (iHour == 24)
					{
						iHour = 0;
					}
				}

				iSecond = 0;

				char cTime[20];
				sprintf(cTime, "%02d:%02d:%02d", iHour, iMinute, iSecond);

				newBar->sTime = cTime;
				newBar->sInstrumentID = bar->sInstrumentID;
				newBar->dOpen = bar->dOpen;
				newBar->dHigh = bar->dHigh;
				newBar->dLow = bar->dLow;
				newBar->dClose = bar->dClose;
				newBar->dVolume = bar->dVolume;
				newBar->dOpenInterest = bar->dOpenInterest;
				m_mDequeMinBar[sInstrumentID].push_back(newBar);

				cout << "New Bar in mDequeMinBar:" << newBar->sInstrumentID << "Time is:" << newBar->sTime << "Close is" << newBar->dClose << endl;

				if (IsSubBar(newBar->sTime, m_iBarCount))
				{
					CalculateSubBar(newBar->sInstrumentID);
				}

				// 更新mbar中的数据
				bar->dVolume = double(pMarketData->iVolume);
				bar->dOpenInterest = double(pMarketData->iOpenInterest);
				bar->dClose = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->dHigh = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->dLow = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->dOpen = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
				bar->sTime = sUpdateTime.substr(0, 6) + "00";
			}
		}
	}
	else
	{
		shared_ptr<Bar> bar = make_shared<Bar>();

		int iHour = stoi(sUpdateTime.substr(0, 2));
		int iMinute = stoi(sUpdateTime.substr(3, 2));
		int iSecond = stoi(sUpdateTime.substr(6, 2));

		if (iHour == 8 && iMinute == 59)
		{
			iHour = 9;
			iMinute = 0;
			iSecond = 0;
		}
		else if (iHour == 13 && iMinute == 29)
		{
			iHour = 13;
			iMinute = 30;
			iSecond = 0;
		}
		else if (iHour == 20 && iMinute == 59)
		{
			iHour = 21;
			iMinute = 0;
			iSecond = 0;
		}
		else
		{
			iSecond = 0;
		}

		// 把iHour,iMinute,iSecond转换为"09:00:00"类型的时间格式
		char cTime[20];
		sprintf(cTime, "%02d:%02d:%02d", iHour, iMinute, iSecond);

		bar->sDate = pMarketData->szTradingDay;
		bar->sTime = cTime;
		bar->sInstrumentID = sInstrumentID;
		bar->dOpen = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
		bar->dHigh = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
		bar->dLow = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
		bar->dClose = double(pMarketData->iLastPrice / PRICE_MULTIPLE);
		bar->dVolume = double(pMarketData->iVolume);
		bar->dOpenInterest = double(pMarketData->iOpenInterest);

		m_mBar.insert(pair<string, shared_ptr<Bar>>(sInstrumentID, bar));

		cout << "New Bar in mBar:" << bar->sInstrumentID << "Time is:" << bar->sTime << "Close is" << bar->dClose << endl;
	}
}

void CStrategy::CalculateSubBar(string sInstrumentId)
{
	if (m_mmDequeSubBar[sInstrumentId].count(m_iBarCount) == 0)
	{
		shared_ptr<Bar> bar = make_shared<Bar>();

		deque<shared_ptr<Bar>> dBar;

		dBar.push_back(bar);

		map<int, deque<shared_ptr<Bar>>> mCountBar;

		mCountBar.insert(pair<int, deque<shared_ptr<Bar>>>(m_iBarCount, dBar));

		m_mmDequeSubBar.insert(pair<string, map<int, deque<shared_ptr<Bar>>>>(sInstrumentId, mCountBar));
	}

	while (m_mmDequeSubBar[sInstrumentId][m_iBarCount].size() > m_iSubCount)
	{
		m_mmDequeSubBar[sInstrumentId][m_iBarCount].pop_front();
	}

	shared_ptr<Bar> subBar;

	if (m_mmDequeSubBar[sInstrumentId][m_iBarCount].size() == m_iSubCount)
	{
		subBar = m_mmDequeSubBar[sInstrumentId][m_iBarCount].front();
		m_mmDequeSubBar[sInstrumentId][m_iBarCount].pop_front();
	}
	else
	{
		subBar = make_shared<Bar>();
	}

	deque<shared_ptr<Bar>> dBar = m_mDequeMinBar[sInstrumentId];

	if (m_iBarCount == 1)
	{
		subBar->sDate = dBar[0]->sDate;
		subBar->sInstrumentID = dBar[0]->sInstrumentID;
		subBar->sTime = dBar[0]->sTime;
		subBar->dVolume = dBar[0]->dVolume;
		subBar->dOpenInterest = dBar[0]->dOpenInterest;
		subBar->dClose = dBar[0]->dClose;
		subBar->dOpen = dBar[0]->dOpen;
		subBar->dHigh = dBar[0]->dHigh;
		subBar->dLow = dBar[0]->dLow;
	}
	else
	{
		subBar->sDate = dBar[0]->sDate;
		subBar->sInstrumentID = dBar[0]->sInstrumentID;
		subBar->dOpen = dBar[0]->dOpen;
		subBar->dHigh = dBar[0]->dHigh;
		subBar->dLow = dBar[0]->dLow;

		for (auto it = dBar.begin() + 1; it != dBar.end(); it++)
		{
			subBar->dHigh = max(subBar->dHigh, (*it)->dHigh);
			subBar->dLow = min(subBar->dLow, (*it)->dLow);
			subBar->dClose = (*it)->dClose;
			subBar->dOpenInterest = (*it)->dOpenInterest;
			subBar->dVolume += (*it)->dVolume;
			subBar->sTime = (*it)->sTime;
		}
	}

	cout << "New Bar in mmDequeSubBar: " << subBar->sInstrumentID << "Time is: " << subBar->sTime << "Close is" << subBar->dClose << endl;

	m_mmDequeSubBar[sInstrumentId][m_iBarCount].push_back(subBar);

	WriteBarData(*subBar);

	m_bIsNewBar = true;

}

//获取可执行文件路径
string CStrategy::GetExecutablePath()
{
	char processdir[1000];
	if (readlink("/proc/self/exe", processdir, 1000) <= 0)
	{
		printf("readlink  fail\n");
		return  "";
	}
	//函数查找字符在指定字符串中最后一次出现的位置如果成功，则返回指定字符最后一次出现位置的地址
	char*filename = strrchr(processdir, '/');
	if (filename == NULL)
	{
		return "";
	}
	*filename = '\0';

	string ret = processdir;
	return ret;
}

//创建目录、创建文件、写入标题
void CStrategy::CreateFile()
{

    time_t now = time(0);
    tm *ltm = localtime(&now);
    string szToday = to_string((1900 + ltm->tm_year) * 10000 + (1 + ltm->tm_mon) * 100 + ltm->tm_mday);
	string szExecutablePath = GetExecutablePath();

	//创建bar目录
	string szMkdir = string("cd ") + szExecutablePath + string("; mkdir bar_data; cd bar_data; mkdir ") + szToday + string(";");
	int Ret = system(szMkdir.c_str());
    if (Ret == -1)
    {
        cout << "创建目录失败" << endl;
    }
	//创建kama目录
	string szMkdirKama = string("cd ") + szExecutablePath + string("; mkdir kama_data; cd kama_data; mkdir ") + szToday + string(";");
	int RetKama = system(szMkdirKama.c_str());
	if (RetKama == -1)
	{
		cout << "创建目录失败" << endl;
	}

    //创建bar文件
	string szFile = szExecutablePath + "/bar_data/" + szToday + "/Bar.csv";

	m_BarStream.open(szFile.c_str(), ios::out);

	if ( !m_BarStream.is_open() )
	{

    	std::cerr << "Failed to open the file: " << szFile << std::endl;
    	//return;

	}

	//创建kama文件
	string szFileKama = szExecutablePath + "/kama_data/" + szToday + "/Kama.csv";
	m_KamaStream.open(szFileKama.c_str(), ios::out);

	if (!m_KamaStream.is_open())
	{
		std::cerr << "Failed to open the file: " << szFileKama << std::endl;
		//return;
	}


	//写入bar标题
	string szTitle = "Date,Time,InstID,Open,High,Low,Close,Volume,OpenInterest,\n";
	m_BarStream.write(szTitle.c_str(), szTitle.size());
	m_BarStream.flush();

	//写入kama标题
	string szTitleKama = "Date,Time,InstID,Kama,ER,High,Low\n";
	m_KamaStream.write(szTitleKama.c_str(), szTitleKama.size());
	m_KamaStream.flush();
}

void CStrategy::WriteBarData(Bar &bar)
{
	string szDate = bar.sDate;
	string szTime = bar.sTime;
	string szInstrumentID = bar.sInstrumentID;
	double dOpen = bar.dOpen;
	double dHigh = bar.dHigh;
	double dLow = bar.dLow;
	double dClose = bar.dClose;
	double dVolume = bar.dVolume;
	double dOpenInterest = bar.dOpenInterest;

	string szBarData = szDate + "," + szTime + "," + szInstrumentID + "," + to_string(dOpen) + "," + to_string(dHigh) + "," + to_string(dLow) + "," + to_string(dClose) + "," + to_string(dVolume) + "," + to_string(dOpenInterest) + ",\n";

	m_BarStream.write(szBarData.c_str(), szBarData.size());
	m_BarStream.flush();
}

void CStrategy::ReadBarData()
{
	//获取../bar_data/目录下的所有文件夹名称，并转化为int类型，找出最大的文件夹名称
	string szExecutablePath = GetExecutablePath();
	string szMkdir = string("cd ") + szExecutablePath + string("; cd bar_data; ls -l | grep ^d | awk '{print $9}'");
	FILE *fp = popen(szMkdir.c_str(), "r");
	if (fp == NULL)
	{
		cout << "popen fail" << endl;
		return;
	}
	char szBuf[1024];
	int iMax = 0;
	while (fgets(szBuf, sizeof(szBuf), fp) != NULL)
	{
		int i = atoi(szBuf);
		if (i > iMax)
		{
			iMax = i;
		}
	}
	pclose(fp);

	//读取最大文件夹下的Bar.csv文件
	string szFile = szExecutablePath + "/bar_data/" + to_string(iMax) + "/Bar.csv";
	ifstream BarStream(szFile.c_str());
	if (!BarStream.is_open())
	{
		cout << "Failed to open the file: " << szFile << endl;
		return;
	}

	//读取标题
	string szTitle;
	getline(BarStream, szTitle);

	//读取数据
	string szLine;
	while (getline(BarStream, szLine))
	{

		stringstream ss(szLine);
		string szDate;
		string szTime;
		string szInstrumentID;
		string szOpen;
		string szHigh;
		string szLow;
		string szClose;
		string szVolume;
		string szOpenInterest;

		getline(ss, szDate, ',');
		getline(ss, szTime, ',');
		getline(ss, szInstrumentID, ',');
		getline(ss, szOpen, ',');
		getline(ss, szHigh, ',');
		getline(ss, szLow, ',');
		getline(ss, szClose, ',');
		getline(ss, szVolume, ',');
		getline(ss, szOpenInterest, ',');

		shared_ptr<Bar> bar = make_shared<Bar>();

		bar->sDate = szDate;
		bar->sTime = szTime;
		bar->sInstrumentID = szInstrumentID;
		bar->dOpen = stod(szOpen);
		bar->dHigh = stod(szHigh);
		bar->dLow = stod(szLow);
		bar->dClose = stod(szClose);
		bar->dVolume = stod(szVolume);
		bar->dOpenInterest = stod(szOpenInterest);

		m_mmDequeSubBar[szInstrumentID][m_iBarCount].push_back(bar);

		if(m_mmDequeSubBar[szInstrumentID][m_iBarCount].size() > m_iSubCount)
		{
			m_mmDequeSubBar[szInstrumentID][m_iBarCount].pop_front();
		}

	}

	return;

}

void CStrategy::SelectStrategy(int iNumStrategy, string sInstrumentID)
{
	switch (iNumStrategy)
	{
	case 1:
		// KAMA均线策略
		KamaStrategy(sInstrumentID);
		break;
	default:
		break;
	}
}

void CStrategy::KamaStrategy(string sInstrumentID)
{
	//计算KAMA均线
	if(m_bIsNewBar)
	{
		CalculateKama(sInstrumentID);


		m_bIsNewBar = false;

	}

}

void CStrategy::CalculateKama(string sInstrumentID)
{
	deque<shared_ptr<Bar>> dBar = m_mmDequeSubBar[sInstrumentID][m_iBarCount];

	if (dBar.size() > m_iDirection)
	{
		double dSum = 0;

		double dDirection = 0;

		//从第20个元素遍历到最后一个元素，用智能指针的方式访问dBar中的元素
		deque<shared_ptr<Bar>>::iterator it = dBar.begin();

		int ishift = dBar.size() - m_iDirection;

		advance(it, ishift);

		for(;it != dBar.end(); it++)
		{
			dSum += abs((*it)->dClose - (*(it - 1))->dClose);
		}

		dDirection = dBar.back()->dClose - dBar[ishift - 1]->dClose;


		double dER = abs(dDirection) / dSum;

		double smooth = dER * (m_dKamaFast - m_dKamaSlow) + m_dKamaSlow;

		double c = pow(smooth, 2);

		// 计算KAMA

		shared_ptr<KAMA> kama = make_shared<KAMA>();

//  	if (m_mDequeKama.count(sInstrumentID) == 0)
// 		{
// 			deque<shared_ptr<KAMA>> dKama;
// 			m_mDequeKama.insert(pair<string, deque<shared_ptr<KAMA>>>(sInstrumentID, dKama));
// 		}

		if (m_mDequeKama[sInstrumentID].size() == 0)
		{

			double KamaOld = dBar[dBar.size() - 2]->dClose;

			kama->dKama = KamaOld + c * (dBar.back()->dClose - KamaOld);
			kama->sDate = dBar.back()->sDate;
			kama->sTime = dBar.back()->sTime;
			kama->sInstrumentID = dBar.back()->sInstrumentID;
			kama->dEfficiencyRatio = dER;
			kama->dHigh = dBar.back()->dClose;
			kama->dLow = dBar.back()->dClose;

			deque<shared_ptr<KAMA>> dKama;
			m_mDequeKama.insert(pair<string, deque<shared_ptr<KAMA>>>(sInstrumentID, dKama));

			m_mDequeKama[sInstrumentID].push_back(kama);

			WriteKamaData(*kama);
		}
		else
		{

			double KamaOld = m_mDequeKama[sInstrumentID].back()->dKama;

			kama->dKama = KamaOld + c * (dBar.back()->dClose - KamaOld);
			kama->sDate = dBar.back()->sDate;
			kama->sTime = dBar.back()->sTime;
			kama->sInstrumentID = dBar.back()->sInstrumentID;
			kama->dEfficiencyRatio = dER;

			if(kama->dKama < KamaOld)
			{
				kama->dLow = kama->dKama;
			}
			else
			{
				kama->dLow = m_mDequeKama[sInstrumentID].back()->dLow;
			}

			if(kama->dKama > KamaOld)
			{
				kama->dHigh = kama->dKama;
			}
			else
			{
				kama->dHigh = m_mDequeKama[sInstrumentID].back()->dHigh;
			}

			m_mDequeKama[sInstrumentID].push_back(kama);

			WriteKamaData(*kama);

			if (m_mDequeKama[sInstrumentID].size() > m_iKamaCount)
			{
				m_mDequeKama[sInstrumentID].pop_front();
			}
		}
	}
	else
	{
		// 如果dBar的元素个数小于m_iDirection，kama的值等于dBar的最后一个元素的收盘价

		shared_ptr<KAMA> kama = make_shared<KAMA>();

		// if (m_mDequeKama.count(sInstrumentID) == 0)
		// {
		// 	deque<shared_ptr<KAMA>> dKama;
		// 	m_mDequeKama.insert(pair<string, deque<shared_ptr<KAMA>>>(sInstrumentID, dKama));
		// }

		if (m_mDequeKama[sInstrumentID].size() == 0)
		{

			kama->dKama = dBar.back()->dClose;
			kama->sDate = dBar.back()->sDate;
			kama->sTime = dBar.back()->sTime;
			kama->sInstrumentID = dBar.back()->sInstrumentID;
			kama->dEfficiencyRatio = 0;
			kama->dHigh = dBar.back()->dClose;
			kama->dLow = dBar.back()->dClose;

			deque<shared_ptr<KAMA>> dKama;
			m_mDequeKama.insert(pair<string, deque<shared_ptr<KAMA>>>(sInstrumentID, dKama));

			m_mDequeKama[sInstrumentID].push_back(kama);

			WriteKamaData(*kama);
		}
		else
		{
			double KamaOld = m_mDequeKama[sInstrumentID].back()->dKama;

			kama->dKama = dBar.back()->dClose;
			kama->sDate = dBar.back()->sDate;
			kama->sTime = dBar.back()->sTime;
			kama->sInstrumentID = dBar.back()->sInstrumentID;
			kama->dEfficiencyRatio = 0;

			if(kama->dKama < KamaOld)
			{
				kama->dLow = kama->dKama;
			}
			else
			{
				kama->dLow = m_mDequeKama[sInstrumentID].back()->dLow;
			}

			if(kama->dKama > KamaOld)
			{
				kama->dHigh = kama->dKama;
			}
			else
			{
				kama->dHigh = m_mDequeKama[sInstrumentID].back()->dHigh;
			}

			m_mDequeKama[sInstrumentID].push_back(kama);

			WriteKamaData(*kama);
		}
	}
}

void CStrategy::WriteKamaData(KAMA &kama)
{
	string szDate = kama.sDate;
	string szTime = kama.sTime;
	string szInstrumentID = kama.sInstrumentID;
	double dKama = kama.dKama;
	double dER = kama.dEfficiencyRatio;
	double dHigh = kama.dHigh;
	double dLow = kama.dLow;

	string szKamaData = szDate + "," + szTime + "," + szInstrumentID + "," + to_string(dKama) + "," + to_string(dER) + "," + to_string(dHigh) + "," + to_string(dLow) + ",\n";

	m_KamaStream.write(szKamaData.c_str(), szKamaData.size());
	m_KamaStream.flush();

}

void CStrategy::ReadKamaData()
{
	//获取../kama_data/目录下的所有文件夹名称，并转化为int类型，找出最大的文件夹名称
	string szExecutablePath = GetExecutablePath();
	string szMkdir = string("cd ") + szExecutablePath + string("; cd kama_data; ls -l | grep ^d | awk '{print $9}'");
	FILE *fp = popen(szMkdir.c_str(), "r");
	if (fp == NULL)
	{
		cout << "popen fail" << endl;
		return;
	}
	char szBuf[1024];
	int iMax = 0;
	while (fgets(szBuf, sizeof(szBuf), fp) != NULL)
	{
		int i = atoi(szBuf);
		if (i > iMax)
		{
			iMax = i;
		}
	}
	pclose(fp);

	//读取最大文件夹下的Kama.csv文件
	string szFile = szExecutablePath + "/kama_data/" + to_string(iMax) + "/Kama.csv";
	ifstream KamaStream(szFile.c_str());
	if (!KamaStream.is_open())
	{
		cout << "Failed to open the file: " << szFile << endl;
		return;
	}

	//读取标题
	string szTitle;
	getline(KamaStream, szTitle);

	//读取数据
	string szLine;
	while (getline(KamaStream, szLine))
	{
		KAMA kama;
		stringstream ss(szLine);
		string szDate;
		string szTime;
		string szInstrumentID;
		string szKama;
		string szER;
		string szHigh;
		string szLow;

		getline(ss, szDate, ',');
		getline(ss, szTime, ',');
		getline(ss, szInstrumentID, ',');
		getline(ss, szKama, ',');
		getline(ss, szER, ',');
		getline(ss, szHigh, ',');
		getline(ss, szLow, ',');

		kama.sDate = szDate;
		kama.sTime = szTime;
		kama.sInstrumentID = szInstrumentID;
		kama.dKama = stod(szKama);
		kama.dEfficiencyRatio = stod(szER);
		kama.dHigh = stod(szHigh);
		kama.dLow = stod(szLow);

		m_mDequeKama[szInstrumentID].push_back(make_shared<KAMA>(kama));

		if(m_mDequeKama[szInstrumentID].size() > m_iKamaCount)
		{
			m_mDequeKama[szInstrumentID].pop_front();
		}

	}
}

double CStrategy::CalculateFilter(string sInstrumentID)
{
	deque<shared_ptr<KAMA>> dKama = m_mDequeKama[sInstrumentID];

	double stdev = 0.0;

	//计算dKama最后m_iFilterCount+1个元素的差值，并且计算这些差值的标准差
	if (dKama.size() > m_iFilterCount)
	{
		vector<double> vDiff;
		for(int i = dKama.size() - 1; i > dKama.size() - m_iFilterCount - 1; i--)
		{
			double dDiff = dKama[i]->dKama - dKama[i - 1]->dKama;
			vDiff.push_back(dDiff);
		}
		//用函数计算vDiff中元素的标准差
		double sum = std::accumulate(vDiff.begin(), vDiff.end(), 0.0);
		double mean = sum / vDiff.size();

		double sq_sum = std::inner_product(vDiff.begin(), vDiff.end(), vDiff.begin(), 0.0);
		stdev = std::sqrt(sq_sum / vDiff.size() - mean * mean);
	}

    return stdev * m_dFilter;

}

void CStrategy::CalculateKamaSignal(string sInstrumentID)
{

}

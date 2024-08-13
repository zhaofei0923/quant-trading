#include "save_data.h"
#include "config.h"
#include "sim_log.h"
#include "const.h"
#include "struct.h"
#include "message.h"
#include "convert.h"
#include "helper_time.h"
#include "mem_pool_mng.h"
#include <unistd.h>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

std::string GetExecutablePath()
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
CSaveData::CSaveData()
{
	m_pConvertBuf = new char[CONVERT_BUF_LENGTH];
}

CSaveData::~CSaveData()
{
	m_QuoteStream.close();
}
int CSaveData::Init()
{
	SetThreadCount(1);

	//创建目录
	string szToday = to_string(zutil::GetToday());
	string szExecutablePath = GetExecutablePath();
	string szMkdir = string("cd ") + szExecutablePath + string("; mkdir market_data; cd market_data; mkdir ") + szToday + string(";");
	system(szMkdir.c_str());

	//创建文件
	string szFile = szExecutablePath + "/market_data/" + szToday + "/Quote.csv";

	m_QuoteStream.open(szFile.c_str(), ios::out);

	if ( !m_QuoteStream.is_open() )
	{

    	std::cerr << "Failed to open the file: " << szFile << std::endl;
    	//return;

	}

	//写入标题
	string szTitle = "TradingDay,ActionDay,ExchangeId,StockCode,StockName,UpdateTime,LastPrice,PreSettlePrice,SettlePrice,PreClosePrice,OpenPrice,LowPrice,HighPrice,ClosePrice,UpperLimitPrice,LowerLimitPrice,Volume,PreOpenInterest,TurnOver,OpenInterest,PreDelta,CurrDelta,UpdateMillisec,AveragePrice,SellPrice1,SellPrice2,SellPrice3,SellPrice4,SellPrice5,BuyPrice1,BuyPrice2,BuyPrice3,BuyPrice4,BuyPrice5,SellQuantity1,SellQuantity2,SellQuantity3,SellQuantity4,SellQuantity5,BuyQuantity1,BuyQuantity2,BuyQuantity3,BuyQuantity4,BuyQuantity5,\n";

	m_QuoteStream.write(szTitle.c_str(), szTitle.size());
	m_QuoteStream.flush();

	return 0;
}
int CSaveData::Start()
{
	LInfo("Start CSaveData ");
	RunThreads();

	return 0;
}
int CSaveData::Stop()
{
	StopThreads();
	return 0;
}
int CSaveData::Release()
{
	return 0;
}
int CSaveData::DealMsg(void*pMsg, int iThreadId)
{
	if (pMsg != NULL)
	{
		ServerMsg*pServerMsg = (ServerMsg*)pMsg;
		ParseMsg(pServerMsg, iThreadId);

		MemPoolMng*pMemPoolMng = MemPoolMng::GetInstance();
		char*pBody = pServerMsg->pBody;
		if (pBody != NULL)
		{
			pMemPoolMng->DeleteMaxMemBlock(pBody);
		}

		pMemPoolMng->DeleteMaxMemBlock((char*)pServerMsg);
	}

	return 0;
}
int CSaveData::ParseMsg(ServerMsg*pMsg, int iThreadId)
{
	//转换格式
	memset(m_pConvertBuf, 0, CONVERT_BUF_LENGTH);
	MarketData*pQuote = (MarketData*)m_pConvertBuf;
	ConvertMarketData((CThostFtdcDepthMarketDataField *)pMsg->pBody, pQuote);

	//写入数据
	WriteData(*pQuote);

	return 0;
}
int CSaveData::WriteData(MarketData&Quote)
{

	stringstream szBuf;
	szBuf << Quote.szTradingDay << "," << Quote.szActionDay << "," << Quote.szExchangeId << "," << Quote.szStockCode << "," << Quote.szStockName << "," \
		<< Quote.szUpdateTime << "," << Quote.iLastPrice << "," << Quote.iPreSettlePrice << "," << Quote.iSettlePrice << "," << Quote.iPreClosePrice << ","  \
		<< Quote.iOpenPrice << "," << Quote.iLowPrice << "," << Quote.iHighPrice << "," << Quote.iClosePrice << "," << Quote.iUpperLimitPrice << "," << Quote.iLowerLimitPrice << ","\
		<< Quote.iVolume << "," << Quote.iPreOpenInterest << "," << Quote.iTurnOver << "," << Quote.iOpenInterest << "," << Quote.iPreDelta << "," << Quote.iCurrDelta << "," \
		<< Quote.iUpdateMillisec << "," << Quote.iAveragePrice << "," << Quote.iSellPrice1 << "," << Quote.iSellPrice2 << "," << Quote.iSellPrice3 << ","  \
		<< Quote.iSellPrice4 << "," << Quote.iSellPrice5 << "," << Quote.iBuyPrice1 << "," << Quote.iBuyPrice2 << "," << Quote.iBuyPrice3 << ","  \
		<< Quote.iBuyPrice4 << "," << Quote.iBuyPrice5 << "," << Quote.iSellQuantity1 << "," << Quote.iSellQuantity2 << "," << Quote.iSellQuantity3 << "," << Quote.iSellQuantity4 << "," \
		<< Quote.iSellQuantity5 << "," << Quote.iBuyQuantity1 << "," << Quote.iBuyQuantity2 << "," << Quote.iBuyQuantity3 << "," << Quote.iBuyQuantity4 << "," << Quote.iBuyQuantity5 << "," << endl;

	string szStr = szBuf.str();

	m_QuoteStream.write(szStr.c_str(), szStr.size());
	m_QuoteStream.flush();

	return 0;
}


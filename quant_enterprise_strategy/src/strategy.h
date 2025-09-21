#ifndef _STRATEGY_
#define _STRATEGY_
#include <iostream>
#include <map>
#include <list>
#include <string>
#include <memory>
#include "thread_group.h"
#include "const.h"
#include "message.h"
#include "struct.h"
#include <numeric>
#include <cmath>

using namespace std;

class  CStrategy :public CThreadGroup
{
private:
	CStrategy();
public:

	~CStrategy();

	static CStrategy *GetInstance()
	{
		// C++11 style singleton
		static CStrategy CTcpServerImpl;
		return &CTcpServerImpl;
	}

public:
	int Init();
	int Start();
	int Stop();
	int Release();
	virtual int DealMsg(void*pMsg, int iThreadId);
	void ExecuteStrategy(ServerMsg*pServerMsg);
public:
	//生成交易信号
	void MakeTradeSignal(ServerMsg*pServerMsg);
	//平均线策略
	bool AverageLine(MarketData*pMarketData, PlaceOrderReqT&OrderReq);
public:
	list<MarketData> m_LongQuote; //存储最新20个快照
	list<MarketData> m_ShortQuote; //存储最新5个快照
	int  m_iCount;
	bool m_IsSendOrder;

///生成订阅的Bar
public:
	bool IsValidTick(string sTickTime);//判断是否是有效的tick
	bool IsSubBar(string sBarTime, int iBarCount);//判断是否是订阅的Bar
	int  CalculateTimeDiff(string sTime1, string sTime2);//计算两个"09:00:00"类型时间格式的差值，单位为秒,stime2大于stime1
	void CalculateBar(MarketData*pMarketData);//计算一分钟Bar,用一分钟bar合成订阅的Bar
	void CalculateSubBar(string sInstrumentId);//计算订阅的Bar,用一分钟线合成。可以订阅1分钟，5分钟，15分钟，30分钟，60分钟，日线，周线，月线等
	//保存Bar数据到CSV文件
	string GetExecutablePath();//获取可执行文件路径
	void CreateFile();//创建目录、创建文件、写入标题
	void WriteBarData(Bar &bar);//写入Bar数据到CSV文件
public:
	int m_iBarCount;//订阅的Bar周期长度
	int m_iSubCount;//订阅的Bar数量
	map<string, shared_ptr<Bar>> m_mBar;//按照instrumentID存储正在更新的一分钟Bar值
	map<string, deque<shared_ptr<Bar>>> m_mDequeMinBar;//更新后的一分钟Bar值，存储到Deque容器中，map的key值为InstrumentID
	map<string, map<int, deque<shared_ptr<Bar>>>> m_mmDequeSubBar;//map的key值为InstrumentID，map的value值为map类型，map的key值为int,Bar的周期长度，map的value值为Deque容器，存储Bar类型的值
	fstream m_BarStream;//保存Bar数据到CSV文件
	bool m_bIsNewBar;//是否是新的Bar

public:
	void ReadBarData();//读取Bar.csv文件中的bar数据

//选择策略
public:
	int m_iStrategy;//选择的策略
	void SelectStrategy(int iNumStrategy,string sInstID);//选择策略 1:KAMA均线策略

//KAMA均线策略
public:

	int m_iKamaCount;//存储Kama的个数
	int m_iDirection;//Kama方向的计算长度
	double m_dKamaFast;//KAMA均线快速EMA的权重
	double m_dKamaSlow;//KAMA均线慢速EMA的权重
	int m_iFilterCount;//过滤器的计算长度
	double m_dFilter;//过滤器的权重
	map<string, deque<shared_ptr<KAMA>>> m_mDequeKama;//更新后的Kama值，存储到Deque容器中，map的key值为InstrumentID
	fstream m_KamaStream;//保存Kama数据到CSV文件

public:
	void KamaStrategy(string SInstrumentID);//KAMA均线策略
	void CalculateKama(string SInstrumentID);//计算KAMA均线
	void WriteKamaData(KAMA &kama);//写入Kama数据到CSV文件
	void ReadKamaData();//读取Kama.csv文件中的Kama数据
	double CalculateFilter(string sInstrumentID);//计算过滤器
	void CalculateKamaSignal(string sInstrumentID);//计算Kama信号

public:
	bool PlaceOrder(const string &stockCode, char action, double price, int entrustNum, string *outClientOrderId = nullptr);

};


#endif


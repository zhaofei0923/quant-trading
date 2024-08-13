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
	//���ɽ����ź�
	void MakeTradeSignal(ServerMsg*pServerMsg);
	//ƽ���߲���
	bool AverageLine(MarketData*pMarketData, PlaceOrderReqT&OrderReq);
public:
	list<MarketData> m_LongQuote; //�洢����20������
	list<MarketData> m_ShortQuote; //�洢����5������
	int  m_iCount;
	bool m_IsSendOrder;

///���ɶ��ĵ�Bar
public:
	bool IsValidTick(string sTickTime);//�ж��Ƿ�����Ч��tick
	bool IsSubBar(string sBarTime, int iBarCount);//�ж��Ƿ��Ƕ��ĵ�Bar
	int  CalculateTimeDiff(string sTime1, string sTime2);//��������"09:00:00"����ʱ���ʽ�Ĳ�ֵ����λΪ��,stime2����stime1
	void CalculateBar(MarketData*pMarketData);//����һ����Bar,��һ����bar�ϳɶ��ĵ�Bar
	void CalculateSubBar(string sInstrumentId);//���㶩�ĵ�Bar,��һ�����ߺϳɡ����Զ���1���ӣ�5���ӣ�15���ӣ�30���ӣ�60���ӣ����ߣ����ߣ����ߵ�
	//����Bar���ݵ�CSV�ļ�
	string GetExecutablePath();//��ȡ��ִ���ļ�·��
	void CreateFile();//����Ŀ¼�������ļ���д�����
	void WriteBarData(Bar &bar);//д��Bar���ݵ�CSV�ļ�
public:
	int m_iBarCount;//���ĵ�Bar���ڳ���
	int m_iSubCount;//���ĵ�Bar����
	map<string, shared_ptr<Bar>> m_mBar;//����instrumentID�洢���ڸ��µ�һ����Barֵ
	map<string, deque<shared_ptr<Bar>>> m_mDequeMinBar;//���º��һ����Barֵ���洢��Deque�����У�map��keyֵΪInstrumentID
	map<string, map<int, deque<shared_ptr<Bar>>>> m_mmDequeSubBar;//map��keyֵΪInstrumentID��map��valueֵΪmap���ͣ�map��keyֵΪint,Bar�����ڳ��ȣ�map��valueֵΪDeque�������洢Bar���͵�ֵ
	fstream m_BarStream;//����Bar���ݵ�CSV�ļ�
	bool m_bIsNewBar;//�Ƿ����µ�Bar

public:
	void ReadBarData();//��ȡBar.csv�ļ��е�bar����

//ѡ�����
public:
	int m_iStrategy;//ѡ��Ĳ���
	void SelectStrategy(int iNumStrategy,string sInstID);//ѡ����� 1:KAMA���߲���

//KAMA���߲���
public:

	int m_iKamaCount;//�洢Kama�ĸ���
	int m_iDirection;//Kama����ļ��㳤��
	double m_dKamaFast;//KAMA���߿���EMA��Ȩ��
	double m_dKamaSlow;//KAMA��������EMA��Ȩ��
	int m_iFilterCount;//�������ļ��㳤��
	double m_dFilter;//��������Ȩ��
	map<string, deque<shared_ptr<KAMA>>> m_mDequeKama;//���º��Kamaֵ���洢��Deque�����У�map��keyֵΪInstrumentID
	fstream m_KamaStream;//����Kama���ݵ�CSV�ļ�

public:
	void KamaStrategy(string SInstrumentID);//KAMA���߲���
	void CalculateKama(string SInstrumentID);//����KAMA����
	void WriteKamaData(KAMA &kama);//д��Kama���ݵ�CSV�ļ�
	void ReadKamaData();//��ȡKama.csv�ļ��е�Kama����
	double CalculateFilter(string sInstrumentID);//���������
	void CalculateKamaSignal(string sInstrumentID);//����Kama�ź�

};


#endif


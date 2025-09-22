#ifndef TRADE_DATA_H_
#define TRADE_DATA_H_
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include "sqlite3.h"
#include "struct.h"
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

class TradeData
{
public:
	TradeData();
	~TradeData();
public:
	static void Init();
	static void Close();
public:
	static void GetInstruments(vector<shared_ptr<InstrumentT>>&InstrumentDatas);

private:
	static sqlite3 *SqliteDb; // db handler
	static  map<string, shared_ptr<InstrumentT>> m_InstrumentMap; //�г�+��Լ���룬��Լ��Ϣ

};




#endif

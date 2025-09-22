#ifndef CFG_H_
#define CFG_H_
#include <string>
#include <fstream>
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

class ConfigData
{
public:
	ConfigData()
	{
	}

	~ConfigData()
	{

	}
public:
	static ConfigData* GetInstance()
	{
		static ConfigData instance;
		return &instance;
	}
public:
	//------------data in config file---------------
	int iLogLevel;

	string szTradeIp;
	int    iTradePort;
	string szMarketIp;
	int    iMarketPort;
	string szAdmin;
	string szAdminPassword;
	string szTrader;
	string szTraderPassword;

public:
	bool LoadConfig();

};




#endif

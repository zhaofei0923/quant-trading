#ifndef CFG_H_
#define CFG_H_
#include <string>
#include <fstream>
#include "struct.h"
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
	LogInfo  LogCfgData;
	CtpConfig CtpCfgData;
	TcpSvrInfo   TcpCfgData;

	string szDataBase;//���ݿ�·��
public:
	bool LoadConfig();


};




#endif

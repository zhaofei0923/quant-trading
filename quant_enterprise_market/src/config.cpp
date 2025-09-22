#include "config.h"
#include <iostream>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

bool ConfigData::LoadConfig()
{
	string config_file_path = string("./config/") + "config.json";
	ifstream file_in(config_file_path, ios::in);
	rapidjson::IStreamWrapper isw(file_in);

	rapidjson::Document config_doc;
	config_doc.ParseStream(isw);
	if (config_doc.HasParseError())
	{
		std::cout << "parse config file failed: " << config_doc.GetParseError() << endl;
		return false;
	}

	// get value
	try
	{
		if (false == config_doc.IsObject())
		{
			logic_error ex("config root parse error");
			throw exception(ex);
		}

		if ((false == config_doc.HasMember("Log")) || (false == config_doc["Log"].IsObject()))
		{
			logic_error ex("config Log parse error");
			throw exception(ex);;
		}


		if ((false == config_doc.HasMember("Ctp")) || (false == config_doc["Ctp"].IsObject()))
		{
			logic_error ex("config Ctp parse error");
			throw exception(ex);
		}

		if ((false == config_doc.HasMember("TcpSvr")) || (false == config_doc["TcpSvr"].IsObject()))
		{
			logic_error ex("config TcpSvr parse error");
			throw exception(ex);
		}


		if ((false == config_doc.HasMember("database")) || (false == config_doc["database"].IsString()))
		{
			logic_error ex("config database parse error");
			throw exception(ex);
		}



		//解析字段
		{

			rapidjson::Value &LogData = config_doc["Log"];
			LogCfgData.iLoglevel = LogData["LogLevel"].GetInt();

			//std::cout << "LogCfgData.iLoglevel: " << LogCfgData.iLoglevel << endl;

			rapidjson::Value &CtpData = config_doc["Ctp"];
			CtpCfgData.szTradeIp = CtpData["TradeIp"].GetString();
			CtpCfgData.iTradePort = CtpData["TradePort"].GetInt();
			CtpCfgData.szMarketIp = CtpData["MarketIp"].GetString();
			CtpCfgData.iMarketPort = CtpData["MarketPort"].GetInt();
			CtpCfgData.szBrokerId = CtpData["BrokerId"].GetString();
			CtpCfgData.szUserId = CtpData["User"].GetString();
			CtpCfgData.szPassword = CtpData["Password"].GetString();
			CtpCfgData.szAuthCode = CtpData["AuthCode"].GetString();
			CtpCfgData.szAppID = CtpData["AppID"].GetString();

			/*
			std::cout << "CtpCfgData.szTradeIp: " << CtpCfgData.szTradeIp << endl;
			std::cout << "CtpCfgData.iTradePort: " << CtpCfgData.iTradePort << endl;
			std::cout << "CtpCfgData.szMarketIp: " << CtpCfgData.szMarketIp << endl;
			std::cout << "CtpCfgData.iMarketPort: " << CtpCfgData.iMarketPort << endl;
			std::cout << "CtpCfgData.szBrokerId: " << CtpCfgData.szBrokerId << endl;
			std::cout << "CtpCfgData.szUser: " << CtpCfgData.szUserId << endl;
			std::cout << "CtpCfgData.szPassword: " << CtpCfgData.szPassword << endl;
			std::cout << "CtpCfgData.szAuthCode: " << CtpCfgData.szAuthCode << endl;
			std::cout << "CtpCfgData.szAppID: " << CtpCfgData.szAppID << endl;
			*/

			rapidjson::Value &TcpSvrData = config_doc["TcpSvr"];
			TcpCfgData.iPort = TcpSvrData["Port"].GetInt();
			//std::cout << "TcpCfgData.iPort: " << TcpCfgData.iPort << endl;

			szDataBase = config_doc["database"].GetString();
			//std::cout << "szDataBase: " << szDataBase << endl;


		}

	}
	catch (exception &e)
	{
		std::cout << "parse config file failed: " << e.what() << endl;
		file_in.close();
		return false;
	}

	file_in.close();

	return true;

}
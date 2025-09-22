#include "config.h"
#include "sim_log.h"
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
		std::cout << "parse config file failed:" << config_doc.GetParseError() << endl;
		return false;
	}

	// get value
	try
	{
		if (config_doc.IsObject()
			&& config_doc.HasMember("Trade") && config_doc["Trade"].IsObject()
			&& config_doc.HasMember("Market") && config_doc["Market"].IsObject()
			&& config_doc.HasMember("Log") && config_doc["Log"].IsObject())
		{

			rapidjson::Value &trade_config = config_doc["Trade"];
			szTradeIp = trade_config["Ip"].GetString();
			iTradePort = trade_config["Port"].GetInt();
			szAdmin = trade_config["Admin"].GetString();
			szAdminPassword = trade_config["AdminPassword"].GetString();
			szTrader = trade_config["Trader"].GetString();
			szTraderPassword = trade_config["TraderPassword"].GetString();


			rapidjson::Value &market_config = config_doc["Market"];
			szMarketIp = market_config["Ip"].GetString();
			iMarketPort = market_config["Port"].GetInt();


			rapidjson::Value &log_config = config_doc["Log"];
			iLogLevel = log_config["LogLevel"].GetInt();


			printf("iLogLevel=%d\n", iLogLevel);

			printf("szTradeIp=%s\n", szTradeIp.c_str());
			printf("iTradePort=%d\n", iTradePort);
			printf("szMarketIp=%s\n", szMarketIp.c_str());
			printf("iMarketPort=%d\n", iMarketPort);
			printf("szAdmin=%s\n", szAdmin.c_str());
			printf("szAdminPassword=%s\n", szAdminPassword.c_str());
			printf("szTrader=%s\n", szTrader.c_str());
			printf("szTraderPassword=%s\n", szTraderPassword.c_str());

		}
		else
		{
			logic_error ex("config root parse error");
			throw exception(ex);
		}
	}
	catch (exception &e)
	{
		std::cout << "parse config file failed:" << e.what() << endl;
		file_in.close();
		return false;
	}

	file_in.close();

	return true;

}
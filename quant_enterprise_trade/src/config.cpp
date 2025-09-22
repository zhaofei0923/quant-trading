#include "config.h"
#include <iostream>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

//判断字符串是否是数字
bool AllisNum(string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		int tmp = (int)str[i];
		if (tmp >= 48 && tmp <= 57)
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

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
			std::cout << "config root parse error " << endl;
			logic_error ex("config root parse error");
			throw exception(ex);
		}

		if (false == config_doc.HasMember("Log") && config_doc["Log"].IsObject())
		{
			std::cout << "config Log parse error " << endl;
			logic_error ex("config Log parse error");
			throw exception(ex);
		}

		if (false == config_doc.HasMember("TcpSvr") && config_doc["TcpSvr"].IsObject())
		{
			std::cout << "config TcpSvr parse error " << endl;
			logic_error ex("config TcpSvr parse error");
			throw exception(ex);
		}

		if (false == config_doc.HasMember("Users") && config_doc["Users"].IsArray())
		{
			std::cout << "config Users parse error" << endl;
			logic_error ex("config Users parse error");
			throw exception(ex);
		}

		if (config_doc.HasMember("database") && config_doc["database"].IsString())
		{
			//
		}
		else
		{
			std::cout << "config database parse error " << endl;
			logic_error ex("config database parse error");
			throw exception(ex);
		}

		//解析字段
		{

			rapidjson::Value &LogData = config_doc["Log"];
			LogCfgData.iLoglevel = LogData["LogLevel"].GetInt();

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

			rapidjson::Value &TcpSvrData = config_doc["TcpSvr"];
			TcpCfgData.iPort = TcpSvrData["Port"].GetInt();

			rapidjson::Value &UsersData = config_doc["Users"];
			for (auto &UserValue : UsersData.GetArray())
			{
				User UserInfo;
				UserInfo.iRoletype = UserValue["Role"].GetInt();
				UserInfo.szUserId = UserValue["UserId"].GetString();
				UserInfo.szPassword = UserValue["Password"].GetString();

				if (ROLE_TYPE_TRADER == UserInfo.iRoletype)
				{
					//交易账户格式：首位非0的VIR_ACCOUNT_LENGTH位数字
					//位数必须是VIR_ACCOUNT_LENGTH
					if (UserInfo.szUserId.size() != VIR_ACCOUNT_LENGTH)
					{
						std::cout << "trade user error,UserId: " << UserInfo.szUserId << endl;
					}

					//检查是否是数字
					if (AllisNum(UserInfo.szUserId))
					{
						//首位非0
						if (UserInfo.szUserId.c_str()[0] != '0')
						{
							TradeUsers.push_back(UserInfo);
						}
						else
						{
							std::cout << "trade user error,UserId: " << UserInfo.szUserId << endl;
						}
					}
					else
					{
						std::cout << "trade user error,UserId: " << UserInfo.szUserId << endl;
					}

				}
				else if (ROLE_TYPE_ADMIN == UserInfo.iRoletype)
				{
					AdminUsers.push_back(UserInfo);
				}
			}

			szDataBase = config_doc["database"].GetString();
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
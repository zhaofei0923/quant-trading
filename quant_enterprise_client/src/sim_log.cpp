#include "sim_log.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include <sstream>
#include <time.h>
#include <iostream>
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/


SimLog& SimLog::Instance()
{
	static SimLog log;
	return log;
}

void SimLog::InitSimLog(int log_level)
{
	std::string logger_name = "in"; //���ֶ���ʱ��ʹ��
	std::string file_name = "log";
	std::stringstream log_full_path;
	std::string log_part_path = "./log/";
	file_name = file_name + std::string("_") + GetDate() + std::string(".txt");
	log_full_path << log_part_path << file_name;

#ifdef _WIN32
	if (_access(log_part_path.c_str(), 0) != 0)
	{
		_mkdir(log_part_path.c_str());
	}
#else
	if (access(log_part_path.c_str(), F_OK) != 0)
	{
		mkdir(log_part_path.c_str(), S_IRWXU);  //���ȼ��logĿ¼�Ƿ���ڣ��������򴴽�
	}
#endif //_WIN32

	//������־Ϊ�첽��־��������������־�ļ���һֱд��
	//spdlog::set_level(static_cast<spdlog::level::level_enum>(log_level));
	//my_logger_ = spdlog::basic_logger_mt<spdlog::async_factory >(logger_name, file_name);
	//my_logger_ = spdlog::basic_logger_mt<spdlog::async_factory >(logger_name, log_full_path.str());

	// ���ڹ����ļ�����־��ÿ���ļ�10MB��9999���ļ�
	my_logger_ = spdlog::rotating_logger_mt(logger_name, log_full_path.str(), 1024 * 1024 * 10, 9999);
	spdlog::set_default_logger(my_logger_);
	my_logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
	//my_logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");

	//���������󼶱����ϵ�����ˢ�µ���־
	//my_logger_->flush_on(spdlog::level::err);

	my_logger_->flush_on(spdlog::level::trace);

	//ÿ1��ˢ��һ��
	spdlog::flush_every(std::chrono::seconds(1));


}

void SimLog::EndLog()
{
	spdlog::shutdown();
}

SimLog::SimLog()
{

}

SimLog::~SimLog()
{
	EndLog();
}


void SimLog::SetLevel(int level)
{
	spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
}
std::string  SimLog::GetDate()
{
	time_t now;
	struct tm *tm_now;
	time(&now);
	//windows���� ����---C/C++---Ԥ������---Ԥ���������� ����_CRT_SECURE_NO_WARNINGS;
	tm_now = localtime(&now);
	char szBuf[100] = { 0 };
	//sprintf(szBuf,"%d-%d-%d %d:%d:%d",tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(szBuf, "%d-%d-%d-%d-%d-%d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

	return szBuf;
}
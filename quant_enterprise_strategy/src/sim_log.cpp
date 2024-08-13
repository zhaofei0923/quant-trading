#include "sim_log.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include <sstream>
#include <time.h>
#include <iostream>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/


SimLog& SimLog::Instance()
{
	static SimLog log;
	return log;
}

void SimLog::InitSimLog(int log_level)
{
	std::string logger_name = "in"; //此字段暂时不使用
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
		mkdir(log_part_path.c_str(), S_IRWXU);  //首先检查log目录是否存在，不存在则创建
	}
#endif //_WIN32

	//设置日志为异步日志，不带滚动，日志文件会一直写入
	//spdlog::set_level(static_cast<spdlog::level::level_enum>(log_level));
	//my_logger_ = spdlog::basic_logger_mt<spdlog::async_factory >(logger_name, file_name);
	//my_logger_ = spdlog::basic_logger_mt<spdlog::async_factory >(logger_name, log_full_path.str());

	// 基于滚动文件的日志，每个文件10MB，9999个文件
	my_logger_ = spdlog::rotating_logger_mt(logger_name, log_full_path.str(), 1024 * 1024 * 10, 9999);
	spdlog::set_default_logger(my_logger_);
	my_logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
	//my_logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");

	//当遇到错误级别以上的立刻刷新到日志
	//my_logger_->flush_on(spdlog::level::err);

	my_logger_->flush_on(spdlog::level::trace);

	//每1秒刷新一次
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
	//windows：在 属性---C/C++---预处理器---预处理器定义 增加_CRT_SECURE_NO_WARNINGS;
	tm_now = localtime(&now);
	char szBuf[100] = { 0 };
	//sprintf(szBuf,"%d-%d-%d %d:%d:%d",tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(szBuf, "%d-%d-%d-%d-%d-%d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

	return szBuf;
}
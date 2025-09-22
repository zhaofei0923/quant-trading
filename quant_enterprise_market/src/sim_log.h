#ifndef _SIMLOG_H_
#define _SIMLOG_H_

//windows编译spdlog的时候，会报错，解决办法：在工程属性—>C/C++—>命令行—>其他选项 中添加：-D_SCL_SECURE_NO_WARNINGS
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#ifdef _WIN32
//strrchr:查找字符在指定字符串从右面开始的第一次出现的位置，如果成功，返回该字符以及后面的字符，如果失败，返回NULL
//strcgr:查找字符在指定字符串首次出现的位置
#define __FILENAME__ (strrchr(__FILE__,'\\')?(strrchr(__FILE__,'\\')+1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__,'/')?(strrchr(__FILE__,'/')+1):__FILE__)
#endif //_WIN32

#ifndef SUFFIX
//在错误级别的日志后面追加文件名，函数名，行号
/*
#define SUFFIX(msg) std::string(msg).append("  <")\
					.append(__FILENAME__).append("> <").append(__FUNCTION__)\
					.append("> <").append(std::to_string(__LINE__))\
					.append(">").c_str()
*/
#define SUFFIX(msg) (std::string(msg).append("  <")\
                    .append(__FILENAME__).append("> <").append(__FUNCTION__)\
                    .append("> <").append(std::to_string(__LINE__))\
                    .append(">").c_str())
#endif //suffix

/*
日志等级：trace,debug,info,warn,err ,critical
使用方法：包含simlog.h头文件,调用初始化函数,使用LDebug等打印日志信息
例：
SimLog::Instance().InitSimLog("scenario_edit", "scenario_edit_log.txt");
int i = 10;
double d_number = 10.01;
LDebug("SimLog::Async message");
LDebug("SimLog::Async message #{0},d_number:{1}", i,d_number);
注：使用{}格式化字符串，里面的数字为占位符
*/

#define LTrace(msg,...)  SimLog::Instance().GetLogger()->trace(SUFFIX(msg),__VA_ARGS__)
#define LDebug(...)  SimLog::Instance().GetLogger()->debug(__VA_ARGS__)
#define LInfo(...)  SimLog::Instance().GetLogger()->info(__VA_ARGS__)
#define LWarn(...) SimLog::Instance().GetLogger()->warn(__VA_ARGS__)

#ifdef _WIN32
#define LError(msg,...)  SimLog::Instance().GetLogger()->error(SUFFIX(msg),__VA_ARGS__)
#else
#define LError(msg,...)  SimLog::Instance().GetLogger()->error(SUFFIX(msg),##__VA_ARGS__)
#endif 

#define LCritical(...)  SimLog::Instance().GetLogger()->critical(__VA_ARGS__)
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class  SimLog
{
public:
	static SimLog&Instance();

	void InitSimLog(int log_level = spdlog::level::trace);

	void EndLog();

	void SetLevel(int level = spdlog::level::trace);
	std::string GetDate();

	std::shared_ptr<spdlog::logger>  GetLogger()
	{
		return my_logger_;
	}

private:
	//私有构造函数，拷贝构造函数和拷贝赋值函数，禁止在类外声明实例
	SimLog();
	~SimLog();
	SimLog(const SimLog &other) = delete;
	SimLog& operator=(const SimLog &other) = delete;

private:
	std::shared_ptr<spdlog::logger> my_logger_;
};

#endif //_SIMLOG_H_
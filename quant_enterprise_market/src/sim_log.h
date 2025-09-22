#ifndef _SIMLOG_H_
#define _SIMLOG_H_

//windows����spdlog��ʱ�򣬻ᱨ������취���ڹ������ԡ�>C/C++��>�����С�>����ѡ�� ����ӣ�-D_SCL_SECURE_NO_WARNINGS
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#ifdef _WIN32
//strrchr:�����ַ���ָ���ַ��������濪ʼ�ĵ�һ�γ��ֵ�λ�ã�����ɹ������ظ��ַ��Լ�������ַ������ʧ�ܣ�����NULL
//strcgr:�����ַ���ָ���ַ����״γ��ֵ�λ��
#define __FILENAME__ (strrchr(__FILE__,'\\')?(strrchr(__FILE__,'\\')+1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__,'/')?(strrchr(__FILE__,'/')+1):__FILE__)
#endif //_WIN32

#ifndef SUFFIX
//�ڴ��󼶱����־����׷���ļ��������������к�
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
��־�ȼ���trace,debug,info,warn,err ,critical
ʹ�÷���������simlog.hͷ�ļ�,���ó�ʼ������,ʹ��LDebug�ȴ�ӡ��־��Ϣ
����
SimLog::Instance().InitSimLog("scenario_edit", "scenario_edit_log.txt");
int i = 10;
double d_number = 10.01;
LDebug("SimLog::Async message");
LDebug("SimLog::Async message #{0},d_number:{1}", i,d_number);
ע��ʹ��{}��ʽ���ַ��������������Ϊռλ��
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
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//˽�й��캯�����������캯���Ϳ�����ֵ��������ֹ����������ʵ��
	SimLog();
	~SimLog();
	SimLog(const SimLog &other) = delete;
	SimLog& operator=(const SimLog &other) = delete;

private:
	std::shared_ptr<spdlog::logger> my_logger_;
};

#endif //_SIMLOG_H_
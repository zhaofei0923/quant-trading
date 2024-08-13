#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

using namespace std;

//获取可执行文件路径
string GetExecutablePath()
{
	char processdir[1000];
	if (readlink("/proc/self/exe", processdir, 1000) <= 0)
	{
		printf("readlink  fail\n");
		return  "";
	}
	//函数查找字符在指定字符串中最后一次出现的位置如果成功，则返回指定字符最后一次出现位置的地址
	char*filename = strrchr(processdir, '/');
	if (filename == NULL)
	{
		return "";
	}
	*filename = '\0';

	string ret = processdir;
	return ret;
}

//创建目录、创建文件、写入标题
void CreateFile()
{
    //创建目录
    //获取今天日期
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[20];
    sprintf(date, "%d%02d%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
    string szToday = date;
	string szExecutablePath = GetExecutablePath();
	string szMkdir = string("cd ") + szExecutablePath + string("; mkdir bar_data; cd bar_data; mkdir ") + szToday + string(";");
	int Ret = system(szMkdir.c_str());
    if (Ret == -1)
    {
        cout << "创建目录失败" << endl;
    }

    //创建文件
	string szFile = szExecutablePath + "/bar_data/" + szToday + "/Quote.csv";

	m_QuoteStream.open(szFile.c_str(), ios::out);

	if ( !m_QuoteStream.is_open() )
	{

    	std::cerr << "Failed to open the file: " << szFile << std::endl;
    	//return;

	}


}


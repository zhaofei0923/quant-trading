#ifndef _TOOLS_H
#define _TOOLS_H

#include <time.h>
#include <chrono>
#include <string>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

namespace zutil
{

	using namespace std;


	//dValue:待转换的值，iMULTIPLE:转换的倍数,例如1000
	inline unsigned long long ConvertDoubleToInt(double dValue, int iMULTIPLE)
	{
		unsigned long long iRet = 0;

		//由于19.9乘以1000时，可能会变成了19.89999*1000，所以加1
		//也有可能是19.90*1000
		iRet = dValue*iMULTIPLE + 1;
		//19901或者19900除以10
		iRet = iRet / 10;
		//1990乘以10
		iRet = iRet * 10;

		return iRet;
	}

}; // namespace zutil

#endif

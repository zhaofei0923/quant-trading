#ifndef _TOOLS_H
#define _TOOLS_H

#include <time.h>
#include <chrono>
#include <string>
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

namespace zutil
{

	using namespace std;


	//dValue:��ת����ֵ��iMULTIPLE:ת���ı���,����1000
	inline unsigned long long ConvertDoubleToInt(double dValue, int iMULTIPLE)
	{
		unsigned long long iRet = 0;

		//����19.9����1000ʱ�����ܻ�����19.89999*1000�����Լ�1
		//Ҳ�п�����19.90*1000
		iRet = dValue*iMULTIPLE + 1;
		//19901����19900����10
		iRet = iRet / 10;
		//1990����10
		iRet = iRet * 10;

		return iRet;
	}

}; // namespace zutil

#endif

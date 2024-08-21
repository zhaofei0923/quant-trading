// StdFrame.cpp : �������̨Ӧ�ó������ڵ㡣
#include "main_impl.h"
#include <stdio.h>
#include <chrono>
#include <thread>
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

int main()
{
	MainImpl*pMainImpl = new MainImpl();
	if (pMainImpl->Init() != 0)
	{
		printf("----MainImpl  Init fail-----\n");
		return -1;
	}

	if (pMainImpl->Start() != 0)
	{
		printf("----MainImpl  Start fail------\n");
		return -1;
	}

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 5));
	}

	return 0;
}


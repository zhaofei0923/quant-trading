// StdFrame.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <iostream>
#include <stdio.h>
#include <thread>
#include "main_impl.h"
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

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 5));
	}

	return 0;
}


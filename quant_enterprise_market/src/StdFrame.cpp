// StdFrame.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include <stdio.h>
#include <thread>
#include "main_impl.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
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


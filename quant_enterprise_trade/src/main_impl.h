#ifndef MAIN_IMPL_H_
#define MAIN_IMPL_H_
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class MainImpl
{
public:
	MainImpl()
	{
		m_bIsInit = false;
	}

	~MainImpl()
	{

	}
public:
	int Init();
	int Start();
	int Stop();
	int Release();
public:
	bool m_bIsInit;

};




#endif

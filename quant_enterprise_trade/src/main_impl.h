#ifndef MAIN_IMPL_H_
#define MAIN_IMPL_H_
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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

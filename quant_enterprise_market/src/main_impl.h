#ifndef MAIN_IMPL_H_
#define MAIN_IMPL_H_
#include "md_adapter_ctp.h"
#include "const.h"
#include "struct.h"
#include "message.h"
#include <vector>
#include <memory>
using namespace std;
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
		Stop();
		Release();
	}
public:
	int Init();
	int Start();
	int Stop();
	int Release();
public:
	bool m_bIsInit;
	MdAdapterCtp*m_pMdAdapterCtp;
};




#endif

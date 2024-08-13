#include "memory_pool.h"
#include <string.h>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

MemoryPool::MemoryPool(unsigned long long iBlockSize, unsigned long long iBlockNum)
{
	m_iBlockSize = iBlockSize;
	m_iBlockNum = iBlockNum;
	m_MemBlocks = new CThreadSafeStack<char*>();
	m_MemAddress = new CThreadSafeStack<char*>();

}
MemoryPool::~MemoryPool()
{
	Release();
}

int MemoryPool::Init()
{
	int iRet = CreateMemBlocks();
	return iRet;
}
int MemoryPool::Release()
{
	//释放申请的内存
	while (1)
	{
		char*pOut = m_MemAddress->pop();
		if (nullptr == pOut)
		{
			delete[] pOut;
			break;
		}
	}

	return 0;
}
int MemoryPool::CreateMemBlocks()
{
	//申请内存
	unsigned long long iLength = m_iBlockSize*m_iBlockNum;
	char*pData = new char[iLength];
	if (NULL == pData)
	{
		return -1;
	}
	m_MemAddress->push(pData);

	//分割m_iBlockNum个m_iBlockSize大小的内存块
	for (unsigned long long i = 0; i < m_iBlockNum; i++)
	{
		char*pBlock = &pData[i*m_iBlockSize];
		m_MemBlocks->push(pBlock);
	}

	printf("------CreateMemBlocks-----size=%d-----\n", m_iBlockNum);

	return 0;
}

char* MemoryPool::GetMemBlock()
{
	char*pOut = m_MemBlocks->pop();

	//如果地址用完，则重新申请地址
	if (nullptr == pOut)
	{
		CreateMemBlocks();
		pOut = m_MemBlocks->pop();
	}

	return pOut;
}
void MemoryPool::DeleteMemBlock(char*pMemBlock)
{
	m_MemBlocks->push(pMemBlock);
}
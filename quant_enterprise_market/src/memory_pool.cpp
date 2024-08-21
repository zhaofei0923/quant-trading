#include "memory_pool.h"
#include <string.h>
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//�ͷ�������ڴ�
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
	//�����ڴ�
	unsigned long long iLength = m_iBlockSize*m_iBlockNum;
	char*pData = new char[iLength];
	if (NULL == pData)
	{
		return -1;
	}
	m_MemAddress->push(pData);

	//�ָ�m_iBlockNum��m_iBlockSize��С���ڴ��
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

	//�����ַ���꣬�����������ַ
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
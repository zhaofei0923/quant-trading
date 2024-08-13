#include "mem_pool_mng.h"
#include <string.h>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

MemPoolMng::MemPoolMng()
{
	m_iMaxBlockSize = 1024;
	m_iMaxBlockNum = 1024;

	m_iMidBlockSize = 512;
	m_iMidBlockNum = 10;

	m_iMinBlockSize = 256;
	m_iMinBlockNum = 10;

	m_pMaxMemoryPool = NULL;
	m_pMidMemoryPool = NULL;
	m_pMinMemoryPool = NULL;
}
MemPoolMng::~MemPoolMng()
{
	Release();
}

int MemPoolMng::Init()
{
	m_pMaxMemoryPool = new MemoryPool(m_iMaxBlockSize, m_iMaxBlockNum);
	m_pMidMemoryPool = new MemoryPool(m_iMidBlockSize, m_iMidBlockNum);
	m_pMinMemoryPool = new MemoryPool(m_iMinBlockSize, m_iMinBlockNum);

	int iRet = 0;
	iRet = m_pMaxMemoryPool->Init();
	if (iRet != 0)
	{
		return -1;
	}

	iRet = m_pMidMemoryPool->Init();
	if (iRet != 0)
	{
		return -1;
	}

	iRet = m_pMinMemoryPool->Init();
	if (iRet != 0)
	{
		return -1;
	}

	return 0;
}

int MemPoolMng::Release()
{
	if (m_pMaxMemoryPool != NULL)
	{
		delete m_pMaxMemoryPool;
		m_pMaxMemoryPool = NULL;
	}

	if (m_pMidMemoryPool != NULL)
	{
		delete m_pMidMemoryPool;
		m_pMidMemoryPool = NULL;
	}

	if (m_pMinMemoryPool != NULL)
	{
		delete m_pMinMemoryPool;
		m_pMinMemoryPool = NULL;
	}

	return 0;
}
char* MemPoolMng::GetMaxMemBlock(int iSize)
{
	if (iSize > m_iMaxBlockSize)
	{
		//超过内存块大小，则返回NULL
		return NULL;
	}

	if (m_pMaxMemoryPool != NULL)
	{
		return m_pMaxMemoryPool->GetMemBlock();
	}

	return NULL;

}
void MemPoolMng::DeleteMaxMemBlock(char*pMemBlock)
{
	m_pMaxMemoryPool->DeleteMemBlock(pMemBlock);
}
char* MemPoolMng::GetMidMemBlock(int iSize)
{
	if (iSize > m_iMidBlockSize)
	{
		return NULL;
	}

	if (m_pMidMemoryPool != NULL)
	{
		return m_pMidMemoryPool->GetMemBlock();
	}

	return NULL;

}
void MemPoolMng::DeleteMidMemBlock(char*pMemBlock)
{
	m_pMidMemoryPool->DeleteMemBlock(pMemBlock);
}
char* MemPoolMng::GetMinMemBlock(int iSize)
{
	if (iSize > m_iMinBlockSize)
	{
		return NULL;
	}

	if (m_pMinMemoryPool != NULL)
	{
		return m_pMinMemoryPool->GetMemBlock();
	}

	return NULL;
}
void MemPoolMng::DeleteMinMemBlock(char*pMemBlock)
{
	m_pMinMemoryPool->DeleteMemBlock(pMemBlock);
}
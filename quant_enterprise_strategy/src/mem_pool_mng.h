#ifndef MEM_POOL_MNG_H_
#define MEM_POOL_MNG_H_
#include "memory_pool.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class MemPoolMng
{
private:
	MemPoolMng();
public:

	~MemPoolMng();
public:
	static MemPoolMng* GetInstance() {
		static MemPoolMng instance;
		return &instance;

	}
public:
	int Init();
	int Release();
	//获取大内存块
	char* GetMaxMemBlock(int iSize);
	//把大内存块放到栈中
	void DeleteMaxMemBlock(char*pMemBlock);
private:
	char* GetMidMemBlock(int iSize);
	void DeleteMidMemBlock(char*pMemBlock);
	char* GetMinMemBlock(int iSize);
	void DeleteMinMemBlock(char*pMemBlock);

public:
	int m_iMaxBlockSize; //大内存块的大小
	int m_iMaxBlockNum;  //大内存块的数量
	MemoryPool*m_pMaxMemoryPool;

	int m_iMidBlockSize;//中内存块的大小
	int m_iMidBlockNum;//中内存块的数量
	MemoryPool*m_pMidMemoryPool;

	int m_iMinBlockSize;//小内存块的大小
	int m_iMinBlockNum;	//小内存块的数量
	MemoryPool*m_pMinMemoryPool;

};




#endif

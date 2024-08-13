#ifndef MEM_POOL_H_
#define MEM_POOL_H_
#include <iostream> 
#include <string>
#include <vector>
#include <map>
#include <list>
#include <exception>
#include <mutex>
#include <thread>
#include "thread_safe_stack.h"
#define MEM_POOL_BLOCK_SIZE 1024  //块大小
#define MEM_POOL_BLOCK_NUM 1024   //块数量
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

class MemoryPool
{
public:
	MemoryPool(unsigned long long iBlockSize = MEM_POOL_BLOCK_SIZE, unsigned long long iBlockNum = MEM_POOL_BLOCK_NUM);
	~MemoryPool();

public:
	int Init();
	int Release();
	int CreateMemBlocks();
	char* GetMemBlock();
	void DeleteMemBlock(char*pMemBlock);

public:
	CThreadSafeStack<char*>* m_MemBlocks;//存放的分段的内存块
	CThreadSafeStack<char*>* m_MemAddress;//申请的内存的首地址
	unsigned long long m_iBlockSize;//内存块的大小
	unsigned long long m_iBlockNum; //内存块的数量




};




#endif

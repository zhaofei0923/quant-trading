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
#define MEM_POOL_BLOCK_SIZE 1024  //���С
#define MEM_POOL_BLOCK_NUM 1024   //������
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	CThreadSafeStack<char*>* m_MemBlocks;//��ŵķֶε��ڴ��
	CThreadSafeStack<char*>* m_MemAddress;//������ڴ���׵�ַ
	unsigned long long m_iBlockSize;//�ڴ��Ĵ�С
	unsigned long long m_iBlockNum; //�ڴ�������




};




#endif

#ifndef MEM_POOL_MNG_H_
#define MEM_POOL_MNG_H_
#include "memory_pool.h"
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
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
	//��ȡ���ڴ��
	char* GetMaxMemBlock(int iSize);
	//�Ѵ��ڴ��ŵ�ջ��
	void DeleteMaxMemBlock(char*pMemBlock);
private:
	char* GetMidMemBlock(int iSize);
	void DeleteMidMemBlock(char*pMemBlock);
	char* GetMinMemBlock(int iSize);
	void DeleteMinMemBlock(char*pMemBlock);

public:
	int m_iMaxBlockSize; //���ڴ��Ĵ�С
	int m_iMaxBlockNum;  //���ڴ�������
	MemoryPool*m_pMaxMemoryPool;

	int m_iMidBlockSize;//���ڴ��Ĵ�С
	int m_iMidBlockNum;//���ڴ�������
	MemoryPool*m_pMidMemoryPool;

	int m_iMinBlockSize;//С�ڴ��Ĵ�С
	int m_iMinBlockNum;	//С�ڴ�������
	MemoryPool*m_pMinMemoryPool;

};




#endif

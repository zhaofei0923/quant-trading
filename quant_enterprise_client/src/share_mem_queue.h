#ifndef __SHMFIFO_H__
#define __SHMFIFO_H__
#include<string.h>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#define COM_QUEUE_COMPONENT_NUM (6000)      //通讯队列中的元素个数
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

struct ShmQueueHead
{
	int iBlockNum;	//块数量	
	std::atomic<uint32_t> iReadIndex; //读索引
	std::atomic<uint32_t> iWriteIndex; //写索引   
};

template <class T>
class ShmQueue
{
public:
	//cType:c-客户端， s-服务端。服务端：创建共享内存，可以读写共享内存的内容。客户端：不创建共享内存，可以读写共享内存的内容
	//iBlockNum:客户端填0
	ShmQueue(int32_t iKey, char cType, uint32_t iBlockNum)
	{
		m_iKey = iKey;
		m_iBlockNum = iBlockNum;
		m_pT = NULL;
		m_cType = cType;
	}

	~ShmQueue()
	{
		printf("class ShmQueue:~ShmQueue()\n");
		//s-服务端  c-客户端
		if ('s' == m_cType)
		{
			//先卸载（即断开连接）
			shmdt(m_pStart);
			//销毁（即删除）
			shmctl(m_iShmId, IPC_RMID, 0);
		}
	}
	int Init()
	{

		void* pShm = NULL;
		int iLen = sizeof(ShmQueueHead) + sizeof(T) *m_iBlockNum;
		int iShmId = 0;

		//s-服务端  c-客户端
		if ('s' == m_cType)
		{
			//打开iLen大小的共享内存
			//0：取共享内存标识符，若不存在则函数会报错
			iShmId = shmget(m_iKey, iLen, 0);
			//存在共享内存，则释放
			if (-1 != iShmId)
			{
				printf("class ShmQueue:share memory is exist,start free share memory! key=[%d]  iShmId=[%d]\n", m_iKey, iShmId);
				void*pTmpShm = shmat(iShmId, 0, 0);
				//先卸载（即断开连接）
				shmdt(pTmpShm);
				//销毁（即删除）
				shmctl(iShmId, IPC_RMID, 0);
			}

			printf("class ShmQueue:start create share memory\n");
			//0644,它表示允许一个进程创建的共享内存被内存创建者所拥有的进程向共享内存读取和写入数据，同时其他用户创建的进程只能读取共享内存。
			iShmId = shmget(m_iKey, iLen, IPC_CREAT | 0666);
			if (-1 == iShmId)//创建失败
			{
				printf("class ShmQueue:start create share memory fail! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				return -1;
			}
			else//创建成功，则初始化共享内存结构体，p指向这个结构体
			{
				//创建成功
				printf("class ShmQueue:start create share memory OK! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				pShm = shmat(iShmId, 0, 0);
				memset(pShm, 0, iLen);
				m_pStart = (char*)pShm;

				m_pShmQueueHead = (ShmQueueHead*)((void*)m_pStart);
				m_pShmQueueHead->iBlockNum = m_iBlockNum;
				m_pShmQueueHead->iReadIndex.store(0);
				m_pShmQueueHead->iWriteIndex.store(0);
				m_pReadIndex = &(m_pShmQueueHead->iReadIndex); //读索引
				m_pWriteIndex = &(m_pShmQueueHead->iWriteIndex); //写索引				

				m_pData = m_pStart + sizeof(ShmQueueHead);
				m_pT = static_cast<T*>((void*)m_pData);
				m_iShmId = iShmId;

			}

		}
		else if ('c' == m_cType)
		{
			//打开iLen大小的共享内存
			//0：取共享内存标识符，若不存在则函数会报错
			iShmId = shmget(m_iKey, iLen, 0);
			//存在共享内存
			if (-1 != iShmId)
			{
				printf("class ShmQueue:share memory is exist! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				pShm = shmat(iShmId, 0, 0);
				m_pStart = (char*)pShm;

				m_pShmQueueHead = (ShmQueueHead*)((void*)m_pStart);
				m_iBlockNum = m_pShmQueueHead->iBlockNum;
				printf("class ShmQueue:m_iBlockNum=[%d]\n", m_iBlockNum);
				m_pReadIndex = &(m_pShmQueueHead->iReadIndex); //读索引
				m_pWriteIndex = &(m_pShmQueueHead->iWriteIndex); //写索引
				int r = m_pReadIndex->load();
				int w = m_pWriteIndex->load();
				printf("class ShmQueue:ReadIndex=[%d]   WriteIndex=[%d]\n", r, w);

				m_pData = m_pStart + sizeof(ShmQueueHead);
				m_pT = static_cast<T*>((void*)m_pData);
				m_iShmId = iShmId;

			}
			else //不存在共享内存
			{
				printf("class ShmQueue:share memory  is not exist! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				return -3;
			}

		}
		else
		{
			return -2;
		}


		return 0;

	}


	bool Write(T&data)
	{
		auto const currentWrite = m_pWriteIndex->load(std::memory_order_relaxed);
		auto nextRecord = currentWrite + 1;
		if (nextRecord == m_iBlockNum) {
			nextRecord = 0;
		}
		if (nextRecord != m_pReadIndex->load(std::memory_order_acquire))
		{
			memcpy(&m_pT[currentWrite], &data, sizeof(data));
			m_pWriteIndex->store(nextRecord, std::memory_order_release);
			return true;
		}

		return false;
	}


	bool Read(T& data)
	{
		auto const currentRead = m_pReadIndex->load(std::memory_order_relaxed);
		if (currentRead >= m_pWriteIndex->load(std::memory_order_acquire))
		{
			return false;
		}

		auto nextRecord = currentRead + 1;
		if (nextRecord == m_iBlockNum)
		{
			nextRecord = 0;
		}

		memcpy(&data, &m_pT[currentRead], sizeof(data));
		m_pReadIndex->store(nextRecord, std::memory_order_release);

		return true;
	}

private:
	uint32_t m_iBlockNum; //块数量
	int m_iKey;
	char*m_pStart; //共享内存起始地址
	char*m_pData;//共享内存装载数据的地址		
	T*  m_pT; //T数据首地址
	int m_iShmId;
	std::atomic<uint32_t>* m_pReadIndex; //读索引
	std::atomic<uint32_t>* m_pWriteIndex; //写索引
	char m_cType;  //s-服务端  c-客户端

	ShmQueueHead*m_pShmQueueHead;
};

#endif

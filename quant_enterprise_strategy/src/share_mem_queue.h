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
#define COM_QUEUE_COMPONENT_NUM (6000)      //ͨѶ�����е�Ԫ�ظ���
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

struct ShmQueueHead
{
	int iBlockNum;	//������	
	std::atomic<uint32_t> iReadIndex; //������
	std::atomic<uint32_t> iWriteIndex; //д����   
};

template <class T>
class ShmQueue
{
public:
	//cType:c-�ͻ��ˣ� s-����ˡ�����ˣ����������ڴ棬���Զ�д�����ڴ�����ݡ��ͻ��ˣ������������ڴ棬���Զ�д�����ڴ������
	//iBlockNum:�ͻ�����0
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
		//s-�����  c-�ͻ���
		if ('s' == m_cType)
		{
			//��ж�أ����Ͽ����ӣ�
			shmdt(m_pStart);
			//���٣���ɾ����
			shmctl(m_iShmId, IPC_RMID, 0);
		}
	}
	int Init()
	{

		void* pShm = NULL;
		int iLen = sizeof(ShmQueueHead) + sizeof(T) *m_iBlockNum;
		int iShmId = 0;

		//s-�����  c-�ͻ���
		if ('s' == m_cType)
		{
			//��iLen��С�Ĺ����ڴ�
			//0��ȡ�����ڴ��ʶ�����������������ᱨ��
			iShmId = shmget(m_iKey, iLen, 0);
			//���ڹ����ڴ棬���ͷ�
			if (-1 != iShmId)
			{
				printf("class ShmQueue:share memory is exist,start free share memory! key=[%d]  iShmId=[%d]\n", m_iKey, iShmId);
				void*pTmpShm = shmat(iShmId, 0, 0);
				//��ж�أ����Ͽ����ӣ�
				shmdt(pTmpShm);
				//���٣���ɾ����
				shmctl(iShmId, IPC_RMID, 0);
			}

			printf("class ShmQueue:start create share memory\n");
			//0644,����ʾ����һ�����̴����Ĺ����ڴ汻�ڴ洴������ӵ�еĽ��������ڴ��ȡ��д�����ݣ�ͬʱ�����û������Ľ���ֻ�ܶ�ȡ�����ڴ档
			iShmId = shmget(m_iKey, iLen, IPC_CREAT | 0666);
			if (-1 == iShmId)//����ʧ��
			{
				printf("class ShmQueue:start create share memory fail! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				return -1;
			}
			else//�����ɹ������ʼ�������ڴ�ṹ�壬pָ������ṹ��
			{
				//�����ɹ�
				printf("class ShmQueue:start create share memory OK! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				pShm = shmat(iShmId, 0, 0);
				memset(pShm, 0, iLen);
				m_pStart = (char*)pShm;

				m_pShmQueueHead = (ShmQueueHead*)((void*)m_pStart);
				m_pShmQueueHead->iBlockNum = m_iBlockNum;
				m_pShmQueueHead->iReadIndex.store(0);
				m_pShmQueueHead->iWriteIndex.store(0);
				m_pReadIndex = &(m_pShmQueueHead->iReadIndex); //������
				m_pWriteIndex = &(m_pShmQueueHead->iWriteIndex); //д����				

				m_pData = m_pStart + sizeof(ShmQueueHead);
				m_pT = static_cast<T*>((void*)m_pData);
				m_iShmId = iShmId;

			}

		}
		else if ('c' == m_cType)
		{
			//��iLen��С�Ĺ����ڴ�
			//0��ȡ�����ڴ��ʶ�����������������ᱨ��
			iShmId = shmget(m_iKey, iLen, 0);
			//���ڹ����ڴ�
			if (-1 != iShmId)
			{
				printf("class ShmQueue:share memory is exist! key=[%d] iShmId=[%d]\n", m_iKey, iShmId);
				pShm = shmat(iShmId, 0, 0);
				m_pStart = (char*)pShm;

				m_pShmQueueHead = (ShmQueueHead*)((void*)m_pStart);
				m_iBlockNum = m_pShmQueueHead->iBlockNum;
				printf("class ShmQueue:m_iBlockNum=[%d]\n", m_iBlockNum);
				m_pReadIndex = &(m_pShmQueueHead->iReadIndex); //������
				m_pWriteIndex = &(m_pShmQueueHead->iWriteIndex); //д����
				int r = m_pReadIndex->load();
				int w = m_pWriteIndex->load();
				printf("class ShmQueue:ReadIndex=[%d]   WriteIndex=[%d]\n", r, w);

				m_pData = m_pStart + sizeof(ShmQueueHead);
				m_pT = static_cast<T*>((void*)m_pData);
				m_iShmId = iShmId;

			}
			else //�����ڹ����ڴ�
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
	uint32_t m_iBlockNum; //������
	int m_iKey;
	char*m_pStart; //�����ڴ���ʼ��ַ
	char*m_pData;//�����ڴ�װ�����ݵĵ�ַ		
	T*  m_pT; //T�����׵�ַ
	int m_iShmId;
	std::atomic<uint32_t>* m_pReadIndex; //������
	std::atomic<uint32_t>* m_pWriteIndex; //д����
	char m_cType;  //s-�����  c-�ͻ���

	ShmQueueHead*m_pShmQueueHead;
};

#endif

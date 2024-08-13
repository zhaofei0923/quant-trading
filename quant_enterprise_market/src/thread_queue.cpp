#include "thread_queue.h"
#include "thread_group.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

int CThreadQueue::Init()
{

	return 0;
}
int CThreadQueue::Start()
{
	m_bRunFlag = true;

	m_pThread = new std::thread(&CThreadQueue::ThreadRun, this);
	m_pThread->detach();

	return 0;
}
void CThreadQueue::ThreadRun()
{
	while (true)
	{
		if (false == m_bRunFlag)
		{
			break;
		}

		std::unique_lock<std::mutex> lk(m_Mutex);

		while (m_Queue.empty())
		{
			m_Cond.wait(lk);
		}


		void *pMsg = m_Queue.front();
		m_Queue.pop();
		//DealMsg函数可能耗时比较长，使用unlock释放锁，让PostMsg输入数据
		lk.unlock();
		DealMsg(pMsg);

	}
	return;
}
int CThreadQueue::Release()
{
	if (m_pThread != NULL)
	{
		delete m_pThread;
		m_pThread = NULL;
	}
	return 0;
}
int CThreadQueue::PostMsg(void*pMsg)
{
	std::lock_guard<std::mutex> lk(m_Mutex);
	m_Queue.push(pMsg);
	m_Cond.notify_one();

	return 0;
}
int  CThreadQueue::Stop()
{
	m_bRunFlag = false;
	return 0;
}
int CThreadQueue::SetThreadId(int iThread)
{
	m_iThreadId = iThread;

	return 0;
}
int  CThreadQueue::SetThreadGroup(void*pThreadGroup)
{
	if (pThreadGroup != NULL)
	{
		m_pThreadGroup = pThreadGroup;
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;
}
int CThreadQueue::DealMsg(void*pMsg)
{
	CThreadGroup*pCThreadGroup = (CThreadGroup*)m_pThreadGroup;
	pCThreadGroup->DealMsg(pMsg, m_iThreadId);
	return 0;
}

#include "thread_group.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

int CThreadGroup::SetThreadCount(int iThreadCount)
{
	m_iThreadCount = iThreadCount;
	return 0;
}
int CThreadGroup::RunThreads()
{
	for (int i = 0; i < m_iThreadCount; i++)
	{
		CThreadQueue *pCThreadQueue = new CThreadQueue();
		m_vThread.push_back(pCThreadQueue);
		pCThreadQueue->SetThreadId(i);
		pCThreadQueue->SetThreadGroup(this);
		pCThreadQueue->Init();
		pCThreadQueue->Start();

	}

	m_bRunFlag = true;
	return 0;
}
int CThreadGroup::StopThreads()
{
	for (int i = 0; i < m_iThreadCount; i++)
	{
		CThreadQueue *pCThreadQueue = m_vThread[i];
		pCThreadQueue->Stop();
		pCThreadQueue->Release();
	}

	m_bRunFlag = false;
	return 0;
}
int CThreadGroup::PostMsg(void*pMsg)
{
	if (false == m_bRunFlag)
	{
		return -1;
	}

	int iSelect = 0;
	{
		std::lock_guard<std::mutex> lk(m_Mutex);
		m_iSelect++;
		m_iSelect %= m_iThreadCount;
	}
	iSelect = m_iSelect;
	m_vThread[iSelect]->PostMsg(pMsg);

	return 0;
}
int CThreadGroup::DealMsg(void*pMsg, int iThreadId)
{
	return 0;
}
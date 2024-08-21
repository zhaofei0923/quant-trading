#ifndef _THREAD_QUEUE_H_
#define _THREAD_QUEUE_H_
#include <fstream>
#include <iostream>
#include <string>
#include <atomic>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable> 
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

class  CThreadQueue
{
public:
	CThreadQueue()
	{
		m_pThread = NULL;
		m_bRunFlag = false;
		m_iThreadId = 0;
		m_pThreadGroup = NULL;
	}

	~CThreadQueue()
	{

	}

public:
	int Init();
	int Start();
	void ThreadRun();
	int SetThreadId(int iThread);
	int Release();
	int PostMsg(void*pMsg);
	int DealMsg(void*pMsg);
	int SetThreadGroup(void*pThreadGroup);
	int Stop();
public:
	int m_iThreadId;
	bool m_bRunFlag;
	void*m_pThreadGroup;
	std::mutex m_Mutex;
	std::queue<void*> m_Queue;
	std::condition_variable m_Cond;
	std::thread*m_pThread;

};



/*
void wait(unique_lock<mutex>& lck);
void wait(unique_lock<mutex>& lck, Predicate pred);
std::condition_variable�ṩ������ wait() ��������ǰ�̵߳��� wait() �󽫱�����(��ʱ��ǰ�߳�Ӧ�û��������mutex�������������� lck)��ֱ������ĳ���̵߳��� notify_* �����˵�ǰ�̡߳�
���̱߳�����ʱ���ú������Զ����� lck.unlock() �ͷ�����ʹ���������������������ϵ��̵߳��Լ���ִ�С����⣬һ����ǰ�̻߳��֪ͨ(notified��ͨ��������ĳ���̵߳��� notify_* �����˵�ǰ�߳�)��wait()����Ҳ���Զ����� lck.lock()��ʹ��lck��״̬�� wait ����������ʱ��ͬ��
�ڵڶ�������£��������� Predicate����ֻ�е� pred ����Ϊfalse ʱ���� wait() �Ż�������ǰ�̣߳��������յ������̵߳�֪ͨ��ֻ�е� pred Ϊ true ʱ�Żᱻ�����������˵ڶ�������������´��룺
while (!pred()) wait(lck);
*/

/*
���Ա������Ϊ�߳����ʱ����Ȼʮ�ּ�: ��this��Ϊ��һ���������ݽ�ȥ���ɡ�
#include <thread>
#include <iostream>

class Greet
{
	const char *owner = "Greet";
public:
	void SayHello(const char *name) {
		std::cout << "Hello " << name << " from " << this->owner << std::endl;
	}
};
int main() {
	Greet greet;

	std::thread thread(&Greet::SayHello, &greet, "C++11");
	thread.join();

	return 0;
}
//�����Hello C++11 from Greet
*/


#endif


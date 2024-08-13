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
作者：赵志根
微信：401985690
qq群号：450286917
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
std::condition_variable提供了两种 wait() 函数。当前线程调用 wait() 后将被阻塞(此时当前线程应该获得了锁（mutex），不妨设获得锁 lck)，直到另外某个线程调用 notify_* 唤醒了当前线程。
在线程被阻塞时，该函数会自动调用 lck.unlock() 释放锁，使得其他被阻塞在锁竞争上的线程得以继续执行。另外，一旦当前线程获得通知(notified，通常是另外某个线程调用 notify_* 唤醒了当前线程)，wait()函数也是自动调用 lck.lock()，使得lck的状态和 wait 函数被调用时相同。
在第二种情况下（即设置了 Predicate），只有当 pred 条件为false 时调用 wait() 才会阻塞当前线程，并且在收到其他线程的通知后只有当 pred 为 true 时才会被解除阻塞。因此第二种情况类似以下代码：
while (!pred()) wait(lck);
*/

/*
类成员函数做为线程入口时，仍然十分简单: 把this做为第一个参数传递进去即可。
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
//输出：Hello C++11 from Greet
*/


#endif


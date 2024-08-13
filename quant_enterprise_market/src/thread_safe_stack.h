#ifndef  THREAD_SAFE_STACK_H_
#define  THREAD_SAFE_STACK_H_
#include <atomic>
#include <string>
#include <iostream>
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

template<typename T>
struct node
{
	T data;
	node* next;
	node(const T& data) : data(data), next(nullptr) {}
};

template<typename T>
class CThreadSafeStack
{
	std::atomic<node<T>*> head;

public:
	CThreadSafeStack() :head(nullptr) {}
public:
	void push(const T& data)
	{
		node<T>* new_node = new node<T>(data);

		// put the current value of head into new_node->next
		new_node->next = head.load(std::memory_order_relaxed);

		// now make new_node the new head, but if the head
		// is no longer what's stored in new_node->next
		// (some other thread must have inserted a node just now)
		// then put that new head into new_node->next and try again
		while (!head.compare_exchange_weak(new_node->next, new_node, std::memory_order_release, std::memory_order_relaxed))
		{
			; // the body of the loop is empty
		}

	}

	T pop()
	{
		while (1) {
			auto result = head.load(std::memory_order_relaxed);
			if (result == nullptr)
			{
				//throw std::string("Cannot pop from empty stack");
				return nullptr;
			}

			if (head.compare_exchange_weak(result, result->next, std::memory_order_release, std::memory_order_relaxed))
			{
				return result->data;
			}

		}
	}
};



#endif
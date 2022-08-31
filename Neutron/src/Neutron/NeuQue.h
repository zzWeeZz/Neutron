#pragma once
#include "NetCommon.h"
namespace Neutron
{
	template<typename T>
	class NeuQue
	{
	public:
		NeuQue() = default;
		NeuQue(const NeuQue<T>&) = delete;
		void PushBack(const T& item);
		void PushFront(const T& item);
		T PopFront();
		T PopBack();
		T& Front();
		void Wait();
		bool Empty();
		size_t GetSize() const;
		void Clear();

	private:
		std::mutex m_Lock;
		std::mutex m_HardLock;
		std::condition_variable m_CV;
		std::deque<T> m_DeQueue;
	};

	template <typename T>
	void NeuQue<T>::PushBack(const T& item)
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		m_DeQueue.emplace_back(item);
		std::unique_lock<std::mutex> hardLock(m_HardLock);
		m_CV.notify_one();
	}

	template <typename T>
	void NeuQue<T>::PushFront(const T& item)
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		m_DeQueue.emplace_front(std::move(item));
		std::unique_lock<std::mutex> hardLock(m_HardLock);
		m_CV.notify_one();
	}

	template <typename T>
	T NeuQue<T>::PopFront()
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		auto front = std::move(m_DeQueue.front());
		m_DeQueue.pop_front();
		return front;
	}

	template <typename T>
	T NeuQue<T>::PopBack()
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		auto back = std::move(m_DeQueue.back());
		m_DeQueue.pop_back();
		return back;
	}

	template <typename T>
	T& NeuQue<T>::Front()
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		return m_DeQueue.front();
	}

	template <typename T>
	void NeuQue<T>::Wait()
	{
		while (Empty())
		{
			std::unique_lock<std::mutex> lock(m_HardLock);
			m_CV.wait(lock);
		}
	}

	template <typename T>
	bool NeuQue<T>::Empty()
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		return m_DeQueue.empty();
	}

	template <typename T>
	size_t NeuQue<T>::GetSize() const
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		return m_DeQueue.size();
	}

	template <typename T>
	void NeuQue<T>::Clear()
	{
		std::scoped_lock<std::mutex> lock(m_Lock);
		m_DeQueue.clear();
	}
}

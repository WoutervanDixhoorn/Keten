#pragma once

#include <queue>
#include <mutex>
#include <optional>

#define LOCK(x) std::lock_guard<std::mutex> guard(x)

namespace Keten {

	template <typename T>
	class ThreadSafeQueue {
	private:
		std::queue<T> m_queue;
		std::mutex m_queueMutex;

	public:
		ThreadSafeQueue() = default;

		void Push(const T& item)
		{
			LOCK(m_queueMutex);
			m_queue.push(item);
		}

		bool TryPop(T& outMessage)
		{
			LOCK(m_queueMutex);
			if (m_queue.empty()) return false;

			outMessage = m_queue.front();
			m_queue.pop();
			return true;
		}
		inline size_t Size() const 
		{ 
			LOCK(m_queueMutex);
			return m_queue.size(); 
		}
	};

}
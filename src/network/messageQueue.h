#pragma once

#include <queue>
#include <mutex>

namespace Keten {

	template <typename T>
	class MessageQueue {
	public:
		MessageQueue() = default;

		bool TryPop(T& outMessage) {
			std::lock_guard<std::mutex> lock(m_queueMutex);
			if (m_queue.empty()) return false;
			
			outMessage = m_queue.front();
			m_queue.pop();
			return true;
		}

		void Push(T& message) {
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_queue.push(message);
		}

	private:
		std::queue<T> m_queue;
		std::mutex m_queueMutex;
	};

}
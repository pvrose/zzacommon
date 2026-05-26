/*
	Copyright 2017-2026, Philip Rose, GM3ZZA

	This file is part of ZZACOMMON.

	ZZACOMMON is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

	ZZACOMMON is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with ZZACOMMON.
	If not, see <https://www.gnu.org/licenses/>.

*/

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

//! \brief A thread-safe queue for asynchronous communication between threads.
template<typename T>
class zc_async_queue {
	//! \brief The underlying queue to hold the data.
	std::queue<T> queue_;
	//! \brief Mutex to protect access to the queue.
	mutable std::mutex mutex_;
	//! \brief Condition variable to notify waiting threads when new data is available.
	std::condition_variable cond_;


public:
	//! \brief Push a new value into the queue and notify one waiting thread.
	void push(T value) {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(std::move(value));
		cond_.notify_one();
	}

	//! \brief Try to pop a value from the queue without blocking. Returns true if successful.
	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty()) return false;
		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	//! \brief Get a reference to the front element of the queue. Caller must ensure the queue is not empty.
	T& front() {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.front();
	}

	//! \brief Remove the front element of the queue. Caller must ensure the queue is not empty.
	void pop() {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.pop();
	}
	
	//! \brief Wait until the queue is not empty and pop the front element.
	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()) {
			cond_.wait(lock);  // Wait until notified by push()
		}
		value = std::move(queue_.front());
		queue_.pop();
	}

	//! \brief Check if the queue is empty.
	bool empty() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	//! \brief Clear all elements from the queue.
	void clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		while (!queue_.empty()) {
			queue_.pop();
		}
	}

	//! \brief Get the number of elements currently in the queue.
	size_t size() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}
};

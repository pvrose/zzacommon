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
#include <deque>
#include <mutex>
#include <condition_variable>

//! \brief A thread-safe queue for asynchronous communication between threads.
//! 
//! \todo Implement push_front and pop_back
template<typename T>
class zc_async_deque {
	//! \brief The underlying queue to hold the data.
	std::deque<T> queue_;
	//! \brief Mutex to protect access to the queue.
	mutable std::mutex mutex_;
	//! \brief Condition variable to notify waiting threads when new data is available.
	std::condition_variable cond_;


public:
	//! \brief Push a new value into the queue and notify one waiting thread.
	void push_back(T value) {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push_back(std::move(value));
		cond_.notify_one();
	}

	//! \brief Try to pop a value from the queue without blocking. Returns true if successful.
	bool try_pop_front(T& value) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty()) return false;
		value = std::move(queue_.front());
		queue_.pop_front();
		return true;
	}

	//! \brief Get a reference to the front element of the queue. Caller must ensure the queue is not empty.
	T& front() {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.front();
	}

	//! \brief Remove the front element of the queue. Caller must ensure the queue is not empty.
	void pop_front() {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.pop_front();
	}

	//! \brief Wait until the queue is not empty and pop the front element.
	void wait_and_pop_front(T& value) {
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()) {
			cond_.wait(lock);  // Wait until notified by push()
		}
		value = std::move(queue_.front());
		queue_.pop_front();
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

	//! \brief Get the indexed element current in the queue.
	T operator[](size_t i) const {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_[i];
	}

	//! \brief Manually lock the mutex for explicit multi-operation locking.
	//! \warning Must be paired with unlock(). Consider using a std::unique_lock or lock_guard wrapper instead.
	void lock() {
		mutex_.lock();
	}

	//! \brief Manually unlock the mutex after explicit locking.
	//! \warning Only call after lock() or try_lock() succeeded.
	void unlock() {
		mutex_.unlock();
	}

	//! \brief Try to manually lock the mutex without blocking.
	//! \return true if the lock was acquired, false otherwise.
	//! \warning If returns true, must be followed by unlock().
	bool try_lock() {
		return mutex_.try_lock();
	}

};

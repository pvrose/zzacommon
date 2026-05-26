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
	std::queue<T> queue_;
	mutable std::mutex mutex_;
	std::condition_variable cond_;


public:
	void push(T value) {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(std::move(value));
		cond_.notify_one();
	}

	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty()) return false;
		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	T& front() {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.front();
	}

	void pop() {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.pop();
	}

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()) {
			cond_.wait(lock);  // Wait until notified by push()
		}
		value = std::move(queue_.front());
		queue_.pop();
	}


	bool empty() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	size_t size() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}
};

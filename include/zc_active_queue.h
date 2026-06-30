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
#include <functional>

//! \brief A queue that asks for more data when it is empty.
//! It is not thread-safe and should be used in a single thread context.
template<typename T>
class zc_active_queue {
	//! \brief The underlying queue to hold the data.
	std::queue<T> queue_;
	//! \brief Function to call when the queue is empty.
	std::function<void(void*)> low_callback_;
	//! \brief User data to pass to the low_callback_ function.
	void* user_data_;
	//! \brief low limit (set in constructor)
	size_t low_limit_ = 0;

public:
	//! \brief Constructor.
	zc_active_queue(size_t low_limit)
		: low_limit_(low_limit) {
	}
	//! \brief Set the callback function to call when the queue is empty.
	//! \param callback The callback function to call when the queue is empty.
	//! \param user_data User data to pass to the callback function.
	//! \param immediate If true, call the callback immediately if the queue is empty.
	void set_low_callback(std::function<void(void*)> callback, void* user_data, bool immediate = false) {
		low_callback_ = callback;
		user_data_ = user_data;
		if (immediate && queue_.empty() && low_callback_) {
			low_callback_(user_data_);
		}
	}

	//! \brief Push a new value into the queue.
	void push(T value) {
		queue_.push(std::move(value));
	}

	//! \brief Pop a value from the queue. If the queue is empty, call the low_callback_ function.
	T pop() {
		if (queue_.empty()) {
			if (low_callback_) {
				low_callback_(user_data_);
			}
			if (queue_.empty()) {
				throw std::runtime_error("Queue is empty after low callback");
			}
		}
		T value = std::move(queue_.front());
		queue_.pop();
		return value;
	}

	//! \brief Check if the queue is empty.
	bool empty() const {
		return queue_.empty();
	}
};

/*
	Copyright 2026, Philip Rose, GM3ZZA

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

#include <stdexcept>
#include <array>

//! This class provides a running average (arithmetic mean) of the last N values.
//! 
//! It comprises a FIFO and an accumulator. During a steady state
//! as a number is popped into the FIFO it added to the accumulator
//! and the value popped from the FIFO is subtracted from the
//! accumulator. During the start-up phase when the size of the 
//! FIFO is not at the maximum size, numbers are pushed without
//! one being popped.
//! 
//! The mean value of the contents of the FIFO is returned 
//! calculated as the value of the accumulator divided by the
//! current size.
//! 
//! To avoid unnecessary reallocation of memory, the FIFO is 
//! implemented as a fixed size array and pointers.

template <class T, size_t N>
class zc_running_average {
	
public:
	//! Initialises all variables.
	zc_running_average() {
		clear();
	}

	//! Destructor
	~zc_running_average() {}

	//! Add a value onto the queue.
	//! 
	//! \param value The value to add.
	void add(T value) {
		if (size_ == N) {
			// Remove existing head
			accumulator_ -= fifo_[head_];
			head_ = (head_ + 1) % N;
		}
		else {
			size_ += 1;
		}
		fifo_[tail_] = value;
		tail_ = (tail_ + 1) % N;
		accumulator_ += value;
		mean_ = accumulator_ / size_;
	}

	//! Clear the running score
	void clear() {
		head_ = 0;
		tail_ = 0;
		size_ = 0;
		accumulator_ = static_cast<T>(0);
		mean_ = static_cast<T>(0);
	}

	//! Return the running average
	T value() const {
		if (size_ == 0) {
			throw(std::logic_error("No data in accumulator - cannot provide running average!"));
		}
		return mean_;
	}

	//! Return empty to allow user to interpret as they may.
	bool empty() const {
		return size_ == 0;
	}

private:
	//! The FIFO
	std::array<T, N> fifo_;
	//! Accumulator
	T accumulator_;
	//! Running arithmetic mean
	T mean_;
	//! Pointer to head of FIFO, i.e. first out
	size_t head_;
	//! Pointer to tail of FIFO, i.e. last in
	size_t tail_;
	//! Current size of FIFO.
	size_t size_;

};
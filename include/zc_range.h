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

#include <cfloat>
#include <limits>
#include <utility>

//! \brief Minimum and maximum values for data coordinates for an
//! individual coordinate.
//! 
//! Reset value indicates an empty range, which can be readily expanded.
template <class T>
class zc_range : public std::pair<T, T> {

public:

	static constexpr T UPPER_BOUND = std::numeric_limits<T>::max();
	static constexpr T LOWER_BOUND = std::numeric_limits<T>::lowest();

	//! \brief Default constructor - initializes the range to an empty range with first = DBL_MAX and second = -DBL_MAX.
	zc_range() : std::pair<T, T>(UPPER_BOUND, LOWER_BOUND) {}

	//! \brief Constructor with specified minimum and maximum values.
	zc_range(std::pair<T, T> range) : std::pair<T, T>(range) {}

	//! \brief Constructor with specified minimum and maximum values.
	zc_range(T min, T max) : std::pair<T, T>(min, max) {}

	//! \brief Union assignment operator - expands this range to include another range.
	//! \param other The other range to union with this range.
	//! \return A reference to this range after the union operation.
	zc_range& operator|=(const zc_range& other) {
		this->first = std::min(this->first, other.first);
		this->second = std::max(this->second, other.second);
		return *this;
	}

	//! \brief Add a single \p value to this range, expanding the range if necessary to include the value.
	zc_range& operator|=(double value) {
		this->first = std::min(this->first, value);
		this->second = std::max(this->second, value);
		return *this;
	}

	//! \brief Intersection assignment operator - narrows this range to the overlap with another range.
	//! \param other The other range to intersect with this range.
	//! \return A reference to this range after the intersection operation.
	zc_range& operator&=(const zc_range& other) {
		this->first = std::max(this->first, other.first);
		this->second = std::min(this->second, other.second);
		if (this->first > this->second) {
			// No intersection - set to empty range
			this->first = UPPER_BOUND;
			this->second = LOWER_BOUND;
		}
		return *this;
	}

	//! \brief Get the union of this range and another range, returning a new range object.
	zc_range operator|(const zc_range& other) const {
		zc_range result = *this;
		result |= other;
		return result;
	}

	//! \brief Get the intersection of this range and another range, returning a new range object.
	zc_range operator&(const zc_range& other) const {
		zc_range result = *this;
		result &= other;
		return result;
	}

	//! \brief Return true if ranges are equal (i.e. first and second are the same).
	bool operator==(const zc_range& other) const {
		return first == other.first && second == other.second;
	}

	//! \brief Return true if ranges are not equal (i.e. first or second are different).
	bool operator!=(const zc_range& other) const {
		return !(*this == other);
	}

	//! \brief Return true of other range is wholly contained within this range (i.e. this range is a superset of the other range).
	bool contains(const zc_range& other) const {
		return first <= other.first && second >= other.second;
	}

	//! \brief Return true if the range contains a specific value.
	bool contains(double value) const {
		return first <= value && second >= value;
	}

	//! \brief Return that the range is valid (i.e. first is less than or equal to second).
	bool is_valid() const {
		return first <= second;
	}

	//! \brief Return the size of the range (i.e. second - first).
	T size() const {
		return second - first;
	}
};


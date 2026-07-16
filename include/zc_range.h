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

//! \brief Minimum and maximum values for data coordinates for an
//! individual coordinate.
//! 
//! Reset value indicates an empty range, which can be readily expanded.
struct zc_range {
	double min = DBL_MAX; //!< Minimum value for the coordinate
	double max = -(DBL_MAX); //!< Maximum value for the coordinate

	//! \brief Union assignment operator - expands this range to include another range.
	//! \param other The other range to union with this range.
	//! \return A reference to this range after the union operation.
	zc_range& operator|=(const zc_range& other) {
		min = std::min(min, other.min);
		max = std::max(max, other.max);
		return *this;
	}

	//! \brief Add a single \p value to this range, expanding the range if necessary to include the value.
	zc_range& operator|=(double value) {
		min = std::min(min, value);
		max = std::max(max, value);
		return *this;
	}

	//! \brief Intersection assignment operator - narrows this range to the overlap with another range.
	//! \param other The other range to intersect with this range.
	//! \return A reference to this range after the intersection operation.
	zc_range& operator&=(const zc_range& other) {
		min = std::max(min, other.min);
		max = std::min(max, other.max);
		if (min > max) {
			// No intersection - set to empty range
			min = DBL_MAX;
			max = -DBL_MAX;
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

	//! \brief Return true if ranges are equal (i.e. min and max are the same).
	bool operator==(const zc_range& other) const {
		return min == other.min && max == other.max;
	}

	//! \brief Return true if ranges are not equal (i.e. min or max are different).
	bool operator!=(const zc_range& other) const {
		return !(*this == other);
	}

	//! \brief Return true of other range is wholly contained within this range (i.e. this range is a superset of the other range).
	bool contains(const zc_range& other) const {
		return min <= other.min && max >= other.max;
	}

	//! \brief Return true if the range contains a specific value.
	bool contains(double value) const {
		return min <= value && max >= value;
	}

	//! \brief Return that the range is valid (i.e. min is less than or equal to max).
	bool is_valid() const {
		return min <= max;
	}
};


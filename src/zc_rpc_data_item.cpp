/*
	Copyright 2025-2026, Philip Rose, GM3ZZA
	
    This file is part of ZZALOG. Amateur Radio Logging Software.

    ZZALOG is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZALOG is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZALOG. 
	If not, see <https://www.gnu.org/licenses/>. 

*/
#include "zc_rpc_data_item.h"
#include "zc_utils.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>

// Constructor - sets default values
zc_rpc_data_item::zc_rpc_data_item()
: type_(XRT_EMPTY)
, i_(0)
, s_("")
, d_(nan(""))
, array_(nullptr)
, struct_(nullptr)

{
}

// Set the data to integer value (for either integer or Boolean RPC item
void zc_rpc_data_item::set(int32_t i, rpc_data_t type) {
	if (type == XRT_INT || type == XRT_BOOLEAN) {
		type_ = type;
		i_ = i;
	}
}

// Set the data to a string type
void zc_rpc_data_item::set(std::string s, rpc_data_t type) {
	if (type == XRT_BYTES || type == XRT_DATETIME || type == XRT_STRING) {
		type_ = type;
		s_ = s;
	}
	else if (type == XRT_DEFAULT) {
		// Some servers send bad RPC with integers and doubles supplied as default 
		// type which is string
		type_ = type;
		s_ = s;
		// Try it as integer
		try {
			i_ = std::stoi(s_.c_str());
		}
		catch (std::invalid_argument&) {
			i_ = 0;
		}
		// Try it as double
		try {
			d_ = std::stod(s_.c_str());
		}
		catch (std::invalid_argument&) {
			d_ = nan("");
		}
	}
}

// Set a double
void zc_rpc_data_item::set(double d) {
	type_ = XRT_DOUBLE;
	d_ = d;
}

// Set an array
void zc_rpc_data_item::set(zc_rpc_data_item::rpc_array* ap) {
	type_ = XRT_ARRAY;
	array_ = ap;
}

// Set a struct
void zc_rpc_data_item::set(zc_rpc_data_item::rpc_struct* mp) {
	type_ = XRT_STRUCT;
	struct_ = mp;
}

// Destructor
zc_rpc_data_item::~zc_rpc_data_item()
{
	// Destroy compound types
	switch (type_) {
	case XRT_ARRAY:
		// For each element of the array
		for (auto it = array_->begin(); it != array_->end(); it++) {
			// destroy it
			delete *it;
		}
		array_->clear();
		delete array_;
		break;
	case XRT_STRUCT:
		// For each element in a structure
		for (auto it = struct_->begin(); it != struct_->end(); it++) {
			// destroy it
			delete it->second;
		}
		struct_->clear();
		delete struct_;
		break;
	default:
		break;
	}
}

// Return the data type
rpc_data_t zc_rpc_data_item::type() const {
	return type_;
}

// Get the integer
bool zc_rpc_data_item::get(int32_t& i) const {
	if (type_ == XRT_INT || type_ == XRT_BOOLEAN || type_ == XRT_DEFAULT) {
		i = i_;
		return true;
	}
	else {
		// Not an integer type
		return false;
	}
}

// Get the integer
int zc_rpc_data_item::get_int() const {
	return i_;
}

// Get the string
bool zc_rpc_data_item::get(std::string& s) const {
	if (type_ == XRT_STRING || type_ == XRT_BYTES || type_ == XRT_DATETIME) {
		s = s_;
		return true;
	}
	else {
		// Not a string type
		return false;
	}
}

// return string
std::string zc_rpc_data_item::get_string() {
	return s_;
}

// Get the double
bool zc_rpc_data_item::get(double& d) const {
	if (type_ == XRT_DOUBLE || type_ == XRT_DEFAULT) {
		d = d_;
		return true;
	}
	else {
		// Not a double
		return false;
	}
}

// Return the double
double zc_rpc_data_item::get_double() const {
	return d_;
}

// Get the array
bool zc_rpc_data_item::get(rpc_array*& ap) {
	if (type_ == XRT_ARRAY) {
		ap = array_;
		return true;
	}
	else {
		// Not an array
		return false;
	}
}

// Returns the array
zc_rpc_data_item::rpc_array* zc_rpc_data_item::get_array() {
	return array_;
}

// Get the structure
bool zc_rpc_data_item::get(rpc_struct*& mp) {
	if (type_ == XRT_STRUCT) {
		mp = struct_;
		return true;
	}
	else {
		// Not a structure
		return false;
	}
}

// Returns the struct
zc_rpc_data_item::rpc_struct* zc_rpc_data_item::get_struct() {
	return struct_;
}

// Convert the item to text for display
std::string zc_rpc_data_item::print_item() {
	char temp[1024];
	std::string result = "";
	switch (type_) {
	case XRT_BOOLEAN:
		// Display 0 or 1
		snprintf(temp, 1024, "Boolean: %1d\n", i_);
		result = temp;
		break;
	case XRT_INT:
		// Integer
		snprintf(temp, 1024, "Int: %d\n", i_);
		result = temp;
		break;
	case XRT_DOUBLE:
		// Double
		snprintf(temp, 1024, "Double: %f\n", d_);
		result = temp;
		break;
	case XRT_STRING:
		// String
		snprintf(temp, 1024, "String: %s\n", s_.c_str());
		result = temp;
		break;
	case XRT_DATETIME:
		// Date/Time as string
		snprintf(temp, 1024, "Date/Time: %s\n", s_.c_str());
		result = temp;
		break;
	case XRT_BYTES:
		// Base64 encoding as string
		snprintf(temp, 1024, "Base64: %s\n", zc::encode_base_64(s_).c_str());
		result = temp;
		break;
	case XRT_ARRAY:
		// Array
		result = "rpc_array:\n";
		// For each item in the array - append it text
		for (auto it = array_->begin(); it != array_->end(); it++) {
			result += (*it)->print_item();
		}
		break;
	case XRT_STRUCT:
		// Structure
		result = "rpc_struct:\n";
		// For each element in the structure - appends its nae and text
		for (auto it = struct_->begin(); it != struct_->end(); it++) {
			std::string key = it->first;
			zc_rpc_data_item* item = it->second;
			snprintf(temp, 1024, "Name: %s\nValue: ", key.c_str());
			result += temp;
			result += item->print_item();
		}
		break;
	default:
		break;
	}
	return result;
}

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

#include <string>

//! \file zc_app.h
//! This file provides generic application-level data and methods.

class zc_app {

public:

	static std::string APP_NAME;    //!< The name of the application
	static std::string APP_VERSION; //!< The version of the application
	static std::string APP_VENDOR;  //!< The vendor of the application
	static std::string APP_TIMESTAMP;  //!< The build timestamp of the application	

	typedef uint32_t debug_flag;  //!< Type for debug flags - use bitwise operations to set and test individual flags

	//! \brief Returns true if the debug flag is set.
	static bool debug(debug_flag flag) {
		return (debug_flags_ & flag) != 0;
	}

	//! \brief Set the debug flag.
	static void set_debug(debug_flag flag) {
		debug_flags_ |= flag;
	}

	//! \brief Clear the debug flag.
	static void clear_debug(debug_flag flag) {
		debug_flags_ &= ~flag;
	}


private:
	static debug_flag debug_flags_; //!< Debug flags.

};

//! \brief Next spate debug flag
const zc_app::debug_flag DEBUG_QUICK = 1 << 0;
const zc_app::debug_flag DEBUG_NEXT = DEBUG_QUICK << 1;

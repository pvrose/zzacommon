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

#include <cstdint>
#include <string>

//! \file zc_app.h
//! This file provides generic application-level data and methods.

typedef uint32_t debug_flag;  //!< Type for debug flags - use bitwise operations to set and test individual flags

//! \brief Class to provide generic application-level data and methods.
//! These will generally be set from the CMakeLists.txt file through the use
//! of CMake's configure_file() function on file zc_app.cpp.in.
//! This allows a standard look-and-feel for all GM3ZZA applications.
//! The following global variables are available to all applications by 
//! declaring them as extern.

//! \code
//! std::string APP_NAME = "@APP_NAME@";
//! std::string APP_VERSION = "@APP_VERSION@";
//! std::string APP_VENDOR = "@APP_VENDOR@";
//! std::string APP_TIMESTAMP = "@APP_TIMESTAMP@";
//! std::string APP_SOURCE_DIR = "@APP_SOURCE_DIR@";
//! //Program copyright - displayed in all windows.
//! std::string COPYRIGHT = "\302\251 Philip Rose GM3ZZA";
//! //Third-party acknowledgments.
//! std::string PARTY3RD_COPYRIGHT = "This software includes contributions from various third-party projects. See Userguide.";
//! //Contact address for use in FLTK widget labels.
//! std::string CONTACT = "gm3zza@@btinternet.com";
//! //Contact address for use in general texts.
//! std::string CONTACT2 = "gm3zza@btinternet.com";
//! //Copyright placed in exported data items.
//! std::string DATA_COPYRIGHT = "\302\251 Philip Rose %s. This data may be copied for the purpose of correlation and analysis";
//! 
//! std::string ZZACOMMON_TIMESTAMP = "@ZZACOMMON_TIMESTAMP@";
//! std::string ZZACOMMON_VERSION = "@ZZACOMMON_VERSION@";
//! \endcode

class zc_app {

public:

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


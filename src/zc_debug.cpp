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
#include "zc_debug.h"

#include <string>

std::string ZZACOMMON_TIMESTAMP = "2026-04-18T08:31:10Z";
std::string ZZACOMMON_VERSION = "1.0.10";

debug_flag zc_app::debug_flags_ = 0;
//! \brief Next spate debug flags
debug_flag DEBUG_QUICK = 1 << 0;
debug_flag DEBUG_THREADS = 1 << 1;
debug_flag DEBUG_SOCKET = 1 << 2;
debug_flag DEBUG_XMLRPC = 1 << 3;
debug_flag DEBUG_CURL = 1 << 4;
debug_flag DEBUG_DEVELOPMENT = 1 << 5;
debug_flag DEBUG_NEXT = 1 << 8;


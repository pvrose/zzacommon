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
#ifndef __SERIAL__
#define __SERIAL__


#include <set>
#include <string>

//! Include boost asio for cross-platform serial port access.
#include <boost/asio.hpp>



	//! This class provides utilities to support zc_serial port access.
	class zc_serial
	{
	public:
	
		//! Provides a set of all available ports
		
		//! \param num_ports the size of array \p ports.
		//! \param ports An array of stringsto receive the port names.
		//! \param all_ports Provide all ports even if they are not available for use.
		//! \param actual_ports Receives the number of ports in the list.
		//! \return true if the array \p ports was big eneough.
		static bool available_ports(int num_ports, std::string* ports, bool all_ports, int& actual_ports);

        //! As available_ports but returns a set of strings instead of an array.
		static std::set<std::string> available_ports(bool all_ports);

	};


#endif

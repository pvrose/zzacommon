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

#include <boost/asio.hpp>


	//! \brief This class provides utilities to support serial port access.
	//! 
	//! It provides a lightweight wrapper around boost::asio::serial_port to
	//! provide a simple API for reading and writing to a serial port.
	//! It also allows the user to query the available serial ports on the system.
	class zc_serial
	{
	public:
	
		//! Constructor for zc_serial.
		//! \param port The name of the serial port to connect to.
		//! \param baud_rate The baud rate for the serial connection.
		zc_serial(const std::string& port, int baud_rate);

		//! Destructor for zc_serial. It closes the serial connection.
		~zc_serial();

		//! Read a line of text from the serial port. 
		//! This is a blocking call that waits until a line of text is received from the serial port.
		//! \param line A string to receive the line of text read from the serial port.
		//! \return true if a line was successfully read, false otherwise.
		bool read_line(std::string& line);

		//! Read whatever data is currently available on the serial port without blocking.
		//! \param data A string to receive the data read from the serial port.
		//! \return true if data was successfully read, false otherwise.
		bool read_any(std::string& data);

		//! Write a line of text to the serial port.
		//! \param line The line of text to write to the serial port.
		//! \return true if the line was successfully written, false otherwise.
		bool write_line(const std::string& line);

		//! Check if the serial port is open and ready for communication.
		bool is_connected() const {
			return serial_port_ && serial_port_->is_open();
		}

		//! Provides a set of all available ports
		
		//! \param num_ports the size of array \p ports.
		//! \param ports An array of stringsto receive the port names.
		//! \param all_ports Provide all ports even if they are not available for use.
		//! \param actual_ports Receives the number of ports in the list.
		//! \return true if the array \p ports was big eneough.
		static bool available_ports(int num_ports, std::string* ports, bool all_ports, int& actual_ports);

        //! As available_ports but returns a set of strings instead of an array.
		static std::set<std::string> available_ports(bool all_ports);

	private:
		boost::asio::serial_port* serial_port_ = nullptr;

	};


#endif

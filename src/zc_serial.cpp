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
#include "zc_serial.h"

#include <cstdio>
#include <set>
#include <string>

//! Include boost asio for cross-platform serial port access.
#include <boost/asio.hpp>

// Constructor for zc_serial.
zc_serial::zc_serial(const std::string& port, int baud_rate) {
	try {
		boost::asio::io_service io;
		serial_port_ = new boost::asio::serial_port(io, port);
		serial_port_->set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
	}
	catch (boost::system::system_error& e) {
		if (serial_port_) {
			delete serial_port_;
			serial_port_ = nullptr;
		}
	}
}

// Destructor for zc_serial. It closes the serial connection.
zc_serial::~zc_serial() {
	if (serial_port_) {
		try {
			serial_port_->close();
		}
		catch (boost::system::system_error& e) {
			// Ignore errors on close.
		}
		delete serial_port_;
	}
}

// Read a line of text from the serial port.
bool zc_serial::read_line(std::string& line) {
	if (!serial_port_ || !serial_port_->is_open()) {
		return false;
	}
	try {
		boost::asio::read_until(*serial_port_, boost::asio::dynamic_buffer(line), "\n");
		return true;
	}
	catch (boost::system::system_error& e) {
		return false;
	}
}

// Read whatever data is currently available on the serial port without blocking.
bool zc_serial::read_any(std::string& data) {
	if (!serial_port_ || !serial_port_->is_open()) {
		return false;
	}
	try {
		char buf[1024];
		size_t bytes_read = serial_port_->read_some(boost::asio::buffer(buf));
		if (bytes_read > 0) {
			data.append(buf, bytes_read);
			return true;
		}
		else {
			return false;
		}
	}
	catch (boost::system::system_error& e) {
		return false;
	}
}

// Write a line of text to the serial port.
bool zc_serial::write_line(const std::string& line) {
	if (!serial_port_ || !serial_port_->is_open()) {
		return false;
	}
	try {
		boost::asio::write(*serial_port_, boost::asio::buffer(line + "\n"));
		return true;
	}
	catch (boost::system::system_error& e) {
		return false;
	}
}

// Find all existing COM ports - upto COM255
// Returns true if the string array was large enough for all ports.
bool zc_serial::available_ports(int num_ports, std::string* ports, bool all_ports, int& actual_ports) {
	actual_ports = 0;
	const unsigned int MAX_TTY = 255;

	// Try and open each port to see if it exists and is available for use.
	for (unsigned int i = 0; i < MAX_TTY; i++) {
		std::string port_name;
		// Windows uses COM1, COM2, etc. Linux uses /dev/ttyS0, /dev/ttyUSB0, etc.
#ifdef _WIN32
		std::set<std::string> port_prefixes = { "COM" };
#else
		std::set<std::string> port_prefixes = { "/dev/ttyS", "/dev/ttyUSB", "/dev/ttyACM" };
#endif
		for (auto& prefix : port_prefixes) {
			port_name = prefix + std::to_string(i);
			try {
				boost::asio::io_service io;
				boost::asio::serial_port serial(io, port_name);
				// If we got here, the port exists and is available for use.
				if (actual_ports < num_ports) {
					ports[actual_ports] = port_name;
				}
				actual_ports++;
			}
			catch (boost::system::system_error& e) {
				// If the error is "file not found", the port doesn't exist. If it's "access denied", the port exists but is in use.
				if (e.code() == boost::system::errc::no_such_file_or_directory) {
					// Port doesn't exist - do nothing.
				}
				else if (e.code() == boost::system::errc::permission_denied && all_ports) {
					// Port exists but is in use - include it if we're including all ports.
					if (actual_ports < num_ports) {
						ports[actual_ports] = port_name;
					}
					actual_ports++;
				}
			}
		}
	}

	return (actual_ports <= num_ports);

}

// As available_ports but returns a set of strings instead of an array.
std::set<std::string> zc_serial::available_ports(bool all_ports) {
	std::set<std::string> ports;
	int actual_ports = 0;
	// First call available_ports to get the number of ports, then call it again with an array of the correct size to get the port names.
	available_ports(0, nullptr, all_ports, actual_ports);
	if (actual_ports > 0) {
		std::string* port_array = new std::string[actual_ports];
		if (available_ports(actual_ports, port_array, all_ports, actual_ports)) {
			for (int i = 0; i < actual_ports; i++) {
				ports.insert(port_array[i]);
			}
		}
		delete[] port_array;
	}
	return ports;
}




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
#include <string>

#ifdef _WIN32
// Note the code is Windows only - need Linux version
#include <Windows.h>
#include <sys/stat.h>
#else 
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

// Constructor - initialise "file" handle to port
zc_serial::zc_serial() {
};

// Destrutor
zc_serial::~zc_serial() {
}

// Find all existing COM ports - upto COM255
// Returns true if the string array was large enough for all ports.
bool zc_serial::available_ports(int num_ports, std::string* ports, bool all_ports, int& actual_ports) {
	const unsigned int MAX_TTY = 255;
	const unsigned int PATH_MAX = 255;
	actual_ports = 0;
	struct stat st {};
	char ttyname[PATH_MAX + 1];

#ifdef _WIN32
	// Find which ports exists (not just available) by trying to open the port
	for (unsigned int i = 0; i < MAX_TTY; i++) {
		char dev[16];
		bool use_port = false;
		snprintf(dev, sizeof(dev), "//./COM%u", i);
		HANDLE fd = CreateFile(dev, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (fd != INVALID_HANDLE_VALUE) {
			// Accessed OK - so exists and available for use
			CloseHandle(fd);
			use_port = true;
		}
		else {
			// Check if the port was there but access denied - implies exists but in use
			long error_code = GetLastError();
			if (error_code == ERROR_ACCESS_DENIED && all_ports) {
				use_port = true;
			}
		}
		// Add it to the list of ports
		if (use_port) {
			if (actual_ports < num_ports) {
				char port[16];
				snprintf(port, 16, "COM%d", i);
				ports[actual_ports] = port;
			}
			actual_ports++;
		}
	}
#else
	// Currently limited to USB TTY ports
	const char* tty_fmt[] = {
//		"/dev/tty%u",
//		"/dev/ttyS%u",
		"/dev/ttyUSB%u",
		"/dev/ttyACM%u" //,
//		"/dev/usb/ttyUSB%u"
	};
	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		for (unsigned j = 0; j < MAX_TTY; j++) {
			snprintf(ttyname, sizeof(ttyname), tty_fmt[i], j);
			if ( !(stat(ttyname, &st) == 0 && S_ISCHR(st.st_mode)) )
				continue;
// TODO check whether port is available 
			if (actual_ports < num_ports) {
				ports[actual_ports] = ttyname;
			} 
			actual_ports++;
		}
	}
#endif
	return (actual_ports <= num_ports);

}

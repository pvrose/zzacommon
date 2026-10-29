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

#include <algorithm>
#include <cerrno>
#include <set>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

#ifndef _WIN32
namespace {
bool baud_rate_to_speed(int baud_rate, speed_t& speed) {
	switch (baud_rate) {
	case 0: speed = B0; return true;
	case 50: speed = B50; return true;
	case 75: speed = B75; return true;
	case 110: speed = B110; return true;
	case 134: speed = B134; return true;
	case 150: speed = B150; return true;
	case 200: speed = B200; return true;
	case 300: speed = B300; return true;
	case 600: speed = B600; return true;
	case 1200: speed = B1200; return true;
	case 1800: speed = B1800; return true;
	case 2400: speed = B2400; return true;
	case 4800: speed = B4800; return true;
	case 9600: speed = B9600; return true;
	case 19200: speed = B19200; return true;
	case 38400: speed = B38400; return true;
#ifdef B57600
	case 57600: speed = B57600; return true;
#endif
#ifdef B115200
	case 115200: speed = B115200; return true;
#endif
#ifdef B230400
	case 230400: speed = B230400; return true;
#endif
#ifdef B460800
	case 460800: speed = B460800; return true;
#endif
#ifdef B921600
	case 921600: speed = B921600; return true;
#endif
	default: return false;
	}
}
}
#endif

namespace {
std::intptr_t open_serial_port(const std::string& port, int baud_rate, bool configure, int& error) {
#ifdef _WIN32
	std::string device = port;
	if (port.size() > 3 && port.compare(0, 3, "COM") == 0 &&
		std::all_of(port.begin() + 3, port.end(), [](unsigned char ch) { return ch >= '0' && ch <= '9'; })) {
		device = "\\\\.\\" + port;
	}
	HANDLE handle = CreateFileA(device.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (handle == INVALID_HANDLE_VALUE) {
		error = static_cast<int>(GetLastError());
		return -1;
	}
	if (configure) {
		DCB settings{};
		settings.DCBlength = sizeof(settings);
		if (!GetCommState(handle, &settings)) {
			error = static_cast<int>(GetLastError());
			CloseHandle(handle);
			return -1;
		}
		settings.BaudRate = static_cast<DWORD>(baud_rate);
		settings.ByteSize = 8;
		settings.Parity = NOPARITY;
		settings.StopBits = ONESTOPBIT;
		settings.fBinary = TRUE;
		settings.fInX = FALSE;
		settings.fOutX = FALSE;
		if (!SetCommState(handle, &settings)) {
			error = static_cast<int>(GetLastError());
			CloseHandle(handle);
			return -1;
		}
		COMMTIMEOUTS timeouts{};
		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.ReadTotalTimeoutConstant = 100;
		if (!SetCommTimeouts(handle, &timeouts)) {
			error = static_cast<int>(GetLastError());
			CloseHandle(handle);
			return -1;
		}
	}
	error = 0;
	return reinterpret_cast<std::intptr_t>(handle);
#else
	int descriptor = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (descriptor < 0) {
		error = errno;
		return -1;
	}
	if (configure) {
		termios settings{};
		speed_t speed;
		if (tcgetattr(descriptor, &settings) != 0) {
			error = errno;
			close(descriptor);
			return -1;
		}
		if (!baud_rate_to_speed(baud_rate, speed)) {
			error = EINVAL;
			close(descriptor);
			return -1;
		}
		settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
		settings.c_oflag &= ~OPOST;
		settings.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		settings.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
#ifdef CRTSCTS
		settings.c_cflag &= ~CRTSCTS;
#endif
		settings.c_cflag |= CS8 | CLOCAL | CREAD;
		settings.c_cc[VMIN] = 1;
		settings.c_cc[VTIME] = 0;
		if (cfsetispeed(&settings, speed) != 0 || cfsetospeed(&settings, speed) != 0 ||
			tcsetattr(descriptor, TCSANOW, &settings) != 0) {
			error = errno;
			close(descriptor);
			return -1;
		}
	}
	error = 0;
	return descriptor;
#endif
}

void close_serial_port(std::intptr_t handle) {
#ifdef _WIN32
	if (handle != -1) CloseHandle(reinterpret_cast<HANDLE>(handle));
#else
	if (handle != -1) close(static_cast<int>(handle));
#endif
}
}

zc_serial::zc_serial(const std::string& port, int baud_rate) {
	int error = 0;
	serial_port_ = open_serial_port(port, baud_rate, true, error);
}

zc_serial::~zc_serial() {
	close_serial_port(serial_port_);
}

bool zc_serial::is_connected() const {
	return serial_port_ != -1;
}

bool zc_serial::read_line(std::string& line) {
	if (!is_connected()) {
		return false;
	}
	for (;;) {
		char ch;
#ifdef _WIN32
		DWORD bytes_read = 0;
		if (!ReadFile(reinterpret_cast<HANDLE>(serial_port_), &ch, 1, &bytes_read, nullptr)) return false;
		if (bytes_read == 0) continue;
#else
		int descriptor = static_cast<int>(serial_port_);
		fd_set readable;
		FD_ZERO(&readable);
		FD_SET(descriptor, &readable);
		int ready = select(descriptor + 1, &readable, nullptr, nullptr, nullptr);
		if (ready < 0) {
			if (errno == EINTR) continue;
			return false;
		}
		ssize_t bytes_read = read(descriptor, &ch, 1);
		if (bytes_read < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
			return false;
		}
		if (bytes_read == 0) return false;
#endif
		line.push_back(ch);
		if (ch == '\n') return true;
	}
}

bool zc_serial::read_any(std::string& data) {
	if (!is_connected()) {
		return false;
	}
	char buf[1024];
#ifdef _WIN32
	DWORD errors = 0;
	COMSTAT status{};
	if (!ClearCommError(reinterpret_cast<HANDLE>(serial_port_), &errors, &status) || status.cbInQue == 0) {
		return false;
	}
	DWORD bytes_read = 0;
	DWORD bytes_to_read = static_cast<DWORD>((std::min)(sizeof(buf), static_cast<size_t>(status.cbInQue)));
	if (!ReadFile(reinterpret_cast<HANDLE>(serial_port_), buf, bytes_to_read, &bytes_read, nullptr)) return false;
	data.append(buf, bytes_read);
	return bytes_read > 0;
#else
	ssize_t bytes_read;
	do {
		bytes_read = read(static_cast<int>(serial_port_), buf, sizeof(buf));
	} while (bytes_read < 0 && errno == EINTR);
	if (bytes_read <= 0) return false;
	data.append(buf, static_cast<size_t>(bytes_read));
	return true;
#endif
}

bool zc_serial::write_line(const std::string& line) {
	if (!is_connected()) {
		return false;
	}
	std::string output = line + "\n";
	size_t written = 0;
	while (written < output.size()) {
#ifdef _WIN32
		DWORD bytes_written = 0;
		DWORD remaining = static_cast<DWORD>((std::min)(output.size() - written,
			static_cast<size_t>(MAXDWORD)));
		if (!WriteFile(reinterpret_cast<HANDLE>(serial_port_), output.data() + written,
			remaining, &bytes_written, nullptr) || bytes_written == 0) return false;
		written += bytes_written;
#else
		ssize_t bytes_written = write(static_cast<int>(serial_port_), output.data() + written,
			output.size() - written);
		if (bytes_written < 0) {
			if (errno == EINTR) continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				int descriptor = static_cast<int>(serial_port_);
				fd_set writable;
				FD_ZERO(&writable);
				FD_SET(descriptor, &writable);
				if (select(descriptor + 1, nullptr, &writable, nullptr, nullptr) < 0 && errno != EINTR) return false;
				continue;
			}
			return false;
		}
		if (bytes_written == 0) return false;
		written += static_cast<size_t>(bytes_written);
#endif
	}
	return true;
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
			int error = 0;
			std::intptr_t serial = open_serial_port(port_name, 0, false, error);
			if (serial != -1) {
				if (actual_ports < num_ports) {
					ports[actual_ports] = port_name;
				}
				actual_ports++;
				close_serial_port(serial);
			} else {
#ifdef _WIN32
				bool access_denied = error == ERROR_ACCESS_DENIED || error == ERROR_SHARING_VIOLATION;
#else
				bool access_denied = error == EACCES || error == EPERM;
#endif
				if (access_denied && all_ports) {
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
	std::vector<std::string> port_array(255 * 3);
	if (available_ports(static_cast<int>(port_array.size()), port_array.data(), all_ports, actual_ports)) {
		for (int i = 0; i < actual_ports; i++) {
			ports.insert(port_array[i]);
		}
	}
	return ports;
}

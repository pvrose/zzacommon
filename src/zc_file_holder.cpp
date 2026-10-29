/*
	Copyright 2017-2026, Philip Rose, GM3ZZA
	
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
#include "zc_file_holder.h"

#include "zc_debug.h"
#include "zc_status.h"
#include "zc_utils.h"

#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <FL/fl_utf8.h>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Window.H>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

zc_file_holder* file_holder_ = nullptr;
uint32_t DEBUG_RESET_CONFIG = 0;
extern std::string APP_VENDOR;
extern std::string APP_VERSION;
extern std::string APP_NAME;
extern std::string APP_SOURCE_DIR;
extern std::string APP_CONFIG_NAME;
extern debug_flag DEBUG_DEVELOPMENT;
extern debug_flag DEBUG_TEST_PRODUCT;
//! File control datra

namespace {
std::filesystem::path executable_path() {
#ifdef _WIN32
	std::string path(32768, '\0');
	DWORD length = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
	if (length == 0 || length >= path.size()) {
		DWORD error = GetLastError();
		if (length >= path.size()) error = ERROR_INSUFFICIENT_BUFFER;
		throw std::system_error(static_cast<int>(error), std::system_category(), "GetModuleFileNameA");
	}
	path.resize(length);
	return std::filesystem::path(path);
#elif defined(__APPLE__)
	uint32_t size = 0;
	_NSGetExecutablePath(nullptr, &size);
	std::string path(size, '\0');
	if (_NSGetExecutablePath(path.data(), &size) != 0) {
		throw std::system_error(errno, std::system_category(), "_NSGetExecutablePath");
	}
	path.resize(std::char_traits<char>::length(path.c_str()));
	return std::filesystem::path(path);
#else
	std::string path(4096, '\0');
	for (;;) {
		ssize_t length = readlink("/proc/self/exe", path.data(), path.size());
		if (length < 0) {
			throw std::system_error(errno, std::system_category(), "readlink /proc/self/exe");
		}
		if (static_cast<size_t>(length) < path.size()) {
			path.resize(static_cast<size_t>(length));
			return std::filesystem::path(path);
		}
		path.resize(path.size() * 2);
	}
#endif
}

bool get_file_timestamp(const std::string& filename,
	std::chrono::system_clock::time_point& timestamp, std::error_code& error) {
	const auto file_time = std::filesystem::last_write_time(filename, error);
	if (error) return false;
	using file_clock = std::filesystem::file_time_type::clock;
	timestamp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		file_time - file_clock::now() + std::chrono::system_clock::now());
	return true;
}
}

zc_file_holder::zc_file_holder(const char* arg0, 
	const std::map<uint8_t, file_control_t>& control
) : control_data_(control) {
	(void)arg0;
	char pwd[1024];
	fl_getcwd(pwd, sizeof(pwd));
	auto exe_path = executable_path();
	std::string s_exe_path = exe_path.string();
	exec_directory_ = zc::directory(s_exe_path);
	std::string app_name = zc::terminal(s_exe_path);
	// Try reading from run directory first - if present then
	// we are development
#ifdef _WIN32
	default_source_directory_ = exec_directory_ + "\\";
#else
	default_source_directory_ = exec_directory_ + "/";
#endif

	default_git_directory_ = APP_SOURCE_DIR + "/reference/";
	// Test the path using the icon
	std::string logo = get_filename(FILE_ICON_ZZA);
	Fl_RGB_Image* ilog = new Fl_PNG_Image(logo.c_str());
	if (ilog && !ilog->fail()) {
		zc_app::set_debug(DEBUG_DEVELOPMENT);
		APP_VERSION += " DEVT";
	} else {
		zc_app::clear_debug(DEBUG_DEVELOPMENT);
#ifdef _WIN32
		default_source_directory_ += "..\\etc\\";
#else
		default_source_directory_ += "../etc/" + app_name + "/";
#endif
		// Try again in release directory
		logo = get_filename(FILE_ICON_ZZA);
		ilog = new Fl_PNG_Image(logo.c_str());
		if (!ilog || ilog->fail()) {
			printf("ZZALOG: Unable to find logo file - file accesses will fail\n");
			default_source_directory_ = "";
			default_ref_directory_ = "";
			default_html_directory_ = "";
		}
	}
	Fl_Window::default_icon(ilog);
	std::string app_dir_name = APP_NAME;
	if (!APP_CONFIG_NAME.empty()) app_dir_name += APP_CONFIG_NAME;
	if (zc_app::debug(DEBUG_DEVELOPMENT) && !zc_app::debug(DEBUG_TEST_PRODUCT)) app_dir_name += "_DEVT";
#ifdef _WIN32
	default_html_directory_ = default_source_directory_;
	// Working directory
	default_ref_directory_ =
		std::string(getenv("APPDATA")) + "\\" + APP_VENDOR + "\\" + app_dir_name + "\\";
	// Create the working directory
	std::string unixified = default_ref_directory_;
	for (size_t pos = 0; pos < unixified.length(); pos++) {
		if (unixified[pos] == '\\') unixified[pos] = '/';
	}
	fl_make_path(unixified.c_str());
	// Code directory
#else 
	default_html_directory_ = default_source_directory_;
	// Working directory
	default_ref_directory_ =
		std::string(getenv("HOME")) + "/.config/" + APP_VENDOR + "/" + app_dir_name + "/";
	// Create the working directory
	fl_make_path(default_ref_directory_.c_str());
#endif
}

bool zc_file_holder::get_file(uint8_t type, std::ifstream& is, std::string& filename) {
	const file_control_t& ctrl = file_holder_->file_control(type);
	char msg[128];
	if (ctrl.reference) {
		if (ctrl.read_only) {
			// Open source directory
			filename = default_source_directory_ + ctrl.filename;
			is.open(filename);
			if (is.fail()) {
				snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
				if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
				else {
					printf(msg);
					printf("\n");
				}
				return false;
			}
			else {
				remember_timestamp(type, filename);
				return true;
			}
		}
		else if (DEBUG_RESET_CONFIG & ctrl.reset_mask) {
			filename = default_ref_directory_ + ctrl.filename;
			// Copy source to working
			if (copy_source_to_working(type)) {
				is.open(filename);
				if (is.fail()) {
					snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
					if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
					else {
						printf(msg);
						printf("\n");
					}
					return false;
				}
				return true;
			}
			else {
				return false;
			}
		}
		else {
			filename = default_ref_directory_ + ctrl.filename;
			is.open(filename);
			if (is.fail()) {
				snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
				if (status_) status_->misc_status(ST_WARNING, msg);
				else {
					printf(msg);
					printf("\n");
				}
				// Copy source to working
				if (copy_source_to_working(type)) {
					is.open(filename);
					if (is.fail()) {
						snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
						if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
						else {
							printf(msg);
							printf("\n");
						}
						return false;
					}
					return true;
				}
				else {
					return false;
				}
			}
			else {
				remember_timestamp(type, filename);
				return  true;
			}
		}
	}
	else {
		filename = default_ref_directory_ + ctrl.filename;
		if (DEBUG_RESET_CONFIG & ctrl.reset_mask) {
			DEBUG_RESET_CONFIG &= ~(ctrl.reset_mask);
			snprintf(msg, sizeof(msg), "FILE: Ignoring %s", filename.c_str());
			if (status_) status_->misc_status(ST_WARNING, msg);
			else {
				printf(msg);
				printf("\n");
			}
			return false;
		}
		is.open(filename);
		if (is.fail()) {
			snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
			if (status_) status_->misc_status(ST_ERROR, msg);
			else {
				printf(msg);
				printf("\n");
			}
			return false;
		}
		remember_timestamp(type, filename);
		return true;
	}
}

bool zc_file_holder::copy_source_to_working(uint8_t type) {
	file_control_t ctrl = control_data_.at(type);
	// Copy source to working
	std::string source = default_source_directory_ + ctrl.filename;
	std::string filename = default_ref_directory_ + ctrl.filename;
	std::error_code ec;
	if (std::filesystem::remove(filename, ec)) {
		if (status_) {
			status_->misc_status(ST_NOTE, "FILE: Existing %s removed.", filename.c_str());
		}
	}
	if (ec) {
		char msg[256];
		snprintf(msg, sizeof(msg), "FILE: Failed to remove %s: %s", filename.c_str(), ec.message().c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	std::filesystem::path dest_path(filename);
	std::filesystem::create_directories(dest_path.parent_path(), ec);
	if (ec) {
		char msg[256];
		snprintf(msg, sizeof(msg), "FILE: Failed to create directories for %s: %s", 
			filename.c_str(), 
			ec.message().c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	std::filesystem::copy_file(source, filename, std::filesystem::copy_options::update_existing, ec);
	char msg[256];
	if (ec) {
		snprintf(msg, sizeof(msg), "FILE: Copy %s to %s failed %s", 
			source.c_str(), 
			filename.c_str(), 
			ec.message().c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "FILE: Copied %s to %s", source.c_str(), filename.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
		remember_timestamp(type, source, true);
	}
	DEBUG_RESET_CONFIG &= ~(ctrl.reset_mask);
	return true;
}

bool zc_file_holder::get_file(uint8_t type, std::ofstream& os, std::string& filename) const {
	const file_control_t ctrl = file_holder_->file_control(type);
	char msg[128];
	if (ctrl.read_only) {
		snprintf(msg, sizeof(msg), "FILE: Tring to open %s (read-only)", ctrl.filename.c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	filename = default_ref_directory_ + ctrl.filename;
	os.open(filename);
	if (os.fail()) {
		snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	return true;
}

// Copy from source to git
bool zc_file_holder::copy_source_to_git(uint8_t type) const {
	file_control_t ctrl = file_holder_->file_control(type);
	// Copy source to working
	std::string source = default_source_directory_ + ctrl.filename;
	std::string git = default_git_directory_ + ctrl.filename;
	std::error_code ec;
	std::filesystem::copy_file(source, git, std::filesystem::copy_options::update_existing, ec);
	char msg[256];
	if (ec) {
		snprintf(msg, sizeof(msg), "FILE: Copy failed %s", ec.message().c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "FILE: Copied %s to %s", source.c_str(), git.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
	}
	//char msg[256];
	return true;
}

// Copy working to source
bool zc_file_holder::copy_working_to_source(uint8_t type) const {
	file_control_t ctrl = file_holder_->file_control(type);
	// Copy source to working
	std::string source = default_source_directory_ + ctrl.filename;
	std::string working = default_ref_directory_ + ctrl.filename;
	std::error_code ec;
	std::filesystem::copy_file(working, source, std::filesystem::copy_options::update_existing, ec);
	char msg[256];
	if (ec) {
		snprintf(msg, sizeof(msg), "FILE: Copy failed %s", ec.message().c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "FILE: Copied %s to %s", working.c_str(), source.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
	}
	//char msg[256];
	return true;
}

file_control_t zc_file_holder::file_control(uint8_t type) const {
	return control_data_.at(type);
}

// Add file to list of files to be managed. Look for next available type
uint8_t zc_file_holder::add_file(const file_control_t& ctrl, uint8_t type) {
	while (control_data_.find(type) != control_data_.end()) {
		type++;
	}
	control_data_[type] = ctrl;
	return type;
}

// Display file info
void zc_file_holder::display_info() const {
	if (status_) {
		status_->misc_status(ST_NOTE, "FILE: Binary: %s", exec_directory_.c_str());
		status_->misc_status(ST_NOTE, "FILE: Source reference:- %s", default_source_directory_.c_str());
		status_->misc_status(ST_NOTE, "FILE: Working reference:- %s", default_ref_directory_.c_str());
	}
}

// Get timestamp for file type
std::chrono::system_clock::time_point zc_file_holder::timestamp(uint8_t type) {
	if (timestamps_.find(type) != timestamps_.end()) {
		return timestamps_.at(type);
	} else {
		// This may be because we haven't read it
		std::string filename = get_filename(type);
		std::error_code ec;
		std::chrono::system_clock::time_point timestamp;
		if (get_file_timestamp(filename, timestamp, ec)) {
			timestamps_[type] = timestamp;
		}
		else {
			if (status_) status_->misc_status(ST_WARNING, "FILE: Cannot get timestamp for %s - error %s", filename.c_str(), ec.message().c_str());
		}
		if (timestamps_.find(type) != timestamps_.end()) {
			return timestamps_.at(type);
		}
		return std::chrono::system_clock::time_point::min();
	}
}

// Remember the timestamp for the file (first time only)
void zc_file_holder::remember_timestamp(uint8_t type, const std::string& filename, bool overwrite) {
	if (!overwrite && timestamps_.find(type) != timestamps_.end()) {
		return;
	} else {
		std::error_code ec;
		std::chrono::system_clock::time_point timestamp;
		if (get_file_timestamp(filename, timestamp, ec)) {
			timestamps_[type] = timestamp;
		}
	}
}

// Return the true top-level storage root for Windows or Linux. This is used to determine if a file is on a different drive.
std::filesystem::path zc_file_holder::storage_root(const std::filesystem::path& path) const {
	std::error_code ec;
	
	// Resolve absolute path, symlinks, and relative dots (e.g. "./")
	std::filesystem::path abs_path = std::filesystem::canonical(path, ec);
	if (ec) {
		abs_path = std::filesystem::absolute(path, ec);
	}
	if (ec) return {};

	// Grab the first element (e.g., "C:" on Windows, or "/" on Linux)
	std::filesystem::path::iterator it = abs_path.begin();
	if (it == abs_path.end()) return {};
	std::filesystem::path root = *it;

	// On Linux/POSIX, "/" is universal. We need the next layer (e.g., "/home" vs "/cloud")
#ifndef _WIN32
	if (root == "/") {
		++it;
		if (it != abs_path.end()) {
			root /= *it; // Combines "/" and "home" into "/home"
		}
	}
#endif
	return root;
}

// File is on a different drive than the executable
bool zc_file_holder::on_different_drive(const std::string& filename) const {
	std::filesystem::path exec_path = std::filesystem::current_path();
	std::filesystem::path file_path(filename);
	return storage_root(exec_path) != storage_root(file_path);
}

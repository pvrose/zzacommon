#include "zc_file_holder.h"

#include "zc_status.h"
#include "zc_utils.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#include <FL/fl_utf8.h>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Window.H>

#include <boost/filesystem.hpp>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/dll/runtime_symbol_info.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

zc_file_holder* file_holder_ = nullptr;
uint32_t DEBUG_RESET_CONFIG = 0;
extern std::string APP_VENDOR;
extern std::string APP_NAME;
extern std::string APP_SOURCE_DIR;
extern bool DEVELOPMENT_MODE;
//! File control datra


zc_file_holder::zc_file_holder(const char* arg0, const std::map<uint8_t, file_control_t>& control) {
	control_data_ = control;
	char * pwd = fl_getcwd(nullptr, 256);
	std::string run_dir = zc::directory(arg0);
	auto exe_path = boost::dll::program_location();
	exec_directory_ = zc::directory(exe_path.string());
	std::string app_name = zc::terminal(exe_path.string());
	// Try reading from run directory first - if present then
	// we are development
#ifdef _WIN32
	default_source_directory_ = exec_directory_ + "\\";
#else
	default_source_directory_ = exec_directory_ + "/";
#endif

	default_code_directory_ = default_code_directory_;
	default_git_directory_ = APP_SOURCE_DIR + "/reference/";
	// Test the path using the icon
	std::string logo = get_filename(FILE_ICON_ZZA);
	Fl_PNG_Image* ilog = new Fl_PNG_Image(logo.c_str());
	if (ilog && !ilog->fail()) {
		DEVELOPMENT_MODE = true;
	} else {
		DEVELOPMENT_MODE = false;
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
			default_code_directory_ = "";
			default_data_directory_ = "";
			default_html_directory_ = "";
		}
	}
	Fl_Window::default_icon(ilog);

#ifdef _WIN32
	default_html_directory_ = default_source_directory_;
	// Working directory
	default_data_directory_ =
		std::string(getenv("APPDATA")) + "\\" + APP_VENDOR + "\\" + APP_NAME + "\\";
	// Create the working directory
	std::string unixified = default_data_directory_;
	for (size_t pos = 0; pos < unixified.length(); pos++) {
		if (unixified[pos] == '\\') unixified[pos] = '/';
	}
	fl_make_path(unixified.c_str());
	// Code directory
#else 
	default_html_directory_ = default_source_directory_;
	// Working directory
	default_data_directory_ =
		std::string(getenv("HOME")) + "/.config/" + APP_VENDOR + "/" + APP_NAME + "/";
	// Create the working directory
	fl_make_path(default_data_directory_.c_str());
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
			filename = default_data_directory_ + ctrl.filename;
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
			filename = default_data_directory_ + ctrl.filename;
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
		filename = default_data_directory_ + ctrl.filename;
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
	std::string filename = default_data_directory_ + ctrl.filename;
// #ifdef _WIN32
// 	std::string command = "copy " + source + " " + filename;
// #else
// 	std::string command = "cp " + source + " " + filename;
// #endif
// 	int result = system(command.c_str());
	boost::system::error_code ec;
	if (boost::filesystem::remove(filename.c_str())) {
		if (status_) {
			status_->misc_status(ST_NOTE, "FILE: Existing %s removed.", filename.c_str());
		}
	}
	boost::filesystem::copy(
		source.c_str(),
		filename.c_str(),
		boost::filesystem::copy_options::update_existing |
		boost::filesystem::copy_options::synchronize,
		ec
	);
	char msg[256];
	if (ec) {
		snprintf(msg, sizeof(msg), "FILE: Copy %s to %s failed %d", source.c_str(), filename.c_str(), ec);
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
	filename = default_data_directory_ + ctrl.filename;
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
	boost::system::error_code ec;
	boost::filesystem::copy(
		source.c_str(),
		git.c_str(),
		boost::filesystem::copy_options::update_existing |
		boost::filesystem::copy_options::synchronize,
		ec
	);
	char msg[256];
	if (ec) {
		snprintf(msg, sizeof(msg), "FILE: Copy failed %d", ec);
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "File Copied %s to %s", source.c_str(), git.c_str());
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
	std::string working = default_data_directory_ + ctrl.filename;
	boost::system::error_code ec;
	boost::filesystem::copy(
		working.c_str(),
		source.c_str(),
		boost::filesystem::copy_options::update_existing |
		boost::filesystem::copy_options::synchronize,
		ec
	);
	char msg[256];
	if (ec) {
		snprintf(msg, sizeof(msg), "FILE: Copy failed %d", ec);
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "File Copied %s to %s", working.c_str(), source.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
	}
	//char msg[256];
	return true;
}

file_control_t zc_file_holder::file_control(uint8_t type) const {
	return control_data_.at(type);
}

// Display file info
void zc_file_holder::display_info() const {
	if (status_) {
		status_->misc_status(ST_NOTE, "FILE: Binary: %s", exec_directory_.c_str());
		status_->misc_status(ST_NOTE, "FILE: Source data:- %s", default_source_directory_.c_str());
		status_->misc_status(ST_NOTE, "FILE: Working data:- %s", default_data_directory_.c_str());
	}
}

// Get timestamp for file type
std::chrono::system_clock::time_point zc_file_holder::timestamp(uint8_t type) const {
	if (timestamps_.find(type) != timestamps_.end()) {
		return timestamps_.at(type);
	} else {
		// Return the earliest it can be
		return std::chrono::system_clock::time_point::min();
	}
}

// Remember the timestamp for the file (first time only)
void zc_file_holder::remember_timestamp(uint8_t type, const std::string& filename, bool overwrite) {
	if (!overwrite && timestamps_.find(type) != timestamps_.end()) {
		return;
	} else {
		boost::system::error_code ec;
		std::time_t ts = boost::filesystem::last_write_time(filename.c_str(), ec);
		if (!ec) {
			timestamps_[type] = std::chrono::system_clock::from_time_t(ts);
		}
	}
}

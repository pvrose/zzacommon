#include "zc_status.h"

#include "zc_banner.h"
#include <zc_drawing.h>
#include "zc_file_holder.h"
// #include "main.h"
// #include "main_window.h"
#include "zc_utils.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>

extern std::string APP_NAME;
extern std::string APP_VERSION;
extern bool DEVELOPMENT_MODE;

// Constructor
zc_status::zc_status(uint8_t features, const object_data_map& data_map) :
	  report_filename_("")
	, report_file_(nullptr)
	, file_unusable_(false)
{
	feature_set_ = features;
	object_map_ = data_map;

	if (feature_set_ & HAS_LOGFILE) {
		// Get report filename from the settings
		report_filename_ = file_holder_->get_filename(FILE_STATUS);
	}
	if (feature_set_ & HAS_BANNER) {
		// Create banner
		banner_ = new zc_banner(400, 200);
		std::string title = APP_NAME + " " + APP_VERSION;
		if (DEVELOPMENT_MODE) title += " DEVT";
		banner_->copy_label(title.c_str());

	}

}

// Destructor
zc_status::~zc_status()
{
	if (report_file_) report_file_->close();
	if (!keep_banner_) Fl::delete_widget(banner_);
}

// Add a progress item to the stack
void zc_status::progress(uint64_t max_value, uint8_t object, const char* description, const char* suffix) {
	// Turrn off file viewer update to improve performance
	// no_update_viewer = true;
	// Initialise it
	// Reset previous value as a new progress
	// Start a new progress bar process - create the progress item (total expected count, objects being counted, up/down and what view it's for)
	if (banner_) banner_->start_progress(
		max_value, 
		object_map_.at(object).name, 
		object_map_.at(object).colour,
		description, 
		suffix);
}

// Update progress to the new specified value
void zc_status::progress(uint64_t value, uint8_t object) {
	// Update progress item
	if (banner_) banner_->add_progress(value);
}

// Update progress with a message - e.g. cancel it and display why cancelled
void zc_status::progress(const char* message, uint8_t object) {
	if (banner_) banner_->cancel_progress(message);
}

// Update miscellaneous status - set text and colour, log the status
void zc_status::misc_status(status_t status, const char* label, ... ) {
	char llabel[256];
	va_list args;
	va_start(args, label);
	vsprintf(llabel, label, args);
	va_end(args);
	// Start each entry with a timestamp
	std::string timestamp = zc::now(false, "%Y/%m/%d %H:%M:%S", true);
	char f_message[256];
	// X YYYY/MM/DD HH:MM:SS Message 
	// X is a single letter indicating the message severity
	snprintf(f_message, sizeof(f_message), "%c %s %s\n", STATUS_CODES.at(status), timestamp.c_str(), label);
	if (banner_) {
		banner_->add_message(status, llabel, timestamp.substr(11).c_str());
	}

	if (feature_set_ & HAS_CONSOLE) {
		if (true) {
			// Restore default colours
			const char restore[] = "\033[0m";
			const char faint[] = "\033[2m";
			printf("%s%s%s %s%s%s%s\n",
				faint,
				timestamp.c_str(),
				restore,
				colour_code(status, true).c_str(),
				colour_code(status, false).c_str(),
				llabel,
				restore);
		}
		else {
			printf("%s %s %s\n",
				STATUS_ABBREV.at(status),
				timestamp.c_str(),
				llabel);
		}
	}

	if (!report_file_ && (feature_set_ & HAS_LOGFILE)) {
		// Append the status to the file
		// Try to open the file. Open and close it each message
		// Save previous files
		for (char c = '8'; c > '0'; c--) {
			// Rename will fail if file does not exist, so no need to test file exists
			std::string oldfile = report_filename_ + c;
			char c2 = c + 1;
			std::string newfile = report_filename_ + c2;
			fl_rename(oldfile.c_str(), newfile.c_str());
		}
		fl_rename(report_filename_.c_str(), (report_filename_ + '1').c_str());
		// Create a new file 
		report_file_ = new std::ofstream(report_filename_, std::ios::out | std::ios::trunc);
		if (!report_file_->good()) {
			// File didn't open correctly
			delete report_file_;
			report_file_ = nullptr;
			file_unusable_ = true;
			fl_alert("STATUS: Failed to open status report file %s", report_filename_.c_str());
		}
	}
	if (report_file_) {
		// File did open correctly - write all message to file
		*report_file_ << f_message;
		report_file_->flush();
	}

	// Depending on the severity: LOG, NOTE, OK, WARNING, ERROR, SEVERE or FATAL
	// Beep on the last three.
	switch(status) {
	case ST_SEVERE:
		// Open status file viewer and update it.
		if (banner_) banner_->show();
		fl_beep(FL_BEEP_ERROR);
		// A severe error - ask the user whether to continue
		if (fl_choice("An error that resulted in reduced functionality occurred:\n%s\n\nDo you want to try to continue or quit?", "Continue", "Quit", nullptr, label, report_filename_.c_str()) == 1) {
			// Set the flag to continue showing the file viewer after all other windows have been hidden.
			keep_banner_ = true;
			close_(window_, nullptr);
		}
		break;
	case ST_FATAL:
		// Open status file viewer and update it. Set the flag to keep it displayed after other windows have been hidden
		if (banner_) banner_->show();
		fl_beep(FL_BEEP_ERROR);
		// A fatal error - quit the application
		fl_message("An unrecoverable error has occurred, closing down - check status log");
		// Close the application down
		keep_banner_ = true;
		close_(window_, nullptr);
		break;
	case ST_ERROR:
		// Open status file viewer and continue
		if (banner_) banner_->show();
		fl_beep(FL_BEEP_ERROR);
		break;
	default:
		break;
	}
}

// Return the terminal escape code for the particular colour
std::string zc_status::colour_code(status_t status, bool fg) {
	char result[25];
	unsigned char r, g, b;
	if (fg) {
		Fl_Color colour = STATUS_COLOURS.at(status).fg;
		Fl::get_color(colour, r, g, b);
		snprintf(result, sizeof(result), "\033[38;2;%d;%d;%dm", r, g, b);
	} else {
		Fl_Color colour = STATUS_COLOURS.at(status).bg;
		Fl::get_color(colour, r, g, b);
		snprintf(result, sizeof(result), "\033[48;2;%d;%d;%dm", r, g, b);
	}
	return std::string(result);
}

// Set callback
void zc_status::callback(Fl_Window* w, void(*close)(Fl_Window*, void*)) {
	close_ = close;
	window_ = w;
}

// SEt Cloisng
void zc_status::close() {
	if (banner_) {
		banner_->close();
		banner_->show();
		banner_->redraw();
	}
}

// Return banner
zc_banner* zc_status::get_banner() {
	return banner_;
}
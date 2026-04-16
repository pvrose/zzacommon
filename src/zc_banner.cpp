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
#include "zc_banner.h"

#include "zc_app.h"
#include "zc_drawing.h"
#include "zc_file_holder.h"
#include "zc_settings.h"
#include "zc_status.h"
#include "zc_utils.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

// FLTK classes
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Fill_Dial.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

const int NUMBER_STYLES = 10;
const double PROGRESS_INCREMENT = 0.01;
Fl_Text_Display::Style_Table_Entry style_table_[NUMBER_STYLES] = {
	{ STATUS_COLOURS.at(ST_NONE).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NONE).bg },
	{ STATUS_COLOURS.at(ST_LOG).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_LOG).bg },
	{ STATUS_COLOURS.at(ST_DEBUG).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_DEBUG).bg },
	{ STATUS_COLOURS.at(ST_NOTE).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NOTE).bg },
	{ STATUS_COLOURS.at(ST_PROGRESS).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_PROGRESS).bg,},
	{ STATUS_COLOURS.at(ST_OK).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_OK).bg },
	{ STATUS_COLOURS.at(ST_WARNING).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_WARNING).bg },
	{ STATUS_COLOURS.at(ST_ERROR).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_ERROR).bg },
	{ STATUS_COLOURS.at(ST_SEVERE).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_SEVERE).bg },
	{ STATUS_COLOURS.at(ST_FATAL).fg, FL_SCREEN, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_FATAL).bg }
};

extern std::string APP_NAME;
extern std::string APP_VERSION;
extern debug_flag DEBUG_DEVELOPMENT;
extern std::string COPYRIGHT;
extern std::string CONTACT;
extern zc_status* status_;

std::thread::id main_thread_id_ = std::this_thread::get_id();

zc_banner::zc_banner(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L)
{
	callback(cb_close);
	load_settings();
	create_form();
	enable_widgets();
}

zc_banner::~zc_banner() {
	save_settings();
}

void zc_banner::create_form() {
	const int HMULT = 2 * HBUTTON;
	const int HICON = HMULT * 2 + GAP;
	const int H_TOP = HICON + GAP + HBUTTON;

	int curr_x = x() + GAP;
	int curr_y = y() + GAP;
	
	Fl_Group* g_top = new Fl_Group(x(), curr_y, w(), H_TOP);
	g_top->box(FL_FLAT_BOX);

	// Group all but the display widgets in the top of the banner.
	Fl_Group* g_topleft = new Fl_Group(curr_x, curr_y, w() - GAP - GAP, HICON);
	g_topleft->box(FL_FLAT_BOX);
	// Create a box to hild the icon and resize it thereinto
	bx_icon_ = new Fl_Box(curr_x, curr_y, HICON, HICON);
	std::string fn_icon = file_holder_->get_filename(FILE_ICON_ZZA);
	Fl_PNG_Image* main_icon = new Fl_PNG_Image(fn_icon.c_str());
	Fl_Image* image = main_icon->copy();
	image->scale(HICON, HICON);
	bx_icon_->image(image);

	curr_x += HICON + GAP;
	int avail_w = g_topleft->w() - HICON - GAP;

	// App title
	op_app_title_ = new Fl_Box(curr_x, curr_y, avail_w, HMULT);
	op_app_title_->labelsize(FL_NORMAL_SIZE * 2);
	op_app_title_->labelfont(FL_BOLD);
	std::string title = APP_NAME + " " + APP_VERSION;
	if (zc_app::debug(DEBUG_DEVELOPMENT)) title += "\n(DEVELOPMENT)";
	op_app_title_->copy_label(title.c_str());
	op_app_title_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);


	// Progress "clock"
	curr_y += HMULT + GAP;
	fd_progress_ = new Fl_Fill_Dial(curr_x, curr_y, HMULT, HMULT);
	fd_progress_->minimum(0.0);
	fd_progress_->maximum(1.0);
	fd_progress_->color(FL_BACKGROUND_COLOR);
	fd_progress_->angles(180, 540);
	fd_progress_->box(FL_OVAL_BOX);

	avail_w -= HMULT + GAP;
	curr_x += HMULT + GAP;

	// Progress value
	bx_prog_value_ = new Fl_Box(curr_x, curr_y, avail_w, HMULT);
	bx_prog_value_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	// Verbosity choice
	curr_x = x() + GAP + WLABEL;
	curr_y += HMULT + GAP;

	ch_verbosity_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Verbosity");
	ch_verbosity_->align(FL_ALIGN_LEFT);
	ch_verbosity_->callback(cb_verbosity, &verbosity_);
	ch_verbosity_->tooltip("Set the verbosity level for messages displayed in the banner");
	// Populate the choice with the verbosity levels
	ch_verbosity_->add("Minimum");
	ch_verbosity_->add("Errors");
	ch_verbosity_->add("Warnings");
	ch_verbosity_->add("Notes");
	ch_verbosity_->add("Full");
	ch_verbosity_->value(verbosity_);
	ch_verbosity_->selection_color(FL_YELLOW);

	curr_x += WSMEDIT + GAP + WLABEL;
	ch_filter_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Topic");
	ch_filter_->align(FL_ALIGN_LEFT);
	ch_filter_->callback(cb_filter, this);
	ch_filter_->tooltip("Set the topic filter for messages displayed in the banner - blank for all topics");
	ch_filter_->value(0); // default to "All"
	ch_filter_->selection_color(FL_YELLOW);

	g_topleft->end();

	curr_x = g_topleft->x() + g_topleft->w();
	curr_y = g_topleft->y();

	Fl_Group* g_topright = new Fl_Group(curr_x, curr_y, GAP, g_topleft->h());
	g_topright->box(FL_FLAT_BOX);

	g_topright->end();
	g_top->resizable(g_topright);
	g_top->end();

	curr_x = x() + GAP;
	curr_y = g_topleft->y() + g_topleft->h() + GAP;

	int h_display = h() - (curr_y + HBUTTON);

	curr_y += HBUTTON;
	h_display -= HBUTTON;
	curr_x = x() + GAP;

	// Text display for all messages
	display_ = new Fl_Text_Display(curr_x, curr_y, w() - GAP - GAP, h_display);
	display_->color(STATUS_COLOURS.at(ST_NONE).bg);
	Fl_Text_Buffer* text = new Fl_Text_Buffer;
	display_->buffer(text);
	Fl_Text_Buffer* style = new Fl_Text_Buffer;
	display_->highlight_data(style, &style_table_[0], NUMBER_STYLES, ' ', nullptr, nullptr);

	curr_y += display_->h();
	Fl_Box* bx_copyright = new Fl_Box(curr_x, curr_y, w() - GAP - GAP, HBUTTON);
	char msg[128];
	snprintf(msg, sizeof(msg), "%s %s     ", COPYRIGHT.c_str(), CONTACT.c_str());
	bx_copyright->copy_label(msg);
	bx_copyright->labelsize(FL_NORMAL_SIZE - 1);
	bx_copyright->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	curr_y += HBUTTON + GAP;

	resizable(display_);

	size_range(w(), h());
	// Center the banner in the middle of the screen (windows manager permitting)
	int sx, sy, sw, sh;
	Fl::screen_xywh(sx, sy, sw, sh, x(), y());
	int nx = sx + (sw / 2) - (w() / 2);
	int ny = sy + (sh / 2) - (h() / 2);
	resize(nx, ny, w(), h());
	end();
	show();
}

void zc_banner::enable_widgets() {
}

void zc_banner::load_settings() {
	zc_settings settings;
	zc_settings behav_settings(&settings, "Behaviour");
	zc_settings banner_settings(&settings, "Banner");
	behav_settings.get("Verbosity", verbosity_, VB_INFO);
}

// Save settings.
void zc_banner::save_settings() {
	zc_settings settings;
	zc_settings behav_settings(&settings, "Behaviour");
	zc_settings banner_settings(&settings, "Banner");
	behav_settings.set("Verbosity", verbosity_);
};

// Add a message to the banner
void zc_banner::add_message(status_t type, const char* msg, const char* ts) {
	// Trap any call that is not from the main thread
	if (std::this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::add_message(%s) when not in the main thread\n", msg);
		throw;
	}
	switch (type) {
	case ST_PROGRESS:
	case ST_NONE:
	case ST_LOG:
	case ST_DEBUG:
	case ST_NOTE:
	case ST_OK: 
	case ST_WARNING:
	case ST_ERROR:
		break;
	case ST_SEVERE:
	case ST_FATAL:
	{
		hide();
		set_non_modal();
		show();
		break;
	}
	}
	copy_msg_display(type, msg, ts);
	display_->redraw();
	Fl::check();
}

// Add progress
void zc_banner::start_progress(
	uint64_t max_value, 
	const char* object, 
	Fl_Color colour, 
	const char* msg, 
	const char* suffix) {
	// Trap any call that is not from the main thread
	if (std::this_thread::get_id() != main_thread_id_) {
		printf("Calling banner_start_progress(%s) when not in the main thread\n", msg);
		throw;
	}
	char text[128];
	max_value_ = max_value;
	delta_ = (uint64_t)((double)max_value * PROGRESS_INCREMENT);
	prev_value_ = 0L;
	prg_msg_ = msg;
	prg_unit_ = suffix;
	prg_object_ = object;
	prg_colour_ = colour;
	// Set display message
	snprintf(text, sizeof(text), "%s: PROGRESS: Starting %s - %lld %s", object, msg, max_value, suffix);
	status_->misc_status(ST_PROGRESS, text);
	snprintf(text, sizeof(text), "0 out of %lld %s", max_value, suffix);
	bx_prog_value_->copy_label(text);
	fd_progress_->selection_color(colour);
	fd_progress_->value(0.0);

	// Only redraw these two items.
	fd_progress_->redraw();
	bx_prog_value_->redraw();
	Fl::check();
}

// Update progress dial and output
void zc_banner::add_progress(uint64_t value) {
	// Trap any call that is not from the main thread
	if (std::this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::add_progress(%lld) when not in the main thread\n", value);
		throw;
	}
	char text[128];
	if ((value == max_value_) || (value - prev_value_) > delta_) {
		snprintf(text, sizeof(text), "%lld out of %lld %s", value, max_value_, prg_unit_);
		bx_prog_value_->copy_label(text);
		if (max_value_ == 0) {
			fd_progress_->value(1.0);
		} else {
			fd_progress_->value((double)value / double(max_value_));
		}
		prev_value_ = value;
		if (value == max_value_) {
			end_progress();
		}
		else {
			// Only redraw these two items.
			fd_progress_->redraw();
			bx_prog_value_->redraw();
			Fl::check();
		}
	}
}

// Ending the progress - log message
void zc_banner::end_progress() {
	// Trap any call that is not from the main thread
	if (std::this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::end_progress(%s) when not in the main thread\n", prg_msg_);
		throw;
	}
	char text[128];
	// Set display message
	snprintf(text, sizeof(text), "%s: PROGRESS: Ending %s", prg_object_, prg_msg_);
	status_->misc_status(ST_PROGRESS, text);
	redraw();
	if (visible()) Fl::check();
}

// cancelling the progress - log message
void zc_banner::cancel_progress(const char* msg) {
	// Trap any call that is not from the main thread
	if (std::this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::cancel_progress(%s) when not in the main thread\n", msg);
		throw;
	}
	char text[128];
	// Set display message
	snprintf(text, sizeof(text), "%s: PROGRESS: cancelling %s - %s", prg_object_, prg_msg_, msg);
	status_->misc_status(ST_PROGRESS, text);
	redraw();
	if (visible()) Fl::check();
}

// Callback - close button - close app.
void zc_banner::cb_close(Fl_Widget* w, void* v) {
	zc_banner* banner = zc::ancestor_view<zc_banner>(w);
	banner->op_app_title_->copy_label("CLOSING!");
	banner->op_app_title_->labelsize(FL_NORMAL_SIZE * 2);
	banner->op_app_title_->labelcolor(FL_RED);
	// Pretend to click the close button on the main window
	status_->close_(status_->window_, nullptr);
}

// Add message to display (with colour)
void zc_banner::copy_msg_display(status_t type, const char* msg, const char* ts) {
	// Extract the topic from the message - the topic is the first word in the message, up to the first ":"
	std::string topic;
	const char* colon = strchr(msg, ':');
	if (colon) {
		topic = std::string(msg, colon - msg);
	}
	add_topic(topic);
	// Create the message to be added to the display - timestamp + message
	std::string text = std::string(ts) + " " + std::string(msg) + "\n";
	// Create the style string for the message - one character per character in the message,
	// with the character being the style to apply to that character
	std::string style(text.size(), (char)type + 'A');
	// Set the style of the timestamp to the default
	std::fill_n(&style[0], strlen(ts) + 1, 'A');
	// Add the message to the saved messages.
	message_history_.push_back({ type, topic, text, style });
	// If the message is above the verbosity level, add it to the display
	if (display_message(type) && message_for_current_topic(topic)) {
		Fl_Text_Buffer* buffer = display_->buffer();
		buffer->append(text.c_str());
		Fl_Text_Buffer* s_buffer = display_->style_buffer();
		s_buffer->append(style.c_str());
		display_->scroll(buffer->count_lines(0, buffer->length()), 0);
	}
}

// Callback - verbosity choice - set verbosity level
void zc_banner::cb_verbosity(Fl_Widget* w, void* v) {
	zc_banner* banner = zc::ancestor_view<zc_banner>(w);
	banner->verbosity_ = (verbosity_t)((Fl_Choice*)w)->value();
	banner->update_display();
	banner->save_settings();
}

// Callback - topic filter choice - set topic filter
void zc_banner::cb_filter(Fl_Widget* w, void* v) {
	zc_banner* banner = zc::ancestor_view<zc_banner>(w);
	int sel = ((Fl_Choice*)w)->value();
	if (sel == 0) {
		banner->filter_topic_ = "";
	} else {
		banner->filter_topic_ = ((Fl_Choice*)w)->text(sel);
	}
	banner->update_display();
}

// Update the display to show messages at or above the current verbosity level
void zc_banner::update_display() {
	Fl_Text_Buffer* buffer = display_->buffer();
	Fl_Text_Buffer* s_buffer = display_->style_buffer();
	buffer->text("");
	s_buffer->text("");
	for (const auto& msg : message_history_) {
		if (display_message(msg.type) && message_for_current_topic(msg.topic)) {
			buffer->append(msg.text.c_str());
			s_buffer->append(msg.style.c_str());
		}
	}
	display_->scroll(buffer->count_lines(0, buffer->length()), 0);
}

// Set closing
void zc_banner::close() {
	closing_ = true;
}

// Set font in font table
void zc_banner::font(Fl_Font f, Fl_Fontsize sz) {
	for (size_t ix = 0; ix < NUMBER_STYLES; ix++) {
		style_table_[ix].font = f;
		style_table_[ix].size = sz;
	}
	redraw();
}

// Get font 
Fl_Font zc_banner::font() {
	return style_table_[0].font;
}

// Get font size
Fl_Fontsize zc_banner::fontsize() {
	return style_table_[0].size;
}

// Determine whether to display a message of the given type at the current verbosity level
bool zc_banner::display_message(status_t type) {
	switch (verbosity_) {
	case VB_MINIMAL:
		return type >= ST_SEVERE;
	case VB_ERRORS:
		return type >= ST_ERROR;
	case VB_WARNINGS:
		return type >= ST_WARNING;
	case VB_INFO:
		return type >= ST_NOTE;
	case VB_FULL:
		return true;
	default:
		return false;
	}
}

// Determine whether to display a message of the given topic at the current topic filter
bool zc_banner::message_for_current_topic(const std::string& topic) {
	return filter_topic_.empty() || topic == filter_topic_;
}

// Add a topic to the topic filter choice if it is not already there
void zc_banner::add_topic(const std::string& topic) {
	if (topic.empty()) return;
	if (topics_.find(topic) == topics_.end()) {
		topics_.insert(topic);
		// Recreate the topic filter choice with the new topic added
		ch_filter_->clear();
		ch_filter_->add("All");
		for (const auto& t : topics_) {
			ch_filter_->add(t.c_str());
		}
		ch_filter_->value(0); // default to "All"
	}
}
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
#include "zc_fltk.h"

#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Window.H>

#include <string>

extern std::string APP_NAME;

//! \file zc_fltk.cpp
//! This file provides a number of common utility methods for use with the FLTK GUI library.

// Create a tip window - tip text, position(root_x, root_y)
Fl_Window* zc::tip_window(const std::string& tip, int x_root, int y_root) {
	// get the size of the text, set the font, default width
	fl_font(Fl_Tooltip::font(), Fl_Tooltip::size());
	int width = Fl_Tooltip::wrap_width();
	int height = 0;
	// Now get the actual width and height
	fl_measure(tip.c_str(), width, height, 0);
	// adjust sizes to allow for margins - use Fl_Tooltip margins.
	width += (2 * Fl_Tooltip::margin_width());
	height += (2 * Fl_Tooltip::margin_height());
	// Create the window
	Fl_Window* win = new Fl_Window(x_root, y_root, width, height, 0);
	win->clear_border();

	// Create the output widget.
	Fl_Multiline_Output* op = new Fl_Multiline_Output(0, 0, width, height, 0);
	// Copy the attributes of tool-tips
	op->color(Fl_Tooltip::color());
	op->textcolor(Fl_Tooltip::textcolor());
	op->textfont(Fl_Tooltip::font());
	op->textsize(Fl_Tooltip::size());
	op->wrap(true);
	op->box(FL_BORDER_BOX);
	op->value(tip.c_str());
	win->add(op);
	win->end();
	// set the window parameters: always on top, tooltip
	win->set_non_modal();
	win->set_tooltip_window();
	// Must be after set_tooltip_window.
	win->show();

	return win;
}

// Create an upper-case version of a string
std::string zc::to_upper(const std::string& data) {
	size_t len = data.length();
	char* result = new char[3 * len + 1];
	memset(result, 0, 3 * len + 1);
	fl_utf_toupper((unsigned char*)data.c_str(), (int)len, result);
	std::string ret_value(result);
	delete[] result;
	return ret_value;
}

// Create an lower-case version of a string
std::string zc::to_lower(const std::string& data) {
	size_t len = data.length();
	char* result = new char[3 * len + 1];
	memset(result, 0, 3 * len + 1);
	fl_utf_tolower((unsigned char*)data.c_str(), (int)len, result);
	std::string ret_value(result);
	delete[] result;
	return ret_value;
}

// Customise FLTK feature
void zc::customise_fltk() {
	// Set default font size for all widgets
	FL_NORMAL_SIZE = 10;
	// FLTK 1.4 default contrast algorithm
	fl_contrast_mode(FL_CONTRAST_CIELAB);
#ifndef _WIN32
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, "Courier New");
	Fl::set_font(FL_COURIER_BOLD, "Courier New Bold");
	Fl::set_font(FL_COURIER_ITALIC, "Courier New Italic");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "Courier New Bold Italic");
	// Use liberation fonts as closest to Windows fonts
	Fl::set_font(FL_TIMES, "Liberation Serif");
	Fl::set_font(FL_TIMES_BOLD, "Liberation Serif Bold");
	Fl::set_font(FL_TIMES_ITALIC, "Liberation Serif Italic");
	Fl::set_font(FL_TIMES_BOLD_ITALIC, "Liberation Serif Bold Italic");
	// Fl::set_font(FL_HELVETICA,            "Liberation Sans");
	// Fl::set_font(FL_HELVETICA_BOLD,       "Liberation Sans Bold");
	// Fl::set_font(FL_HELVETICA_ITALIC,     "Liberation Sans Italic");
	// Fl::set_font(FL_HELVETICA_BOLD_ITALIC,"Liberation Sans Bold Italic");	
#else 
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, " Courier New");
	Fl::set_font(FL_COURIER_BOLD, "BCourier New");
	Fl::set_font(FL_COURIER_ITALIC, "ICourier New");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "PCourier New");
	// Lucida Console is more readable than default Terminal
	Fl::set_font(FL_SCREEN, " Lucida Console");
	Fl::set_font(FL_SCREEN_BOLD, "BLucida Console");
#endif
	// Default message properties
	fl_message_size_ = FL_NORMAL_SIZE;
	fl_message_font_ = 0;
	fl_message_title_default(APP_NAME.c_str());
	// Default scrollbar
	Fl::scrollbar_size(10);
}

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
#include <FL/fl_draw.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Window.H>

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


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

//! \file zc_fltk.cpp
//! This file provides a number of common utility methods for use with the FLTK GUI library.

#include "zc_fltk.h"

#include "zc_drawing.h"

#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Window.H>

#include <string>

extern std::string APP_NAME;

// Global variables for default sizes - defined in zc_fltk.h
const int DEFAULT_DEFAULT_SIZE = 12;           //!< Default base size - also used to calculate other sizes
int DEFAULT_SIZE = DEFAULT_DEFAULT_SIZE; //!< Default font size
int MIN_SIZE = 8;                        //!< Minimum font size
int MAX_SIZE = 16;                       //!< Maximum font size
Fl_Fontsize FONT_SIZE = DEFAULT_SIZE;    //!< Default font size
Fl_Font FONT = FL_HELVETICA;             //!< Default font

int MENU_HEIGHT = 3 * DEFAULT_SIZE;   //!< Height of menu bar
int TOOL_HEIGHT = 2 * DEFAULT_SIZE;   //!< Height of tool bar
int FOOT_HEIGHT = DEFAULT_SIZE;       //!< Height of fotter - used for copyright statements in windows.
int TOOL_GAP = DEFAULT_SIZE / 2;      //!< Gap between groups of toolbar items
int BORDER_SIZE = 5;                  //!< Width of window borders
int TAB_HEIGHT = 2 * DEFAULT_SIZE;    //!< Height of tab bar in tabbed views

int HBUTTON = 2 * DEFAULT_SIZE;                //!< Height of a normal button
int WBUTTON = 6 * DEFAULT_SIZE;                //!< Width of a normal button
int GAP = DEFAULT_SIZE;                        //!< Gap between non-related widgets
int HTEXT = 2 * DEFAULT_SIZE;                  //!< Gap to leave for text
int WRADIO = DEFAULT_SIZE + 5;                 //!< Width of a box-less rado button
int HRADIO = WRADIO;                           //!< Height of a boxless button
int WLABEL = 5 * DEFAULT_SIZE;                 //!< gap for a label outwith widget
int WLLABEL = 10 * DEFAULT_SIZE;               //!< gap for a large label outwith widget
int HMLIN = 3 * HTEXT;                         //!< Height of a multi-line text box
int WEDIT = 3 * WBUTTON;                       //!< Width of a text edit box
int WSMEDIT = 2 * WBUTTON;                     //!< Width of a small text edit box
int ROW_HEIGHT = DEFAULT_SIZE + 4;             //!< Default height for table rows


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
void zc::customise_fltk(int base_size) {
	set_base_size(base_size);
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
	Fl::set_font(FL_HELVETICA,            "Liberation Sans");
	Fl::set_font(FL_HELVETICA_BOLD,       "Liberation Sans Bold");
	Fl::set_font(FL_HELVETICA_ITALIC,     "Liberation Sans Italic");
	Fl::set_font(FL_HELVETICA_BOLD_ITALIC,"Liberation Sans Bold Italic");	
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
#ifdef _DEBUG
	for (int i = 0; i < 16; i++) {
		int dummy;
		const char* name = Fl::get_font_name(i, &dummy);
		int* sizes;
		int num_sizes = Fl::get_font_sizes(i, sizes);
		printf("Font %d: %s (%d) %d sizes\n", i, name, dummy, num_sizes);
	}
#endif
	// Default message properties
	fl_message_size_ = FL_NORMAL_SIZE;
	fl_message_font_ = 0;
	fl_message_title_default(APP_NAME.c_str());
	// Default scrollbar
	Fl::scrollbar_size(10);
}

bool zc::change_base_size(bool increase, bool update) {
	int delta = increase ? 1 : -1;
	int current_size = DEFAULT_SIZE;
	DEFAULT_SIZE += delta;
	if (DEFAULT_SIZE < MIN_SIZE) DEFAULT_SIZE = MIN_SIZE;
	if (DEFAULT_SIZE > MAX_SIZE) DEFAULT_SIZE = MAX_SIZE;
	if (current_size != DEFAULT_SIZE || update) {
		update_sizes();
		return true;
	}
	return false;
}

bool zc::set_base_size(int new_size, bool update) {
	if (new_size < MIN_SIZE) new_size = MIN_SIZE;
	if (new_size > MAX_SIZE) new_size = MAX_SIZE;
	int current_size = DEFAULT_SIZE;
	DEFAULT_SIZE = new_size;
	if (current_size != DEFAULT_SIZE || update) {
		update_sizes();
		return true;
	}
	return false;
}

// Update all the sizes based on the current base size.
void zc::update_sizes() {
	FONT_SIZE = DEFAULT_SIZE;
	MENU_HEIGHT = 3 * DEFAULT_SIZE;
	TOOL_HEIGHT = 2 * DEFAULT_SIZE;
	FOOT_HEIGHT = DEFAULT_SIZE;
	TOOL_GAP = DEFAULT_SIZE / 2;
	TAB_HEIGHT = 2 * DEFAULT_SIZE;
	HBUTTON = 2 * DEFAULT_SIZE;
	WBUTTON = 6 * DEFAULT_SIZE;
	GAP = DEFAULT_SIZE;
	HTEXT = 2 * DEFAULT_SIZE;
	WRADIO = DEFAULT_SIZE + 5;
	HRADIO = WRADIO;
	WLABEL = 5 * DEFAULT_SIZE;
	WLLABEL = 10 * DEFAULT_SIZE;
	HMLIN = 3 * HTEXT;
	WEDIT = 3 * WBUTTON;
	WSMEDIT = 2 * WBUTTON;
	ROW_HEIGHT = DEFAULT_SIZE + 4;
	// Update the normal size for FLTK
	FL_NORMAL_SIZE = FONT_SIZE;
}
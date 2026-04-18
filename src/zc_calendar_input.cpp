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
#include "zc_calendar_input.h"

#include "zc_button_input.h"
#include "zc_calendar.h"
#include "zc_fltk.h"
#include "zc_utils.h"

#include <cstring>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Widget.H>



zc_calendar_input::zc_calendar_input(int X, int Y, int W, int H, const char* L) :
	zc_button_input(X, Y, W, H, L)
{
	bn_->label("@calendar");
	bn_->callback(cb_button, nullptr);

}

zc_calendar_input::~zc_calendar_input() {}

// Overload callback
void zc_calendar_input::callback(Fl_Callback* cb, void* v) {
	// Default behaviour of button input - sets ip_ callback
	zc_button_input::callback(cb, v);
	// Now set the user data on the button
	bn_->user_data(v);
}

// Overload user data
void zc_calendar_input::user_data(void* v) {
	ip_->user_data(v);
	bn_->user_data(v);
}

// Call back for the button
void zc_calendar_input::cb_button(Fl_Widget* w, void* v) {
	zc_calendar_input* that = zc::ancestor_view<zc_calendar_input>(w);
	zc_calendar* cal = new zc_calendar(w->x(), w->y());
	const char* ip_value = that->input()->value();
	if (strlen(ip_value) == 0) {
		std::string today = zc::now(false, that->format_).c_str();
		cal->value(today.c_str());
		that->input()->value(today.c_str());
	}
	else {
		cal->value(ip_value);
	}
	cal->format(that->format_);
	cal->callback(cb_calendar, that->input());
	cal->show();
	Fl_Widget_Tracker wt(cal);
	while (wt.exists()) Fl::check();
}

// Call back for calendar
void zc_calendar_input::cb_calendar(Fl_Widget* w, void* v) {
	Fl_Input* ip = (Fl_Input*)v;
	zc_calendar* cal = (zc_calendar*)w;
	ip->value(cal->value());
	ip->redraw();
	ip->do_callback();
}

// Set Format
void zc_calendar_input::format(const char* value) {
	format_ = value;
}

// Get format
const char* zc_calendar_input::format() {
	return format_;
}

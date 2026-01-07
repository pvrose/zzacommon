#include "calendar_input.h"

#include "button_input.h"
#include "calendar.h"
#include "utils.h"

#include <cstring>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Widget.H>



calendar_input::calendar_input(int X, int Y, int W, int H, const char* L) :
	button_input(X, Y, W, H, L)
{
	bn_->label("@calendar");
	bn_->callback(cb_button, nullptr);

}

calendar_input::~calendar_input() {}

// Overload callback
void calendar_input::callback(Fl_Callback* cb, void* v) {
	// Default behaviour of button input - sets ip_ callback
	button_input::callback(cb, v);
	// Now set the user data on the button
	bn_->user_data(v);
}

// Overload user data
void calendar_input::user_data(void* v) {
	ip_->user_data(v);
	bn_->user_data(v);
}

// Call back for the button
void calendar_input::cb_button(Fl_Widget* w, void* v) {
	calendar_input* that = ancestor_view<calendar_input>(w);
	calendar* cal = new calendar(w->x(), w->y());
	const char* ip_value = that->input()->value();
	if (strlen(ip_value) == 0) {
		std::string today = now(false, that->format_).c_str();
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
void calendar_input::cb_calendar(Fl_Widget* w, void* v) {
	Fl_Input* ip = (Fl_Input*)v;
	calendar* cal = (calendar*)w;
	ip->value(cal->value());
	ip->redraw();
	ip->do_callback();
}

// Set Format
void calendar_input::format(const char* value) {
	format_ = value;
}

// Get format
const char* calendar_input::format() {
	return format_;
}

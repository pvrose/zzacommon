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
#include "zc_graph.h"

#include "zc_drawing.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_Widget.H>

//! Multiplier to prefix map
static std::map<int, uint32_t> SI_PREFIXES = {
	{ -24, 'y' },
	{  -21, 'z' },
	{  -18, 'a' },
	{  -15, 'f' },
	{ -12, 'p' },
	{ -9, 'n' },
	{ -6, 0x3Bc },  // Greek letter mu
	{ -3, 'm' },
	{ 0, ' ' },
	{ 3, 'k' },
	{ 6, 'M' },
	{ 9, 'G'},
	{ 12, 'T'},
	{ 15, 'P' },
	{ 18, 'E' },
	{ 21, 'Z' },
	{ 24, 'Y' }
};


zc_graph::zc_graph(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L)
{	
}

zc_graph::~zc_graph() {
}

void zc_graph::set_params(const options_t& x_options, const options_t& y_options) 
{
	x_options_ = x_options;
	y_options_.clear();
	y_options_[Y_LEFT] = y_options;
	set_factors(true);
}

void zc_graph::set_params(const options_t& x_options, const options_t& y_left_options, const options_t& y_right_options) {
	x_options_ = x_options;
	y_options_[Y_LEFT] = y_left_options;
	y_options_[Y_RIGHT] = y_right_options;
	set_factors(true);
}

void zc_graph::set_drawing_area() {
	// Booleans to show left and right Y-axes
	bool show_left_axis = y_options_.find(Y_LEFT) != y_options_.end();
	bool show_right_axis = y_options_.find(Y_RIGHT) != y_options_.end();
	// See how much is needed for a label
	fl_font(labelfont(), labelsize());
	int dw = 0, dh = 0;
	fl_measure("Dummy", dw, dh);
	// Add some padding.
	dh += 4;
	// Set width of area.
	int ax = x();
	int aw = w();
	// Leave space for left Y-axis label and tick labels if needed
	if (show_left_axis) {
		ax += dh;
		aw -= dh;
	}
	// Leave space for right Y-axis label and tick labels if needed
	if (show_right_axis) {
		aw -= dh;
	}
	// Leave enough for label and outward (downward) tick labels
	int ay = y();
	int ah = h() - dh - dh;
	drawing_area_ = Fl_Rect(ax, ay, aw, ah);
}

void zc_graph::set_factors(bool unzoom) {
	set_drawing_area();
	x_options_.set_origin_length(drawing_area_.x(), drawing_area_.w());
	// Set X scaling factor (units per pixel)
	x_options_.set_factors(unzoom);
	// And repeat for all Y - Note increasing Y data value is decreasing pixel position
	for (auto& y_option : y_options_) {
		y_option.second.set_origin_length(drawing_area_.y() + drawing_area_.h(), -drawing_area_.h());
		y_option.second.set_factors(unzoom);
	}
}

void zc_graph::options_t::set_ticks() {
	// Now optimise the ticks - aim for a tick about every 20 pixels
	float xtick = abs(suggested_gap * scale);
	float ntick;
	float ptick;
	float xline;   // Draw a line every few ticks.
	int si_prefix = ' ';
	normalise(xtick, ntick, ptick, si_prefix);
	switch (xier_type) {
	case SI_PREFIX:
		// If 20 pixels is graeter that 7*10^N - set tick at 10^(N+1), line at 5*10^(N+1)
		if (ntick > 70.0F) {
			xtick = 100.0 * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 3.2 and 7 set tick at 5, line at 20.
		else if (ntick > 32.0F) {
			xtick = 50.0F * ptick;
			xline = 4.0F * xtick;
		}
		// If 20 pixels is between 1.4 and 3.2 set tick at 2, line at 10.
		else if (ntick > 14.0F) {
			xtick = 20.0F * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 0.7 and 1.4 set tick at 1, line at 5.
		else if (ntick > 7.0F) {
			xtick = 10.0F * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 3.2 and 7 set tick at 5, line at 20.
		else if (ntick > 3.2F) {
			xtick = 5.0F * ptick;
			xline = 4.0F * xtick;
		}
		// If 20 pixels is between 1.4 and 3.2 set tick at 2, line at 10.
		else if (ntick > 1.4F) {
			xtick = 2.0F * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 0.7 and 1.4 set tick at 1, line at 5.
		else if (ntick > 0.7F) {
			xtick = ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 0.32 and 0.7
		else if (ntick > 0.32F) {
			xtick = 0.5 * ptick;
			xline = 4.0F * xtick;
		}
		// If 20 pixels > 0.14
		else if (ntick > 0.14F) {
			xtick = 0.2 * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels > 0.1
		else {
			xtick = 0.1 * ptick;
			xline = 5.0F * xtick;
		}
		break;
	case POWER_10:
	case NONE:
		if (ntick > 7.0F) {
			xtick = 10.0 * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 3.2 and 7 set tick at 5
		else if (ntick > 3.2F) {
			xtick = 5.0F * ptick;
			xline = 4.0F * xtick;
		}
		// If 20 pixels is between 1.4 and 3.2 set tick at 2
		else if (ntick > 1.4F) {
			xtick = 2.0F * ptick;
			xline = 5.0F * xtick;
		}
		// If 20 pixels is between 0.7 and 1.4 set tick at 1
		else {
			xtick = ptick;
			xline = 5.0F * xtick;
		}
		break;
	}
	// Calculate the label positions
	ticks.clear();
	lines.clear();
	// Y < 0
	float tick = -xtick;
	while (tick >= minimum) {
		char l[10];
		if (tick <= maximum) {
			snprintf(l, sizeof(l), "%g", xier_type == NONE ? tick : tick / ptick);
			ticks.push_back({ float_to_point(tick), std::string(l)});
		}
		tick -= xtick;
	}
	// Y >= 0
	tick = 0;
	while (tick <= maximum) {
		char l[10];
		if (tick >= minimum) {
			snprintf(l, sizeof(l), "%g", xier_type == NONE ? tick : tick / ptick);
			ticks.push_back({ float_to_point(tick), std::string(l) });
		}
		tick += xtick;
	}
	// Set the lines
	for (float line = 0; line <= maximum; line += xline) {
		lines.push_back(float_to_point(line));
	}
	for (float line = -xline; line >= minimum; line -= xline) {
		lines.push_back(float_to_point(line));
	}
	char ll[128];
	switch (xier_type) {
	case SI_PREFIX:
		snprintf(ll, sizeof(ll), "%c%s", si_prefix, base_label);
		break;
	case POWER_10:
		snprintf(ll, sizeof(ll), "\303\227%g %s", ptick, base_label);
		break;
	case NONE:
		strncpy(ll, base_label, sizeof(ll));
		break;
	}
	label = ll;
}

// Handle mouse events for zooming and scrolling
int zc_graph::handle(int event) {
	// Only handle events if we have zoomable or scrollable axes
	// Mouse actions will only apply if we are in the axis areas.
	// Get the mouse position.
	int mx = Fl::event_x();
	int my = Fl::event_y();
	bool in_left_axis = y_options_.find(Y_LEFT) != y_options_.end() && mx >= x() && mx < drawing_area_.x() && y_options_[Y_LEFT].zoomable;
	bool in_right_axis = y_options_.find(Y_RIGHT) != y_options_.end() && mx >= drawing_area_.x() + drawing_area_.w() && mx < x() + w() && y_options_[Y_RIGHT].zoomable;
	bool in_x_axis = my >= drawing_area_.y() + drawing_area_.h() && my < y() + h() && x_options_.zoomable;
	// Flag to indicate zoom or scroll has occurred - used to trigger redraw at end of event handling.
	bool zoom_or_scroll = false;
	// If in two axes, be in neither axis - e.g. in either corner where they meet.
	if (in_x_axis && (in_left_axis || in_right_axis)) {
		in_left_axis = false;
		in_right_axis = false;
		in_x_axis = false;
	}
	switch (event)
	{
		// If the mouse has moved into or out of the axes, change the cursor to indicate
		// that zooming and scrolling is possible on this axis.
	case FL_MOVE:
		if (in_left_axis) {
			fl_cursor(FL_CURSOR_NS);
		}
		else if (in_right_axis) {
			fl_cursor(FL_CURSOR_NS);
		}
		else if (in_x_axis) {
			fl_cursor(FL_CURSOR_WE);
		}
		else {
			fl_cursor(FL_CURSOR_DEFAULT);
		}
		break;
		// If the mouse wheel is scrolled while in an axis, zoom in or out on that axis.
	case FL_MOUSEWHEEL:
		// Get the scroll direction and amount - positive is scroll up, negative is scroll down.
		if (in_left_axis) {
			int scroll = Fl::event_dy();
			y_options_[Y_LEFT].update_zoom(my, scroll);
			zoom_or_scroll = true;
		}
		else if (in_right_axis) {
			int scroll = Fl::event_dy();
			y_options_[Y_RIGHT].update_zoom(my, scroll);
			zoom_or_scroll = true;
		}
		else if (in_x_axis) {
			int scroll = Fl::event_dy();
			// Zoom in is positive scroll, zoom out is negative scroll. Zoom by 10% per scroll step.
			x_options_.update_zoom(mx, scroll);
			zoom_or_scroll = true;
		}
		else {
			return Fl_Widget::handle(event);
		}
		break;
		// If the mouse is pushed while in an axis, start a scroll operation on that axis.
	case FL_PUSH:
		// If left button pushed in an axis, start scroll operation on that axis.
		if (Fl::event_button() == FL_LEFT_MOUSE) {
			if (in_left_axis) {
				y_options_[Y_LEFT].start_scroll(my);
				return true;
			}
			else if (in_right_axis) {
				y_options_[Y_RIGHT].start_scroll(my);
				return true;
			}
			else if (in_x_axis) {
				x_options_.start_scroll(mx);
				return true;
			}
			else {
				return Fl_Widget::handle(event);
			}
		}
		// If right button pushed, reset zoom on that axis.
		else if (Fl::event_button() == FL_RIGHT_MOUSE) {
			if (in_left_axis) {
				y_options_[Y_LEFT].set_factors(true);
				zoom_or_scroll = true;
			}
			else if (in_right_axis) {
				y_options_[Y_RIGHT].set_factors(true);
				zoom_or_scroll = true;
			}
			else if (in_x_axis) {
				x_options_.set_factors(true);
				zoom_or_scroll = true;
			}
			else {
				return Fl_Widget::handle(event);
			}
		}
		else {
			return Fl_Widget::handle(event);
		}
		// If the mouse is released, end any scroll operation.
	case FL_RELEASE:
		if (y_options_.find(Y_LEFT) != y_options_.end() && y_options_[Y_LEFT].is_scrolling()) {
			y_options_[Y_LEFT].end_scroll();
		}
		if (y_options_.find(Y_RIGHT) != y_options_.end() && y_options_[Y_RIGHT].is_scrolling()) {
			y_options_[Y_RIGHT].end_scroll();
		}
		if (x_options_.is_scrolling()) {
			x_options_.end_scroll();
		}
		break;
		// If the mouse is moved while a scroll operation is in progress, update the scroll offset for that axis.
	case FL_DRAG:
		if (y_options_.find(Y_LEFT) != y_options_.end() && y_options_[Y_LEFT].is_scrolling()) {
			y_options_[Y_LEFT].update_scroll(my);
			zoom_or_scroll = true;
		}
		else if (y_options_.find(Y_RIGHT) != y_options_.end() && y_options_[Y_RIGHT].is_scrolling()) {
			y_options_[Y_RIGHT].update_scroll(my);
			zoom_or_scroll = true;
		}
		else if (x_options_.is_scrolling()) {
			x_options_.update_scroll(mx);
			zoom_or_scroll = true;
		}
		else {
			return Fl_Widget::handle(event);
		}
		break;
	case FL_ENTER:
		return true;
	default:
		break;
	}
	if (zoom_or_scroll) redraw();
	return true;
}


// Resize the widget - reset scaling factors
void zc_graph::resize(int X, int Y, int W, int H) {
	// If we have actually resized...
	if (X != x() || Y != y() || W != w() || H != h()) {
		Fl_Widget::resize(X, Y, W, H);
		set_factors(true);
//		convert_data_to_points();
		redraw();
	}
}

void zc_graph::draw() {
	Fl_Color save = fl_color();
	draw_box();
	draw_label();
	//! Clear the background - leave a 1 pixel border to show the box.
	fl_color(color());
	fl_rectf(x() + 1, y() + 1, w() - 2, h() - 2);
	// Draw the axes
	draw_axes();
	// Draw the points
	draw_points();
	fl_color(save);
}

void zc_graph::draw_points() {
	// Restrict the drawing area
	fl_push_clip(drawing_area_.x(), drawing_area_.y(), drawing_area_.w(), drawing_area_.h());
	// Set FG colour
	fl_color(color());

	for (auto& ds : data_sets_) {
		auto data = ds.data;
		fl_color(ds.style.colour);
		fl_line_style(ds.style.style, ds.style.width);
		if (data && data->size() > 1) {
			for (size_t ix = 0; ix < data->size() - 1; ix++) {
				int x1 = x_options_.float_to_point((*data)[ix].x);
				int x2 = x_options_.float_to_point((*data)[ix + 1].x);
				int y1 = y_options_[ds.y_axis].float_to_point((*data)[ix].y);
				int y2 = y_options_[ds.y_axis].float_to_point((*data)[ix + 1].y);
				// If either end is in the drawing area...
				if (in_drawing_area(x1, y1) || in_drawing_area(x2, y2)) {
					// Draw the line between them - should be clipped at the edge of the area.
					fl_line(x1, y1, x2, y2);
				}
			}
		}
		fl_line_style(0);
	}

	// POP the clip area
	fl_pop_clip();
}

// Draw the axes
void zc_graph::draw_axes() {
	// Restrict the drawing area
	fl_push_clip(x(), y(), w(), h());
	// Flagsto draw y-axes.
	bool show_left_axis = y_options_.find(Y_LEFT) != y_options_.end();
	bool show_right_axis = y_options_.find(Y_RIGHT) != y_options_.end();
	// Tick width and height
	int tw = 0;
	int th = 0;
	// Drawing area bounds
	int dl = drawing_area_.x();
	int dr = drawing_area_.x() + drawing_area_.w();
	int drt = x() + w() - 4;
	int dch = drawing_area_.x() + drawing_area_.w() / 2;
	int dt = drawing_area_.y();
	int db = drawing_area_.y() + drawing_area_.h();
	int dcv = drawing_area_.y() + drawing_area_.h() / 2;
	// Draw the left Y-axis
	if (show_left_axis) {
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(dl, dt, dl, db);
		// Each tick extends into the drawing area
		for (auto& l : y_options_[Y_LEFT].ticks) {
			fl_line(dl, l.pos, dl + 5, l.pos);
			fl_measure(l.label.c_str(), tw, th);
			fl_draw(l.label.c_str(), dl + 5, l.pos + th / 2);
		}
		// Now add the Y- label
		tw = 0;
		th = 0;
		fl_measure(y_options_[Y_LEFT].label.c_str(), tw, th);
		// Position it in the middle of the axiis
		fl_draw(90, y_options_[Y_LEFT].label.c_str(), dl - 3, dcv);
		// Draw the lines every few ticks
		fl_color(fl_lighter(FL_FOREGROUND_COLOR));
		fl_line_style(FL_DOT);
		for (auto& l : y_options_[Y_LEFT].lines) {
			fl_line(dl, l, dr, l);
		}
		fl_line_style(0);
	}
	// Draw the right Y-axis
	if (show_right_axis) {
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(dr, dt, dr, db);
		// Each tick extends into the drawing area
		for (auto& l : y_options_[Y_RIGHT].ticks) {
			fl_line(dr, l.pos, dr - 5, l.pos);
			fl_measure(l.label.c_str(), tw, th);
			fl_draw(l.label.c_str(), dr - 5 - tw, l.pos + th / 2);
		}
		// Now add the Y- label
		tw = 0;
		th = 0;
		fl_measure(y_options_[Y_RIGHT].label.c_str(), tw, th);
		// Position it in the middle of the axiis
		fl_draw(90, y_options_[Y_RIGHT].label.c_str(), drt, dcv);
	}

	// Draw the X-axis	
	fl_line(dl, db, dr, db);
	// Now put the Y-axis
	tw = 0;
	th = 0;
	// Now add the X ticks
	for (auto& l : x_options_.ticks) {
		fl_line(l.pos, db, l.pos, db + 5);
		fl_measure(l.label.c_str(), tw, th);
		fl_draw(l.label.c_str(), l.pos - tw / 2, db + 5 + th);
	}
	// Now add the X-label
	fl_measure(x_options_.label.c_str(), tw, th);
	int tx = dch - tw / 2;
	int ty = y() + h() - 2;
	fl_draw(x_options_.label.c_str(), tx, ty);

	fl_pop_clip();
}

//! \brief Set value as data to display.
//! Sets into the first 
void zc_graph::set_data(std::vector<coord>* data) {
	if (data_sets_.size() == 0) {
		// If we don't have any data sets yet, add one for this data.
		// Set default style for this data set - 
		// -  Colour is the same as the graph colour, 
		// -  Line width is 1 and 
		// -  Style is solid.
		add_data_set({ Y_LEFT, zc_line_style(selection_color(), 1, FL_SOLID), data});
	}
	else {
		data_sets_[0].data = data;
	}
}

//! \brief Set value into specified data set.
//! \param index The index of the data set to update.
//! \param data The data to set for this data set.
void zc_graph::set_data(int index, std::vector<coord>* data) {
	if (index >= 0 && index < data_sets_.size()) {
		data_sets_[index].data = data;
	}
}	

//! \brief Add a set of data to display.
int zc_graph::add_data_set(const data_set_t& ds) {
	data_sets_.push_back(ds);
	return data_sets_.size() - 1;
}

//! \brief Clear all data sets.
void zc_graph::clear_data_sets() {
	data_sets_.clear();
}

//! \brief. Convert data point \p f from float to y position
int zc_graph::options_t::float_to_point(float f) {
	float fx = f < minimum ? minimum : (f > maximum ? maximum : f);
	int result = position_0 + fx * inv_scale;
	return result;
}

// Normalise the number
void zc_graph::options_t::normalise(float fin, float& norm, float& exp10, int& si_prefix) {
	exp10 = 1.0F;
	norm = fabs(fin);
	float step = 10.0F;
	float inv_step = 0.01F;
	float upper = 10.0F;
	float lower = 1.0F;
	int pow_10 = 0;
	int pow_step = 1;
	switch (xier_type) {
	case SI_PREFIX:
		step = 1000.0F;
		pow_step = 3;
		inv_step = 0.001F;
		upper = 100.0F;
		lower = 0.1F;
		break;
	case POWER_10:
	case NONE:
		step = 10.0F;
		pow_step = 1;
		inv_step = 0.1F;
		upper = 10.0F;
		lower = 1.0F;
		break;
	}
	if (norm == 0.0F) return;
	// The input value is > 10 reduce it until it's less than 1
	while (norm > upper) {
		norm *= inv_step;
		exp10 *= step;
		pow_10 += pow_step;
	}
	// If it was less than 0.1 increase it until it's greater than 0.1
	while (norm < lower) {
		norm *= step;
		exp10 *= inv_step;
		pow_10 -= pow_step;
	}
	if (fin < 0) norm = -norm;
	if (xier_type == SI_PREFIX) si_prefix = SI_PREFIXES.at(pow_10);
}

// Returns true if inwith drawing area, false otherwise
bool zc_graph::in_drawing_area(int px, int py) {
	if (px < drawing_area_.x()) return false;
	if (px > drawing_area_.x() + drawing_area_.w()) return false;
	if (py < drawing_area_.y()) return false;
	if (py > drawing_area_.y() + drawing_area_.h()) return false;
	return true;
}


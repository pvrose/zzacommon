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
	{ -12, 'p' },
	{ -9, 'n' },
	{ -6, 0x3Bc },
	{ -3, 'm' },
	{ 0, ' ' },
	{ 3, 'k' },
	{ 6, 'M' },
	{ 9, 'G'},
	{ 12, 'T'},
};


zc_graph::zc_graph(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L)
{
	data_ = nullptr;
	set_drawing_area();
}

zc_graph::~zc_graph() {
}

void zc_graph::set_params(const options_t& x_options, const options_t& y_options) 
{
	x_options_ = x_options;
	y_options_ = y_options;
	set_factors();
}

void zc_graph::set_drawing_area() {
	// See how much is needed for a label
	fl_font(labelfont(), labelsize());
	int dw = 0, dh = 0;
	fl_measure("Dummy", dw, dh);
	// Just leave one line of text - tick marks are inside area
	int ax = x() + dh;
	int aw = w() - dh;
	// Leave enough for label and outward (downward) tick labels
	int ay = y();
	int ah = h() - dh - dh;
	drawing_area_ = Fl_Rect(ax, ay, aw, ah);
}

void zc_graph::set_factors() {
	// Assumes drawing area has been set up.
	// Set X scaling factor (units per pixel)
	float x_range = x_options_.maximum - x_options_.minimum;
	x_options_.scale = x_range / drawing_area_.w();
	x_options_.inv_scale = 1.0F / x_options_.scale;
	// Set X origin
	x_options_.position_0 = drawing_area_.x() + (x_options_.minimum * x_options_.inv_scale);
	set_ticks(x_options_, x_ticks_, x_label_);
	// And repeat for Y - Note increasing Y data value is decreasing pixel position
	float y_range = y_options_.minimum - y_options_.maximum;
	y_options_.scale = y_range / drawing_area_.h();
	y_options_.inv_scale = 1.0F / y_options_.scale;
	y_options_.position_0 = drawing_area_.y() - (y_options_.maximum * y_options_.inv_scale);
	set_ticks(y_options_, y_ticks_, y_label_);
}

void zc_graph::set_ticks(const options_t& options, std::vector<tick_t>& ticks, std::string& label) {
	// Now optimise the ticks - aim for a tick about every 20 pixels
	float xtick = abs(options.suggested_gap * options.scale);
	float ntick;
	float ptick;
	int si_prefix = ' ';
	normalise(xtick, ntick, ptick, si_prefix, options.xier_type);
	switch (options.xier_type) {
	case SI_PREFIX:
		// If 20 pixels is graeter that 7*10^N - set tick at 10^(N+1)
		if (ntick > 70.0F) xtick = 100.0 * ptick;
		// If 20 pixels is between 3.2 and 7 set tick at 5
		else if (ntick > 32.0F) xtick = 50.0F * ptick;
		// If 20 pixels is between 1.4 and 3.2 set tick at 2
		else if (ntick > 14.0F) xtick = 20.0F * ptick;
		// If 20 pixels is between 0.7 and 1.4 set tick at 1
		else if (ntick > 7.0F) xtick = 10.0 * ptick;
		// If 20 pixels is between 3.2 and 7 set tick at 5
		else if (ntick > 3.2F) xtick = 5.0F * ptick;
		// If 20 pixels is between 1.4 and 3.2 set tick at 2
		else if (ntick > 1.4F) xtick = 2.0F * ptick;
		// If 20 pixels is between 0.7 and 1.4 set tick at 1
		else if (ntick > 0.7F) xtick = ptick;
		// If 20 pixels is between 0.32 and 0.7
		else if (ntick > 0.32F) xtick = 0.5 * ptick;
		// If 20 pixels > 0.14
		else if (ntick > 0.14F) xtick = 0.2 * ptick;
		// If 20 pixels > 0.1
		else xtick = 0.1 * ptick;
		break;
	case POWER_10:
	case NONE:
		if (ntick > 7.0F) xtick = 10.0 * ptick;
		// If 20 pixels is between 3.2 and 7 set tick at 5
		else if (ntick > 3.2F) xtick = 5.0F * ptick;
		// If 20 pixels is between 1.4 and 3.2 set tick at 2
		else if (ntick > 1.4F) xtick = 2.0F * ptick;
		// If 20 pixels is between 0.7 and 1.4 set tick at 1
		else xtick = ptick;
		break;
	}
	// Calculate the label positions
	ticks.clear();
	// Y < 0
	float tick = -xtick;
	while (tick >= options.minimum) {
		char l[10];
		if (tick <= options.maximum) {
			snprintf(l, sizeof(l), "%g", options.xier_type == NONE ? tick : tick / ptick);
			ticks.push_back({ float_to_point(tick, options), std::string(l)});
		}
		tick -= xtick;
	}
	// Y > 0
	tick = xtick;
	while (tick <= options.maximum) {
		char l[10];
		if (tick >= options.minimum) {
			snprintf(l, sizeof(l), "%g", options.xier_type == NONE ? tick : tick / ptick);
			ticks.push_back({ float_to_point(tick, options), std::string(l) });
		}
		tick += xtick;
	}
	char ll[128];
	switch (options.xier_type) {
	case SI_PREFIX:
		snprintf(ll, sizeof(ll), "%c%s", si_prefix, options.base_label);
		break;
	case POWER_10:
		snprintf(ll, sizeof(ll), "\303\227%g %s", ptick, options.base_label);
		break;
	case NONE:
		strncpy(ll, options.base_label, sizeof(ll));
		break;
	}
	label = ll;
}

// Resize the widget - reset scaling factors
void zc_graph::resize(int X, int Y, int W, int H) {
	// If we have actually resized...
	if (X != x() || Y != y() || W != w() || H != h()) {
		Fl_Widget::resize(X, Y, W, H);
		set_drawing_area();
		set_factors();
//		convert_data_to_points();
		redraw();
	}
}

void zc_graph::draw() {
	Fl_Color save = fl_color();
	draw_box();
	draw_label();
	//! \todo Add detaild
	fl_color(FL_BACKGROUND_COLOR);
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

	if (data_ && data_->size() > 1) {
		for (size_t ix = 0; ix < data_->size() - 1; ix++) {
			int x1 = float_to_point((*data_)[ix].x, x_options_);
			int x2 = float_to_point((*data_)[ix + 1].x, x_options_);
			int y1 = float_to_point((*data_)[ix].y, y_options_);
			int y2 = float_to_point((*data_)[ix + 1].y, y_options_);
			// If either end is in the drawing area...
			if (in_drawing_area(x1, y1) || in_drawing_area(x2, y2)) {
				// Draw the line between them - should be clipped at the edge of the area.
				fl_line(x1, y1, x2, y2);
			}
		}
	}

	// POP the clip area
	fl_pop_clip();
}

// Draw the axes
void zc_graph::draw_axes() {
	// Restrict the drawing area
	fl_push_clip(x(), y(), w(), h());
	// Draw the Y-axis
	fl_color(FL_FOREGROUND_COLOR);
	fl_line(x_options_.position_0, drawing_area_.y(), x_options_.position_0, drawing_area_.y() + drawing_area_.h());
	// Draw the X-axis
	fl_line(drawing_area_.x(), y_options_.position_0, drawing_area_.x() + drawing_area_.w(), y_options_.position_0);
	// Now put the Y-axis
	int tw = 0;
	int th = 0;
	// Each tick extends into the drawing area
	for (auto& l : y_ticks_) {
		fl_line(x_options_.position_0, l.pos, x_options_.position_0 + 5, l.pos);
		fl_measure(l.label.c_str(), tw, th);
		fl_draw(l.label.c_str(), x_options_.position_0 + 5, l.pos + th / 2);
	}
	// Now add the Y- label
	tw = 0;
	th = 0;
	fl_measure(x_label_.c_str(), tw, th);
	// Position it in the middle of the axiis
	int tx = x_options_.position_0 - 1;
	int ty = drawing_area_.y() + (drawing_area_.h() + tw) / 2;
	fl_draw(90, y_label_.c_str(), tx, ty);
	// Now add the X ticks
	for (auto& l : x_ticks_) {
		fl_line(l.pos, y_options_.position_0, l.pos, y_options_.position_0 + 5);
		fl_measure(l.label.c_str(), tw, th);
		fl_draw(l.label.c_str(), l.pos - tw / 2, y_options_.position_0 + 5 + th);
	}
	// Now add the X-label
	fl_measure(x_label_.c_str(), tw, th);
	tx = drawing_area_.x() + (drawing_area_.w() - tw) / 2;
	ty = y() + h() - 2;
	fl_draw(x_label_.c_str(), tx, ty);

	fl_pop_clip();
}

//! \brief Set value as data to display.
void zc_graph::set_data(std::vector<coord>* data) {
	data_ = data;
//	convert_data_to_points();
}

//! \brief. Convert data point \p f from float to y position
int zc_graph::float_to_point(float f, const options_t& options) {
	float fx = f < options.minimum ? options.minimum : (f > options.maximum ? options.maximum : f);
	int result = options.position_0 + fx * options.inv_scale;
	return result;
}

// Normalise the number
void zc_graph::normalise(float fin, float& norm, float& exp10, int& si_prefix, const axis_xier_t& xier) {
	exp10 = 1.0F;
	norm = fabs(fin);
	float step = 10.0F;
	float inv_step = 0.01F;
	float maximum = 10.0F;
	float minimum = 1.0F;
	int pow_10 = 0;
	int pow_step = 1;
	switch (xier) {
	case SI_PREFIX:
		step = 1000.0F;
		pow_step = 3;
		inv_step = 0.001F;
		maximum = 100.0F;
		minimum = 0.1F;
		break;
	case POWER_10:
	case NONE:
		step = 10.0F;
		pow_step = 1;
		inv_step = 0.1F;
		maximum = 10.0F;
		minimum = 1.0F;
		break;
	}
	if (norm == 0.0F) return;
	// The input value is > 10 reduce it until it's less than 1
	while (norm > maximum) {
		norm *= inv_step;
		exp10 *= step;
		pow_10 += pow_step;
	}
	// If it was less than 0.1 increase it until it's greater than 0.1
	while (norm < minimum) {
		norm *= step;
		exp10 *= inv_step;
		pow_10 -= pow_step;
	}
	if (fin < 0) norm = -norm;
	if (xier == SI_PREFIX) si_prefix = SI_PREFIXES.at(pow_10);
}

//// Convert data to points
//void zc_graph::convert_data_to_points() {
//	int num_points = drawing_area_.w();
//	int num_samples = data_->size() - 1;
//	y_points_.resize(num_points + 1);
//	std::vector<float> y_values;
//	y_values.resize(num_points + 1);
//	for (size_t ix = 0; ix <= num_points; ix++) {
//		// Scale the drawing point number along the data items
//		float fx = (float)ix * (float)num_samples / (float)num_points;
//		// Get the sample_num immediately below the wanted value
//		float sample_num = trunc(fx);
//		// The fractional part 
//		float fraction = fx - sample_num;
//		// Interpolate the required value between the adjacent samples
//		float value;
//		if (sample_num >= num_samples) {
//			value = (*data_)[(int)sample_num];
//		}
//		else {
//			value =
//				((*data_)[(int)sample_num] * (1.0F - fraction)) +
//				((*data_)[(int)sample_num + 1] * (fraction));
//		}
//		// And store as the pixel position
//		y_values[ix] = value;
//		int y_value = float_to_y(value);
//		y_points_[ix] = y_value;
//	}
//}

// Returns true if inwith drawing area, false otherwise
bool zc_graph::in_drawing_area(int px, int py) {
	if (px < drawing_area_.x()) return false;
	if (px > drawing_area_.x() + drawing_area_.w()) return false;
	if (py < drawing_area_.y()) return false;
	if (py > drawing_area_.y() + drawing_area_.h()) return false;
	return true;
}
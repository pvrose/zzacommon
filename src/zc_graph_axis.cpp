/*
	Copyright 2026, Philip Rose, GM3ZZA

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

#include "zc_graph_axis.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Widget.H>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>

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
	{ 0, 0x200B },  // Zero-width space for no prefix
	{ 3, 'k' },
	{ 6, 'M' },
	{ 9, 'G'},
	{ 12, 'T'},
	{ 15, 'P' },
	{ 18, 'E' },
	{ 21, 'Z' },
	{ 24, 'Y' }
};

//! \brief Constructor
zc_graph_axis::zc_graph_axis(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L),
	orientation_(X_AXIS),
	modifier_(NO_MODIFIER),
	unit_(""),
	tick_spacing_pixels_(20),
	scale_(1.0F),
	inv_scale_(1.0F),
	origin_(0)
{
}

//! \brief Destructor
zc_graph_axis::~zc_graph_axis() {
}

//! \brief Set the parameters for this axis.
void zc_graph_axis::set_params(const axis_params_t& params) {
	orientation_ = params.orientation;
	outer_range_ = params.outer_range;
	inner_range_ = params.inner_range;
	default_range_ = params.default_range;
	current_range_ = default_range_;
	modifier_ = params.modifier;
	unit_ = params.unit;
	tick_spacing_pixels_ = params.tick_spacing_pixels;
	copy_label(params.label.c_str());
	// Set the current range to the default range and calculate the scale factors and ticks.
	set_range(default_range_);
	redraw();
}

//! \brief Attempt to set the range to \p new_range.
//! \param new_range The new range to set.
void zc_graph_axis::set_range(range new_range) {
	// Check if the new range is valid.
	if (new_range.min >= new_range.max) {
		// Invalid range - do not update.
		/*throw std::invalid_argument("Invalid range: min must be less than max");*/
		return;
	}
	// Check if the new range is within the zoom limits.
	current_range_ = outer_range_.get_intersection(new_range);
	scale_ = (current_range_.max - current_range_.min) / (orientation_ == X_AXIS ? w() : -h());
	inv_scale_ = 1.0F / scale_;
	zoom_limit_range_.set_union(new_range);
	origin_ = (orientation_ == X_AXIS ? x() : y() + h()) - current_range_.min * inv_scale_;
	set_ticks();
}

//! \brief Zoom by a factor of \p zoom_factor around the value at \p mouse_pos.
//! \param mouse_pos The pixel position of the mouse along the axis.	
//! \param zoom_factor The factor to zoom by: +10 indictaes x2 zoom, -10 indicates x0.5 zoom.
void zc_graph_axis::zoom(int mouse_pos, int zoom_factor) {
	// Calculate the value at the mouse position before the zoom change.
	float mouse_value = pixel_to_float(mouse_pos);
	// Zoom change is 2^^(delta/10) - so every 10 units of delta doubles the zoom factor, every -10 units halves it.
	float zoom_change = powf(2.0F, (float)zoom_factor / 10.0F);
	// Calculate the new range based on the zoom change and mouse value.
	float new_min = mouse_value - (mouse_value - current_range_.min) * zoom_change;
	float new_max = mouse_value + (current_range_.max - mouse_value) * zoom_change;
	// Check if the new range is within the zoom limits.
	range new_range = zoom_limit_range_.get_intersection({ new_min, new_max });
	set_range(new_range);
}

//! \brief Scroll by an offset of \p scroll_offset pixels.
//! \param scroll_offset The offset to scroll by in pixels (positive or negative).
//! \return True if the scroll was applied, False if the scroll was not applied due to limits.
void zc_graph_axis::scroll(int scroll_offset) {
	// Calculate the new range based on the scroll offset and current scale.
	float new_min = current_range_.min + scroll_offset * scale_;
	float new_max = current_range_.max + scroll_offset * scale_;
	// Check if the new range is within the zoom limits.
	range new_range = zoom_limit_range_.get_intersection({ new_min, new_max });
	set_range(new_range);
}

//! \brief Override draw to draw the axis.
void zc_graph_axis::draw() {
	draw_axis_line();
	draw_ticks();
	draw_label();
};

//! \brief Draw the axis line.
void zc_graph_axis::draw_axis_line() {
	// Set the color and line width for the axis line.
	fl_color(FL_FOREGROUND_COLOR);
	fl_line_style(FL_SOLID, 1);
	// Draw the axis line based on the orientation.
	switch (orientation_) {
	case zc_graph_axis::X_AXIS:
		// Draw along the top of the widget area.
		fl_line(x(), y(), x() + w(), y());
		break;
	case zc_graph_axis::YL_AXIS:
		// Draw along the right side of the widget area.
		fl_line(x() + w(), y(), x() + w(), y() + h());
		break;
	case zc_graph_axis::YR_AXIS:
		// Draw along the left side of the widget area.
		fl_line(x(), y(), x(), y() + h());	
		break;
	default:
		break;
	}
}

//! \brief Draw the ticks and tick labels.
void zc_graph_axis::draw_ticks() {
	// Set the color and line width for the ticks.
	fl_color(FL_FOREGROUND_COLOR);
	fl_line_style(FL_SOLID, 1);
	// Get the font and size for the tick labels.
	fl_font(labelfont(), labelsize() - 2);
	// Draw the ticks and labels based on the orientation.
	for (const auto& tick : ticks_) {
		// Get the size of the tick label.
		int tw = 0, th = 0;
		fl_measure(tick.label.c_str(), tw, th);
		// Draw the tick and label based on the orientation.
		switch (orientation_) {
		case zc_graph_axis::X_AXIS:
			// Draw the tick extending down from the axis line.
			fl_line(tick.position, y(), tick.position, y() + 5);
			// Draw the tick label centered below the tick.
			fl_draw(tick.label.c_str(), tick.position - tw / 2, y() + 5 + th);
			break;
		case zc_graph_axis::YL_AXIS:
			// Draw the tick extending left from the axis line.
			fl_line(x() + w(), tick.position, x() + w() - 5, tick.position);
			// Draw the tick label centered to the right of the tick.
			fl_draw(tick.label.c_str(), x() + w() - 5 - tw, tick.position + th / 2);
			break;
		case zc_graph_axis::YR_AXIS:
			// Draw the tick extending right from the axis line.
			fl_line(x(), tick.position, x() + 5, tick.position);
			// Draw the tick label centered to the left of the tick.
			fl_draw(tick.label.c_str(), x() + 5, tick.position + th / 2);
			break;
		}
	}
}

//! \brief Draw the axis label.
void zc_graph_axis::draw_label() {
	//Centre the label on the axis and draw it.
	// Set the color and font for the label.
	fl_color(FL_FOREGROUND_COLOR);
	fl_font(labelfont(), labelsize());
	// Get the size of the label.
	int tw = 0, th = 0;
	fl_measure(label_.c_str(), tw, th);
	// Draw the label based on the orientation.
	int lx = 0, ly = 0, lw = 0, lh = 0, ltx = 0, lty = 0, angle = 0;
	switch (orientation_) {
	case zc_graph_axis::X_AXIS:
		// Draw centered below the axis line.
		angle = 0;
		lx = x() + w() / 2 - tw / 2;
		ly = y() + h() / 4;
		lw = tw;
		lh = th;
		ltx = lx;
		lty = ly + th;
		break;
	case zc_graph_axis::YL_AXIS:
		// Draw centered to the left of the axis line.
		angle = 90;
		lx = x() + w() / 4;
		ly = y() + h() / 2 - tw / 2;
		lw = th;
		lh = tw;
		ltx = lx + th;
		lty = ly + lh;
		break;
	case zc_graph_axis::YR_AXIS:
		// Draw centered to the right of the axis line.
		angle = 90;
		lx = x() + w() / 4;
		ly = y() + h() / 2 - tw / 2;
		lw = th;
		lh = tw;
		ltx = lx + th;
		lty = ly + lh;
		break;
	}
	fl_rectf(lx - 1, ly - 1, lw + 2, lh + 2, FL_WHITE);
	fl_color(FL_FOREGROUND_COLOR);
	fl_draw(angle, label_.c_str(), ltx, lty);
}

//! \brief Calculate the tick positions and labels based on the current range and modifier.
void zc_graph_axis::set_ticks() {
	// Clear the existing ticks.
	ticks_.clear();
	// Calculate the tick spacing in data units based on the desired pixel spacing and current scale.
	float tick_spacing_units = abs(tick_spacing_pixels_ * scale_);
	float tick_mantissa;
	float tick_power10;
	float grid_spacing_units = tick_spacing_units;
	std::string format = "%0.0f";
	uint32_t si_prefix_exponent = ' ';
	normalise(tick_spacing_units, tick_mantissa, tick_power10, si_prefix_exponent);
	// Calculate the actual tick spacing in data units.
	switch (modifier_) {
	case NO_MODIFIER:
		if (tick_spacing_units >= 0.7F) format = "%0.0f";
		else if (tick_spacing_units >= 0.07F) format = "%0.1F";
		else if (tick_spacing_units >= 0.007F) format = "%0.2F";
		else format = "%g";
		// Fall through to the same tick spacing as POWER_OF_10 - but with different formatting.
		[[fallthrough]];
	case POWER_OF_10:
		// If normalised value > 7 - set tick at 10 * power10 
		if (tick_mantissa > 7.0F) {
			tick_spacing_units = 10.0F * tick_power10;
		}
		// If normalised value is between 3.2 and 7 - set tick at 5*10^N		
		else if (tick_mantissa > 3.2F) {
			tick_spacing_units = 5.0F * tick_power10;
		}
		// If normalised value is between 1.4 and 3.2 - set tick at 2*10^N
		else if (tick_mantissa > 1.4F) {
			tick_spacing_units = 2.0F * tick_power10;
		}
		// Otherwise set tick at 10^N
		else {
			tick_spacing_units = tick_power10;
		}
		break;
	case SI_PREFIX:
		// If normalised value > 70 - set tick at 100 * power10
		if (tick_mantissa > 70.0F) {
			tick_spacing_units = 100.0F * tick_power10;
		}
		// If normalised value is between 32 and 70 - set tick at 50*10^N		
		else if (tick_mantissa > 32.0F) {
			tick_spacing_units = 50.0F * tick_power10;
		}
		// If normalised value is between 14 and 32 - set tick at 20*10^N
		else if (tick_mantissa > 14.0F) {
			tick_spacing_units = 20.0F * tick_power10;
		}
		// If normalised value is between 7 and 14 - set tick at 10*10^N
		else if (tick_mantissa > 7.0F) {
			tick_spacing_units = 10.0F * tick_power10;
		}
		// If normalised value is between 3.2 and 7 - set tick at 5*10^N		
		else if (tick_mantissa > 3.2F) {
			tick_spacing_units = 5.0F * tick_power10;
		}
		// If normalised value is between 1.4 and 3.2 - set tick at 2*10^N
		else if (tick_mantissa > 1.4F) {
			tick_spacing_units = 2.0F * tick_power10;
		}
		// If normalised value is between 0.7 and 1.4 - set tick at 10^N
		else if (tick_mantissa > 0.7F) {
			tick_spacing_units = tick_power10;
		}
		// If normalised value is between 0.32 and 0.7 - set tick at 0.5*10^N
		else if (tick_mantissa > 0.32F) {
			tick_spacing_units = 0.5F * tick_power10;
			format = "%0.1f";
		}
		// If normalised value is between 0.14 and 0.32 - set tick at 0.2*10^N
		else if (tick_mantissa > 0.14F) {
			tick_spacing_units = 0.2F * tick_power10;
			format = "%0.1f";
		}
		// Otherwise set tick at 0.1*10^N
		else {
			tick_spacing_units = 0.1F * tick_power10;
			format = "%0.1f";
		}
		break;
	}
	// Always have the grid every two ticks. Every 5 units with ticks every 2 looked daft.
	grid_spacing_units = 2.0 * tick_spacing_units;
	// Calculate the tick positions and labels based on the current range and tick spacing.
	ticks_.clear();
	// Start at the first tick position less than or equal to the minimum of the current range.
	float tick_value = floorf(current_range_.min / tick_spacing_units) * tick_spacing_units;
	while (tick_value <= current_range_.max) {
		// Ignore tick_value ledd than the minimum.
		if (tick_value < current_range_.min) {
			tick_value += tick_spacing_units;
			continue;
		}
		// Format the tick label based on the modifier.
		char label[20];
		switch (modifier_) {
		case NO_MODIFIER:
			snprintf(label, sizeof(label), format.c_str(), tick_value);
			break;
		case POWER_OF_10:
			snprintf(label, sizeof(label), format.c_str(), tick_value / tick_power10);
			break;
		case SI_PREFIX:
			snprintf(label, sizeof(label), format.c_str(), tick_value / tick_power10);
			break;
		}
		// Calculate the pixel position of the tick and add it to the list of ticks.
		int tick_position = float_to_pixel(tick_value);
		ticks_.push_back({ tick_position, std::string(label) });
		tick_value += tick_spacing_units;
	}
	// Calculate the grid line positions based on the grid spacing.
	grid_lines_.clear();
	// Start at the first grid line position less than or equal to the minimum of the current range.
	float grid_value = floorf(current_range_.min / grid_spacing_units) * grid_spacing_units;
	while (grid_value <= current_range_.max)
	{
		grid_lines_.push_back(grid_value);
		grid_value += grid_spacing_units;
	}
	// Add the SI prefix or power of 10 to the axis label if applicable.
	char lu[100];
	switch (modifier_) {
	case NO_MODIFIER:
		snprintf(lu, sizeof(lu), "(%s)", unit_.c_str());
		break;
	case POWER_OF_10:
		snprintf(lu, sizeof(lu), "(\303\227%g %s)", tick_power10, unit_.c_str());
		break;
	case SI_PREFIX: {
		// Convert Unicode code point to UTF-8
		char prefix_utf8[5] = { 0 };  // UTF-8 can be up to 4 bytes + null terminator
		fl_utf8encode(si_prefix_exponent, prefix_utf8);
		snprintf(lu, sizeof(lu), "(%s%s)", prefix_utf8, unit_.c_str());
		break;
	}
	}
	char ll[100];
	if (unit_.length() > 0) {
		snprintf(ll, sizeof(ll), "%s %s", label(), lu);
	} else {
		snprintf(ll, sizeof(ll), "%s", label());
	}
	label_ = ll;
}

//! \brief Normalise the value \p fin to a mantissa and power of 10.
void zc_graph_axis::normalise(float fin, float& mantissa, float& power10, uint32_t& si_prefix_exponent) const {
	power10 = 1.0F;
	mantissa = fabs(fin);
	float step = 10.0F;
	float inv_step = 0.01F;
	float upper = 10.0F;
	float lower = 1.0F;
	int pow_10 = 0;
	int pow_step = 1;
	switch (modifier_) {
	case SI_PREFIX:
		step = 1000.0F;
		pow_step = 3;
		inv_step = 0.001F;
		upper = 100.0F;
		lower = 0.1F;
		break;
	case POWER_OF_10:
	case NO_MODIFIER:
		step = 10.0F;
		pow_step = 1;
		inv_step = 0.1F;
		upper = 10.0F;
		lower = 1.0F;
		break;
	}
	if (mantissa == 0.0F) return;
	// The input value is > 10 reduce it until it's less than 1
	while (mantissa > upper) {
		mantissa *= inv_step;
		power10 *= step;
		pow_10 += pow_step;
	}
	// If it was less than 0.1 increase it until it's greater than 0.1
	while (mantissa < lower) {
		mantissa *= step;
		power10 *= inv_step;
		pow_10 -= pow_step;
	}
	if (fin < 0) mantissa = -mantissa;
	if (modifier_ == SI_PREFIX) si_prefix_exponent = SI_PREFIXES.at(pow_10);

}


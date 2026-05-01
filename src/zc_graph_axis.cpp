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

#include "zc_fltk.h"
#include "zc_graph_base.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Widget.H>

#include <algorithm>
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
	orientation_(XB_AXIS),
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
	if (!is_valid_orientation(params.orientation)) {
		// Invalid orientation for this axis type - do not update.
		throw std::invalid_argument("Invalid orientation for this axis type");
		return;
	}
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
	scale_ = (current_range_.max - current_range_.min) / get_axis_length_pixels();
	inv_scale_ = 1.0F / scale_;
	zoom_limit_range_.set_union(new_range);
	origin_ = get_axis_min_pixel() - current_range_.min * inv_scale_;
	set_ticks();
	set_grid_lines();
}

//! \brief Zoom by a factor of \p zoom_factor around the value at \p mouse_pos.
//! \param mouse_pos The pixel position of the mouse along the axis.	
//! \param zoom_factor The factor to zoom by: +10 indictaes x2 zoom, -10 indicates x0.5 zoom.
void zc_graph_axis::zoom(int mouse_pos, int zoom_factor) {
	// Calculate the value at the mouse position before the zoom change.
	// Check zoom capability to determine the mouse value for zooming - either 0 for zoom on origin, or the value at the mouse position for zoom on cursor.
	float mouse_value;
	switch (get_zoom_capability()) {
	case ZOOM_ON_ORIGIN:
		mouse_value = 0.0F;
		break;
	case ZOOM_ON_CURSOR:
		mouse_value = pixel_to_float(mouse_pos);
		break;
	default:
		// No zoom capability - do not update.
		return;
	}
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
	// Ignore scroll if the axis does not support scrolling.
	if (!can_scroll()) {
		return;
	}
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


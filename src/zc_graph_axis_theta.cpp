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
#include "zc_graph_axis_theta.h"

#include "zc_graph_axis.h"

#include "zc_utils.h"

#include <FL/fl_draw.H>

//! \brief Constructor
zc_graph_axis_theta::zc_graph_axis_theta(int X, int Y, int W, int H, const char* L) :
	zc_graph_axis(X, Y, W, H, L) {
}

//! \brief Destructor
zc_graph_axis_theta::~zc_graph_axis_theta() {
}

//! \brief Set range to -180 to +180 degrees.
void zc_graph_axis_theta::set_range(range new_range) {
	// Ignore setting the range to anything other than +/- 180 degrees.
	current_range_ = range{ -180.0F, 180.0F };
	// dimensions of the annular axis area
	int radius = w() / 2.0F - annular_width_;
	int circumference = 2.0F * zc::PI * radius;
	// Scale 360 degrees to the circumference to get the tick spacing in pixels.
	scale_ = circumference / 360.0F;
	inv_scale_ = 1.0F / scale_;
	// Zooming is not allowed on the theta axis, so the zoom limit range is just the current range.
	zoom_limit_range_ = current_range_;
	// Origin is not used for the theta axis.
	set_ticks();
	set_grid_lines();
}

//!\brief Set the tick positions and labels based on the current range and tick spacing.
//! For the theta axis, we want to set ticks at regular angular intervals
//! set by a suitable submultiple of 360 degrees, and label them in degrees.
void zc_graph_axis_theta::set_ticks() {
	// Clear the existing ticks.
	ticks_.clear();
	// Calculate the tick spacing in degrees based on the desired pixel spacing and current scale.
	tick_spacing_ = tick_spacing_pixels_ * scale_;
	// Choose a suitable tick spacing in degrees based on the desired pixel spacing and current scale.
	if (tick_spacing_ >= 60.0F) tick_spacing_ = 90.0F;
	else if (tick_spacing_ >= 32.0F) tick_spacing_ = 45.0F;
	else if (tick_spacing_ >= 25.0F) tick_spacing_ = 30.0F;
	else if (tick_spacing_ >= 15.0F) tick_spacing_ = 20.0F;
	else if (tick_spacing_ >= 7.0F) tick_spacing_ = 10.0F;
	else if (tick_spacing_ >= 3.5F) tick_spacing_ = 5.0F;
	else if (tick_spacing_ >= 1.5F) tick_spacing_ = 2.0F;
	else tick_spacing_ = 1.0F;

	// Now populate the ticks vector with the tick positions and labels.
	float tick_value = -180.0F;
	char text[16];
	ticks_.push_back({ tick_value, "\xC2\xB1" "180\xC2\xB0" });
	while(tick_value < 180.0F) {
		tick_value += tick_spacing_;
		snprintf(text, sizeof(text), "%.0f\xc2\xB0", tick_value);
		ticks_.push_back({ tick_value, std::string(text) });
	}
}

//! \brief Generate the grid lines for the axis based on the current tick positions.
void zc_graph_axis_theta::set_grid_lines() {
	grid_values_.clear();
	for (const auto& tick : ticks_) {
		grid_values_.push_back(tick.position);
	}
}

//!\brief Draw the line for the axis.
//! This is doing more than just drawing the line. It will be
//! infilling the background of the annular axis area and the 
//! graph area going out the way from the axis line.
void zc_graph_axis_theta::draw_axis_line() {
	// The background to be filled is a complex shape
	// made of the whole widget rectangle with a circular hole
	// in the middle for the plot area.
	// We are using pixels so create a null transformation.
	fl_push_matrix();
	fl_color(FL_WHITE);
	fl_begin_complex_polygon();
	// Outer rectangle for the whole widget area.
	double dx0 = static_cast<double>(x());
	double dy0 = static_cast<double>(y());
	double dx1 = static_cast<double>(x() + w());
	double dy1 = static_cast<double>(y() + h());
	fl_vertex(dx0, dy0);
	fl_vertex(dx0, dy1);
	fl_vertex(dx1, dy1);
	fl_vertex(dx1, dy0);
	fl_gap();
	// Inner circle for the plot area.
	double cx = x() + w() / 2.0;
	double cy = y() + h() / 2.0;
	double radius = w() / 2.0 - annular_width_;
	// Draw the circle clockwise to generate a hole.
	fl_arc(cx, cy, radius, 360.0, 0.0);
	fl_end_complex_polygon();
	// Draw the axis line as a circle at the inner edge of the annular axis.
	fl_color(FL_FOREGROUND_COLOR);
	fl_arc(cx, cy, radius + 1.0, 0.0, 360.0);
}

// !\brief Draw the ticks for the axis.
void zc_graph_axis_theta::draw_ticks() {
	// Draw the ticks as small lines pointing outwards from the axis line.
	fl_color(FL_FOREGROUND_COLOR);
	double cx = x() + w() / 2.0;
	double cy = y() + h() / 2.0;
	double radius = w() / 2.0 - annular_width_;
	for (const auto& tick : ticks_) {
		double angle_rad = tick.position * zc::PI / 180.0;
		double sin_a = sin(angle_rad);
		double cos_a = cos(angle_rad);
		int x1 = cx + radius * cos_a;
		int y1 = cy + radius * sin_a;
		int x2 = cx + (radius + 5) * cos_a;
		int y2 = cy + (radius + 5) * sin_a;
		fl_line(x1, y1, x2, y2);
		// For now just draw the text spinning out from the label.
		// TODO - look at orientation and position of the text.
		fl_draw(tick.position, tick.label.c_str(), x2, y2);
	}
}

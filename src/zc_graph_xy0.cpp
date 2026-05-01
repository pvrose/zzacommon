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
#include "zc_graph_xy0.h"

#include "zc_drawing.h"
#include "zc_graph_axis_linx.h"
#include "zc_graph_axis_liny.h"
#include "zc_graph_base.h"
#include "zc_graph_plot.h"
#include "zc_graph_xy.h"
#include "zc_line_style.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <set>
#include <stdexcept>
#include <vector>

//! \brief Constructor
zc_graph_xy0::zc_graph_xy0(int X, int Y, int W, int H, const char* L) :
	zc_graph_xy(X, Y, W, H, L) {
	default_x_axis = zc_graph_axis::X0_AXIS;
	default_y_axis = zc_graph_axis::Y0_AXIS;
}


//! \brief Destructor
zc_graph_xy0::~zc_graph_xy0() {
}

void zc_graph_xy0::define_data_types() {
	// This graph displays X and Y data.

	// The valid combination is x vs y.
	data_type_combos_ = { { X_VALUE, Y_VALUE } };
	// X data maps to X0 axis, Y data maps to Y0 axis.
	// Note the orientation of the axes is dynamically determined by the data
	// For the X-axis if Y data is all negative, the X-axis is at the top of the plot area.
	// If Y data is all positive, the X-axis is at the bottom of the plot area.
	// Otherwise the X-axis is at the zero position in the plot area.
	// Similarly, the Y-axis.
	data_type_to_axis_ = {
		{ X_VALUE, zc_graph_axis::X0_AXIS },
		{ Y_VALUE, zc_graph_axis::Y0_AXIS }
	};
	// We need two axes - X and Y at the zero position.
	axes_[zc_graph_axis::X0_AXIS] = nullptr;
	axes_[zc_graph_axis::Y0_AXIS] = nullptr;
}

void zc_graph_xy0::create_components() {
	resizable(nullptr);
	// Get the height of the font to use for labels and ticks.
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int dw = 0, dh = 0;
	fl_measure("dummy", dw, dh);
	// Add label height and tick label height to axis width.
	axis_width_ = 2 * dh;
	// Add components - the positions will be dynamically updated in draw().
	int cx = x();
	int cy = y();
	int cw = w();
	int ch = h();
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);

	// Add Y axis in the middle.
	cx = x() + w() / 2 - axis_width_ / 2;
	axes_.at(zc_graph_axis::Y0_AXIS) = new zc_graph_axis_liny(cx, cy, axis_width_, ch, "Y");
	add(axes_.at(zc_graph_axis::Y0_AXIS));
	// Add X axis in the middle.
	cx = x();
	cy = y() + h() / 2 - axis_width_ / 2;
	axes_.at(zc_graph_axis::X0_AXIS) = new zc_graph_axis_linx(cx, cy, cw, axis_width_, "X");
	add(axes_.at(zc_graph_axis::X0_AXIS));
	end();
}

// reposition the axes (and if necessary the plot area ) based on the data origins.
void zc_graph_xy0::position_axes() {
	// Check whether the axes need to be repositioned based on the data origins.
	int x_origin = axes_.at(zc_graph_axis::X0_AXIS)->get_origin();
	int y_origin = axes_.at(zc_graph_axis::Y0_AXIS)->get_origin();
	// Set the positions of the axes and plot area based on the origins.
	int xx, xy, xw, xh;
	int yx, yy, yw, yh;
	int px, py, pw, ph;
	// If the X-origin is within the plot area, the plot is full width and the
	// Y-axis is at the X-origin.
	if (x_origin >= x() && x_origin <= x() + w()) {
		px = x();
		pw = w();
		yx = x_origin - axis_width_;
		yw = axis_width_;
		xx = x();
		xw = w();
		axes_.at(zc_graph_axis::Y0_AXIS)->set_orientation(zc_graph_axis::Y0_AXIS);
	}
	else if (x_origin < x()) {
		// If the X-origin is to the left of the plot area, the plot is
		// shifted to the right and the Y-axis is at the left edge of the plot area.
		px = x() + axis_width_;
		pw = w() - axis_width_;
		yx = x();
		yw = axis_width_;
		axes_.at(zc_graph_axis::Y0_AXIS)->set_orientation(zc_graph_axis::YL_AXIS);
		xx = x() + axis_width_;
		xw = w() - axis_width_;
	} else {
		// If the X-origin is to the right of the plot area, the plot is
		// shifted to the left and the Y-axis is at the right edge of the plot area.
		px = x();
		pw = w() - axis_width_;
		yx = x() + w() - axis_width_;
		yw = axis_width_;
		axes_.at(zc_graph_axis::Y0_AXIS)->set_orientation(zc_graph_axis::YR_AXIS);
		xx = x();
		xw = w() - axis_width_;
	}
	// Adjust X-axis position based on Y-origin.
	if (y_origin >= y() && y_origin <= y() + h()) {
		py = y();
		ph = h();
		xy = y_origin;
		xh = axis_width_;
		axes_.at(zc_graph_axis::X0_AXIS)->set_orientation(zc_graph_axis::X0_AXIS);
		yy = y();
		yh = h();
	}
	else if (y_origin < y()) {
		py = y() + axis_width_;
		ph = h() - axis_width_;
		xy = y();
		xh = axis_width_;
		axes_.at(zc_graph_axis::X0_AXIS)->set_orientation(zc_graph_axis::XT_AXIS);
		yy = y() + axis_width_;
		yh = h() - axis_width_;
	} else {
		py = y();
		ph = h();
		xy = y() + h();
		xh = axis_width_;
		axes_.at(zc_graph_axis::X0_AXIS)->set_orientation(zc_graph_axis::XB_AXIS);
		yy = y();
		yh = h();
	}
	// Set the positions of the axes and plot area.
	axes_.at(zc_graph_axis::X0_AXIS)->resize(xx, xy, xw, xh);
	axes_.at(zc_graph_axis::Y0_AXIS)->resize(yx, yy, yw, yh);
	plot_->resize(px, py, pw, ph);
}

//! \brief Override the data to points conversion to add the axis modifications for the X0 and Y0 axes.
void zc_graph_xy0::convert_data_to_points(data_set_t* ds) {
	// Convert the data to points using the base class method.
	zc_graph_xy::convert_data_to_points(ds);
	// Reposition the axes based on the data origins.
	position_axes();
	// Request a redraw to update the display with the new axis positions.
	redraw();
}
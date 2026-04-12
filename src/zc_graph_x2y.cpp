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
#include "zc_graph_x2y.h"

#include "zc_drawing.h"
#include "zc_graph_axis.h"
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
zc_graph_x2y::zc_graph_x2y(int X, int Y, int W, int H, const char* L) :
	zc_graph_xy(X, Y, W, H, L) {
}


//! \brief Destructor
zc_graph_x2y::~zc_graph_x2y() {
}

void zc_graph_x2y::define_data_types() {
	// This graph displays X and Y(X) data.

	// The valid combination is x vs y.
	data_type_combos_ = { { X_VALUE, Y_VALUE }, { X_VALUE, Y2_VALUE } };
	// X data maps to X axis, Y data maps to left Y axis.
	data_type_to_axis_ = {
		{ X_VALUE, zc_graph_axis::X_AXIS },
		{ Y_VALUE, zc_graph_axis::YL_AXIS },
		{ Y2_VALUE, zc_graph_axis::YR_AXIS  }
	};
	// We need three axes - X, Y left, and Y right.
	axes_[zc_graph_axis::X_AXIS] = nullptr;
	axes_[zc_graph_axis::YL_AXIS] = nullptr;
	axes_[zc_graph_axis::YR_AXIS] = nullptr;
}

void zc_graph_x2y::create_components() {
	resizable(nullptr);
	// Get the height of the font to use for labels and ticks.
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int dw = 0, dh = 0;
	fl_measure("dummy", dw, dh);
	// Add label height and tick label height to axis width.
	int axis_width = 2 * dh;
	// Add components.
	int cx = x();
	int cy = y() + GAP;
	int cw = w();
	int ch = h() - axis_width - GAP;
	// Add Y axis on the left.
	axes_.at(zc_graph_axis::YL_AXIS) = new zc_graph_axis(cx, cy, axis_width, ch, "Y");
	add(axes_.at(zc_graph_axis::YL_AXIS));
	// Add the plot area
	cx += axis_width;
	cw -= axis_width;
	cw -= axis_width;
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);
	// Add Y axis on the right.
	cx += cw;
	axes_.at(zc_graph_axis::YR_AXIS) = new zc_graph_axis(cx, cy, axis_width, ch, "Y2");	
	add(axes_.at(zc_graph_axis::YR_AXIS));
	// Add X axis on the bottom.
	cx = plot_->x();
	cw = plot_->w();
	cy += ch;
	axes_.at(zc_graph_axis::X_AXIS) = new zc_graph_axis(cx, cy, cw, axis_width, "X");
	add(axes_.at(zc_graph_axis::X_AXIS));
	end();
	resizable(plot_);
}

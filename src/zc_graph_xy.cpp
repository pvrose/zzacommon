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
#include "zc_graph_xy.h"

#include "zc_graph_axis.h"
#include "zc_graph_base.h"
#include "zc_graph_plot.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <stdexcept>
#include <vector>

//! \brief Constructor
zc_graph_xy::zc_graph_xy(int X, int Y, int W, int H, const char* L) :
	zc_graph_base(X, Y, W, H, L) {
	define_data_types();
	create_components();
	show();
}

//! \brief Destructor
zc_graph_xy::~zc_graph_xy() {
}

void zc_graph_xy::define_data_types() {
	// This graph displays X and Y(X) data.

	// The valid combination is x vs y.
	data_type_combos_ = { { X_VALUE, Y_VALUE } };
	// X data maps to X axis, Y data maps to left Y axis.
	data_type_to_axis_ = {
		{ X_VALUE, zc_graph_axis::X_AXIS },
		{ Y_VALUE, zc_graph_axis::YL_AXIS }
	};
	// We need two axes - X and Y left.
	axes_[zc_graph_axis::X_AXIS] = nullptr;
	axes_[zc_graph_axis::YL_AXIS] = nullptr;
}

void zc_graph_xy::create_components() {
	// Calculate the "width" of each axis based on tick length and label size.
	int axis_width = 5;
	// Get the height of the font to use for labels and ticks.
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int dw = 0, dh = 0;
	fl_measure("dummy", dw, dh);
	// Add label height and tick label height to axis width.
	axis_width += dh + dh;
	// Add components.
	int cx = x();
	int cy = y();
	int cw = w();
	int ch = h() - axis_width;
	// Add Y axis on the left.
	axes_.at(zc_graph_axis::YL_AXIS) = new zc_graph_axis(cx, cy, axis_width, ch, "Y");
	add(axes_.at(zc_graph_axis::YL_AXIS));
	// Add the plot area
	cx += axis_width;
	cw -= axis_width;
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);
	// Add X axis on the bottom.
	cy += ch;
	axes_.at(zc_graph_axis::X_AXIS) = new zc_graph_axis(cx, cy, cw, axis_width, "X");
	add(axes_.at(zc_graph_axis::X_AXIS));
	end();
}

// Convert data to points for plotting.
void zc_graph_xy::convert_data_to_points(data_set_t* ds) {
	// Convert the data in the data set to points for plotting.
	zc_graph_plot::plot_data_t* pd = new zc_graph_plot::plot_data_t;
	// Check that the data set exists
	if (!ds || !ds->data) {
		throw std::invalid_argument("Data set is null");
		return;
	}
	// This dataset should be X vs Y, so type_a is X and type_b is Y. Throw if not.
	if (ds->type_a != X_VALUE || ds->type_b != Y_VALUE) {
		throw std::invalid_argument("Invalid data type combination for zc_graph_xy - expected X_VALUE vs Y_VALUE");
		return;
	}
	// Get the axes
	auto& x_axis = axes_.at(zc_graph_axis::X_AXIS);
	auto& y_axis = axes_.at(zc_graph_axis::YL_AXIS);
	// Convert each data point to a plot point.
	for (const auto& datum : *ds->data) {
		zc_graph_plot::plot_point_t p;
		p.x = x_axis->float_to_pixel(datum.a);
		p.y = y_axis->float_to_pixel(datum.b);
		(pd->points).push_back(p);
	}
	// Set the line style for the plot data.
	pd->style = ds->style;
	// Set the plot type to CONNECTED_POINTS for line graph.
	pd->type = zc_graph_plot::CONNECTED_POINTS;
	// Add the plot data to be sent to the plot for drawing.
	plot_points_.push_back(pd);
}

// Generate grid lines for the graph.
void zc_graph_xy::generate_grid() {
	zc_graph_plot::plot_data_t* pd = new zc_graph_plot::plot_data_t;
	// Generate grid lines for the graph based on the tick spacing of the axes.
	// Get X and Y axes.
	auto x_axis = axes_.at(zc_graph_axis::X_AXIS);
	auto y_axis = axes_.at(zc_graph_axis::YL_AXIS);
	// Get the grid line values for the X axis.
	std::vector<float> x_grid_values = x_axis->get_grid_lines();
	// Add vertical grid lines for each X grid value.
	for (const auto& x_val : x_grid_values) {
		int lx = x_axis->float_to_pixel(x_val);
		zc_graph_plot::plot_line_t l;
		l.x1 = lx;
		l.y1 = plot_->y();
		l.x2 = lx;
		l.y2 = plot_->y() + plot_->h();
		pd->lines.push_back(l);
	}
	// Get the grid line values for the Y axis.
	std::vector<float> y_grid_values = y_axis->get_grid_lines();
	// Add horizontal grid lines for each Y grid value.
	for (const auto& y_val : y_grid_values) {
		int ly = y_axis->float_to_pixel(y_val);
		zc_graph_plot::plot_line_t l;
		l.x1 = plot_->x();
		l.y1 = ly;
		l.x2 = plot_->x() + plot_->w();
		l.y2 = ly;
		pd->lines.push_back(l);
	}
	// Set the line style for the grid lines.
	pd->style = { FL_LIGHT2, 1, FL_DOT };
	// Set the plot type to LINES for grid lines.
	pd->type = zc_graph_plot::LINES;
	// Add the plot data to be sent to the plot for drawing.
	plot_points_.push_back(pd);
}
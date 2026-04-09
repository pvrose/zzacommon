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

#include "zc_graph_axis.h"
#include "zc_graph_base.h"
#include "zc_graph_plot.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <stdexcept>
#include <vector>

//! \brief Constructor
zc_graph_x2y::zc_graph_x2y(int X, int Y, int W, int H, const char* L) :
	zc_graph_base(X, Y, W, H, L) {
	define_data_types();
	create_components();
	show();
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
	cw -= axis_width - axis_width;
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);
	// Add Y axis on the right.
	cx += cw;
	axes_.at(zc_graph_axis::YR_AXIS) = new zc_graph_axis(cx, cy, axis_width, ch, "Y2");	
	add(axes_.at(zc_graph_axis::YR_AXIS));
	// Add X axis on the bottom.
	cy += ch;
	axes_.at(zc_graph_axis::X_AXIS) = new zc_graph_axis(cx, cy, cw, axis_width, "X");
	add(axes_.at(zc_graph_axis::X_AXIS));
	end();
}

// Convert data to points for plotting.
void zc_graph_x2y::convert_data_to_points(data_set_t* ds) {
	// Convert the data in the data set to points for plotting.
	zc_graph_plot::plot_data_t* pd = new zc_graph_plot::plot_data_t;
	// Check that the data set exists
	if (!ds || !ds->data) {
		throw std::invalid_argument("Data set is null");
		return;
	}
	// This dataset should be X vs Y, so type_a is X and type_b is Y. Throw if not.
	if (ds->type_a != X_VALUE || (ds->type_b != Y_VALUE && ds->type_b != Y2_VALUE)) {
		throw std::invalid_argument("Invalid data type combination for zc_graph_xy - expected X_VALUE vs Y_VALUE");
		return;
	}
	// Select the appropriate Y axis based on the data type of type_b.
	zc_graph_axis::orientation_t y_axis_type;
	if (ds->type_b == Y_VALUE) {
		y_axis_type = zc_graph_axis::YL_AXIS;
	} else {
		y_axis_type = zc_graph_axis::YR_AXIS;
	}
	// Get scaling factors for X and Y axes.
	float x_inv_scale = axes_.at(zc_graph_axis::X_AXIS)->get_inv_scale();
	float y_inv_scale = axes_.at(y_axis_type)->get_inv_scale();
	// Get origin for X and Y axes.
	int x_origin = axes_.at(zc_graph_axis::X_AXIS)->get_origin();
	int y_origin = axes_.at(y_axis_type)->get_origin();
	// Convert each data point to a plot point.
	for (const auto& datum : *ds->data) {
		zc_graph_plot::plot_point_t p;
		p.x = x_origin + static_cast<int>(datum.a * x_inv_scale);
		p.y = y_origin - static_cast<int>(datum.b * y_inv_scale);
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
void zc_graph_x2y::generate_grid() {
	zc_graph_plot::plot_data_t* pd = new zc_graph_plot::plot_data_t;
	// Generate grid lines for the graph based on the tick spacing of the axes.
	// Get X and Y axes.
	auto x_axis = axes_.at(zc_graph_axis::X_AXIS);
	auto yl_axis = axes_.at(zc_graph_axis::YL_AXIS);
	auto yr_axis = axes_.at(zc_graph_axis::YR_AXIS);
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

	// Get the grid line values for the Y left axis.
	std::vector<float> y_grid_values = yl_axis->get_grid_lines();
	// Add horizontal grid lines for each Y left grid value.
	for (const auto& y_val : y_grid_values) {
		int ly = yl_axis->float_to_pixel(y_val);
		zc_graph_plot::plot_line_t l;
		l.x1 = plot_->x();
		l.y1 = ly;
		l.x2 = plot_->x() + plot_->w();
		l.y2 = ly;
		pd->lines.push_back(l);
	}
	// Set the line style for the grid lines.
	pd->style = { FL_GRAY0, 1, FL_DOT };
	// Set the plot type to LINES for grid lines.
	pd->type = zc_graph_plot::LINES;
	// Add the plot data to be sent to the plot for drawing.
	plot_points_.push_back(pd);

	// Get the grid line values for the Y right axis.
	std::vector<float> y2_grid_values = yr_axis->get_grid_lines();
	// Add horizontal grid lines for each Y right grid value.
	for (const auto& y_val : y2_grid_values) {
		int ly = yr_axis->float_to_pixel(y_val);
		zc_graph_plot::plot_line_t l;
		l.x1 = plot_->x();
		l.y1 = ly;
		l.x2 = plot_->x() + plot_->w();
		l.y2 = ly;
		pd->lines.push_back(l);
	}
	// Set the line style for the grid lines.
	pd->style = { FL_GRAY0, 1, FL_DASH };
	// Set the plot type to LINES for grid lines.
	pd->type = zc_graph_plot::LINES;
	// Add the plot data to be sent to the plot for drawing.
	plot_points_.push_back(pd);
}
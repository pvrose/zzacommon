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

#include "zc_drawing.h"
#include "zc_graph_axis.h"
#include "zc_graph_base.h"
#include "zc_graph_plot.h"
#include "zc_line_style.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <stdexcept>
#include <vector>

//! \brief Constructor
zc_graph_xy::zc_graph_xy(int X, int Y, int W, int H, const char* L) :
	zc_graph_base(X, Y, W, H, L) {
}

void zc_graph_xy::create() {
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
	axis_width += dh;
	// Add components.
	int cx = x();
	int cy = y() +GAP;
	int cw = w();
	int ch = h() - axis_width - GAP;
	// Add Y axis on the left.
	axes_.at(zc_graph_axis::YL_AXIS) = new zc_graph_axis(cx, cy, axis_width, ch, "Y");
	add(axes_.at(zc_graph_axis::YL_AXIS));
	// Add the plot area
	cx += axis_width;
	cw -= axis_width;
	cw -= GAP;
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);
	// Add X axis on the bottom.
	cy += ch;
	axes_.at(zc_graph_axis::X_AXIS) = new zc_graph_axis(cx, cy, cw, axis_width, "X");
	add(axes_.at(zc_graph_axis::X_AXIS));
	end();
}

// Define the transformation schemata for the plot based on the axis ranges.
void zc_graph_xy::define_plot_xforms() {
	for (const auto& combo : data_type_combos_) {
		// Get the axes for this data type combination.
		auto axis_a = axes_.at(data_type_to_axis_.at(combo.first));
		auto axis_b = axes_.at(data_type_to_axis_.at(combo.second));
		// Get the range of the axes to pass to plot_.
		zc_graph_axis::range x_range = axis_a->get_range();
		zc_graph_axis::range y_range = axis_b->get_range();
		zc_graph_plot::plot_xform_t xform = {
			x_range.min, y_range.min, x_range.max, y_range.max
		};
		plot_->set_xform_schema(combo.second, xform);
	}
}


// Convert data to points for plotting.
void zc_graph_xy::convert_data_to_points(data_set_t* ds) {
	// Check that the data set exists
	if (!ds || !ds->data) {
		throw std::invalid_argument("Data set is null");
		return;
	}
	// Check that the data type combination is valid for this graph.
	bool valid_combo = false;
	for (const auto& combo : data_type_combos_) {
		if (ds->type_a == combo.first && ds->type_b == combo.second) {
			valid_combo = true;
			break;
		}
	}
	if (!valid_combo) {
		throw std::invalid_argument("Invalid data type combination for this graph");
		return;
	}

	// Select the appropriate Y axis based on the data type of type_b.
	zc_graph_axis::orientation_t y_axis_type = data_type_to_axis_.at(ds->type_b);
	zc_graph_axis::orientation_t x_axis_type = data_type_to_axis_.at(ds->type_a);
	// Get axes.
	auto& x_axis = axes_.at(x_axis_type);
	auto& y_axis = axes_.at(y_axis_type);

	// Create new plot line for this data set.
	zc_graph_plot::plot_line_t line;
	line.style = ds->style;
	line.transform = ds->type_b;

	// Convert each data point to a plot point.
	for (const auto& datum : *ds->data) {
		zc_graph_plot::plot_vertex_t v(datum.a, datum.b);
		zc_graph_plot::plot_segment_t segment(v);
		line.segments.push_back(segment);
	}
	// Add the line to the plot for this data type.
	plot_->add_line(ds->type_b, true, line);
}

// Generate grid lines for the graph.
void zc_graph_xy::generate_grid() {
	// Generate grid lines for the graph based on the tick spacing of the axes.
	// For each axis, get the grid line values and add lines to the plot data for drawing.
	for (auto& axis : axes_) {
		if (!axis.second) continue; // Skip if axis not defined
		auto grid_values = axis.second->get_grid_lines();
		for (const auto& value : grid_values) {
			zc_graph_plot::plot_line_t line;
			line.style = zc_line_style(FL_LIGHT2, 1, FL_DOT);
			if (axis.first == zc_graph_axis::X_AXIS) {
				// Default data type for X-axis to Y_VALUE for grid line transform.
				// and use the YL axis range for the grid line endpoints.
				line.transform = zc_graph_base::Y_VALUE;
				// Vertical grid line at x = value
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(value, axes_.at(zc_graph_axis::YL_AXIS)->get_range().min)
				));
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(value, axes_.at(zc_graph_axis::YL_AXIS)->get_range().max)
				));
			}
			else {
				// Get the data type for the grid line transform based on the axis orientation.
				bool found = false;
				for (const auto& dt_axis : data_type_to_axis_) {
					if (dt_axis.second == axis.first) {
						line.transform = dt_axis.first;
						found = true;
						break;
					}
				}
				if (!found) {
					throw std::runtime_error("No data type mapping found for axis in grid generation");
				}
				// Horizontal grid line at y = value
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(axes_.at(zc_graph_axis::X_AXIS)->get_range().min, value)
				));
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(axes_.at(zc_graph_axis::X_AXIS)->get_range().max, value)
				));
			}
			// Add the grid line to the plot data for this data type.
			plot_->add_line(line.transform, false, line);
		}
	}
}
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

#include <stdexcept>
#include <vector>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include "zc_drawing.h"
#include "zc_graph_axis_linxb.h"
#include "zc_graph_axis_linyl.h"
#include "zc_graph_base.h"
#include "zc_graph_plot.h"
#include "zc_line_style.h"

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
		{ X_VALUE, nullptr },
		{ Y_VALUE, nullptr}
	};
}

void zc_graph_xy::create_components() {
	resizable(nullptr);
	// Calculate the "width" of each axis based on tick length and label size.
	// Get the height of the font to use for labels and ticks.
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int dw = 0, dh = 0;
	fl_measure("dummy", dw, dh);
	// Add label height and tick label height to axis width.
	int axis_width = 2 * dh;
	// Add components.
	int cx = x();
	int cy = y() +GAP;
	int cw = w();
	int ch = h() - axis_width - GAP;
	// Add Y axis on the left.
	data_type_to_axis_[Y_VALUE] = new zc_graph_axis_linyl(cx, cy, axis_width, ch, "Y");
	add(data_type_to_axis_[Y_VALUE]);
	// Add the plot area
	cx += axis_width;
	cw -= axis_width;
	cw -= GAP;
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);
	// Add X axis on the bottom.
	cy += ch;
	data_type_to_axis_[X_VALUE] = new zc_graph_axis_linxb(cx, cy, cw, axis_width, "X");
	add(data_type_to_axis_[X_VALUE]);
	end();
	resizable(plot_);
}

// Define the transformation schemata for the plot based on the axis ranges.
void zc_graph_xy::define_plot_xforms() {
	for (const auto& combo : data_type_combos_) {
		// Get the axes for this data type combination.
		auto axis_a = data_type_to_axis_.at(combo.first);
		auto axis_b = data_type_to_axis_.at(combo.second);
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

	// Get axes.
	auto& x_axis = data_type_to_axis_.at(ds->type_a);
	auto& y_axis = data_type_to_axis_.at(ds->type_b);

	// Create new plot line for this data set.
	zc_graph_plot::plot_object_t line;
	line.style = ds->style;
	line.transform = ds->type_b;

	// Convert each data point to a plot point.
	for (const auto& datum : *ds->data) {
		zc_graph_plot::plot_vertex_t v(datum.a, datum.b);
		zc_graph_plot::plot_segment_t segment(v);
		line.segments.push_back(segment);
	}
	// Add the line to the plot for this data type.
	plot_->add_object(ds->type_b, true, line);
}

// Generate grid lines for the graph.
void zc_graph_xy::generate_grid() {
	// Generate grid lines for the graph based on the tick spacing of the axes.
	// For each axis, get the grid line values and add lines to the plot data for drawing.
	for (auto& axis : data_type_to_axis_) {
		if (!axis.second) continue; // Skip if axis not defined
		auto grid_values = axis.second->get_grid_lines();
		for (const auto& value : grid_values) {
			zc_graph_plot::plot_object_t line;
			// For line at 0 - axis should be drawn on top of the grid line.
			line.style = zc_line_style(FL_LIGHT2, 1, FL_DOT);
			if (axis.second->is_horizontal()) {
				// Default data type for X-axis to Y_VALUE for grid line transform.
				// and use the YL axis range for the grid line endpoints.
				auto yaxis = data_type_to_axis_.at(Y_VALUE);
				line.transform = zc_graph_base::Y_VALUE;
				// Vertical grid line at x = value
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(value, yaxis->get_range().min)
				));
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(value, yaxis->get_range().max)
				));
			}
			else {
				// Horizontal grid line at y = value
				auto xaxis = data_type_to_axis_.at(X_VALUE);
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().min, value)
				));
				line.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().max, value)
				));
			}
			// Add the grid line to the plot data for this data type.
			plot_->add_object(line.transform, false, line);
		}
	}
}

//! \brief Override of the base class method to add markers to the graph.
void zc_graph_xy::add_markers() {
	// For each marker.
	for (auto& marker : markers_) {
		zc_graph_plot::plot_object_t object;
		// Get the axis for this marker based on the data type.
		zc_graph_axis* axis = get_axis(marker.type);
		// Ignore if the values are outside the axis range.
		if (marker.value_1 < axis->get_range().min && marker.value_2 < axis->get_range().min) continue;
		if (marker.value_1 > axis->get_range().max && marker.value_2 > axis->get_range().max) continue;
		// If the axis is horizontal, the marker is vertical.
		if (axis->is_horizontal()) {
			// Use YL axis range for the marker endpoints.
			auto yaxis = data_type_to_axis_.at(Y_VALUE);
			object.transform = zc_graph_base::Y_VALUE;
			// Vertical line if value_1 == value_2
			if (marker.value_1 == marker.value_2) {
				// Draw a line at x = value_1 from the bottom to the top of the plot area.
				object.style = marker.style;
				object.shape = zc_graph_plot::LINE_STRIP;
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_1, yaxis->get_range().min)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_1, yaxis->get_range().max)
				));
			} else {
				// Draw a shaded area from x = value_1 to x = value_2 across the full height of the plot area.
				object.style = marker.style;
				object.shape = zc_graph_plot::POLYGON;
				// Add vertices for the corners of the shaded area.
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_1, yaxis->get_range().min)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_2, yaxis->get_range().min)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_2, yaxis->get_range().max)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_1, yaxis->get_range().max)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(marker.value_1, yaxis->get_range().min)
				));
			}
		} else {
			// Use provided axis range for the marker endpoints.
			auto xaxis = data_type_to_axis_.at(X_VALUE);
			object.transform = marker.type;
			// Horizontal line if value_1 == value_2
			if (marker.value_1 == marker.value_2) {
				// Draw a line at y = value_1 from the left to the right of the plot area.
				object.style = marker.style;
				object.shape = zc_graph_plot::LINE_STRIP;
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().min, marker.value_1)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().max, marker.value_1)
				));
			} else {
				// Draw a shaded area from y = value_1 to y = value_2 across the full width of the plot area.
				object.style = marker.style;
				object.shape = zc_graph_plot::POLYGON;
				// Add vertices for the corners of the shaded area.
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().min, marker.value_1)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().max, marker.value_1)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().max, marker.value_2)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().min, marker.value_2)
				));
				object.segments.push_back(zc_graph_plot::plot_segment_t(
					zc_graph_plot::plot_vertex_t(xaxis->get_range().min, marker.value_1)
				));
			}
		}
		// Add the marker object to the plot data for this marker's data type.
		plot_->add_object(object.transform, false, object);
	}
	for (auto& label : labels_) {
		zc_graph_plot::plot_object_t object;
		object.shape = zc_graph_plot::TEXT;
		object.text_style = label.style;
		object.transform = zc_graph_base::Y_VALUE; 
		object.text = label.text;
		// If either value is +/-FLT_MAX, align to the edge of the plot area.
		coord pos = label.position;
		auto xaxis = data_type_to_axis_.at(X_VALUE);
		auto yaxis = data_type_to_axis_.at(Y_VALUE);
		if (pos.a == -FLT_MAX) {
			pos.a = xaxis->get_range().min;
		} else if (pos.a == FLT_MAX) {
			pos.a = xaxis->get_range().max;
		}
		if (pos.b == -FLT_MAX) {
			pos.b = yaxis->get_range().min;
		} else if (pos.b == FLT_MAX) {
			pos.b = yaxis->get_range().max;
		}
		// Ignore if the label position is outside the axis ranges.
		if (pos.a < xaxis->get_range().min || pos.a > xaxis->get_range().max) continue;
		if (pos.b < yaxis->get_range().min || pos.b > yaxis->get_range().max) continue;
		object.segments.push_back(zc_graph_plot::plot_segment_t(
			zc_graph_plot::plot_vertex_t(pos.a, pos.b)
		));
		// Add the label object to the plot data for this label's data type.
		plot_->add_object(object.transform, false, object);
	}
}
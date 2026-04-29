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
#include "zc_graph_polar.h"

#include "zc_drawing.h"
#include "zc_graph_axis.h"
#include "zc_graph_base.h"
#include "zc_graph_plot.h"
#include "zc_line_style.h"
#include "zc_utils.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <cmath>
#include <stdexcept>
#include <vector>



// ! \brief Constructor
zc_graph_polar::zc_graph_polar(int X, int Y, int W, int H, const char* L) :
	zc_graph_base(X, Y, W, H, L) {
}

void zc_graph_polar::create() {
	define_data_types();
	create_components();
	show();
}

//! \brief Destructor
zc_graph_polar::~zc_graph_polar() {
}

void zc_graph_polar::define_data_types() {
	// This graph displays radius and angle data.
	// The valid combination is radius only.
	data_type_combos_ = { { RADIUS, NO_DATA } };
	// Radius maps onto X axis overlayed on to the plot. Theta does not map.
	data_type_to_axis_ = {
		{ RADIUS, zc_graph_axis::R_AXIS }
	};
	// We need one axis.
	axes_[zc_graph_axis::R_AXIS] = nullptr;
};

void zc_graph_polar::create_components() {
	resizable(nullptr);
	// Calculate the "width" of the axis based on twice the height of a label.
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int dw = 0, dh = 0;
	fl_measure("dummy", dw, dh);
	int axis_width = 2 * dh;
	// Add components.
	int cx = x();
	int cy = y();
	int cw = w();
	int ch = h();
	// Add plot area in the whole gtraph area.
	plot_ = new zc_graph_plot(cx, cy, cw, ch);
	add(plot_);

	// Add X axis overlayed on the plot area.
	cx = x() + w() / 2;
	cy = y() + h() / 2;
	cw = w() / 2;
	axes_.at(zc_graph_axis::R_AXIS) = new zc_graph_axis(cx, cy, cw, axis_width, "R");
	add(*axes_.at(zc_graph_axis::R_AXIS));
	end();
	resizable(plot_);
}

// Define the transformation from data to plot coordinates for the graph.
// Although data is in polar coordinates: cover_data_to_points will convert
// these to cartesian coordinates which will be scaled by the drawing
// transformation based on the X axis. So we define the transformation for the radius data.
void zc_graph_polar::define_plot_xforms() {
	auto axis_r = axes_.at(zc_graph_axis::R_AXIS);
	// Get the data range for the radius axis.
	auto range_r = axis_r->get_range();
	// Define the transformation for the radius data for X and scale it for Y.
	zc_graph_plot::plot_xform_t xform = {
		-range_r.max, -plot_->h() * range_r.max / plot_->w(),
		range_r.max, plot_->h() * range_r.max / plot_->w()
	};
	plot_->set_xform_schema(RADIUS, xform);
}

// Convert the data sets to points for plotting. For the polar graph, we convert
// the polar coordinates to cartesian coordinates for plotting.
void zc_graph_polar::convert_data_to_points(data_set_t* ds) {
	// Chack that the data set exists and has data.
	if (!ds || !ds->data) {
		throw std::invalid_argument("Data set is null");
		return;
	}
	// Check that the data types are valid for this graph.
	if (ds->type_a != RADIUS || ds->type_b != NO_DATA) {
		throw std::invalid_argument("Invalid data type combination for polar graph");
		return;
	}

	// Create a new plot line for this data set.
	zc_graph_plot::plot_object_t line;
	line.style = ds->style;
	line.transform = RADIUS; // Use the radius transformation for both X and Y.

	// Convert the polar coordinates to cartesian coordinates for plotting.
	for (const auto& coord : *ds->data) {
		double r = coord.a; // Radius
		double theta = coord.b; // Angle in radians
		double x = r * std::cos(theta);
		double y = r * std::sin(theta);
		zc_graph_plot::plot_vertex_t vertex(x, y);
		zc_graph_plot::plot_segment_t segment(vertex);
		line.segments.push_back(segment);
	}

	// Add the line to the plot.
	plot_->add_object(ds->type_a, true, line);
}

// Generate the grid lines for the graph. 
// For the polar graph, we generate concentric circles for the radius grid
// spaced according to the grid spacing for the radius axis.
// We generate radial lines for the angle grid spaced initially at 30 degree intervals,
// When the circumferential distance is big enough, we add additional radial
// lines at 10 degree intervals and then at 5 degree intervals.
void zc_graph_polar::generate_grid() {
	auto axis_r = axes_.at(zc_graph_axis::R_AXIS);
	auto range_r = axis_r->get_range();
	std::vector<float> grid_lines_r = axis_r->get_grid_lines();
	double spacing_r = axis_r->get_tick_spacing();
	// Generate concentric circles for the radius grid.
	// Mximum radius is at the corners of the plot area, which is the Pythagorean distance
	// from the center to the corner of the plot area.
	double max_radius = range_r.max *
		std::sqrt(plot_->w() * plot_->w() + plot_->h() * plot_->h()) / plot_->w();
	// While we draw the circles check the circumferential distance at the current radius
	// and remember the radius at which the circumferential distance is big enough
	// to add more radial lines.
	// Generate radial lines for the angle grid.
	std::array<int, 4> angle_spacings = { 30, 10, 5, 1 };
	int current_angle_spacing_index = 1;
	int angle_step = angle_spacings[angle_spacings.size() - 1];
	// Radii at which circumferential distance is big enough to add more radial lines.
	std::array<double, angle_spacings.size()> radius_thresholds;
	for (int i = 0; i < radius_thresholds.size(); i++) {
		if (i < grid_lines_r.size()) {
			radius_thresholds[i] = grid_lines_r[i];
		}
		else {
			radius_thresholds[i] = max_radius;
		}
	}
	for (auto& r : grid_lines_r) {
		zc_graph_plot::plot_arc_t arc = { 0, 0, r, 0, 360 };
		zc_graph_plot::plot_segment_t segment(arc);
		zc_graph_plot::plot_object_t line;
		line.segments.push_back(segment);
		line.shape = zc_graph_plot::LINE_STRIP;
		line.style = zc_line_style(FL_LIGHT2, 1, FL_DOT);
		line.transform = RADIUS; // Use the radius transformation for both X and Y.
		// Add the circle to the plot.
		plot_->add_object(RADIUS, false, line);

	}
	// Add the radial lines for the angle grid.
	for (int angle = 0; angle < 360; angle += angle_step) {
		// Work out which spacing group we are in for this angle.
		int spacing_group_index = 0;
		for (int i = 0; i < angle_spacings.size(); i++) {
			if (angle % angle_spacings[i] == 0) {
				spacing_group_index = i;
				break;
			}
		}
		// Now add the line for this angle.
		double theta = angle * zc::PI / 180.0; // Convert to radians for cos/sin
		double cos_theta = std::cos(theta);
		double sin_theta = std::sin(theta);
		double x1 = max_radius * cos_theta;
		double y1 = max_radius * sin_theta;
		double r0 = radius_thresholds[spacing_group_index];
		double x0 = r0 * cos_theta;
		double y0 = r0 * sin_theta;
		// Draw the line from (x0, y0) to (x1, y1).
		zc_graph_plot::plot_object_t line;
		line.style = zc_line_style(FL_LIGHT2, 1, FL_DOT);
		line.transform = RADIUS; // Use the radius transformation for both X and Y.
		zc_graph_plot::plot_vertex_t vertex0(x0, y0);
		line.segments.push_back(zc_graph_plot::plot_segment_t(vertex0));
		zc_graph_plot::plot_vertex_t vertex1(x1, y1);
		line.segments.push_back(zc_graph_plot::plot_segment_t(vertex1));
		plot_->add_object(RADIUS, false, line);
	}
}

// Add markers for the graph. For the polar graph.
void zc_graph_polar::add_markers() {
	// For each marker for the radius axis, add a marker at the corresponding radius.
	for (auto& marker : markers_) {
		// Only support markers on R_AXIS for the polar graph.
		if (marker.type != RADIUS) {
			continue;
		}
		zc_graph_axis* axis = axes_.at(zc_graph_axis::R_AXIS);
		// Ignore the marker if it is outside the radius range.
		auto range = axis->get_range();
		if (marker.value_1 < range.min || marker.value_1 > range.max) {
			continue;
		}
		if (marker.value_2 < range.min || marker.value_2 > range.max) {
			continue;
		}
		zc_graph_plot::plot_object_t object;
		object.transform = RADIUS; // Use the radius transformation for both X and Y.
		// If a single value, draw a circle at that radius.
		if (marker.value_1 == marker.value_2) {
			zc_graph_plot::plot_arc_t arc = { 0, 0, marker.value_1, 0, 360 };
			object.segments.push_back(zc_graph_plot::plot_segment_t(arc));
			object.shape = zc_graph_plot::LINE_STRIP;
			object.style = marker.style;
		}
		else {
			// If a range of values, draw a shaded area between those radii.
			double r0 = std::max(marker.value_1, marker.value_2);
			double r1 = std::min(marker.value_1, marker.value_2);
			zc_graph_plot::plot_arc_t arc1 = { 0, 0, r0, 0, 360 };
			object.segments.push_back(zc_graph_plot::plot_segment_t(arc1));
			// Add a gap to break the line strip into two segments.
			object.segments.push_back(zc_graph_plot::plot_segment_t(true));
			// Draw the inner circle backwards to create a closed loop for the shaded area.
			zc_graph_plot::plot_arc_t arc2 = { 0, 0, r1, 360, 0 };
			object.segments.push_back(zc_graph_plot::plot_segment_t(arc2));
			object.shape = zc_graph_plot::COMPLEX;
			object.style = marker.style;
		}
		plot_->add_object(RADIUS, false, object);
	}
	for (auto& label : labels_) {
		// Only support labels on R_AXIS for the polar graph.
		if (label.type != RADIUS) {
			continue;
		}
		zc_graph_axis* axis = axes_.at(zc_graph_axis::R_AXIS);
		// Ignore the label if it is outside the radius range.
		auto range = axis->get_range();
		if (label.position.a < range.min || label.position.a > range.max) {
			continue;
		}
		zc_graph_plot::plot_object_t object;
		object.transform = RADIUS; // Use the radius transformation for both X and Y.
		object.shape = zc_graph_plot::TEXT;
		object.text = label.text;
		object.text_style = label.style;
		object.segments.push_back(zc_graph_plot::plot_segment_t(
			zc_graph_plot::plot_vertex_t(label.position.a, 0)
		));
		plot_->add_object(RADIUS, false, object);
	}
}

		
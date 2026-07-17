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
#include "zc_graph_.h"

#include "zc_drawing.h"
#include "zc_line_style.h"
#include "zc_range.h"
#include "zc_text_style.h"
#include "zc_utils.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Widget.H>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

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

// Constrcutor
zc_graph_::zc_graph_(int x, int y, int w, int h, const char* label) :
	Fl_Widget(x, y, w, h, label)
{
	// Default text size is set to 80% of the default ZZA app font size.
	default_text_size_ = FL_NORMAL_SIZE;
}

// Destructor
zc_graph_::~zc_graph_()
{
	// Nothing to do here - the derived classes will clean up their own resources.
}

// Set the axis parameters for a data type. This will be called by the derived classes to set up the mapping
void zc_graph_::set_axis_params(
	int axis_number,
	modifier_t modifier,
	const std::string& unit,
	const std::string& label,
	int tick_spacing_pixels
) {
	// Check we are not trying to set parameters for an axis number that is out of range for the coordinate type of this graph.
	if (axis_number >= num_axes_) {
		throw std::out_of_range("Axis number " + std::to_string(axis_number) + " is out of range for this graph.");
		return;
	}
	// Get the existing axis data for this axis number, or create a new one if it doesn't exist
	axis_data_t& axis_data = axes_data_[axis_number];
	axis_data.modifier = modifier;
	axis_data.unit = unit;
	axis_data.label = label;
	axis_data.tick_spacing_pixels = tick_spacing_pixels;
	invalidate_layout();
}

// Set the ranges for the axes for a data type.
void zc_graph_::set_axis_ranges(
	int axis_number,                    //!< Axis number to set the ranges for (e.g. 0 for X or R axis, 1 for Y or Theta axis)
	const zc_range<double>& inner_range,              //!< Range of data values currently displayed for this axis (may be zoomed or scrolled)
	const zc_range<double>& outer_range,              //!< Range of data values for this axis (absolute minimum and maximum for zooming)
	const zc_range<double>& default_range             //!< Default range for this axis in the absence of data
) {
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before setting axis ranges.");
		return;
	}
	// Check that the ranges are valid - ignore if inner_range is empty (i.e. min > max)
	// as this is used to there is no inner range.
	if (inner_range.is_valid()) {
		if ((default_range.is_valid() && !default_range.contains(inner_range)) ||
			!outer_range.contains(inner_range)) {
			// Inner range is outside the default or outer range, throw an error
			throw std::invalid_argument("Inner range must be within the default and outer ranges for axis number " + std::to_string(axis_number) + ".");
			return;
		}
	}
	if (!outer_range.contains(default_range)) {
		// Default range is outside the outer range, throw an error
		throw std::invalid_argument("Default range must be within the outer range for axis number " + std::to_string(axis_number) + ".");
		return;
	}
	// Set the ranges for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	axis_data.inner_range = inner_range;
	axis_data.outer_range = outer_range;
	axis_data.default_range = default_range;
	invalidate_layout();
}

// Get axis current range for a specific axis number.
zc_range<double> zc_graph_::get_axis_range(int axis_number) const {
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before getting axis range.");
		return zc_range<double>();
	}
	const axis_data_t& axis_data = axes_data_[axis_number];
	return axis_data.current_range;
}

// Set the axis bar chart parameters.
void zc_graph_::set_bar_labels(
	int axis_number,
	const std::vector<std::string>& labels,
	double bar_gap,
	double bar_overlap
) {
	// Check that this is only applied to axis 0
	if (axis_number != 0) {
		throw std::invalid_argument("Bar chart parameters can only be applied to axis 0");
		return;
	}
	// Set the ranges to the number of labels.
	const zc_range<double> range = { 0.0, static_cast<double>(labels.size() - 1) };
	axis_data_t& axis_data = axes_data_[axis_number];
	axis_data.is_bar_axis = true;
	axis_data.bar_gap = bar_gap;
	axis_data.bar_overlap = bar_overlap;
	axis_data.inner_range = range;
	axis_data.outer_range = range;
	axis_data.default_range = range;
	axis_data.ticks.clear();
	for (size_t ix = 0; ix < labels.size(); ix++) {
		axis_data.ticks.push_back({ static_cast<double>(ix), labels[ix], true });
	}
}


// Add data to the graph for a specific set of coordinates.
void zc_graph_::add_data_set(
	int axis_number,                    //!< Axis number to add the data set for (e.g. 0 for X or R axis, 1 for Y or Theta axis)
	std::vector<data_point_t>* data,     //!< Reference to a vector of data points to be plotted
	zc_line_style style                   //!< Line style to use for plotting this data set
) {
	// Check we are not trying to add the data set to axis 0.
	if (axis_number == 0) {
		throw std::invalid_argument("Cannot add a data set to axis number 0. This axis is reserved for the primary coordinate (e.g. X or R axis).");
		return;
	}
	// Extend the ranges to include the data.
	for (const data_point_t& point : *data) {
		if (axes_data_[axis_number].outer_range.contains(point.second)) {
			axes_data_[axis_number].current_range |= point.second;
			axes_data_[axis_number].default_range |= point.second;
		}
		if (axes_data_[0].outer_range.contains(point.first)) {
			axes_data_[0].current_range |= point.first;
			axes_data_[0].default_range |= point.first;
		}
	}
	data_set_t data_set = { data, style };
	data_sets_[axis_number].push_back(data_set);
};

//! \brief Add a 3D data set to the graph for a specific set of coordinates.
//! This is only valid for 3D graphs and will throw an error if called on a 2D graph.
//! It also requires that the axis parameters for axes 0, 1 and 2 have been set up before calling this method.
void zc_graph_::add_data_set(
	int axis_number,                    //!< Axis number to add the data set for (should be ignored for 3D graphs as the data set will always be added to axes 0, 1 and 2)
	data_set_dens_t* data,     //!< Reference to a vector of 3D data points to be plotted
	colour_map_t colour_map
) {
	// Check that axis number is 2 (Z-axis)
	if (axis_number != 2) {
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " is invalid for a 3D data set. The data set will always be added to axes 0, 1 and 2.");
		return;
	}
	// Check that the data set is consistent: number of z values = number of x values * number of y values
	if (data->z_values.size() != data->x_values.size() * data->y_values.size()) {
		throw std::invalid_argument("Inconsistent 3D data set: number of z values does not match number of x values * number of y values.");
		return;
	}
	// Add the data points to the default range for each axis.for (const data_point_dens_t& point : *data) {
	for (const auto& x_value : data->x_values) {
		if (axes_data_[0].outer_range.contains(x_value)) {
			axes_data_[0].current_range |= x_value;
			axes_data_[0].default_range |= x_value;
		}
	}
	for (const auto& y_value : data->y_values) {
		if (axes_data_[1].outer_range.contains(y_value)) {
			axes_data_[1].current_range |= y_value;
			axes_data_[1].default_range |= y_value;
		}
	}
	for (const auto& z_value : data->z_values) {
		if (axes_data_[2].outer_range.contains(z_value)) {
			axes_data_[2].current_range |= z_value;
			axes_data_[2].default_range |= z_value;
		}
	}
	density_data_set_ = data;
	set_colour_mapping(colour_map);
};

//! \brief Add a value marker to the graph for a specific axis number.
void zc_graph_::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	double value_1,
	double value_2
) {
	// Create the marker object
	value_marker_t marker;
	marker.style = style;
	marker.value_1 = value_1;
	marker.value_2 = value_2;
	value_markers_[axis_number][layer].push_back(marker);
};

//! \brief Add a point lozenge marker to the graph for a specific axis number.
void zc_graph_::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	data_point_t position
) {
	// Create the marker object
	lozenge_marker_t marker;
	marker.style = style;
	marker.position = position;
	lozenge_markers_[axis_number][layer].push_back(marker);
};

//! \brief Add a text label to the graph at a specific position.
void zc_graph_::add_label(
	int axis_number,
	layer_t layer,
	const std::string& text,
	zc_text_style style,
	data_point_t position,
	text_alignment_t alignment,
	bool opaque
) {
	// Add the label to the list of point markers for this axis number and layer.
	point_marker_t marker;
	marker.position = position;
	marker.text = text;
	marker.style = style;
	marker.alignment = alignment;
	marker.opaque = opaque;
	point_markers_[axis_number][layer].push_back(marker);
}

// Clear the plot_data
void zc_graph_::start_config() {
	// Clear the plot data for all data types and layers
	clear_plot_data();
	// Clear the axis data for all axes
	for (auto& axis_data : axes_data_) {
		axis_data = axis_data_t();
	}
	// Clear the value markers for all axes and layers
	for (auto& marker_map : value_markers_) {
		marker_map.clear();
	}
	// Clear the point markers for all axes and layers
	for (auto& marker_map : point_markers_) {
		marker_map.clear();
	}
	// Clear the point lozenge markers for all axes and layers
	for (auto& marker_map : lozenge_markers_) {
		marker_map.clear();
	}
}

// Initiaite the plot data
void zc_graph_::end_config() {
	axis_width_ = default_text_size_ * 2; // Default width of the axes is twice the default text size.
	v_axis_width_ = default_text_size_ * 3; // Default width of the vertical axis is thrice the default text size.
	// Update the current ranges for each axis to include the default ranges, if they are not already included.
	for (auto& axis_data : axes_data_) {
		axis_data.current_range |= axis_data.default_range;
	}
	// Clear the plot data for all data types and layers.
	clear_plot_data();

	invalidate_layout();
	redraw();
}

//! \brief Normalise the value \p fin to a mantissa and power of 10.
void zc_graph_::normalise(double fin, modifier_t modifier, double& mantissa, double& power10, uint32_t& si_prefix_exponent) {
	power10 = 1.0F;
	mantissa = fabs(fin);
	double step = 10.0F;
	double inv_step = 0.01F;
	double upper = 10.0F;
	double lower = 1.0F;
	int pow_10 = 0;
	int pow_step = 1;
	switch (modifier) {
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
	if (modifier == SI_PREFIX) si_prefix_exponent = SI_PREFIXES.at(pow_10);

}

// Generate axis line
void zc_graph_::generate_axis_line(int axis_number) {
	// Generate the axis line for this axis number based on the current ranges and tick spacing
	value_marker_t marker;
	marker.style = zc_line_style({ FL_BLACK, 1, FL_SOLID });
	marker.value_1 = axes_data_[axis_number].position;
	marker.value_2 = marker.value_1;
	int drawing_axis_number = (axis_number == 0) ? 1 : 0; // The axis number to draw the line along is the other axis
	generate_value_marker(
		drawing_axis_number,
		AXES,
		marker
	);
}

// Generate grid_lines
void zc_graph_::generate_grid_lines(int axis_number) {
	// Generate the grid lines for this axis number based on the current ranges and tick spacing
	zc_line_style grid_line_style({ FL_LIGHT2, 1, FL_DOT });
	for (auto& tick : axes_data_[axis_number].ticks) {
		if (tick.is_major) {
			value_marker_t marker;
			marker.style = grid_line_style;
			marker.value_1 = tick.value;
			marker.value_2 = tick.value;
			generate_value_marker(
				axis_number,
				GRID_LINES,
				marker
			);
		}
	}
}

// Generate ticks
void zc_graph_::set_ticks(
	int axis_number, 
	int tick_spacing_pixels, 
	double inv_scale) {
	axis_data_t& axis_data = axes_data_[axis_number];
	// Check the current range is valid
	if (!axis_data.current_range.is_valid()) {
		// Current range is not valid, throw an error
//		throw std::invalid_argument("Current range is not valid for axis number " + std::to_string(axis_number) + ". Set axis ranges before setting ticks.");
		return;
	}
	axis_data.ticks.clear();
	// Calculate the tick spacing in data coordinates based on the desired spacing in pixels and the current
	double tick_spacing = std::abs(tick_spacing_pixels * inv_scale);
	double tick_mantissa;
	double tick_power10;
	double grid_spacing_units = tick_spacing;
	std::string format = "%0.0f";
	uint32_t si_prefix_exponent = ' ';
	normalise(tick_spacing, axis_data.modifier, tick_mantissa, tick_power10, si_prefix_exponent);
	// Calculate the actual tick spacing in data units.
	switch (axis_data.modifier) {
	case NO_MODIFIER:
		if (tick_spacing >= 0.7F) format = "%0.0f";
		else if (tick_spacing >= 0.07F) format = "%0.1F";
		else if (tick_spacing >= 0.007F) format = "%0.2F";
		else format = "%g";
		// Fall through to the same tick spacing as POWER_OF_10 - but with different formatting.
		[[fallthrough]];
	case POWER_OF_10:
		// If normalised value > 7 - set tick at 10 * power10 
		if (tick_mantissa > 7.0F) {
			tick_spacing = 10.0F * tick_power10;
		}
		// If normalised value is between 3.2 and 7 - set tick at 5*10^N		
		else if (tick_mantissa > 3.2F) {
			tick_spacing = 5.0F * tick_power10;
		}
		// If normalised value is between 1.4 and 3.2 - set tick at 2*10^N
		else if (tick_mantissa > 1.4F) {
			tick_spacing = 2.0F * tick_power10;
		}
		// Otherwise set tick at 10^N
		else {
			tick_spacing = tick_power10;
		}
		break;
	case SI_PREFIX:
		// If normalised value > 70 - set tick at 100 * power10
		if (tick_mantissa > 70.0F) {
			tick_spacing = 100.0F * tick_power10;
		}
		// If normalised value is between 32 and 70 - set tick at 50*10^N		
		else if (tick_mantissa > 32.0F) {
			tick_spacing = 50.0F * tick_power10;
		}
		// If normalised value is between 14 and 32 - set tick at 20*10^N
		else if (tick_mantissa > 14.0F) {
			tick_spacing = 20.0F * tick_power10;
		}
		// If normalised value is between 7 and 14 - set tick at 10*10^N
		else if (tick_mantissa > 7.0F) {
			tick_spacing = 10.0F * tick_power10;
		}
		// If normalised value is between 3.2 and 7 - set tick at 5*10^N		
		else if (tick_mantissa > 3.2F) {
			tick_spacing = 5.0F * tick_power10;
		}
		// If normalised value is between 1.4 and 3.2 - set tick at 2*10^N
		else if (tick_mantissa > 1.4F) {
			tick_spacing = 2.0F * tick_power10;
		}
		// If normalised value is between 0.7 and 1.4 - set tick at 10^N
		else if (tick_mantissa > 0.7F) {
			tick_spacing = tick_power10;
		}
		// If normalised value is between 0.32 and 0.7 - set tick at 0.5*10^N
		else if (tick_mantissa > 0.32F) {
			tick_spacing = 0.5F * tick_power10;
			format = "%0.1f";
		}
		// If normalised value is between 0.14 and 0.32 - set tick at 0.2*10^N
		else if (tick_mantissa > 0.14F) {
			tick_spacing = 0.2F * tick_power10;
			format = "%0.1f";
		}
		// Otherwise set tick at 0.1*10^N
		else {
			tick_spacing = 0.1F * tick_power10;
			format = "%0.1f";
		}
		break;
	}
	// TODO For now set grid spacing at twice the tick spacing - but we could make this a separate parameter in the future.
	grid_spacing_units = 2.0F * tick_spacing;
	// Calculate the tick positions and labels based on the current range and tick spacing.
	// Start at the first tick position less than or equal to the minimum of the current range.
	float tick_value = floorf(axis_data.current_range.first / tick_spacing) * tick_spacing;
	while (tick_value <= axis_data.current_range.second) {
		bool is_major = (fabsf(fmodf(tick_value, grid_spacing_units)) < 1e-6F);
		// Ignore tick_value less than the minimum.
		if (tick_value < axis_data.current_range.first) {
			tick_value += tick_spacing;
			continue;
		}
		// Format the tick label based on the modifier.
		char label[20];
		switch (axis_data.modifier) {
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
		axis_data.ticks.push_back({ tick_value, std::string(label), is_major });
		tick_value += tick_spacing;
	}
	// Add the SI prefix or power of 10 to the axis label if applicable.
	char lu[100];
	switch (axis_data.modifier) {
	case NO_MODIFIER:
		snprintf(lu, sizeof(lu), "(%s)", axis_data.unit.c_str());
		break;
	case POWER_OF_10:
		snprintf(lu, sizeof(lu), "(\303\227%g %s)", tick_power10, axis_data.unit.c_str());
		break;
	case SI_PREFIX: {
		// Convert Unicode code point to UTF-8
		char prefix_utf8[5] = { 0 };  // UTF-8 can be up to 4 bytes + null terminator
		fl_utf8encode(si_prefix_exponent, prefix_utf8);
		snprintf(lu, sizeof(lu), "(%s%s)", prefix_utf8, axis_data.unit.c_str());
		break;
	}
	}
	char ll[100];
	if (axis_data.unit.length() > 0) {
		snprintf(ll, sizeof(ll), "%s %s", axis_data.label.c_str(), lu);
	}
	else {
		snprintf(ll, sizeof(ll), "%s", axis_data.label.c_str());
	}
	axis_data.modified_label = ll;

}

// Generate the ticks on a linear axis.
void zc_graph_::generate_axis_ticks(int axis_number) {
	axis_data_t& axis_data = axes_data_[axis_number];
	// If tick orientation is NO_TICKS, return without generating ticks.
	if (axis_data.tick_orientation == NO_TICKS) {
		return;
	}

	set_ticks(axis_number, axis_data.tick_spacing_pixels, axis_data.inv_scale);

	for (const auto& tick : axis_data.ticks) {
		plot_object_t tick_mark;
		tick_mark.shape = TICK;
		tick_mark.style = zc_line_style({ FL_FOREGROUND_COLOR, 1, FL_SOLID });
		tick_mark.text = tick.label;
		tick_mark.text_style = zc_text_style({ FL_FOREGROUND_COLOR, textfont(), default_text_size_});
		if (axis_number == 0) {
			axis_data_t& other_axis_data = axes_data_[1];
			// Horizontal axis - generate vertical ticks
			// Get the pixel coords of the tick mark start.
			plot_vertex_t start(convert_axis_point({tick.value, axis_data.position}));
			tick_mark.segments.push_back(plot_segment_t(start));
			tick_mark.tick_angle = get_tick_angle(axis_number, axis_data.tick_orientation, tick.value);
			if (tick_mark.tick_angle == 0)
				tick_mark.text_alignment = ALIGN_RIGHT;
			else if (tick_mark.tick_angle < 90)
				tick_mark.text_alignment = ALIGN_RIGHT | ALIGN_ABOVE;
			else if (tick_mark.tick_angle == 90)
				tick_mark.text_alignment = ALIGN_ABOVE;
			else if (tick_mark.tick_angle < 180)	
				tick_mark.text_alignment = ALIGN_LEFT | ALIGN_ABOVE;
			else if (tick_mark.tick_angle == 180) 
				tick_mark.text_alignment = ALIGN_LEFT;
			else if (tick_mark.tick_angle < 270) 
				tick_mark.text_alignment = ALIGN_LEFT | ALIGN_BELOW;
			else if (tick_mark.tick_angle == 270)
				tick_mark.text_alignment = ALIGN_BELOW;
			else
				tick_mark.text_alignment = ALIGN_RIGHT | ALIGN_BELOW;
		}
		else {
			axis_data_t& other_axis_data = axes_data_[0];
			// Vertical axis - generate horizontal ticks
			plot_vertex_t start(convert_axis_point({axis_data.position, tick.value}));
			tick_mark.segments.push_back(plot_segment_t(start));
			tick_mark.tick_angle = get_tick_angle(axis_number, axis_data.tick_orientation, tick.value);
			if (tick_mark.tick_angle == 90)
				tick_mark.text_alignment = ALIGN_ABOVE;
			else if (tick_mark.tick_angle == 270)
				tick_mark.text_alignment = ALIGN_BELOW;
			else if (tick_mark.tick_angle < 90 || tick_mark.tick_angle > 270)
				tick_mark.text_alignment = ALIGN_RIGHT;
			else
				tick_mark.text_alignment = ALIGN_LEFT;
		}
		int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the ticks along is the other axis
		plot_data_[plot_number].layer_data[AXES].push_back(tick_mark);
	}
}

// Generate the axis label. 
// this will located att he position in the axis_data for this axis number.
void zc_graph_::generate_axis_label(int axis_number) {
	axis_data_t& axis_data = axes_data_[axis_number];
	// Add a text label at the specified position
	plot_object_t label;
	label.shape = TEXT_BOX;
	label.text = axis_data.modified_label;
	label.text_style = zc_text_style({ FL_BLACK, textfont(), default_text_size_});
	label.segments.push_back(plot_segment_t(plot_vertex_t(axis_data.label_position)));
	label.text_angle = axis_data.label_angle;
	label.text_alignment = ALIGN_CENTRE;
	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the label along is the other axis
	plot_data_[plot_number].layer_data[AXES].push_back(label);
}

// Add all the necessary components to draw the axes for a specific axis number, including the axis line, ticks, grid lines and label.#
void zc_graph_::generate_axis_grid(int axis_number) {
	generate_axis_line(axis_number);
	generate_axis_ticks(axis_number);
	generate_grid_lines(axis_number);
	generate_axis_label(axis_number);
}


// Convert data set to plot data and add it to the plot data for a specific set of coordinates.
void zc_graph_::generate_data_lines(int axis_number) {
	// Check we are not drawing onto plot_data 0.
	if (axis_number == 0) {
		throw std::invalid_argument("Cannot add a data set for axis number 0. Axis number 0 is reserved for the primary axis and is used to define the X or R coordinates for the plot. Set axis parameters for axis 0 and add data sets for other axis numbers to plot data against the primary axis.");
		return;
	}
	// Delete existing data lines for this axis number, if any.
	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the data along is the other axis
	plot_data_[plot_number].layer_data[DATA].clear();

	axis_data_t& axis_data = axes_data_[axis_number];
	axis_data_t& axis_0_data = axes_data_[0];
	// Do not draw lines for a bar chart.
	if (axis_0_data.is_bar_axis) return;

	for (const auto& data_set : data_sets_[axis_number]) {
		// Add the data set to the plot data for this axis number
		// Create a new plot line for this data set.
		plot_object_t plot_line;
		plot_line.shape = LINE_STRIP;
		plot_line.style = data_set.style;

		data_point_t previous_point;
		// Add a vertex for each data point in the data set.
		// Include the data points in the current ranges for the axes.
		for (const auto& point : *data_set.data) {
			//// Check that the point is within the current ranges for both axes. If not, skip it.
			//if (!axis_data.current_range.contains(point.second) ||
			//	!axis_0_data.current_range.contains(point.first)) {
			//	continue;
			//}
			// Add the point to the default ranges for both axes, if it's within the outer range for that axis. This ensures that if the point is outside the current range but within the outer range.
			//if (axis_data.outer_range.contains(point.second)) {
			//	axis_data.default_range |= point.second;
			//}
			//if (axis_0_data.outer_range.contains(point.first)) {
			//	axis_0_data.default_range |= point.first;
			//}
			// Now add the point to the plot line as a vertex. Convert to Cartesian coordinates if necessary.
			if (!duplicate_point(axis_number, point, previous_point)) {
				plot_vertex_t vertex;
				vertex = plot_vertex_t(convert_point(point));
				plot_segment_t segment(vertex);
				plot_line.segments.push_back(segment);
				previous_point = point;
			}
		}
		// Add the plot line to the plot data for this axis number.
		plot_data_[plot_number].layer_data[DATA].push_back(plot_line);
	}
}

//! \brief Check whether the specified vertex is a duplicate of the previous vertex for the specified axis number.
bool zc_graph_::duplicate_point(int axis_number, const data_point_t& point, const data_point_t& last_point) const {
	int px1, px2, py1, py2;
	data_to_pixel(axis_number, px1, py1, point);
	data_to_pixel(axis_number, px2, py2, last_point);
	return px1 == px2 && py1 == py2;
}

//! \brief Generate a bit map reperesenting the Z-value at each point in the current range for the X and Y axes.
void zc_graph_::generate_density_plot(
    int axis_number) {
	// Check this is axis number 2 (the Z axis).
	if (axis_number != 2) {
		throw std::invalid_argument("Density plot can only be generated for axis number 2 (the Z axis).");
		return;
	}
	// Generate the density plot for this axis number based 
	// on the current ranges for the X and Y axes and the Z values in the data sets for this axis number.
	// We need to create a bit map representing the Z values at each pixel in the current range for the X and Y axes, 
	// then convert this to a plot object and add it to the plot data for this axis number.
	plot_object_t density_plot;
	density_plot.shape = BITMAP;
	axis_data_t& x_axis_data = axes_data_[0];
	axis_data_t& y_axis_data = axes_data_[1];
	axis_data_t& z_axis_data = axes_data_[axis_number];
	// NB bitmap coordinates are in pixels so use the plot area dimensions in pixels 
	// for the bitmap size, not the current ranges for the axes.
	plot_bitmap_t bitmap;
	bitmap.x = plot_x_;
	bitmap.y = plot_y_;
	bitmap.w = plot_w_;
	bitmap.h = plot_h_;
	// Using the density_bitmap_ vector to store the pixel data for the bitmap.
	// This is deliberately avoiding the run-time checks on indexing for performance reasons.
	// so check once here:
	if (density_bitmap_.size() != static_cast<size_t>(plot_w_ * plot_h_ * 3)) {
		throw std::invalid_argument("Density bitmap size does not match plot area dimensions.");
		return;
	}
	bitmap.data = density_bitmap_.data();
	plot_segment_t segment(bitmap);
	density_plot.segments.push_back(segment);

	// Pre-build sorted unique x and y coordinates from the data set ONCE, outside the pixel loops
	std::vector<double> x_coords;
	std::vector<double> y_coords;
	std::map<std::pair<double, double>, double> data_map;

	if (density_data_set_ == nullptr) {
		return;
	}

	// For each pixel in the bitmap, calculate the corresponding X and Y values based on the current ranges for the X and Y axes,
	// then calculate the Z value at that point based on the data sets for this axis number, and set the pixel colour based on the Z value.
	int index = 0;
	auto& z_xform = plot_data_[axis_number].xform_schema.z_xform;
	size_t data_row_size = density_data_set_->x_values.size();
	for (int y = 0; y < plot_h_; y++) {
		for (int x = 0; x < plot_w_; x++) {
			double z_value = 0.0;
			// Calculate the Z value at this point based on the data sets for this axis number.
			// Interpolation coefficients will have been set in layout().
			
			// Use bilinear interpolation between the four nearest data points in the data sets for this axis number to calculate the Z value at this point.
			// density_data_set_ is a pointer to a vector of data_point_3d_t (tuples of x, y, z values)
			auto& z_xform_pt = z_xform[y * plot_w_ + x];
			for (auto& it : z_xform_pt) {
				size_t data_set_index = it.y_index * data_row_size + it.x_index;
				z_value += density_data_set_->z_values[data_set_index] * it.contribution;
			}

			// Set the pixel colour based on the Z value.
			Fl_Color colour = density_colour(z_value);
			// Set the pixel colour in the bitmap. The bitmap is in RGBA format, so we need to set the red, green, blue and alpha values for this pixel.
			Fl::get_color(colour, bitmap.data[index + 0], bitmap.data[index + 1], bitmap.data[index + 2]);
			index += 3;
		}
	}
	// Add the plot line to the plot data for this axis number.
	plot_data_[2].layer_data[DATA].push_back(density_plot);
}

//! \brief Generate the bar polygons
void zc_graph_::generate_bar_polygons(
	int axis_number
) {
	// Check we are not drawing onto plot_data 0.
	if (axis_number == 0) {
		throw std::invalid_argument("Cannot add a data set for axis number 0. Axis number 0 is reserved for the primary axis and is used to define the X or R coordinates for the plot. Set axis parameters for axis 0 and add data sets for other axis numbers to plot data against the primary axis.");
		return;
	}
	// get the bar parameters
	axis_data_t& axis_data = axes_data_[axis_number];
	axis_data_t& axis_0_data = axes_data_[0];

	// Do not draw bars if it is not a bar chart
	if (!axis_0_data.is_bar_axis) return;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the data along is the other axis
	int number_sets = data_sets_[axis_number].size();
	double& gap = axis_0_data.bar_gap;
	double& overlap = axis_0_data.bar_overlap;
	double step = 1.0 + (number_sets - 1) * (1.0 - overlap) + gap;
	double bar_width = 1.0 / step;
	double bar_offset = bar_width * (1.0 - overlap);
	std::vector<data_set_t>& axis_data_sets = data_sets_.at(axis_number);
	for (size_t ix = 0; ix < axis_data_sets.size(); ix++) {
		const data_set_t& data_set = axis_data_sets[ix];
		// Add the data set to the plot data for this axis number
		// Add a vertex for each data point in the data set.
		for (const auto& point : *(data_set.data)) {
			// Create a plot object for each bar.
			plot_object_t plot_bar;
			plot_bar.shape = POLYGON;
			plot_bar.style = data_set.style;
			data_point_t p1;
			// Get the four corners of the bar.
			p1.first = point.first + ix * bar_offset;
			p1.second = point.second;
			plot_bar.segments.push_back(plot_segment_t(plot_vertex_t(p1)));
			p1.second = 0;
			plot_bar.segments.push_back(plot_segment_t(plot_vertex_t(p1)));
			p1.first += bar_width;
			plot_bar.segments.push_back(plot_segment_t(plot_vertex_t(p1)));
			p1.second = point.second;
			plot_bar.segments.push_back(plot_segment_t(plot_vertex_t(p1)));
			// Add the plot bar to the plot data for this axis number.
			plot_data_[plot_number].layer_data[DATA].push_back(plot_bar);
		}
	}
}

//! \brief Add a text label to the graph at a specific position.
void zc_graph_::generate_point_markers(
	int axis_number
) {
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a label.");
		return;
	}
	// Get the other axis number: If axis 0 then the primary axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the other axis data already exists for the other axis number
	if (other_axis_number >= static_cast<int>(axes_data_.size())) {
		// Other axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a label.");
		return;
	}
	for (auto& point_marker : point_markers_[axis_number]) {
		layer_t layer = point_marker.first;
		auto& point_data = point_marker.second;
		// Check the position is within the outer range for both axes
		axis_data_t& axis_data = axes_data_[axis_number];
		axis_data_t& other_axis_data = axes_data_[other_axis_number];
		axis_data_t& x_axis_data = (axis_number == 0) ? axis_data : other_axis_data;
		axis_data_t& y_axis_data = (axis_number == 0) ? other_axis_data : axis_data;
		// Fore each point marker...
		for (const auto& point_datum : point_data) {
			// Add a point marker at the specified position
			plot_object_t marker;
			if (point_datum.opaque) marker.shape = TEXT_BOX;
			else marker.shape = TEXT;
			marker.text = point_datum.text;
			marker.text_style = point_datum.style;
			marker.text_alignment = point_datum.alignment;
			// If the point is set to maximum for either axis, move it to the edge of the current range for that axis.
			data_point_t position = point_datum.position;
			if (position.second == -DBL_MAX) {
				position.second = y_axis_data.current_range.first;
			}
			else if (position.second == DBL_MAX) {
				position.second = y_axis_data.current_range.second;
			} 
			if (position.first == -DBL_MAX) {
				position.first = x_axis_data.current_range.first;
			}
			else if (position.first == DBL_MAX) {
				position.first = x_axis_data.current_range.second;
			}
			// If position is still outside the current range for either axis, skip it.
			if (!x_axis_data.current_range.contains(position.first) ||
				!y_axis_data.current_range.contains(position.second)) {
				continue;
			}
			marker.segments.push_back(plot_segment_t(plot_vertex_t(convert_point(position))));
			int plot_number = (axis_number == 0) ? 1 : axis_number;
			plot_data_[plot_number].layer_data[layer].push_back(marker);
		}
	}
}

// Generate value markers for a specific axis number.
void zc_graph_::generate_value_markers(
	int axis_number
) {
	for (auto& axis_marker : value_markers_[axis_number]) {
		layer_t layer = axis_marker.first;
		auto& layer_markers = axis_marker.second;
		for (const auto& value_datum : layer_markers) {
			generate_value_marker(
				axis_number,
				layer,
				value_datum
			);
		}
	}
}

// Generate lozenge markers for a specific axis number.
void zc_graph_::generate_lozenge_markers(
	int axis_number
) {
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a label.");
		return;
	}
	// Get the other axis number: If axis 0 then the primary axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the other axis data already exists for the other axis number
	if (other_axis_number >= static_cast<int>(axes_data_.size())) {
		// Other axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a label.");
		return;
	}
	for (auto& lozenge_marker : lozenge_markers_[axis_number]) {
		layer_t layer = lozenge_marker.first;
		auto& marker_data = lozenge_marker.second;
		for (const auto& value_datum : marker_data) {
			plot_object_t marker;
			marker.shape = LOZENGE;
			marker.style = value_datum.style;
			plot_vertex_t vertex = convert_point(value_datum.position);
			plot_segment_t segment(vertex);
			marker.segments.push_back(segment);
			plot_data_[axis_number].layer_data[layer].push_back(marker);
		}
	}
}

// Get the tick angle for a specific tick based on the tick orientation and the position of the tick along the axis.
// Default implementation for cartesian axes - where value is ignored.
int zc_graph_::get_tick_angle(
	int axis_number,
	tick_orientation_t tick_orientation, 
	double tick_value) {
	switch (tick_orientation) {
	case TICK_INCREASING:
		return (axis_number == 0) ? 90 : 0;
	case TICK_DECREASING:
		return (axis_number == 0) ? 270 : 180;
	default:
		return 0;
	}
}

//! \brief override of Fl_Group handle to allow for zooming and scrolling on axes.
int zc_graph_::handle(int event) {
	if (event == FL_MOUSEWHEEL) {
		// Get the mouse wheel delta and position, and the state of the modifier keys.
		int dy = Fl::event_dy();
		int mouse_x = Fl::event_x();
		int mouse_y = Fl::event_y();
		bool ctrl_pressed = Fl::event_state() & FL_CTRL;
		bool shift_pressed = Fl::event_state() & FL_SHIFT;

		bool handled = false;

		layout_area_t under_mouse = get_layout_area(mouse_x, mouse_y);

		if (!under_mouse.is_plot_area) {
			if (ctrl_pressed) {
				// Scroll by 10 pixels per click if ctrl is pressed, otherwise zoom.
				scroll_axis(under_mouse.axis_number, dy * 10);
			}
			else {
				zoom_axis(under_mouse.axis_number, mouse_x, mouse_y, dy);
			}
			handled = true;
		}
		else {
			if (!shift_pressed) {
				// If the mouse wheel event was on the plot and Shift is not pressed, 
				// zoom on all axes.
				for (int axis = 0; axis < num_axes_; axis++) {
					zoom_axis(axis, mouse_x, mouse_y, dy);
					handled = true;
				}
			}
		}

		if (handled) {
			redraw();
			return 1;
		}
	}

	// Handle push to enable drag
	else if (event == FL_PUSH) {
		// Save the position of the mouse when the button is pressed to calculate the drag distance in FL_DRAG.
		last_mouse_x_ = Fl::event_x();
		last_mouse_y_ = Fl::event_y();
		if (Fl::event_clicks()) {
			if (Fl::event_button() == FL_LEFT_MOUSE) {
				set_click_value(Fl::event_x(), Fl::event_y());
				do_callback();
				redraw();
				// Return 0 to indicate we do not want a drag event, as this is a click event.
				return 0;
			}
			else if (Fl::event_button() == FL_RIGHT_MOUSE) {
				// If this is a double-click, reset the axis under the mouse to
				// the default range.
				layout_area_t under_mouse = get_layout_area(Fl::event_x(), Fl::event_y());
				if (!under_mouse.is_plot_area) {
					// If the double-click was on an axis, reset that axis to its default range.
					reset_zoom(under_mouse.axis_number);
					redraw();
					// Return 0 to indicate we do not want a drag event.
					return 0;
				}
				else {
					// If the double-click was on the plot, reset all axes to their default range.
					for (int axis = 0; axis < num_axes_; axis++) {
						reset_zoom(axis);
					}
					redraw();
					// Return 0 to indicate we do not want a drag event.
					return 0;
				}
			}
		}
		// If this is not a double-click, we will handle dragging in FL_DRAG.
		return 1;
	}

	// Handle click and drag on axis to scroll.
	else if (event == FL_DRAG) {
		int dx = last_mouse_x_ - Fl::event_x();
		int dy = last_mouse_y_ - Fl::event_y();
		last_mouse_x_ = Fl::event_x();
		last_mouse_y_ = Fl::event_y();
		layout_area_t under_mouse = get_layout_area(Fl::event_x(), Fl::event_y());
		if (!under_mouse.is_plot_area) {
			// If the mouse is being dragged on an axis, scroll that axis.
			if (under_mouse.axis_number == 0) {
				// If dragging on the horizontal axis, scroll horizontally by dx.
				scroll_axis(under_mouse.axis_number, (Fl::event_state() & FL_SHIFT) ? dx * 10 : dx);
			}
			// If dragging on a vertical axis, scroll vertically by -dy.
			else {
				scroll_axis(under_mouse.axis_number, (Fl::event_state() & FL_SHIFT) ? -dy * 10 : -dy);
			}
			redraw();
			return 1;
		} 
		else {
			// If left mouse button is held and the mouse is dragged on the plot,
			// Scroll on horizontal and leftwards vertical axes.
			// Other axes will ignore scroll.
			if (Fl::event_button() == FL_LEFT_MOUSE) {
				scroll_axis(0, dx);
				scroll_axis(1, -dy);
				redraw();
				return 1;
			}
			else if (Fl::event_button() == FL_RIGHT_MOUSE) {
				scroll_axis(0, dx);
				if (2 < static_cast<int>(axes_data_.size())) {
					scroll_axis(2, -dy);
				}
				redraw();
				return 1;
			}
		}
	}
	return Fl_Widget::handle(event);
}

// Zoom the axis under the mouse by 2^^(delta/10).
void zc_graph_::zoom_axis(int axis_number, int mouse_x, int mouse_y, int zoom_factor) {
	if (axis_number == -1) {
		return;
	}
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before zooming.");
		return;
	}
	axis_data_t& axis_data = axes_data_[axis_number];
	data_point_t mouse_position;
	switch (zoom_capability_) {
	case ZOOM_ON_ORIGIN:
		// Zoom on the origin - so mouse position is not relevant, use origin.
		mouse_position = { 0.0F, 0.0F };
		break;
	case ZOOM_ON_CURSOR:
		// Zoom on the cursor - so mouse position is relevant.
		mouse_position = pixel_to_data(axis_number, mouse_x, mouse_y);
		break;
	default:
		return;
	}
	// Set zoom factor
	// Zoom change is 2^^(delta/10) - so every 10 units of delta doubles the zoom factor, every -10 units halves it.
	double zoom_change = pow(2.0, (double)zoom_factor / 10.0);
	zc_range<double> new_range;
	// Calculate the new range based on the zoom change and mouse position.
	if (is_axis_horizontal(axis_number)) {
		double new_min = mouse_position.first - (mouse_position.first - axis_data.current_range.first) * zoom_change;
		double new_max = mouse_position.first + (axis_data.current_range.second - mouse_position.first) * zoom_change;
		new_range = zc_range<double>{ new_min, new_max };
	}
	else {
		double new_min = mouse_position.second - (mouse_position.second - axis_data.current_range.first) * zoom_change;
		double new_max = mouse_position.second + (axis_data.current_range.second - mouse_position.second) * zoom_change;
		new_range = zc_range<double>{ new_min, new_max };
	}
	if (axis_data.outer_range.is_valid()) {
		new_range &= axis_data.outer_range;
	}
	if (axis_data.inner_range.is_valid()) {
		new_range |= axis_data.inner_range;
	}
	axis_data.current_range = new_range;
	invalidate_layout();
}

// Scroll the axis under the mouse by the specified offset in pixels.
void zc_graph_::scroll_axis(int axis_number, int scroll_offset) {
	if (!scrollable_ || axis_number == -1) {
		return;
	}
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before scrolling.");
		return;
	}
	axis_data_t& axis_data = axes_data_[axis_number];
	// Calculate the new range based on the scroll offset and current scale.
	double scroll_amount = scroll_offset * axis_data.inv_scale;
	double new_min, new_max;
	// Limit scroll amount if we are close to the outer limit.
	if (scroll_amount > 0.0) {
		if (axis_data.current_range.second + scroll_amount > axis_data.outer_range.second) {
			scroll_amount = axis_data.outer_range.second - axis_data.current_range.second;
		}
	} else {
		if (axis_data.current_range.first + scroll_amount < axis_data.outer_range.first) {
			scroll_amount = axis_data.outer_range.first - axis_data.current_range.first;
		}
	}
	new_min = axis_data.current_range.first + scroll_amount;
	new_max = axis_data.current_range.second + scroll_amount;
	zc_range<double> new_range = axis_data.outer_range & zc_range<double>{ new_min, new_max };
	axis_data.current_range = new_range;
	invalidate_layout();
}

// Reset the zoom for the specified axis number to the default range.
void zc_graph_::reset_zoom(int axis_number) {
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before resetting zoom.");
		return;
	}
	axis_data_t& axis_data = axes_data_[axis_number];
	axis_data.current_range = axis_data.default_range;
	invalidate_layout();
}

// Set the current range for the specified axis number to the specified range.
void zc_graph_::set_axis_range(int axis_number, const zc_range<double>& range) {
	// Check the axis data already exists for this axis number
	if (axis_number >= static_cast<int>(axes_data_.size())) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before setting range.");
		return;
	}
	axis_data_t& axis_data = axes_data_[axis_number];
	// Set the current range to the specified range, but limit it to the outer and inner ranges.
	axis_data.current_range = (range & axis_data.outer_range) | axis_data.inner_range;
	invalidate_layout();
}

// Resize the widget - reset scaling factors
void zc_graph_::resize(int X, int Y, int W, int H) {
	// If we have actually resized...
	if (X != x() || Y != y() || W != w() || H != h()) {
		Fl_Widget::resize(X, Y, W, H);
		invalidate_layout();
		redraw();
	}
}

// Clear ephemeral plot data
void zc_graph_::clear_plot_data() {
	for (auto& plot_data : plot_data_) {
		for (auto& layer_pair : plot_data.layer_data) {
			layer_pair.second.clear();
		}
	}
}

// draw the widget
void zc_graph_::draw() {

	// Clear the plot layer data for all data types and layers.
	clear_plot_data();
	// Set out the positions of the axes and plot area, set the 
	// drawing transformation schemata.
	// Only recalculate layout if it's been invalidated.
	if (layout_dirty_) {
		layout();
		layout_dirty_ = false;
	}

	if (active()) {
		// Generate the axis lines, ticks, grid lines and labels for each axis based on the current parameters and ranges.
		for (int i = 0; i < num_axes_; ++i) {
			generate_axis_grid(i);
			generate_value_markers(i);
			generate_point_markers(i);
		}

		// Regenerate data for each data set.
		for (auto& data_set_pair : data_sets_) {
			generate_data_lines(data_set_pair.first);
		}

		// Regenerate bar polygons for each data set.
		for (auto& data_set_pair : data_sets_) {
			generate_bar_polygons(data_set_pair.first);
		}

		// Regenerate data for each 3D data set.
		if (supports_3d_plots()) {
			generate_density_plot(2);
		}

		// Generate the data lozenge markers for each axis.
		for (int i = 0; i < num_axes_; ++i) {
			generate_lozenge_markers(i);
		}

		// Clear the drawing area
		fl_push_clip(x(), y(), w(), h());
		fl_color(color());
		fl_rectf(x(), y(), w(), h());
		// For each of the layers....
		for (layer_t layer = BACKGROUND; layer <= MASK; ((uint8_t&)layer)++) {
			bool tighter_clip = false;
			if (layer == BACKGROUND || layer == FOREGROUND || layer == DATA) {
				// Set tighter clipping for these layers to prevent drawing outside the plot area.
				fl_push_clip(plot_x_, plot_y_, plot_w_, plot_h_);
				tighter_clip = true;
			}
			// For each of the data types...
			for (const auto& plot_data : plot_data_) {
				// If there is data for this layer and data type, draw it.
				if (plot_data.layer_data.find(layer) == plot_data.layer_data.end()) {
					continue;
				}
				// Set the transformation for this data type based on the axis parameters
				fl_push_matrix();
				apply_transformations(plot_data.xform_schema);
				const std::vector<plot_object_t>& layer_data = plot_data.layer_data.at(layer);
				// For each of the plot objects in this layer for this data type...
				for (const auto& plot_object : layer_data) {
					draw_plot_object(plot_object);
				}
				fl_pop_matrix();
			}
			if (tighter_clip) {
				fl_pop_clip();
			}
		}
		fl_pop_clip();
	}
	else {
		// Clear the drawing area
		fl_push_clip(x(), y(), w(), h());
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(x(), y(), w(), h());
		fl_pop_clip();
	}
}

//! \brief Apply the necessary transformations to the FLTK drawing context based on the specified transformation schema.
void zc_graph_::apply_transformations(const plot_xform_t& schema) {
	// Calculate the scaling factors and origin for the transformation.
	// I think this is how the transformation should work.
	double scale_x = w() / (schema.x_max_ - schema.x_min_);
	double scale_y = h() / (schema.y_min_ - schema.y_max_);
	double origin_x = x() - schema.x_min_ * scale_x;
	double origin_y = y() + h() - schema.y_min_ * scale_y;
	fl_translate(origin_x, origin_y);
	fl_scale(scale_x, scale_y);
}

//! \brief Convert thw pixel coordinates (x, y) to data coordinates for the specified axis number based on the current transformation schema for that axis.
zc_graph_::data_point_t zc_graph_::pixel_to_data(int axis_number, int pixel_x, int pixel_y) const {
	// For axis 0 use axis 1 parameters.
	int drawing_axis_number = (axis_number == 0) ? 1 : axis_number;
#ifdef _DEBUG
	// Check the plot data already exists for this axis number
	if (drawing_axis_number >= static_cast<int>(plot_data_.size())) {
		// Plot data does not exist for this axis number, throw an error
		throw std::invalid_argument("Plot data does not exist for axis number " + std::to_string(drawing_axis_number) + ". Set axis parameters and add data sets before converting pixel to data coordinates.");
	}
#endif
	const plot_xform_t& xform_schema = plot_data_[drawing_axis_number].xform_schema;
	// Use pre-calculated inverse scales
	double data_x = xform_schema.x_min_ + (pixel_x - x()) * xform_schema.inv_scale_x_;
	double data_y = xform_schema.y_max_ + (pixel_y - y()) * xform_schema.inv_scale_y_;
	return { data_x, data_y };
}

//! \brief Convert data coordinates to pixel coordinates for the specified axis number based on the current transformation schema for that axis.
//! This function is the inverse of pixel_to_data().
void zc_graph_::data_to_pixel(int axis_number, int& pixel_x, int& pixel_y, const data_point_t& data) const {
	// For axis 0 use axis 1 parameters.
	int drawing_axis_number = (axis_number == 0) ? 1 : axis_number;
#ifdef _DEBUG
	// Check the plot data already exists for this axis number
	if (drawing_axis_number >= static_cast<int>(plot_data_.size())) {
		// Plot data does not exist for this axis number, throw an error
		throw std::invalid_argument("Plot data does not exist for axis number " + std::to_string(drawing_axis_number) + ". Set axis parameters and add data sets before converting pixel to data coordinates.");
	}
#endif
	const plot_xform_t& xform_schema = plot_data_[drawing_axis_number].xform_schema;
	pixel_x = x() + (data.first - xform_schema.x_min_) * xform_schema.scale_x_;
	pixel_y = y() + h() - (data.second - xform_schema.y_min_) * xform_schema.scale_y_;
}

//! \brief Draw a plot object using the FLTK drawing functions based on its shape and style.
//! This function assumes that the necessary transformations have already been applied to the FLTK drawing context.
void zc_graph_::draw_plot_object(const plot_object_t& object) {
	switch (object.shape) {
	case POINTS:
		fl_color(object.style.colour);
		fl_begin_points();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_points();
		fl_line_style(0); // reset to default line style
		break;
	case LINE_STRIP:
		fl_color(object.style.colour);
		fl_line_style(object.style.style, object.style.width);
		fl_begin_line();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_line();
		fl_line_style(0); // reset to default line style
		break;
	case LOOP:
		fl_color(object.style.colour);
		fl_line_style(object.style.style, object.style.width);
		fl_begin_loop();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_loop();
		fl_line_style(0); // reset to default line style
		break;
	case POLYGON:
		fl_color(object.style.colour);
		fl_begin_polygon();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_polygon();
		fl_line_style(0); // reset to default line style
		break;
	case COMPLEX:
		fl_color(object.style.colour);
		fl_begin_complex_polygon();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.a1, seg.a.a2);
			}
			else if (seg.type == plot_segment_t::GAP) {
				fl_gap();
			}
		}
		fl_end_complex_polygon();
		fl_line_style(0); // reset to default line style
		break;
	case LOZENGE:
		// Draw a diamond at twice the pixel size of the line width
	{
		int lx = fl_transform_x(object.segments[0].v.x, object.segments[0].v.y);
		int ly = fl_transform_y(object.segments[0].v.x, object.segments[0].v.y);
		int size = object.style.width * 2;
		fl_color(object.style.colour);
		fl_begin_polygon();
		fl_transformed_vertex(lx, ly - size);
		fl_transformed_vertex(lx + size, ly);
		fl_transformed_vertex(lx, ly + size);
		fl_transformed_vertex(lx - size, ly);
		fl_transformed_vertex(lx, ly - size);
		fl_end_polygon();
		fl_line_style(0); // reset to default line style
		break;
	}
	case TICK:
	case TEXT:
	case TEXT_BOX:
	{

		// Transform coordinates from data cords to pixel coords for text position.
		int tx = fl_transform_x(object.segments[0].v.x, object.segments[0].v.y);
		int ty = fl_transform_y(object.segments[0].v.x, object.segments[0].v.y);
		if (object.shape == TICK) {
			fl_begin_line();
			fl_color(object.style.colour);
			fl_transformed_vertex(tx, ty);
			double angle_rad = object.tick_angle * zc::PI / 180.0F;
			// Add a tick mark of length N pixels in the direction of the tick angle.
			tx += object.tick_length * cos(angle_rad);
			ty -= object.tick_length * sin(angle_rad);
			fl_transformed_vertex(tx, ty);
			fl_end_line();
			fl_line_style(0); // reset to default line style
		}
		// Get the dimensions of the label text to position it correctly.
		int tw= 0, th = 0;
		fl_font(object.text_style.font, object.text_style.size);
		fl_measure(object.text.c_str(), tw, th);
		// Adjust the text position based on the specified text alignment.
		if ((object.text_alignment & ALIGN_MASK_LR) == ALIGN_LEFT) {
			tx -= tw / 2;
		} else if ((object.text_alignment & ALIGN_MASK_LR) == ALIGN_RIGHT) {
			tx += tw / 2;
		}
		if ((object.text_alignment & ALIGN_MASK_AB) == ALIGN_ABOVE) {
			ty -= th / 2;
		} else if ((object.text_alignment & ALIGN_MASK_AB) == ALIGN_BELOW) {
			ty += th / 2;
		}
		// Adjust for the size and angle of the label.
		double angle_rad = object.text_angle * zc::PI / 180.0F;
		double cos_a = cos(angle_rad);
		double sin_a = sin(angle_rad);
		// Generate coordinates of all 4 corners of the label text box in pixel coords.
		double half_tw = tw / 2.0F;
		double half_th = th / 2.0F;
		double corners[4][2] = {
			{ tx - half_tw * cos_a + half_th * sin_a,
			  ty + half_tw * sin_a + half_th * cos_a },
			{ tx + half_tw * cos_a + half_th * sin_a,
			  ty - half_tw * sin_a + half_th * cos_a },
			{ tx + half_tw * cos_a - half_th * sin_a,
			  ty - half_tw * sin_a - half_th * cos_a },
			{ tx - half_tw * cos_a - half_th * sin_a,
		      ty + half_tw * sin_a - half_th * cos_a }
		};
		if (object.shape == TEXT_BOX) {
			half_tw += 2;
			half_th += 2;
			double box_corners[4][2] = {
				{ tx - half_tw * cos_a + half_th * sin_a,
				  ty + half_tw * sin_a + half_th * cos_a },
				{ tx + half_tw * cos_a + half_th * sin_a,
				  ty - half_tw * sin_a + half_th * cos_a },
				{ tx + half_tw * cos_a - half_th * sin_a,
				  ty - half_tw * sin_a - half_th * cos_a },
				{ tx - half_tw * cos_a - half_th * sin_a,
				  ty + half_tw * sin_a - half_th * cos_a }
			};

			// Draw a box around the text using the rotated corners.
			fl_color(color());
			fl_begin_polygon();
			for (int i = 0; i < 4; ++i) {
				fl_transformed_vertex(box_corners[i][0], box_corners[i][1]);
			}
			fl_transformed_vertex(box_corners[0][0], box_corners[0][1]);
			fl_end_polygon();
		}

		fl_color(object.text_style.colour);
		// Save the current font and size, and set the font and size for the text.
		Fl_Font old_font = fl_font();
		Fl_Fontsize old_size = fl_size();
		fl_font(object.text_style.font, object.text_style.size);
		// Draw the text at the bottom left of the text box, rotated by the specified angle.
		fl_draw(object.text_angle, object.text.c_str(), (int)corners[0][0], (int)corners[0][1]);
		// Restore the previous font and size.
		fl_font(old_font, old_size);
		break;
	}
	case BITMAP: {
		// Temporarily add the identity transform so that the bitmap is drawn in pixel coordinates, 
		// then pop it after drawing.
		fl_push_matrix();
		fl_load_identity();
		int bx = object.segments[0].b.x;
		int by = object.segments[0].b.y;
		int bw = object.segments[0].b.w;
		int bh = object.segments[0].b.h;
		const unsigned char* data = object.segments[0].b.data;
		// Draw the bitmap using FLTK's fl_draw_image function.
		if (data) {
			fl_draw_image(data, bx, by, bw, bh, 3); // Assuming 3 bytes per pixel (RGB)
		}
		fl_pop_matrix();
	}
	}
}

// Look up Z-value in colour map and return corresponding colour.
Fl_Color zc_graph_::density_colour(double z_value) const {
	if (colour_map_.empty()) {
		return FL_BLACK; // Default to black if no colour map is defined.
	}
	// Find the first colour map entry with a z_value greater than the input z_value.
	// Normalise Z between 0 and 1 within the current range of axis 2
	double z_min = axes_data_.at(2).current_range.first;
	double z_max = axes_data_.at(2).current_range.second / 4.0;
	double z;
	if (z_max > z_min) {
		if (z_value > z_max) {
			z = 1.0;
		}
		else if (z_value < z_min) {
			z = 0.0;
		}
		else {
			z = (z_value - z_min) / (z_max - z_min);
		}
	}
	else {
		z = 0.0; // If the range is invalid, default to 0.
	}
	size_t max_index = colour_map_.size() - 1;
	size_t index = 0;
	while (z > colour_triggers_[index]) index++;
	// Z = 0 select first entry, Z = 1 select last entry, select linearly between them.
	return colour_map_[index];
}

// Set colour map
void zc_graph_::set_colour_mapping(colour_map_t colour_map) {
	size_t num_levels = colour_map.depth;
	colour_triggers_.resize(num_levels);
	colour_map_.resize(num_levels);
	double d = static_cast<double>(num_levels - 1);
	double min_exp = colour_map.lower_limit / 20.0;
	if (colour_map.logarithmic) {
		// generate the trigger levels
		for (size_t i = 0; i < num_levels; i++) {
			// Lower limit is -40dB which is 10^-2 voltage
			double exp = (min_exp) + (- min_exp * static_cast<double>(i) / d);
			double level = pow(10.0, exp);
			colour_triggers_[i] = level;
		}
	}
	else {
		for (size_t i = 0; i < num_levels; i++) {
			colour_triggers_[i] = static_cast<double>(i + 1) / d;
		}
	}
	// Generate the colour map
	uint8_t r = 0, g = 0, b = 0;
	for (int i = 0; i < num_levels; i++) {
		double v = static_cast<double>(i) / d;
		if (v < 1.0 / 3.0) {
			// Gradiate from black to reddish-purple
			r = static_cast<uint8_t>(600.0 * v);
			g = 0.0;
			b = static_cast<uint8_t>(150.0 * v);
		}
		else if (v < 2.0 / 3.0) {
			// Gradiate from reddish-purple to orange-yellow
			r = static_cast<uint8_t>(255.0 + 165.0 * v);
			g = static_cast<uint8_t>(600.0 * v - 200.0);
			b = static_cast<uint8_t>(60.0 * v + 30.0);
		}
		else {
			// Gradiate from orange-yellow to white
			r = 255;
			g = static_cast<uint8_t>(165.0 * v + 90.0);
			b = static_cast<uint8_t>(555.0 * v - 300.0);
		}
		colour_map_[i] = fl_rgb_color(r, g, b);
	}
}

// Layout cartesian 2-axis graph
void zc_graph_cartesian::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	// calculate pixel dimensions for the plot area.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y();
	plot_w_ = w() - v_axis_width_;
	plot_h_ = h() - axis_width_;
	double x_min = axes_data_[0].current_range.first;
	double x_max = axes_data_[0].current_range.second;
	double y_min = axes_data_[1].current_range.first;
	double y_max = axes_data_[1].current_range.second;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max;
	xform_schema.y_min_ = y_min - axis_width_ * dpp_y;
	xform_schema.y_max_ = y_max;
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	plot_data_[1].data_area.display_min = { x_min, y_min };
	plot_data_[1].data_area.display_max = { x_max, y_max };
	// Set the axis sizes and positions for the axes.
	axes_data_[0].position = y_min;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_x;
	// Position the label in the middle of the axis, offset by 0.5 times the axis width in the appropriate direction.
	// This is because the axis width is two text heights, so 0.5 times the axis width is 1 text height, 
	// which should give a good distance between the label and the axis.
	axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, y_min - axis_width_ * dpp_y * 0.5 };
	axes_data_[0].label_angle = 0;
	axes_data_[1].position = x_min;
	axes_data_[1].tick_orientation = TICK_DECREASING;
	axes_data_[1].inv_scale = dpp_y;
	axes_data_[1].label_position = { x_min - v_axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	axes_data_[1].label_angle = 90;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_cartesian::get_layout_area(int mouse_x, int mouse_y) const {
	layout_area_t result = { false, -1 };
	// Check if the mouse is over the plot area.
	if (mouse_x >= plot_x_ && mouse_x <= plot_x_ + plot_w_ &&
		mouse_y >= plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { true, -1 };
	}
	// Check if the mouse is over either of the axes.
	else if (mouse_x >= x() && mouse_x < plot_x_) {
		result = { false, 1 };
	}
	else if (mouse_y >plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { false, 0 };
	}
	return result;
}

// Add a marker for Cartesian graphs
void zc_graph_cartesian::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// Get the other axis number: If axis 0 then axis 1.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = axes_data_[other_axis_number];

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_cartesian::set_click_value(int mouse_x, int mouse_y) {
	// Convert the mouse coordinates to data coordinates
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Store the click values
	value_ = { cx, cy };
}

void zc_graph_cartesian_2y::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y();
	plot_w_ = w() - 2 * v_axis_width_;
	plot_h_ = h() - axis_width_;
	double x_min = axes_data_[0].current_range.first;
	double x_max = axes_data_[0].current_range.second;
	double y_min = axes_data_[1].current_range.first;
	double y_max = axes_data_[1].current_range.second;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max + v_axis_width_ * dpp_x;
	xform_schema.y_min_ = y_min - axis_width_ * dpp_y;
	xform_schema.y_max_ = y_max;
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	plot_data_[1].data_area.display_min = { x_min, y_min };
	plot_data_[1].data_area.display_max = { x_max, y_max };
	// Set the axis sizes and positions for the axes.
	axes_data_[0].position = y_min;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_x;
	axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, y_min - axis_width_ * dpp_y * 0.5 };
	axes_data_[0].label_angle = 0;
	axes_data_[1].position = x_min;
	axes_data_[1].tick_orientation = TICK_DECREASING;
	axes_data_[1].inv_scale = dpp_y;
	axes_data_[1].label_position = { x_min - v_axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	axes_data_[1].label_angle = 90;
	// Now set the transformation schema for the second Y axis to map its data range to the plot area dimensions.
	double y2_min = axes_data_[2].current_range.first;
	double y2_max = axes_data_[2].current_range.second;
	double dpp_y2 = (y2_max - y2_min) / h();
	plot_xform_t xform_schema_2;
	xform_schema_2.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema_2.x_max_ = x_max + v_axis_width_ * dpp_x;
	xform_schema_2.y_min_ = y2_min - axis_width_ * dpp_y2;
	xform_schema_2.y_max_ = y2_max;
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema_2.inv_scale_x_ = (xform_schema_2.x_max_ - xform_schema_2.x_min_) / w();
	xform_schema_2.inv_scale_y_ = (xform_schema_2.y_min_ - xform_schema_2.y_max_) / h();
	xform_schema_2.scale_x_ = 1.0 / xform_schema_2.inv_scale_x_;
	xform_schema_2.scale_y_ = 1.0 / xform_schema_2.inv_scale_y_;
	plot_data_[2].xform_schema = xform_schema_2;
	plot_data_[2].data_area.display_min = { x_min, y2_min };
	plot_data_[2].data_area.display_max = { x_max, y2_max };
	// Set the axis sizes and positions for the second Y axis.
	axes_data_[2].position = x_max;
	axes_data_[2].tick_orientation = TICK_INCREASING;
	axes_data_[2].inv_scale = dpp_y2;
	axes_data_[2].label_position = { x_max + v_axis_width_ * dpp_x * 0.5, y2_min + (y2_max - y2_min) / 2.0F };
	axes_data_[2].label_angle = 90;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_cartesian_2y::get_layout_area(int mouse_x, int mouse_y) const {
	layout_area_t result = { false, -1 };
	// Check if the mouse is over the plot area.
	if (mouse_x >= plot_x_ && mouse_x <= plot_x_ + plot_w_ &&
		mouse_y >= plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { true, -1 };
	}
	// Check if the mouse is over either of the axes.
	else if (mouse_x >= x() && mouse_x < plot_x_) {
		result = { false, 1 };
	}
	// Check if the mouse is over either of the axes.
	else if (mouse_x >= plot_x_ + plot_w_ && mouse_x < x() + w()) {
		result = { false, 2 };
	}
	else if (mouse_y > plot_y_ && mouse_y <= y() + h()) {
		result = { false, 0 };
	}
	return result;
}

// Add a marker for Cartesian 2Y graphs
void zc_graph_cartesian_2y::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// For axis 0 (X), use axis 1 (YL) as the other axis. For axis 1 or 2 (YL or YR), use axis 0 (X) as the other axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = axes_data_[other_axis_number];

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
		}
		else {
			// Horizontal line at Y = value_1 (for axis 1 or 2)
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_cartesian_2y::set_click_value(int mouse_x, int mouse_y) {
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Store the click values
	value_ = { cx, cy };
}


// Cartesian overlay layout
void zc_graph_cart_overlay::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	plot_x_ = x();
	plot_y_ = y();
	plot_w_ = w();
	plot_h_ = h();
	double x_min = axes_data_[0].current_range.first;
	double x_max = axes_data_[0].current_range.second;
	double y_min = axes_data_[1].current_range.first;
	double y_max = axes_data_[1].current_range.second;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min;
	xform_schema.x_max_ = x_max;
	xform_schema.y_min_ = y_min;
	xform_schema.y_max_ = y_max;
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	plot_data_[1].data_area.display_min = { x_min, y_min };
	plot_data_[1].data_area.display_max = { x_max, y_max };
	// Set the axis sizes and positions for the axes.
	// Draw the Y-axis at X=0 unless X=0 is outwith the range.
	if (x_min <= 0 && x_max >= 0) {
		axes_data_[1].position = 0;
	}
	else if (x_min > 0) {
		axes_data_[1].position = x_min;
	}
	else {
		axes_data_[1].position = x_max;
	}
	axes_data_[1].inv_scale = dpp_y;
	// Set the tick orientation down unless the axis width precludes it.
	if (x_min > -(v_axis_width_ * dpp_x)) {
		axes_data_[1].tick_orientation = TICK_DECREASING;
	}
	else {
		axes_data_[1].tick_orientation = TICK_INCREASING;
	}
	if (axes_data_[1].tick_orientation == TICK_DECREASING) {
		axes_data_[1].label_position = { axes_data_[1].position - v_axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	}
	else {
	    axes_data_[1].label_position = { axes_data_[1].position + v_axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	}
	axes_data_[1].label_angle = 90;
	// And repeat for the X axis.
	if (y_min <= 0 && y_max >= 0) {
		axes_data_[0].position = 0;
	}
	else if (y_min > 0) {
		axes_data_[0].position = y_min;
	}
	else {
		axes_data_[0].position = y_max;
	}
	if (y_min > -(axis_width_ * dpp_y)) {
		axes_data_[0].tick_orientation = TICK_INCREASING;
	}
	else {
		axes_data_[0].tick_orientation = TICK_DECREASING;
	}
	axes_data_[0].inv_scale = dpp_x;
	if (axes_data_[0].tick_orientation == TICK_INCREASING) {
		axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, axes_data_[0].position + axis_width_ * dpp_y * 0.5 };
	}
	else {
		axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, axes_data_[0].position - axis_width_ * dpp_y * 0.5 };
	}
	axes_data_[0].label_angle = 0;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_cart_overlay::get_layout_area(int mouse_x, int mouse_y) const {
	// Returm that the mouse is over the plot area, since in 
	// an overlay graph the axes are drawn on top of the plot area
	// and there are no margins.
	return layout_area_t{ true, -1 };
}

// Add a marker for Cartesian overlay graphs
void zc_graph_cart_overlay::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// Get the other axis number: If axis 0 then axis 1.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = axes_data_[other_axis_number];

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_cart_overlay::set_click_value(int mouse_x, int mouse_y) {
	// Convert the mouse coordinates to data coordinates
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Store the click values
	value_ = { cx, cy };
}

// Polar layout
void zc_graph_polar::layout() {
	// For polar coordinates, we will set the transformation schema to map the R and Theta ranges to the plot area dimensions.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y() + axis_width_;
	plot_w_ = w() - 2 * v_axis_width_;
	plot_h_ = h() - 2 * axis_width_;
	double r_max = axes_data_[0].current_range.second;
	// data per pixel values
	double radius_pixels = std::min(plot_w_, plot_h_) / 2.0;
	double dpp_r = r_max / radius_pixels;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = -(w() * dpp_r / 2.0);
	xform_schema.x_max_ = (w() * dpp_r / 2.0);
	xform_schema.y_min_ = -(h() * dpp_r / 2.0);
	xform_schema.y_max_ = (h() * dpp_r / 2.0);
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	//double rmax_x = plot_w * dpp_r / 2.0;
	//double rmax_y = plot_h * dpp_r / 2.0;
	plot_data_[1].data_area.display_min = { -r_max, -r_max };
	plot_data_[1].data_area.display_max = { r_max, r_max };
	// Set the axis sizes and positions for the axes. For polar coordinates, we will set the R axis along the horizontal and the Theta axis along the vertical.
	// Radius axis at "Y" = 0;
	axes_data_[0].position = 0.0;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_r;
	axes_data_[0].label_position = { r_max / 2.0F, -axis_width_ * dpp_r * 0.5 };
	axes_data_[0].label_angle = 0;
	// Theta axis at rmax 
	// data per pixel treated the number of degrees per circumferential pixel.
	double dpp_theta = 180.0 / zc::PI / radius_pixels;
	axes_data_[1].position = r_max;
	axes_data_[1].tick_orientation = TICK_INCREASING;
	axes_data_[1].inv_scale = dpp_theta;
	axes_data_[1].label_position = { 0.0, -r_max - axis_width_ * dpp_r * 0.5 };
	axes_data_[1].label_angle = 0;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_polar::get_layout_area(int mouse_x, int mouse_y) const {
	// TODO: Do we need to check the mouse is within the circular plot area, or just return true?
	return { true, -1 };
}

// Add a marker for Polar graphs
void zc_graph_polar::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// Get the other axis number: If axis 0 (R) then axis 1 (Theta).
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = axes_data_[other_axis_number];

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Circle at R = value_1
			plot_arc_t arc = { 0, 0, marker.value_1, 0, 360 };
			marker_line.segments.push_back(plot_segment_t(arc));
		}
		else {
			// Radial line at Theta = value_1
			plot_vertex_t origin(0.0, 0.0);
			marker_line.segments.push_back(origin);
			data_point_t point1(other_axis_data.outer_range.second, marker.value_1);
			plot_vertex_t vertex1(convert_point(point1));
			marker_line.segments.push_back(vertex1);
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		plot_object_t marker_shape;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			// Draw an annulus from radius = value_1 to value_2
			marker_shape.shape = COMPLEX;
			double r0 = std::max(marker.value_1, marker.value_2);
			double r1 = std::min(marker.value_1, marker.value_2);
			plot_arc_t arc = { 0, 0, r0, 0, 360 };
			marker_shape.segments.push_back(plot_segment_t(arc));
			// Add a gap to break the line strip, then add the inner arc in the opposite direction to create a filled annulus.
			marker_shape.segments.push_back(plot_segment_t(true));
			plot_arc_t inner_arc = { 0, 0, r1, 360, 0 };
			marker_shape.segments.push_back(plot_segment_t(inner_arc));
		}
		else {
			// Draw a wedge from Theta = value_1 to value_2
			marker_shape.shape = COMPLEX;
			plot_vertex_t origin(0.0, 0.0);
			marker_shape.segments.push_back(origin);
			plot_arc_t arc = { 0, 0, other_axis_data.outer_range.second, marker.value_1, marker.value_2 };
			marker_shape.segments.push_back(plot_segment_t(arc));
			// Add the origin again to create a closed shape for the sector.
			marker_shape.segments.push_back(origin);
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

// Get tick angle
int zc_graph_polar::get_tick_angle(
	int axis_number,
	tick_orientation_t tick_orientation,
	double tick_value) {
	// Ticks for the radius axis (axis 0) should be vertical.
	if (axis_number == 0) {
		if (tick_orientation == TICK_INCREASING) {
			return 90; // Ticks pointing upwards
		}
		else {
			return 270; // Ticks pointing downwards
		}
	}
	else {
		int result;
		// Ticks for the theta axis (axis 1) should be radial, so the angle of the tick is equal to the tick value.
		if (tick_orientation == TICK_INCREASING) {
			result = static_cast<int>(tick_value); // Ticks pointing in the direction of increasing theta
		}
		else {
			result = static_cast<int>(tick_value) + 180; // Ticks pointing in the direction of decreasing theta
		}
		return result < 0 ? result + 360 : (result >= 360 ? result - 360 : result); // Normalize the angle to be between 0 and 360 degrees
	}
}

void zc_graph_polar::set_click_value(int mouse_x, int mouse_y) {
	// Convert the mouse coordinates to cartesian coordinates
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Convert the cartesian coordinates to polar coordinates
	double r = std::sqrt(cx * cx + cy * cy);
	double theta = std::atan2(cy, cx) * 180.0 / zc::PI; // Convert to degrees
	value_ = { r, theta };
}

// Smith chart layout
void zc_graph_smith::layout() {
	// For Smith chart the data is plotted in real and imaginary coordinates,
	// with limits of +/- 1.0. This layout will force the plot area to be square
	// and override the current_data values to set the display range to -1.0 to 1.0.
	// Axes and grid-lines will be drawn at the lines of constant resistance
	// and reactance.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y() + axis_width_;
	plot_w_ = w() - 2 * v_axis_width_;
	plot_h_ = h() - 2 * axis_width_;
	double s11_max = 1.0; // The maximum value for the S11 parameter in the Smith chart is 1.0, which corresponds to total reflection.
	// data per pixel values
	double radius_pixels = std::min(plot_w_, plot_h_) / 2.0;
	double dpp_r = s11_max / radius_pixels;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = -(w() * dpp_r / 2.0);
	xform_schema.x_max_ = (w() * dpp_r / 2.0);
	xform_schema.y_min_ = -(h() * dpp_r / 2.0);
	xform_schema.y_max_ = (h() * dpp_r / 2.0);
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	//double rmax_x = plot_w * dpp_r / 2.0;
	//double rmax_y = plot_h * dpp_r / 2.0;
	plot_data_[1].data_area.display_min = { -s11_max, -s11_max };
	plot_data_[1].data_area.display_max = { s11_max, s11_max };
	// Set the axis sizes and positions for the axes. For Smith chart, 
	// we will set the Resistance axis along the horizontal and the
	// Reactance axis along the circumferennce.
	// Resistance axis at "Y" = 0;
	axes_data_[0].position = 0.0;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_r;
	axes_data_[0].label_position = { 0.0, -axis_width_ * dpp_r * 0.5 };
	axes_data_[0].label_angle = 0;
	// Theta axis at rmax 
	// data per pixel treated the number of degrees per circumferential pixel.
	double dpp_theta = 180.0 / zc::PI / radius_pixels;
	axes_data_[1].position = 0.0;
	axes_data_[1].tick_orientation = TICK_INCREASING;
	axes_data_[1].inv_scale = dpp_theta;
	axes_data_[1].label_position = { 0.0, -s11_max - axis_width_ * dpp_r * 0.5 };
	axes_data_[1].label_angle = 0;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_smith::get_layout_area(
	int mouse_x, 
	int mouse_y) const {
	// TODO: Do we need to check the mouse is within the circular plot area, or just return true?
	return { true, -1 };
}

// Markers for the Smith chart will typically be constant resistance 
// and reactance circles.
// A line of constant resistance R will be a circle with centre 
// at (R/(R+1), 0) and radius 1/(R+1).
// A line of constant reactance X will be an arc with centre 
// at (1, 1/X) and radius 1/X. The arc will be above the real 
// axis for positive X and below the real axis for negative X.
// The angle at which the arc intersects the circle for R=0 is
// derived from coordinates of that point and the centre of the arc.
void zc_graph_smith::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// Check the axis data already exists for this axis number
	// Do not support shaded area markers for the Smith chart, so check the values are the same.
	if (marker.value_1 != marker.value_2) {
		throw std::invalid_argument("Smith chart does not support shaded area markers. Value 1 and Value 2 must be the same.");
		return;
	}
	// Lines of constant resistance are circles centred on the real axis.
	if (axis_number == 0) {
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		double R = marker.value_1;
		double center_x = R / (R + 1);
		double center_y = 0.0;
		double radius = 1.0 / (R + 1);
		plot_arc_t arc = { center_x, center_y, radius, 0, 360 };
		marker_line.segments.push_back(plot_segment_t(arc));
		plot_data_[1].layer_data[layer].push_back(marker_line);
	}
	else {
		// Lines of constant reactance are arcs centred on the point (1, 1/X).
		if (marker.value_1 == 0) {
			// For a reactance of 0, the line is the real axis, so we can just draw a horizontal line at Y=0.
			plot_object_t marker_line;
			marker_line.shape = LINE_STRIP;
			marker_line.style = marker.style;
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(-1.0, 0.0)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(1.0, 0.0)));
			plot_data_[1].layer_data[layer].push_back(marker_line);
			return;
		}
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		double X = marker.value_1;
		double center_x = 1.0;
		double center_y = 1.0 / X;
		double radius = std::abs(1.0 / X);
		data_point_t intersection_point = gamma(0.0, X);
		double start_angle;
		double end_angle;
		double dx = intersection_point.first - center_x;
		double dy = intersection_point.second - center_y;
		// Arcs are drawn counter-clockwise.
		// For positive X, the arc is above the real axis and starts at 
		// the intersection with the R=0 circle and ends at the intersection
		// with the R=inf circle. For negative X, the arc is below the real
		// axis and starts at the intersection with the R=inf circle
		// and ends at the intersection with the R=0 circle.
		// The R=inf circle is a single dot at the point (1, 0) so is
		// directly above or below the centre of the arc, and the angle 
		// to that point is either 90 or 270 degrees.
		if (X > 0) {
			start_angle = std::atan2(dy, dx) * 180.0 / zc::PI;
			if (start_angle < 0) {
				start_angle += 360.0;
			}
			end_angle = 270.0;
		}
		else {
			start_angle = 90.0;
			end_angle = std::atan2(dy, dx) * 180.0 / zc::PI;
			if (end_angle < 0) {
				end_angle += 360.0;
			}
		}
		// The following is a frig. It appears that using fl_arc with
		// negative y scaling causes the arc to be drawn mirror-imaged
		// about the horizontal axis.
		end_angle = 360.0 - end_angle;
		start_angle = 360.0 - start_angle;
		plot_arc_t arc = { center_x, center_y, radius, start_angle, end_angle };
		marker_line.segments.push_back(plot_segment_t(arc));
		plot_data_[1].layer_data[layer].push_back(marker_line);
	}
	return;
}

//! This needs to generate a set of tick points that are sort of equally spaced
//! between 0 and infinity for R, and +/- infinity for X.
//! mark all tick points as major so that grid lines will be drawn for each.
void zc_graph_smith::set_ticks(
	int axis_number,
	int tick_spacing_pixels,
	double inv_scale
) {
	// Implementation for setting ticks on the Smith chart
	// TODO:
	// The nanovna-saver has fixed tick values for the Smith chart, at
	// R = 0, 0.2, 0.5, 1, 2, 3 and 5, and X = +/- 0.2, 0.5, 1, 2, 3 and 5.
	// I want to be able to calculate tick values based on the tick spacing in pixels.
	// For now use the same fixed tick values as nanovna-saver.
	// Check the current range is valid
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.is_valid()) {
		// Current range is not valid, throw an error
		throw std::invalid_argument("Current range is not valid for axis number " + std::to_string(axis_number) + ". Set axis ranges before setting ticks.");
		return;
	}
	if (axis_number == 0) {
		// Resistance axis - ticks at R = 0, 0.2, 0.5, 1, 2, 3 and 5.
		std::vector<double> tick_values = { 0.0, 0.2, 0.5, 1.0, 2.0, 3.0, 5.0 };
		for (double tick_value : tick_values) {
			// Format the tick label based on the modifier.
			char label[20];
			snprintf(label, sizeof(label), "%g", tick_value);
			// Calculate the pixel position of the tick and add it to the list of ticks.
			axis_data.ticks.push_back({ tick_value, std::string(label), true });
		}
	}
	else {
		// Reactance axis - ticks at X = +/- 0.2, 0.5, 1, 2, 3 and 5.
		std::vector<double> tick_values = { -5.0, -3.0, -2.0, -1.0, -0.5, -0.2, 
											   0.2, 0.5, 1.0, 2.0, 3.0, 5.0 };
		for (double tick_value : tick_values) {
			// Format the tick label based on the modifier.
			char label[20];
			snprintf(label, sizeof(label), "%g", tick_value);
			// Calculate the pixel position of the tick and add it to the list of ticks.
			axis_data.ticks.push_back({ tick_value, std::string(label), true });
		}
	}
}


// Calculate the tick angle for the Smith chart. For the resistance axis, ticks should be vertical. For the reactance axis, ticks should be radial, so the angle of the tick is equal to the tick value.
int zc_graph_smith::get_tick_angle(
	int axis_number,
	tick_orientation_t tick_orientation,
	double tick_value) {
	// Ticks for the resistance axis (axis 0) should be vertical.
	if (axis_number == 0) {
		if (tick_orientation == TICK_INCREASING) {
			return 90; // Ticks pointing upwards
		}
		else {
			return 270; // Ticks pointing downwards
		}
	}
	else {
		// Ticks for the reactance axis (axis 1) should be radial, so the angle of the tick is equal to the tick value.
		// The tick value is in data units, so we need to convert it to degrees. 
		// The tick value is the reactance value, 
		// Get the data coords of the tick value.
		data_point_t tick_data_coords = gamma(0.0, tick_value);
		int tick_angle = std::atan2(tick_data_coords.second, tick_data_coords.first) * 180.0 / zc::PI;
		if (tick_orientation == TICK_DECREASING) {
			tick_angle += 180; // Ticks pointing in the direction of decreasing theta
		}
		return tick_angle < 0 ? tick_angle + 360 : (tick_angle >= 360 ? tick_angle - 360 : tick_angle); // Normalize the angle to be between 0 and 360 degrees
	}
}

// Calculate the S11 parameter for a given resistance and reactance, for use in 
// generating the constant resistance and reactance markers on the Smith chart.
zc_graph_::data_point_t zc_graph_smith::gamma(double value_r, double value_x) const {
	// The following code is LLM generated. It may save some CPU cycles to
	// avoid translating into a complex number and back. But this is clearer 
	// and less error-prone.

	// The S11 parameter is calculated from the resistance and reactance using the formula:
	// S11 = (Z - Z0) / (Z + Z0)
	// where Z is the complex impedance given by R + jX, and Z0 is the characteristic impedance, which we will assume to be 1.0 for a normalized Smith chart.
	std::complex<double> z(value_r, value_x);
	std::complex<double> one(1.0, 0.0);
	std::complex<double> result = (z - one) / (z + one);
	return { result.real(), result.imag() };
}

void zc_graph_smith::set_click_value(int mouse_x, int mouse_y) {
	// Convert the mouse coordinates to data coordinates
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Store the click values
	value_ = { cx, cy };
}

void zc_graph_density::layout() {
	zc_graph_cartesian::layout();
	// Copy plot data for axis 1 to axis 2, which is used for density plots. We will use the same transformation schema for axis 2 as axis 1, but we will calculate the Z values for each pixel based on the data sets for this axis number and store them in the xform_schema for axis 2.
	plot_data_[2] = plot_data_[1];
	// For density plots, we add the density contribution for each coordinate.
	// For each pixel in the bitmap, calculate the corresponding X and Y values based on the current ranges for the X and Y axes,
	// then calculate the Z value at that point based on the data sets for this axis number, and set the pixel colour based on the Z value.
	int index = 0;
	density_xform_t& z_xform = plot_data_[2].xform_schema.z_xform;
	auto data_copy = *density_data_set_; // Make a copy of the data set for this axis number.
	z_xform.resize(plot_w_ * plot_h_);
	// We can assume that the data sets for this axis number are sorted by X and Y values.
	for (int y = 0; y < plot_h_; y++) {
		for (int x = 0; x < plot_w_; x++) {
			int pixel_index = y * plot_w_ + x;
			auto& z_xform_pt = z_xform[pixel_index];
			data_point_t point = pixel_to_data(2, plot_x_ + x, plot_y_ + y);
			double z_value = 0.0;
			// Calculate the Z value at this point based on the data sets for this axis number.
			// Use bilinear interpolation between the four nearest data points in the data sets for this axis number to calculate the Z value at this point.
			// density_data_set_ is a pointer to a vector of data_point_3d_t (tuples of x, y, z values)

			if (!data_copy.x_values.empty() && !data_copy.y_values.empty()) {
				// Find the bounding box indices for the current point
				// We need to find the four points (x0,y0), (x1,y0), (x0,y1), (x1,y1) that surround point(x,y)

				// Use binary search (O(log n)) instead of linear search (O(n))
				auto x_it = std::lower_bound(data_copy.x_values.begin(), data_copy.x_values.end(), point.first);
				size_t x_idx = 0;
				if (x_it != data_copy.x_values.begin()) {
					x_idx = std::distance(data_copy.x_values.begin(), x_it) - 1;
				}

				auto y_it = std::lower_bound(data_copy.y_values.begin(), data_copy.y_values.end(), point.second);
				size_t y_idx = 0;
				if (y_it != data_copy.y_values.begin()) {
					y_idx = std::distance(data_copy.y_values.begin(), y_it) - 1;
				}

				// Get the four corner coordinates
				double x0 = data_copy.x_values[x_idx];
				double x1 = (x_idx + 1 < data_copy.x_values.size()) ? data_copy.x_values[x_idx + 1] : x0;
				double y0 = data_copy.y_values[y_idx];
				double y1 = (y_idx + 1 < data_copy.y_values.size()) ? data_copy.y_values[y_idx + 1] : y0;

				// Perform bilinear interpolation
				if (x1 != x0 && y1 != y0) {
					// Calculate normalized position within the cell
					double fx = (point.first - x0) / (x1 - x0);
					double fy = (point.second - y0) / (y1 - y0);

					// Clamp to [0,1] range
					fx = std::clamp(fx, 0.0, 1.0);
					fy = std::clamp(fy, 0.0, 1.0);


					z_xform_pt.push_back({ x_idx, y_idx, (1.0 - fx) * (1.0 - fy) });
					z_xform_pt.push_back({ x_idx + 1, y_idx, fx * (1.0 - fy) });
					z_xform_pt.push_back({ x_idx, y_idx + 1, (1.0 - fx) * fy });
					z_xform_pt.push_back({ x_idx + 1, y_idx + 1, fx * fy });
				}
				else if (x1 == x0 && y1 != y0) {
					// Linear interpolation in y direction only
					double fy = (point.second - y0) / (y1 - y0);
					fy = std::clamp(fy, 0.0, 1.0);
					z_xform_pt.push_back({ x_idx, y_idx, (1.0 - fy) });
					z_xform_pt.push_back({ x_idx, y_idx + 1, fy });
				}
				else if (y1 == y0 && x1 != x0) {
					// Linear interpolation in x direction only
					double fx = (point.first - x0) / (x1 - x0);
					fx = std::clamp(fx, 0.0, 1.0);
					z_xform_pt.push_back({ x_idx, y_idx, (1.0 - fx) });
					z_xform_pt.push_back({ x_idx + 1, y_idx, fx });
				}
				else {
					// Point exactly on a data point
					z_xform_pt.push_back({ x_idx, y_idx, 1.0 });
				}
			}
		}
	}
	density_bitmap_.resize(plot_w_ * plot_h_ * 3); // RGB bitmap
}

// Layout vertical bar chart
void zc_graph_bar_vertical::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	// calculate pixel dimensions for the plot area.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y();
	plot_w_ = w() - v_axis_width_;
	plot_h_ = h() - axis_width_;
	double x_min = axes_data_[0].current_range.first;
	// Add 1 to the maximum to allow the full bar width to be displayed.
	double x_max = axes_data_[0].current_range.second + 1;
	double y_min = axes_data_[1].current_range.first;
	double y_max = axes_data_[1].current_range.second;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max;
	xform_schema.y_min_ = y_min - axis_width_ * dpp_y;
	xform_schema.y_max_ = y_max;
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	plot_data_[1].data_area.display_min = { x_min, y_min };
	plot_data_[1].data_area.display_max = { x_max, y_max };
	// Set the axis sizes and positions for the axes.
	axes_data_[0].position = y_min;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_x;
	// Position the label in the middle of the axis, offset by 0.5 times the axis width in the appropriate direction.
	// This is because the axis width is two text heights, so 0.5 times the axis width is 1 text height, 
	// which should give a good distance between the label and the axis.
	axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, y_min - axis_width_ * dpp_y * 0.5 };
	axes_data_[0].label_angle = 0;
	axes_data_[1].position = x_min;
	axes_data_[1].tick_orientation = TICK_DECREASING;
	axes_data_[1].inv_scale = dpp_y;
	axes_data_[1].label_position = { x_min - v_axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	axes_data_[1].label_angle = 90;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_bar_vertical::get_layout_area(int mouse_x, int mouse_y) const {
	layout_area_t result = { false, -1 };
	// Check if the mouse is over the plot area.
	if (mouse_x >= plot_x_ && mouse_x <= plot_x_ + plot_w_ &&
		mouse_y >= plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { true, -1 };
	}
	// Check if the mouse is over either of the axes.
	else if (mouse_x >= x() && mouse_x < plot_x_) {
		result = { false, 1 };
	}
	else if (mouse_y > plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { false, 0 };
	}
	return result;
}

// Add a marker for vertical bar graphs
void zc_graph_bar_vertical::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// Get the other axis number: If axis 0 then axis 1.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}

	// Ignore for the bar axis
	if (axis_data.is_bar_axis) return;

	axis_data_t& other_axis_data = axes_data_[other_axis_number];

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_bar_vertical::set_click_value(int mouse_x, int mouse_y) {
	// Convert the mouse coordinates to data coordinates
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Store the click values
	value_ = { std::floor(cx), cy };
}

// Layout vertical bar chart
void zc_graph_bar_horizontal::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	// calculate pixel dimensions for the plot area.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y();
	plot_w_ = w() - v_axis_width_;
	plot_h_ = h() - axis_width_;
	double x_min = axes_data_[0].current_range.first;
	// Add 1 to the maximum to allow the full bar width to be displayed.
	double x_max = axes_data_[0].current_range.second;
	double y_min = axes_data_[1].current_range.first;
	double y_max = axes_data_[1].current_range.second + 1;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max;
	xform_schema.y_min_ = y_min - axis_width_ * dpp_y;
	xform_schema.y_max_ = y_max;
	// Pre-calculate inverse scales for pixel_to_data conversion
	xform_schema.inv_scale_x_ = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	xform_schema.inv_scale_y_ = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	xform_schema.scale_x_ = 1.0 / xform_schema.inv_scale_x_;
	xform_schema.scale_y_ = 1.0 / xform_schema.inv_scale_y_;
	plot_data_[1].xform_schema = xform_schema;
	plot_data_[1].data_area.display_min = { x_min, y_min };
	plot_data_[1].data_area.display_max = { x_max, y_max };
	// Set the axis sizes and positions for the axes.
	axes_data_[0].position = y_min;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_x;
	// Position the label in the middle of the axis, offset by 0.5 times the axis width in the appropriate direction.
	// This is because the axis width is two text heights, so 0.5 times the axis width is 1 text height, 
	// which should give a good distance between the label and the axis.
	axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, y_min - axis_width_ * dpp_y * 0.5 };
	axes_data_[0].label_angle = 0;
	axes_data_[1].position = x_min;
	axes_data_[1].tick_orientation = TICK_DECREASING;
	axes_data_[1].inv_scale = dpp_y;
	axes_data_[1].label_position = { x_min - v_axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	axes_data_[1].label_angle = 90;
}

// Get the layout area under the mouse based on the current layout of the graph.
zc_graph_::layout_area_t zc_graph_bar_horizontal::get_layout_area(int mouse_x, int mouse_y) const {
	layout_area_t result = { false, -1 };
	// Check if the mouse is over the plot area.
	if (mouse_x >= plot_x_ && mouse_x <= plot_x_ + plot_w_ &&
		mouse_y >= plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { true, -1 };
	}
	// Check if the mouse is over either of the axes.
	else if (mouse_x >= x() && mouse_x < plot_x_) {
		result = { false, 1 };
	}
	else if (mouse_y > plot_y_ && mouse_y <= plot_y_ + plot_h_) {
		result = { false, 0 };
	}
	return result;
}

// Add a marker for vertical bar graphs
void zc_graph_bar_horizontal::generate_value_marker(
	int axis_number,
	layer_t layer,
	const value_marker_t& marker
) {
	// Get the other axis number: If axis 0 then axis 1.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = axes_data_[axis_number];
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}

	// Ignore for the bar axis
	if (axis_data.is_bar_axis) return;

	axis_data_t& other_axis_data = axes_data_[other_axis_number];

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.first)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.second)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.first)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.second, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.first, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_bar_horizontal::set_click_value(int mouse_x, int mouse_y) {
	// Convert the mouse coordinates to data coordinates
	double cx = plot_data_[1].xform_schema.x_min_ + (mouse_x - x()) * (plot_data_[1].xform_schema.x_max_ - plot_data_[1].xform_schema.x_min_) / w();
	double cy = plot_data_[1].xform_schema.y_max_ + (y() - mouse_y) * (plot_data_[1].xform_schema.y_max_ - plot_data_[1].xform_schema.y_min_) / h();
	// Store the click values
	value_ = { std::floor(cy), cx };
}


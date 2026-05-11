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
#include "zc_text_style.h"
#include "zc_utils.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Widget.H>

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>
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
}

// Set the ranges for the axes for a data type.
void zc_graph_::set_axis_ranges(
	int axis_number,                    //!< Axis number to set the ranges for (e.g. 0 for X or R axis, 1 for Y or Theta axis)
	const range_t& inner_range,              //!< Range of data values currently displayed for this axis (may be zoomed or scrolled)
	const range_t& outer_range,              //!< Range of data values for this axis (absolute minimum and maximum for zooming)
	const range_t& default_range             //!< Default range for this axis in the absence of data
) {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before setting axis ranges.");
		return;
	}
	// Check that the ranges are valid - ignore if inner_range is empty (i.e. min > max)
	// as this is used to there is no inner range.
	if (inner_range.is_valid()) {
		if (!default_range.contains(inner_range) ||
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
	axis_data_t& axis_data = it->second;
	axis_data.inner_range = inner_range;
	axis_data.outer_range = outer_range;
	axis_data.default_range = default_range;
}

// Get axis current range for a specific axis number.
zc_graph_::range_t zc_graph_::get_axis_range(int axis_number) const {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before getting axis range.");
		return range_t();
	}
	const axis_data_t& axis_data = it->second;
	return axis_data.current_range;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a data set.");
		return;
	}
	data_set_t data_set = { data, style };
	data_sets_[axis_number].push_back(data_set);
};

//! \brief Add a value marker to the graph for a specific axis number.
void zc_graph_::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	double value_1,
	double value_2
) {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a value marker.");
		return;
	}
	// Create the marker object
	value_marker_t marker;
	marker.style = style;
	marker.value_1 = value_1;
	marker.value_2 = value_2;
	value_markers_[axis_number][layer].push_back(marker);
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a label.");
		return;
	}
	// Get the other axis number: If axis 0 then the primary axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the other axis data already exists for the other axis number
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		// Other axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a label.");
		return;
	}
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
	plot_data_.clear();
	// Clear the axis data for all axes
	axes_data_.clear();
	// Clear the value markers for all axes and layers
	value_markers_.clear();
	// Clear the point markers for all axes and layers
	point_markers_.clear();
}

// Initiaite the plot data
void zc_graph_::end_config() {
	axis_width_ = default_text_size_ * 2; // Default width of the axes is twice the default text size.
	v_axis_width_ = default_text_size_ * 3; // Default width of the vertical axis is thrice the default text size.
	// Update the current ranges for each axis to include the default ranges, if they are not already included.
	for (auto& [axis_number, axis_data] : axes_data_) {
		axis_data.current_range |= axis_data.default_range;
	}
	// Clear the plot data for all data types and layers.
	plot_data_.clear();

	redraw();
}

//! \brief Normalise the value \p fin to a mantissa and power of 10.
void zc_graph_::normalise(double fin, modifier_t modifier, double& mantissa, double& power10, uint32_t& si_prefix_exponent) const {
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before generating an axis line.");
		return;
	}
	axis_data_t& axis_data = it->second;
	// Generate the axis line for this axis number based on the current ranges and tick spacing
	value_marker_t marker;
	marker.style = zc_line_style({ FL_BLACK, 1, FL_SOLID });
	marker.value_1 = axis_data.position;
	marker.value_2 = axis_data.position;
	int drawing_axis_number = (axis_number == 0) ? 1 : 0; // The axis number to draw the line along is the other axis
	generate_value_marker(
		drawing_axis_number,
		AXES,
		marker
	);
}

// Generate grid_lines
void zc_graph_::generate_grid_lines(int axis_number) {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before generating grid lines.");
		return;
	}
	axis_data_t& axis_data = it->second;
	// Generate the grid lines for this axis number based on the current ranges and tick spacing
	zc_line_style grid_line_style({ FL_LIGHT2, 1, FL_DOT });
	for (auto& tick : axis_data.ticks) {
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before setting ticks.");
		return;
	}
	// Check the current range is valid
	if (!it->second.current_range.is_valid()) {
		// Current range is not valid, throw an error
		throw std::invalid_argument("Current range is not valid for axis number " + std::to_string(axis_number) + ". Set axis ranges before setting ticks.");
		return;
	}
	axis_data_t& axis_data = it->second;
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
	float tick_value = floorf(axis_data.current_range.min / tick_spacing) * tick_spacing;
	while (tick_value <= axis_data.current_range.max) {
		bool is_major = (fabsf(fmodf(tick_value, grid_spacing_units)) < 1e-6F);
		// Ignore tick_value less than the minimum.
		if (tick_value < axis_data.current_range.min) {
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before generating linear axis ticks.");
		return;
	}
	axis_data_t& axis_data = it->second;
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
			auto other_axis_it = axes_data_.find(1);
			axis_data_t& other_axis_data = other_axis_it->second;
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
			auto other_axis_it = axes_data_.find(0);
			axis_data_t& other_axis_data = other_axis_it->second;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before generating axis label.");
		return;
	}
	axis_data_t& axis_data = it->second;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a data set.");
		return;
	}
	// Check that axis 0 exists.
	auto axis_0_it = axes_data_.find(0);
	if (axis_0_it == axes_data_.end()) {
		// Axis 0 does not exist, throw an error
		throw std::invalid_argument("Axis number 0 does not exist. Set axis parameters for axis 0 before adding a data set.");
		return;
	}
	// Delete existing data lines for this axis number, if any.
	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the data along is the other axis
	plot_data_[plot_number].layer_data[DATA].clear();

	for (const auto& data_set : data_sets_[axis_number]) {
		axis_data_t& axis_data = it->second;
		axis_data_t& axis_0_data = axis_0_it->second;
		// Add the data set to the plot data for this axis number
		// Create a new plot line for this data set.
		plot_object_t plot_line;
		plot_line.shape = LINE_STRIP;
		plot_line.style = data_set.style;

		// Add a vertex for each data point in the data set.
		// Include the data points in the current ranges for the axes.
		for (const auto& point : *data_set.data) {
			//// Check that the point is within the current ranges for both axes. If not, skip it.
			//if (!axis_data.current_range.contains(point.second) ||
			//	!axis_0_data.current_range.contains(point.first)) {
			//	continue;
			//}
			// Add the point to the default ranges for both axes, if it's within the outer range for that axis. This ensures that if the point is outside the current range but within the outer range.
			if (axis_data.outer_range.contains(point.second)) {
				axis_data.default_range |= point.second;
			}
			if (axis_0_data.outer_range.contains(point.first)) {
				axis_0_data.default_range |= point.first;
			}
			// Now add the point to the plot line as a vertex. Convert to Cartesian coordinates if necessary.
			plot_vertex_t vertex;
			vertex = plot_vertex_t(convert_point(point));
			plot_segment_t segment(vertex);
			plot_line.segments.push_back(segment);
		}
		// Add the plot line to the plot data for this axis number.
		plot_data_[plot_number].layer_data[DATA].push_back(plot_line);
	}

}

//! \brief Add a text label to the graph at a specific position.
void zc_graph_::generate_point_markers(
	int axis_number
) {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a label.");
		return;
	}
	// Get the other axis number: If axis 0 then the primary axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the other axis data already exists for the other axis number
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		// Other axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a label.");
		return;
	}
	for (auto& point_marker : point_markers_[axis_number]) {
		layer_t layer = point_marker.first;
		auto& point_data = point_marker.second;
		// Check the position is within the outer range for both axes
		axis_data_t& axis_data = it->second;
		axis_data_t& other_axis_data = other_axis_it->second;
		axis_data_t& x_axis_data = (axis_number == 0) ? axis_data : other_axis_data;
		axis_data_t& y_axis_data = (axis_number == 0) ? other_axis_data : axis_data;
		// Fore each point marker...
		for (const auto& point_datum : point_data) {
			// Add a point marker at the specified position
			plot_object_t marker;
			marker.shape = TEXT;
			marker.text = point_datum.text;
			marker.text_style = point_datum.style;
			marker.text_alignment = point_datum.alignment;
			// If the point is set to maximum for either axis, move it to the edge of the current range for that axis.
			data_point_t position = point_datum.position;
			if (position.second == -DBL_MAX) {
				position.second = y_axis_data.current_range.min;
			}
			else if (position.second == DBL_MAX) {
				position.second = y_axis_data.current_range.max;
			} 
			if (position.first == -DBL_MAX) {
				position.first = x_axis_data.current_range.min;
			}
			else if (position.first == DBL_MAX) {
				position.first = x_axis_data.current_range.max;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a label.");
		return;
	}
	// Get the other axis number: If axis 0 then the primary axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	// Check the other axis data already exists for the other axis number
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		// Other axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a label.");
		return;
	}
	for (auto& axis_marker : value_markers_[axis_number]) {
		layer_t layer = axis_marker.first;
		auto& layer_markers = axis_marker.second;
		for (const auto& value_datum : layer_markers) {
			generate_value_marker(axis_number, layer, value_datum);
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
// TODO: Implement and test the major functionality before adding 
// zooming and scrolling
int zc_graph_::handle(int event) {
	// TODO: Using the return value from zoom and scroll is not ideal as
	// it is not clear whether the event was handled by the axis or not only
	// that it was completely successful.
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
				for (auto& it : axes_data_) {
					zoom_axis(it.first, mouse_x, mouse_y, dy);
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
			// If this is a double-click, reset the axis under the mouse to
			// the default range.
			layout_area_t under_mouse = get_layout_area(Fl::event_x(), Fl::event_y());
			if (!under_mouse.is_plot_area) {
				// If the double-click was on an axis, reset that axis to its default range.
				reset_zoom(under_mouse.axis_number);
				redraw();
				return 1;
			}
			else {
				// If the double-click was on the plot, reset all axes to their default range.
				for (auto& it : axes_data_) {
					reset_zoom(it.first);
				}
				redraw();
				return 1;
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
			scroll_axis(under_mouse.axis_number, (Fl::event_state() & FL_SHIFT) ? dy * 10 : dy);
			redraw();
			return 1;
		} 
		else {
			// If left mouse button is held and the mouse is dragged on the plot,
			// Scroll on horizontal and leftwards vertical axes.
			// Other axes will ignore scroll.
			if (Fl::event_button() == FL_LEFT_MOUSE) {
				scroll_axis(0, dx);
				scroll_axis(1, dy);
				redraw();
				return 1;
			}
			else if (Fl::event_button() == FL_RIGHT_MOUSE) {
				scroll_axis(0, dx);
				if (axes_data_.find(2) != axes_data_.end()) {
					scroll_axis(2, dy);
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
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before zooming.");
		return;
	}
	axis_data_t& axis_data = it->second;
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
	range_t new_range;
	// Calculate the new range based on the zoom change and mouse position.
	if (is_axis_horizontal(axis_number)) {
		double new_min = mouse_position.first - (mouse_position.first - axis_data.current_range.min) * zoom_change;
		double new_max = mouse_position.first + (axis_data.current_range.max - mouse_position.first) * zoom_change;
		new_range = range_t{ new_min, new_max };
	}
	else {
		double new_min = mouse_position.second - (mouse_position.second - axis_data.current_range.min) * zoom_change;
		double new_max = mouse_position.second + (axis_data.current_range.max - mouse_position.second) * zoom_change;
		new_range = range_t{ new_min, new_max };
	}
	if (axis_data.outer_range.is_valid()) {
		new_range &= axis_data.outer_range;
	}
	if (axis_data.inner_range.is_valid()) {
		new_range |= axis_data.inner_range;
	}
	axis_data.current_range = new_range;
}

// Scroll the axis under the mouse by the specified offset in pixels.
void zc_graph_::scroll_axis(int axis_number, int scroll_offset) {
	if (!scrollable_ || axis_number == -1) {
		return;
	}
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before scrolling.");
		return;
	}
	axis_data_t& axis_data = it->second;
	// Calculate the new range based on the scroll offset and current scale.
	double scroll_amount = scroll_offset * axis_data.inv_scale;
	double new_min, new_max;
	// Limit scroll amount if we are close to the outer limit.
	if (scroll_amount > 0.0) {
		if (axis_data.current_range.max + scroll_amount > axis_data.outer_range.max) {
			scroll_amount = axis_data.outer_range.max - axis_data.current_range.max;
		}
	} else {
		if (axis_data.current_range.min + scroll_amount < axis_data.outer_range.min) {
			scroll_amount = axis_data.outer_range.min - axis_data.current_range.min;
		}
	}
	new_min = axis_data.current_range.min + scroll_amount;
	new_max = axis_data.current_range.max + scroll_amount;
	range_t new_range = axis_data.outer_range & range_t{ new_min, new_max };
	axis_data.current_range = new_range;
}

// Reset the zoom for the specified axis number to the default range.
void zc_graph_::reset_zoom(int axis_number) {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before resetting zoom.");
		return;
	}
	axis_data_t& axis_data = it->second;
	axis_data.current_range = axis_data.default_range;
}

// Resize the widget - reset scaling factors
void zc_graph_::resize(int X, int Y, int W, int H) {
	// If we have actually resized...
	if (X != x() || Y != y() || W != w() || H != h()) {
		Fl_Widget::resize(X, Y, W, H);
		redraw();
	}
}

// draw the widget
void zc_graph_::draw() {

	// Clear the plot data for all data types and layers.
	plot_data_.clear();
	// Set out the positions of the axes and plot area, set the 
	// drawing transformation schemata.
	layout();
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
		for (const auto& data_pair : plot_data_) {
			// If there is data for this layer and data type, draw it.
			if (data_pair.second.layer_data.find(layer) == data_pair.second.layer_data.end()) {
				continue;
			}
			// Set the transformation for this data type based on the axis parameters
			fl_push_matrix();
			apply_transformations(data_pair.second.xform_schema);
			const std::vector<plot_object_t>& layer_data = data_pair.second.layer_data.at(layer);
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

//! \brief Apply the necessary transformations to the FLTK drawing context based on the specified transformation schema.
void zc_graph_::apply_transformations(plot_xform_t schema) {
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
	// Check the plot data already exists for this axis number
	auto plot_data_it = plot_data_.find(drawing_axis_number);
	if (plot_data_it == plot_data_.end()) {
		// Plot data does not exist for this axis number, throw an error
		throw std::invalid_argument("Plot data does not exist for axis number " + std::to_string(drawing_axis_number) + ". Set axis parameters and add data sets before converting pixel to data coordinates.");
		return { 0.0F, 0.0F };
	}
	const plot_xform_t& xform_schema = plot_data_it->second.xform_schema;
	double inv_scale_x = (xform_schema.x_max_ - xform_schema.x_min_) / w();
	double inv_scale_y = (xform_schema.y_min_ - xform_schema.y_max_) / h();
	double data_x = xform_schema.x_min_ + (pixel_x - x()) * inv_scale_x;
	double data_y = xform_schema.y_max_ + (pixel_y - y()) * inv_scale_y;
	return { data_x, data_y };
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
			// Draw a box around the text using the rotated corners.
			fl_color(color());
			fl_begin_polygon();
			for (int i = 0; i < 4; ++i) {
				fl_transformed_vertex(corners[i][0], corners[i][1]);
			}
			fl_transformed_vertex(corners[0][0], corners[0][1]);
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
	double x_min = axes_data_[0].current_range.min;
	double x_max = axes_data_[0].current_range.max;
	double y_min = axes_data_[1].current_range.min;
	double y_max = axes_data_[1].current_range.max;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max;
	xform_schema.y_min_ = y_min - axis_width_ * dpp_y;
	xform_schema.y_max_ = y_max;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a marker.");
		return;
	}
	// Get the other axis number: If axis 0 then axis 1.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a marker.");
		return;
	}
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = it->second;
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.max)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_cartesian_2y::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y();
	plot_w_ = w() - 2 * v_axis_width_;
	plot_h_ = h() - axis_width_;
	double x_min = axes_data_[0].current_range.min;
	double x_max = axes_data_[0].current_range.max;
	double y_min = axes_data_[1].current_range.min;
	double y_max = axes_data_[1].current_range.max;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max + v_axis_width_ * dpp_x;
	xform_schema.y_min_ = y_min - axis_width_ * dpp_y;
	xform_schema.y_max_ = y_max;
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
	double y2_min = axes_data_[2].current_range.min;
	double y2_max = axes_data_[2].current_range.max;
	double dpp_y2 = (y2_max - y2_min) / h();
	plot_xform_t xform_schema_2;
	xform_schema_2.x_min_ = x_min - v_axis_width_ * dpp_x;
	xform_schema_2.x_max_ = x_max + v_axis_width_ * dpp_x;
	xform_schema_2.y_min_ = y2_min - axis_width_ * dpp_y2;
	xform_schema_2.y_max_ = y2_max;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a marker.");
		return;
	}
	// For axis 0 (X), use axis 1 (YL) as the other axis. For axis 1 or 2 (YL or YR), use axis 0 (X) as the other axis.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a marker.");
		return;
	}
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = it->second;
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.max)));
		}
		else {
			// Horizontal line at Y = value_1 (for axis 1 or 2)
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

// Cartesian overlay layout
void zc_graph_cart_overlay::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	plot_x_ = x();
	plot_y_ = y();
	plot_w_ = w();
	plot_h_ = h();
	double x_min = axes_data_[0].current_range.min;
	double x_max = axes_data_[0].current_range.max;
	double y_min = axes_data_[1].current_range.min;
	double y_max = axes_data_[1].current_range.max;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w_;
	double dpp_y = (y_max - y_min) / plot_h_;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min;
	xform_schema.x_max_ = x_max;
	xform_schema.y_min_ = y_min;
	xform_schema.y_max_ = y_max;
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a marker.");
		return;
	}
	// Get the other axis number: If axis 0 then axis 1.
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a marker.");
		return;
	}
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = it->second;
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (marker.value_1 == marker.value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = marker.style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.max)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = marker.style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_2, other_axis_data.current_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(marker.value_1, other_axis_data.current_range.min)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.max, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.current_range.min, marker.value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

// Polar layout
void zc_graph_polar::layout() {
	// For polar coordinates, we will set the transformation schema to map the R and Theta ranges to the plot area dimensions.
	plot_x_ = x() + v_axis_width_;
	plot_y_ = y() + axis_width_;
	plot_w_ = w() - 2 * v_axis_width_;
	plot_h_ = h() - 2 * axis_width_;
	double r_max = axes_data_[0].current_range.max;
	// data per pixel values
	double radius_pixels = std::min(plot_w_, plot_h_) / 2.0;
	double dpp_r = r_max / radius_pixels;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = -(w() * dpp_r / 2.0);
	xform_schema.x_max_ = (w() * dpp_r / 2.0);
	xform_schema.y_min_ = -(h() * dpp_r / 2.0);
	xform_schema.y_max_ = (h() * dpp_r / 2.0);
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a marker.");
		return;
	}
	// Get the other axis number: If axis 0 (R) then axis 1 (Theta).
	int other_axis_number = (axis_number == 0) ? 1 : 0;
	auto other_axis_it = axes_data_.find(other_axis_number);
	if (other_axis_it == axes_data_.end()) {
		throw std::invalid_argument("Other axis number " + std::to_string(other_axis_number) + " does not exist. Set axis parameters for both axes before adding a marker.");
		return;
	}
	// Check the values are within the outer range for this axis
	axis_data_t& axis_data = it->second;
	if (!axis_data.current_range.contains(marker.value_1) || !axis_data.current_range.contains(marker.value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

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
			data_point_t point1(other_axis_data.outer_range.max, marker.value_1);
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
			plot_arc_t arc = { 0, 0, other_axis_data.outer_range.max, marker.value_1, marker.value_2 };
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
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before adding a marker.");
		return;
	}
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
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before setting ticks.");
		return;
	}
	// Check the current range is valid
	if (!it->second.current_range.is_valid()) {
		// Current range is not valid, throw an error
		throw std::invalid_argument("Current range is not valid for axis number " + std::to_string(axis_number) + ". Set axis ranges before setting ticks.");
		return;
	}
	axis_data_t& axis_data = it->second;
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
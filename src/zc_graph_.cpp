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

#include "zc_line_style.h"
#include "zc_text_style.h"
#include "zc_utils.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Widget.H>

#include <algorithm>
#include <cmath>
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
	// Check that the ranges are valid
	if (!(inner_range & outer_range).is_valid() ||
		!(default_range & outer_range).is_valid() ||
		!(inner_range & default_range).is_valid()) {
		// Ranges do not overlap, throw an error
		throw std::invalid_argument("Supplied ranges do not overlap for axis number " + std::to_string(axis_number) + ".");
		return;
	}
	// Check that the default and inner ranges are within the outer range
	if (!outer_range.contains(inner_range) || !outer_range.contains(default_range)) {
		// Default or inner range is outside the outer range, throw an error
		throw std::invalid_argument("Inner and default ranges must be within the outer range for axis number " + std::to_string(axis_number) + ".");
		return;
	}
	// Set the ranges for this axis
	axis_data_t& axis_data = it->second;
	axis_data.inner_range = inner_range;
	axis_data.outer_range = outer_range;
	axis_data.default_range = default_range;
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
	data_set_t& data_set = data_sets_[axis_number];
	data_set.data = data;
	data_set.style = style;
};

//! \brief Add a text label to the graph at a specific position.
void zc_graph_::add_label(
	int axis_number,
	layer_t layer,
	const std::string& text,
	zc_text_style style,
	data_point_t position
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
	// Check the position is within the outer range for both axes
	axis_data_t& axis_data = it->second;
	axis_data_t& other_axis_data = other_axis_it->second;
	if (!axis_data.outer_range.contains(position.first) || !other_axis_data.outer_range.contains(position.second)) {
		// We cannot draw it, return.
		return;
	}
	// Add a text label at the specified position
	plot_object_t label;
	label.shape = TEXT;
	label.text = text;
	label.text_style = style;
	label.segments.push_back(plot_segment_t(plot_vertex_t(convert_point(position))));
	int plot_number = (axis_number == 0) ? 1 : axis_number;
	plot_data_[plot_number].layer_data[layer].push_back(label);
}

// Clear the plot_data
void zc_graph_::clear() {
	// Clear the plot data for all data types and layers
	plot_data_.clear();
	// Clear the axis data for all axes
	axes_data_.clear();

}

// Initiaite the plot data
void zc_graph_::initiate() {
	default_text_size_ = labelsize() * 0.8F; // Default text size for labels and ticks is 80% of the widget label size.
	axis_width_ = default_text_size_ * 2.0F; // Default width of the axes is twice the default text size.
	// Update the current ranges for each axis to include the default ranges, if they are not already included.
	for (auto& [axis_number, axis_data] : axes_data_) {
		axis_data.current_range |= axis_data.default_range;
	}
	layout();
	// Generate the axis lines, ticks, grid lines and labels for each axis based on the current parameters and ranges.
	for (int i = 0; i < num_axes_; ++i) {
		generate_axis_grid(i);
	}
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
	int drawing_axis_number = (axis_number == 0) ? 1 : 0; // The axis number to draw the line along is the other axis
	add_marker(drawing_axis_number, AXES, zc_line_style({ FL_BLACK, 1, FL_SOLID }), axis_data.position);
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
	int drawing_axis_number = (axis_number == 0) ? 1 : 0; // The axis number to draw the lines along is the other axis
	zc_line_style grid_line_style({ FL_LIGHT2, 1, FL_DOT });
	for (auto& tick : axis_data.ticks) {
		if (tick.is_major) {
			add_marker(drawing_axis_number, GRID_LINES, grid_line_style, tick.value);
		}
	}
}

// Generate ticks
void zc_graph_::set_ticks(int axis_number, int tick_spacing_pixels, int length_pixels) {
	// Check the axis data already exists for this axis number
	auto it = axes_data_.find(axis_number);
	if (it == axes_data_.end()) {
		// Axis data does not exist for this axis number, throw an error
		throw std::invalid_argument("Axis number " + std::to_string(axis_number) + " does not exist. Set axis parameters before setting ticks.");
		return;
	}
	axis_data_t& axis_data = it->second;
	axis_data.ticks.clear();
	// Calculate the tick spacing in data coordinates based on the desired spacing in pixels and the current
	double scale = (axis_data.current_range.max - axis_data.current_range.min) / length_pixels;
	double tick_spacing = std::abs(tick_spacing_pixels * scale);
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
	axis_data.label = ll;

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

	set_ticks(axis_number, axes_data_[axis_number].tick_spacing_pixels, (axis_number == 0) ? w() : h());

	for (const auto& tick : axis_data.ticks) {
		plot_object_t tick_mark;
		tick_mark.shape = TICK;
		tick_mark.style = zc_line_style({ FL_FOREGROUND_COLOR, 1, FL_SOLID });
		tick_mark.text = tick.label;
		tick_mark.text_style = zc_text_style({ FL_FOREGROUND_COLOR, 0, default_text_size_ });
		if (axis_number == 0) {
			auto other_axis_it = axes_data_.find(1);
			axis_data_t& other_axis_data = other_axis_it->second;
			// Horizontal axis - generate vertical ticks
			// Get the pixel coords of the tick mark start.
			if (axis_data.tick_orientation == TICK_DECREASING) {
				plot_vertex_t start(tick.value, axis_data.position);
				plot_vertex_t end(tick.value, axis_data.position - 5 * other_axis_data.inv_scale);
				tick_mark.segments.push_back(plot_segment_t(end));
				tick_mark.segments.push_back(plot_segment_t(start));
				tick_mark.text_alignment = ALIGN_BELOW;
			}
			else if (axis_data.tick_orientation == TICK_INCREASING) {
				plot_vertex_t start(tick.value, axis_data.position);
				plot_vertex_t end(tick.value, axis_data.position + 5 * other_axis_data.inv_scale);
				tick_mark.segments.push_back(plot_segment_t(end));
				tick_mark.segments.push_back(plot_segment_t(start));
				tick_mark.text_alignment = ALIGN_ABOVE;
			}
			else {
				plot_vertex_t start(tick.value, axis_data.position - 2.5 * other_axis_data.inv_scale);
				plot_vertex_t end(tick.value, axis_data.position + 2.5 * other_axis_data.inv_scale);
				tick_mark.segments.push_back(plot_segment_t(end));
				tick_mark.segments.push_back(plot_segment_t(start));
				tick_mark.text_alignment = ALIGN_BELOW;
			}
		}
		else {
			auto other_axis_it = axes_data_.find(0);
			axis_data_t& other_axis_data = other_axis_it->second;
			// Vertical axis - generate horizontal ticks
			if (axis_data.tick_orientation == TICK_DECREASING) {
				plot_vertex_t start(axis_data.position, tick.value);
				plot_vertex_t end(axis_data.position - 5 * other_axis_data.inv_scale, tick.value);
				tick_mark.segments.push_back(plot_segment_t(end));
				tick_mark.segments.push_back(plot_segment_t(start));
				tick_mark.text_alignment = ALIGN_LEFT;
			}
			else if (axis_data.tick_orientation == TICK_INCREASING) {
				plot_vertex_t start(axis_data.position, tick.value);
				plot_vertex_t end(axis_data.position + 5 * other_axis_data.inv_scale, tick.value);
				tick_mark.segments.push_back(plot_segment_t(end));
				tick_mark.segments.push_back(plot_segment_t(start));
				tick_mark.text_alignment = ALIGN_RIGHT;
			}
			else {
				plot_vertex_t start(axis_data.position - 2.5 * other_axis_data.inv_scale, tick.value);
				plot_vertex_t end(axis_data.position + 2.5 * other_axis_data.inv_scale, tick.value);
				tick_mark.segments.push_back(plot_segment_t(end));
				tick_mark.segments.push_back(plot_segment_t(start));
				tick_mark.text_alignment = ALIGN_LEFT;
			}
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
	label.text = axis_data.label;
	label.text_style = zc_text_style({ FL_BLACK, 0, default_text_size_ });
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

	data_set_t& data_set = data_sets_[axis_number];
	axis_data_t& axis_data = it->second;
	axis_data_t& axis_0_data = axis_0_it->second;
	// Add the data set to the plot data for this axis number
	// Create a new plot line for this data set.
	plot_object_t plot_line;
	plot_line.shape = LINE_STRIP;
	plot_line.style = data_set.style;

	// Add a vertex for each data point in the data set.
	// Include the data points in the current ramges for the axes.
	for (const auto& point : *data_set.data) {
		// Check that the point is within the outer ranges for both axes. If not, skip it.
		if (!axis_data.outer_range.contains(point.second) ||
			!axis_0_data.outer_range.contains(point.first)) {
			continue;
		}
		// Add the point to the default ranges for both axes.
		axis_data.default_range |= point.second;
		axis_0_data.default_range |= point.first;
		// Now add the point to the plot line as a vertex. Convert to Cartesian coordinates if necessary.
		plot_vertex_t vertex;
		vertex = plot_vertex_t(convert_point(point));
		plot_segment_t segment(vertex);
		plot_line.segments.push_back(segment);

	}
	// Add the plot line to the plot data for this axis number.
	plot_data_[plot_number].layer_data[DATA].push_back(plot_line);

	// Update the current ranges for both axes to include the default ranges, if they are not already included.
	axis_0_data.current_range |= axis_0_data.default_range;
	axis_data.current_range |= axis_data.default_range;
}


// Override of handle
// TODO: Implement and test the major functionality before adding 
// zooming and scrolling
int zc_graph_::handle(int event) {
	return Fl_Widget::handle(event);
}

// Resize the widget - reset scaling factors
void zc_graph_::resize(int X, int Y, int W, int H) {
	// If we have actually resized...
	if (X != x() || Y != y() || W != w() || H != h()) {
		Fl_Widget::resize(X, Y, W, H);
		// Delete any existing axis line, ticks, grid lines and label for this axis number from the plot data.
		for (auto& plot_pair : plot_data_) {
			plot_layer_data_t& layer_data = plot_pair.second.layer_data;
			if (layer_data.find(AXES) != layer_data.end()) {
				layer_data.at(AXES).clear();
			}
			if (layer_data.find(GRID_LINES) != layer_data.end()) {
				layer_data.at(GRID_LINES).clear();
			}
		}
		// For each axis , recalculate the scaling factors based on the new size and the current ranges for that axis.
		for (auto& axis_pair : axes_data_) {
			generate_axis_grid(axis_pair.first);
		}
		redraw();
	}
}

// draw the widget
void zc_graph_::draw() {

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
		if (object.shape == TICK) {
			fl_begin_line();
			fl_color(object.style.colour);
			for (auto& seg : object.segments) {
				if (seg.type == plot_segment_t::VERTEX) {
					fl_vertex(seg.v.x, seg.v.y);
				}
			}
			fl_end_line();
			fl_line_style(0); // reset to default line style
		}
		// Get the dimensions of the label text to position it correctly.
		// Transform coordinates from data cords to pixel coords for text position.
		int tx = fl_transform_x(object.segments[0].v.x, object.segments[0].v.y) + 1;
		int ty = fl_transform_y(object.segments[0].v.x, object.segments[0].v.y) - 1;
		int tw= 0, th = 0;
		fl_font(object.text_style.font, object.text_style.size);
		fl_measure(object.text.c_str(), tw, th);
		// Adjust the text position based on the specified text alignment.
		switch (object.text_alignment) {
		case ALIGN_CENTRE:
			break;
		case ALIGN_LEFT:
			tx -= (tw / 2);
			break;
		case ALIGN_RIGHT:
			tx += (tw / 2);
			break;
		case ALIGN_ABOVE:
			ty -= (th / 2);
			break;
		case ALIGN_BELOW:
			ty += (th / 2);
			break;
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
	int plot_x = x() + axis_width_;
	int plot_y = y();
	int plot_w = w() - axis_width_;
	int plot_h = h() - axis_width_;
	double x_min = axes_data_[0].current_range.min;
	double x_max = axes_data_[0].current_range.max;
	double y_min = axes_data_[1].current_range.min;
	double y_max = axes_data_[1].current_range.max;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w;
	double dpp_y = (y_max - y_min) / plot_h;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - axis_width_ * dpp_x;
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
	axes_data_[1].label_position = { x_min - axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	axes_data_[1].label_angle = 90;
}

// Add a marker for Cartesian graphs
void zc_graph_cartesian::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	double value_1,
	double value_2
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
	if (!axis_data.outer_range.contains(value_1) || !axis_data.outer_range.contains(value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (value_1 == value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.max)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_2, other_axis_data.outer_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_2, other_axis_data.outer_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

void zc_graph_cartesian_2y::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	int plot_x = x() + axis_width_;
	int plot_y = y();
	int plot_w = w() - 2 * axis_width_;
	int plot_h = h() - axis_width_;
	double x_min = axes_data_[0].current_range.min;
	double x_max = axes_data_[0].current_range.max;
	double y_min = axes_data_[1].current_range.min;
	double y_max = axes_data_[1].current_range.max;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w;
	double dpp_y = (y_max - y_min) / plot_h;
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = x_min - axis_width_ * dpp_x;
	xform_schema.x_max_ = x_max + axis_width_ * dpp_x;
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
	axes_data_[1].label_position = { x_min - axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
	axes_data_[1].label_angle = 90;
	// Now set the transformation schema for the second Y axis to map its data range to the plot area dimensions.
	double y2_min = axes_data_[2].current_range.min;
	double y2_max = axes_data_[2].current_range.max;
	double dpp_y2 = (y2_max - y2_min) / h();
	plot_xform_t xform_schema_2;
	xform_schema_2.x_min_ = x_min - axis_width_ * dpp_x;
	xform_schema_2.x_max_ = x_max + axis_width_ * dpp_x;
	xform_schema_2.y_min_ = y2_min - axis_width_ * dpp_y2;
	xform_schema_2.y_max_ = y2_max;
	plot_data_[2].xform_schema = xform_schema_2;
	plot_data_[2].data_area.display_min = { x_min, y2_min };
	plot_data_[2].data_area.display_max = { x_max, y2_max };
	// Set the axis sizes and positions for the second Y axis.
	axes_data_[2].position = x_max;
	axes_data_[2].tick_orientation = TICK_INCREASING;
	axes_data_[2].inv_scale = dpp_y2;
	axes_data_[2].label_position = { x_max + axis_width_ * dpp_x, y2_min + (y2_max - y2_min) / 2.0F };
	axes_data_[2].label_angle = 90;
}

// Add a marker for Cartesian 2Y graphs
void zc_graph_cartesian_2y::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	double value_1,
	double value_2
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
	if (!axis_data.outer_range.contains(value_1) || !axis_data.outer_range.contains(value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (value_1 == value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.max)));
		}
		else {
			// Horizontal line at Y = value_1 (for axis 1 or 2)
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_2, other_axis_data.outer_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_2, other_axis_data.outer_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

// Cartesian overlay layout
void zc_graph_cart_overlay::layout() {
	// This is the default layout for Cartesian coordinates, so we can just call the general layout function.
	int plot_x = x();
	int plot_y = y();
	int plot_w = w();
	int plot_h = h();
	double x_min = axes_data_[0].current_range.min;
	double x_max = axes_data_[0].current_range.max;
	double y_min = axes_data_[1].current_range.min;
	double y_max = axes_data_[1].current_range.max;
	// data per pixel values
	double dpp_x = (x_max - x_min) / plot_w;
	double dpp_y = (y_max - y_min) / plot_h;
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
	if (x_min > -(axis_width_ * dpp_x)) {
		axes_data_[1].tick_orientation = TICK_INCREASING;
	}
	else {
		axes_data_[1].tick_orientation = TICK_DECREASING;
	}
	axes_data_[1].label_position = { axes_data_[1].position + axis_width_ * dpp_x * 0.5, y_min + (y_max - y_min) / 2.0F };
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
	axes_data_[0].label_position = { x_min + (x_max - x_min) / 2.0F, axes_data_[0].position + axis_width_ * dpp_y * 0.5 };
	axes_data_[0].label_angle = 0;
}

// Add a marker for Cartesian overlay graphs
void zc_graph_cart_overlay::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	double value_1,
	double value_2
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
	if (!axis_data.outer_range.contains(value_1) || !axis_data.outer_range.contains(value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (value_1 == value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = style;
		if (axis_number == 0) {
			// Vertical line at X = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.max)));
		}
		else {
			// Horizontal line at Y = value_1
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
			marker_line.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		// Draw a rectangle between value 1 and 2 on 1 axis and min and max on the other
		plot_object_t marker_shape;
		marker_shape.shape = POLYGON;
		marker_shape.style = style;
		if (axis_number == 0) {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_2, other_axis_data.outer_range.min)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_2, other_axis_data.outer_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.max)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(value_1, other_axis_data.outer_range.min)));
		}
		else {
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_1)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.max, value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_2)));
			marker_shape.segments.push_back(plot_segment_t(plot_vertex_t(other_axis_data.outer_range.min, value_1)));
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

// Polar layout
void zc_graph_polar::layout() {
	// For polar coordinates, we will set the transformation schema to map the R and Theta ranges to the plot area dimensions.
	int plot_x = x() + axis_width_;
	int plot_y = y() + axis_width_;
	int plot_w = w() - 2 * axis_width_;
	int plot_h = h() - 2 * axis_width_;
	double r_max = axes_data_[0].current_range.max;
	// data per pixel values
	double dpp_r = r_max / (std::min(plot_w, plot_h) / 2.0);
	// Set the transformation schema for this data type to map the data ranges to the plot area dimensions.
	plot_xform_t xform_schema;
	xform_schema.x_min_ = -(w() * dpp_r / 2.0);
	xform_schema.x_max_ = (w() * dpp_r / 2.0);
	xform_schema.y_min_ = -(h() * dpp_r / 2.0);
	xform_schema.y_max_ = (h() * dpp_r / 2.0);
	plot_data_[1].xform_schema = xform_schema;
	double rmax_x = plot_w * dpp_r / 2.0;
	double rmax_y = plot_h * dpp_r / 2.0;
	plot_data_[1].data_area.display_min = { -rmax_x, -rmax_y };
	plot_data_[1].data_area.display_max = { rmax_x, rmax_y };
	// Set the axis sizes and positions for the axes. For polar coordinates, we will set the R axis along the horizontal and the Theta axis along the vertical.
	// Radius axis at "Y" = 0;
	axes_data_[0].position = 0.0;
	axes_data_[0].tick_orientation = TICK_DECREASING;
	axes_data_[0].inv_scale = dpp_r;
	axes_data_[0].label_position = { r_max / 2.0F, -axis_width_ * dpp_r * 0.5 };
	axes_data_[0].label_angle = 0;
	// Theta axis at rmax 
	axes_data_[1].position = r_max;
	axes_data_[1].tick_orientation = TICK_INCREASING;
	axes_data_[1].inv_scale = 0.0; // Theta is not scaled in the same way as R, so we can set the inverse scale to 0 to indicate that it should not be used for scaling.
	axes_data_[1].label_position = { 0.0, -rmax_y + axis_width_ * dpp_r * 0.5 };
	axes_data_[1].label_angle = 0;
}

// Add a marker for Polar graphs
void zc_graph_polar::add_marker(
	int axis_number,
	layer_t layer,
	zc_line_style style,
	double value_1,
	double value_2
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
	if (!axis_data.outer_range.contains(value_1) || !axis_data.outer_range.contains(value_2)) {
		return;
	}
	axis_data_t& other_axis_data = other_axis_it->second;

	int plot_number = (axis_number == 0) ? 1 : axis_number; // The axis number to draw the marker along is the other axis

	// If the marker values are the same, add a single line marker. If they are different, add a shaded area marker.
	if (value_1 == value_2) {
		// Add a single line marker at value_1
		plot_object_t marker_line;
		marker_line.shape = LINE_STRIP;
		marker_line.style = style;
		if (axis_number == 0) {
			// Circle at R = value_1
			plot_arc_t arc = { 0, 0, value_1, 0, 360 };
			marker_line.segments.push_back(plot_segment_t(arc));
		}
		else {
			// Radial line at Theta = value_1
			plot_vertex_t origin(0.0, 0.0);
			marker_line.segments.push_back(origin);
			data_point_t point1(other_axis_data.outer_range.max, value_1);
			plot_vertex_t vertex1(convert_point(point1));
			marker_line.segments.push_back(vertex1);
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_line);
	}
	else {
		plot_object_t marker_shape;
		marker_shape.style = style;
		if (axis_number == 0) {
			// Draw an annulus from radius = value_1 to value_2
			marker_shape.shape = COMPLEX;
			double r0 = std::max(value_1, value_2);
			double r1 = std::min(value_1, value_2);
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
			plot_arc_t arc = { 0, 0, other_axis_data.outer_range.max, value_1, value_2 };
			marker_shape.segments.push_back(plot_segment_t(arc));
			// Add the origin again to create a closed shape for the sector.
			marker_shape.segments.push_back(origin);
		}
		plot_data_[plot_number].layer_data[layer].push_back(marker_shape);
	}
}

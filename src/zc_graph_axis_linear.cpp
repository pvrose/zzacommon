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

#include "zc_graph_axis_linear.h"
#include "zc_graph_axis.h"

#include <cmath>

//! \brief Constructor
zc_graph_axis_linear::zc_graph_axis_linear(int X, int Y, int W, int H, const char* L) :
	zc_graph_axis(X, Y, W, H, L) {
}

//!\brief Set the tick positions and labels based on the current range and tick spacing.
void zc_graph_axis_linear::set_ticks() {
	// Clear the existing ticks.
	ticks_.clear();
	// Calculate the tick spacing in data units based on the desired pixel spacing and current scale.
	tick_spacing_ = std::abs(tick_spacing_pixels_ * scale_);
	float tick_mantissa;
	float tick_power10;
	float grid_spacing_units = tick_spacing_;
	std::string format = "%0.0f";
	uint32_t si_prefix_exponent = ' ';
	normalise(tick_spacing_, tick_mantissa, tick_power10, si_prefix_exponent);
	// Calculate the actual tick spacing in data units.
	switch (modifier_) {
	case NO_MODIFIER:
		if (tick_spacing_ >= 0.7F) format = "%0.0f";
		else if (tick_spacing_ >= 0.07F) format = "%0.1F";
		else if (tick_spacing_ >= 0.007F) format = "%0.2F";
		else format = "%g";
		// Fall through to the same tick spacing as POWER_OF_10 - but with different formatting.
		[[fallthrough]];
	case POWER_OF_10:
		// If normalised value > 7 - set tick at 10 * power10 
		if (tick_mantissa > 7.0F) {
			tick_spacing_ = 10.0F * tick_power10;
		}
		// If normalised value is between 3.2 and 7 - set tick at 5*10^N		
		else if (tick_mantissa > 3.2F) {
			tick_spacing_ = 5.0F * tick_power10;
		}
		// If normalised value is between 1.4 and 3.2 - set tick at 2*10^N
		else if (tick_mantissa > 1.4F) {
			tick_spacing_ = 2.0F * tick_power10;
		}
		// Otherwise set tick at 10^N
		else {
			tick_spacing_ = tick_power10;
		}
		break;
	case SI_PREFIX:
		// If normalised value > 70 - set tick at 100 * power10
		if (tick_mantissa > 70.0F) {
			tick_spacing_ = 100.0F * tick_power10;
		}
		// If normalised value is between 32 and 70 - set tick at 50*10^N		
		else if (tick_mantissa > 32.0F) {
			tick_spacing_ = 50.0F * tick_power10;
		}
		// If normalised value is between 14 and 32 - set tick at 20*10^N
		else if (tick_mantissa > 14.0F) {
			tick_spacing_ = 20.0F * tick_power10;
		}
		// If normalised value is between 7 and 14 - set tick at 10*10^N
		else if (tick_mantissa > 7.0F) {
			tick_spacing_ = 10.0F * tick_power10;
		}
		// If normalised value is between 3.2 and 7 - set tick at 5*10^N		
		else if (tick_mantissa > 3.2F) {
			tick_spacing_ = 5.0F * tick_power10;
		}
		// If normalised value is between 1.4 and 3.2 - set tick at 2*10^N
		else if (tick_mantissa > 1.4F) {
			tick_spacing_ = 2.0F * tick_power10;
		}
		// If normalised value is between 0.7 and 1.4 - set tick at 10^N
		else if (tick_mantissa > 0.7F) {
			tick_spacing_ = tick_power10;
		}
		// If normalised value is between 0.32 and 0.7 - set tick at 0.5*10^N
		else if (tick_mantissa > 0.32F) {
			tick_spacing_ = 0.5F * tick_power10;
			format = "%0.1f";
		}
		// If normalised value is between 0.14 and 0.32 - set tick at 0.2*10^N
		else if (tick_mantissa > 0.14F) {
			tick_spacing_ = 0.2F * tick_power10;
			format = "%0.1f";
		}
		// Otherwise set tick at 0.1*10^N
		else {
			tick_spacing_ = 0.1F * tick_power10;
			format = "%0.1f";
		}
		break;
	}
	// Calculate the tick positions and labels based on the current range and tick spacing.
	ticks_.clear();
	// Start at the first tick position less than or equal to the minimum of the current range.
	float tick_value = floorf(current_range_.min / tick_spacing_) * tick_spacing_;
	while (tick_value <= current_range_.max) {
		// Ignore tick_value less than the minimum.
		if (tick_value < current_range_.min) {
			tick_value += tick_spacing_;
			continue;
		}
		// Format the tick label based on the modifier.
		char label[20];
		switch (modifier_) {
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
		ticks_.push_back({ tick_value, std::string(label) });
		tick_value += tick_spacing_;
	}
	// Add the SI prefix or power of 10 to the axis label if applicable.
	char lu[100];
	switch (modifier_) {
	case NO_MODIFIER:
		snprintf(lu, sizeof(lu), "(%s)", unit_.c_str());
		break;
	case POWER_OF_10:
		snprintf(lu, sizeof(lu), "(\303\227%g %s)", tick_power10, unit_.c_str());
		break;
	case SI_PREFIX: {
		// Convert Unicode code point to UTF-8
		char prefix_utf8[5] = { 0 };  // UTF-8 can be up to 4 bytes + null terminator
		fl_utf8encode(si_prefix_exponent, prefix_utf8);
		snprintf(lu, sizeof(lu), "(%s%s)", prefix_utf8, unit_.c_str());
		break;
	}
	}
	char ll[100];
	if (unit_.length() > 0) {
		snprintf(ll, sizeof(ll), "%s %s", label(), lu);
	}
	else {
		snprintf(ll, sizeof(ll), "%s", label());
	}
	label_ = ll;
}

// !\brief Generate the grid lines for the axis based on the current tick positions.
void zc_graph_axis_linear::set_grid_lines() {
	float grid_spacing_units = tick_spacing_ * 2;
	// Calculate the grid line positions based on the grid spacing.
	grid_values_.clear();
	// Start at the first grid line position less than or equal to the minimum of the current range.
	float grid_value = floorf(current_range_.min / grid_spacing_units) * grid_spacing_units;
	while (grid_value <= current_range_.max)
	{
		grid_values_.push_back(grid_value);
		grid_value += grid_spacing_units;
	}

}

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
#pragma once

#include <FL/Fl_Widget.H>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

//! \brief Class to represent an axis on a graph, including scaling and zooming options.

class zc_graph_axis : public Fl_Widget {

public:

	//! Data range pair.
	struct range {
		float min = FLT_MAX;   //!< Minimum value 
		float max = -FLT_MAX;      //!< Maximum value

		void set_union(const range& other)
		{
			min = std::min(min, other.min);
			max = std::max(max, other.max);
		};
		void set_intersection(const range& other)
		{
			min = std::max(min, other.min);
			max = std::min(max, other.max);
			if (min > max) {
				// No intersection - set to empty range.
				min = FLT_MAX;
				max = -FLT_MAX;
			}
		}
		range& get_union(const range& other) const
		{
			range result = *this;
			result.set_union(other);
			return result;
		};
		range& get_intersection(const range& other) const
		{
			range result = *this;
			result.set_intersection(other);
			return result;
		}
	};

	//! Axis label modifier type.
	enum modifier_t : uint8_t {
		NO_MODIFIER,            //!< No modifier
		SI_PREFIX,              //!< SI prefix (e.g. k for kilo, M for mega)
		POWER_OF_10,            //!< Power of 10 (e.g. 10^3)
	};

	//!\brief Axis orientation type.
	enum orientation_t : uint8_t {
		X_AXIS,                 //!< X-axis (horizontal)
		YL_AXIS,                //!< Y-axis (vertical to left of graph)
		YR_AXIS,                //!< Y-axis (vertical to right of graph)
		R_AXIS,                 //!< Radius axis for polar graph
	};

	//! \brief Parameter structure for axis configuration.
	struct axis_params_t {
		orientation_t orientation;     //!< Orientation of the axis
		range outer_range;             //!< Absolute minimum and maximum for zooming
		range inner_range;             //!< Current minimum and maximum for display
		range default_range;           //!< Default range in the absence of data
		modifier_t modifier;           //!< Modifier for axis labels
		std::string unit;              //!< Unit to display on the axis (e.g. "Hz")
		std::string label;             //!< Base label for the axis (e.g. "Frequency")
		int tick_spacing_pixels;       //!< Suggested spacing between ticks in pixels

		axis_params_t() {};

		//! Minimalist constructor with mostly default values.
		//! \param orientation Orientation of the axis
		//! \param range Range for the axis (outer, inner, and default all set to the same value)
		//! \param modifier Modifier for axis labels
		//! \param unit Unit to display on the axis (e.g. "Hz")
		//! \param label Base label for the axis (e.g. "Frequency")
		axis_params_t(orientation_t orientation, range range, modifier_t modifier = NO_MODIFIER, const std::string& unit = "", const std::string& label = "") :
			orientation(orientation), 
			outer_range(range), 
			inner_range(range), 
			default_range(range),
			modifier(modifier), 
			unit(unit), 
			label(label), 
			tick_spacing_pixels(30) {
		};
	};

	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the axis drawing area
	//! \param Y The Y coordinate of the top-left corner of the axis drawing area
	//! \param W The width of the axis drawing area
	//! \param H The height of the axis drawing area
	//! \param L The base label for the axis (e.g. "Frequency")
	zc_graph_axis(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Set the parameters for this axis.
	void set_params(const axis_params_t& params);

	//! \brief Destructor
	~zc_graph_axis();

	//!\brief Attempt to set the range to \p new_range.
	//! \param new_range The new range to set.
	//! False if the range was not or only partially updated due to the zoom limits.
	void set_range(range new_range);

	//! \brief Zoom by a factor of \p zoom_factor around the value at \p mouse_pos.
	//! \param mouse_pos The pixel position of the mouse along the axis.
	//! \param zoom_factor The factor to zoom by: +10 indictaes x2 zoom, -10 indicates x0.5 zoom.
	void zoom(int mouse_pos, int zoom_factor);

	//! \brief Scroll by an offset of \p scroll_offset pixels.
	//! \param scroll_offset The offset to scroll by in pixels (positive or negative).
	void scroll(int scroll_offset);

	//! \brief Return current range of the axis.
	const range& get_range() const {
		return current_range_;
	}

	//! \brief Return the current scaling factor (units per pixel).
	float get_scale() const {
		return scale_;
	}

	//! \brief Reset the range after reizing the axis.
	void resize(int X, int Y, int W, int H) override {
		Fl_Widget::resize(X, Y, W, H);
		set_range(current_range_);
	}

	//! \brief Return the current inverse scaling factor (pixels per unit).
	float get_inv_scale() const {
		return inv_scale_;
	}

	//! \brief Return the current position of the origin (0 value) in pixels.
	int get_origin() const {
		return origin_;
	}

	//! \brief Return the pixel value for the given data value \p f.
	int float_to_pixel(float f) const {
		return origin_ + rint(f * inv_scale_);
	}

	//! \brief Return the data value for the given pixel position \p p.
	float pixel_to_float(int p) const {
		return (p - origin_) * scale_;
	}

	//! \brief Return the current tick spacing in units.
	float get_tick_spacing() const {
		return tick_spacing_;
	}

	//! \brief Return the orientation of the axis.
	orientation_t get_orientation() const {
		return orientation_;
	}

	//! Override draw to draw the axis.
	void draw() override;

	//! Reset the zoom and scroll to the default range and position.
	void reset_range() {
		set_range(default_range_);
	}

	//! \brief Return the list of grid lines to draw on the plot.
	std::vector<float> get_grid_lines() const {
		return grid_lines_;
	}

private:

	// Specified parameters
	orientation_t orientation_;   //!< Orientation of the axis in degrees (0 for X-axis, 90 for Y-axis)
	range outer_range_;           //!< Absolute minimum and maximum for zooming
	range inner_range_;           //!< Current minimum and maximum for display
	range default_range_;         //!< Default range in the absence of data
	modifier_t modifier_;         //!< Modifier for axis labels
	std::string unit_;            //!< Unit to display on the axis (e.g. "Hz")
	int tick_spacing_pixels_;     //!< Suggested spacing between ticks in pixels
	
	// Calculated parameters
	float scale_;                //!< Scale factor - number of units per pixel
	float inv_scale_;            //!< Inverse scale factor - number of pixels per unit
	int origin_;             //!< Pixel position of 0 along the axis
	float tick_spacing_;         //!< Spacing between ticks in units
	std::string label_;           //!< Label to display (base label plus multiplier if appropriate)

	// Current state
	range current_range_;         //!< Current range for display (may be zoomed or scrolled)
	// Upper zoom limit - set by the data range, used to prevent zooming out beyound the data range.
	range zoom_limit_range_;

	//! Tick structure to represent a tick mark on the axis.
	struct tick_t {
		int position;           //!< Pixel position of the tick along the axis
		std::string label;      //!< Label to display for the tick
	};

	//! The ticks to display on the axis.
	std::vector<tick_t> ticks_;

	//! Lines to draw every few ticks - float values in units,
	//! converted to pixel positions when drawing onto plot.
	std::vector<float> grid_lines_;

	// Internal methods.

	//!\brief Set the tick positions and labels based on the current range and tick spacing.
	void set_ticks();

	//! \brief Normalise a number to between 1 and 10 plus an exponent and SI prefix if appropriate.
	//! \param fin Input number
	//! \param norm Normalised result (mantissa)
	//! \param exp10 Exponent (power of 10)
	//! \param si_prefix SI Prefix (if appropriate) - UTF-8 character
	void normalise(float fin, float& norm, float& exp10, uint32_t& si_prefix) const;

	//! \brief Draw the line for the axis.
	void draw_axis_line();
	//! \brief Draw the ticks for the axis.
	void draw_ticks();
	//! \brief Draw the label for the axis.
	void draw_label();

};





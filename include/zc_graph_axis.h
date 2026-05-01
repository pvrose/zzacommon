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
//! This class is intended to have derived classes for specific types of axes to control
//! in particular the orientation, labelling and setting of ticks and grid lines.
//! 
//! Most functionality is implemented in this base class.
//! 
//! \image html zc_graph_axis.png "Showing three axes with different orientations and modifiers"
class zc_graph_axis : public Fl_Widget {

public:

	//! Data range pair.
	struct range {
		float min = FLT_MAX;   //!< Minimum value 
		float max = -FLT_MAX;      //!< Maximum value

		//! Set the range to the union of this and another range. 
		//! The resulting range will include all values that are in either this range or the other range.
		//! \param other The other range to union with this range.
		void set_union(const range& other)
		{
			min = std::min(min, other.min);
			max = std::max(max, other.max);
		};
		//! Set the range to the intersection of this and another range.
		//! The resulting range will include only values that are in both this range and the other range.
		//! \param other The other range to intersect with this range.
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
		//! Get the union of this and another range, returning a new range object.
		//! \param other The other range to union with this range.
		//! \return A new range object representing the union of this range and the other range.
		range get_union(const range& other) const
		{
			range result = *this;
			result.set_union(other);
			return result;
		};
		//! Get the intersection of this and another range, returning a new range object.
		//! \param other The other range to intersect with this range.
		//! \return A new range object representing the intersection of this range and the other range.
		range get_intersection(const range& other) const
		{
			range result = *this;
			result.set_intersection(other);
			return result;
		}
	};

	//! \brief Axis label modifier type. This indicates how the axis labels should be
	//! modified to indicate the scale of the axis.
	enum modifier_t : uint8_t {
		NO_MODIFIER,            //!< No modifier: use the base label and unit as is.
		SI_PREFIX,              //!< SI prefix (e.g. k for kilo, M for mega)
		POWER_OF_10,            //!< Power of 10 (e.g. 10^3) - displayed as "x10^3" in the label.
	};


	//! \brief Parameter structure for axis configuration.
	struct axis_params_t {
		range outer_range;             //!< Absolute minimum and maximum for zooming
		range inner_range;             //!< Current minimum and maximum for display
		range default_range;           //!< Default range in the absence of data
		modifier_t modifier;           //!< Modifier for axis labels
		std::string unit;              //!< Unit to display on the axis (e.g. "Hz")
		std::string label;             //!< Base label for the axis (e.g. "Frequency")
		int tick_spacing_pixels;       //!< Suggested spacing between ticks in pixels

		//! \brief Default constructor - sets all values to defaults.
		axis_params_t() {};

		//! Minimalist constructor with mostly default values.
		//! \param orientation Orientation of the axis
		//! \param range Range for the axis (outer, inner, and default all set to the same value)
		//! \param modifier Modifier for axis labels
		//! \param unit Unit to display on the axis (e.g. "Hz")
		//! \param label Base label for the axis (e.g. "Frequency")
		axis_params_t(range range, modifier_t modifier = NO_MODIFIER, const std::string& unit = "", const std::string& label = "") :
			outer_range(range), 
			inner_range(range), 
			default_range(range),
			modifier(modifier), 
			unit(unit), 
			label(label), 
			tick_spacing_pixels(30) {
		};
	};

	//! \brief Zoom capability of the axis.
	enum zoom_capability_t : uint8_t {
		NO_ZOOM,           //!< No zooming allowed - the range is fixed.
		ZOOM_ON_ORIGIN,       //!< Zooming allowed but the origin (0 value) is fixed in place.
		ZOOM_ON_CURSOR,       //!< Zooming allowed and the zoom is centered on the cursor position.
	};

	//! \brief Tick direction for the axis.
	enum tick_direction_t : uint8_t {
		INVALID,	   //!< Invalid tick direction - should not be used.
		UPWARDS,        //!< Ticks point upwards (for horizontal axes)
		DOWNWARDS,      //!< Ticks point downwards (for horizontal axes)
		LEFTWARDS,      //!< Ticks point leftwards (for vertical axes)
		RIGHTWARDS,     //!< Ticks point rightwards (for vertical axes)
		OUTWARDS,      //!< Ticks point outwards from the plot area (surrounding axes only)
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

	//! \brief Attempt to set the range to \p new_range.
	//! \param new_range The new range to set. The range will be limited by the outer range.
	void set_range(range new_range);

	//! \brief Zoom by a factor of \p zoom_factor around the value at \p mouse_pos.
	//! The zoom will be limited by the outer range.
	//! \param mouse_pos The pixel position of the mouse along the axis.
	//! \param zoom_factor The factor to zoom by: +10 indictaes x2 zoom, -10 indicates x0.5 zoom.
	void zoom(int mouse_pos, int zoom_factor);

	//! \brief Scroll by an offset of \p scroll_offset pixels.
	//! The final scroll position will be limited by the outer range.
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

	//! \brief Override the widget default to reset the range after reizing the axis.
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

	//! \brief Set the tick direction for the axis.
	void set_tick_direction(tick_direction_t tick_direction) {
		tick_direction_ = tick_direction;
		redraw();
	}	

	//! \brief Return the tick direction for the axis based on the orientation.
	tick_direction_t get_tick_direction() const {
		return tick_direction_;
	}

	//! Override draw to draw the axis, ticks and labels.
	void draw() override;

	//! Reset the zoom and scroll to the default range and position.
	void reset_range() {
		set_range(default_range_);
	}

	//! \brief Return the list of grid lines to draw on the plot.
	std::vector<float> get_grid_lines() const {
		return grid_values_;
	}

	//! \brief Return true if the axis is horizontal (X-axis) or false if the axis is vertical (Y-axis).
	virtual bool is_horizontal() const = 0;

	//! \brief Returns the zoom capability of the axis.
	virtual zoom_capability_t get_zoom_capability() const = 0;

	//! \brief Returns whether the axis can be scrolled.
	virtual bool can_scroll() const = 0;

	//! Return the pixel position of the axis minimum (either x() for horizontal axis or y() + h() for vertical axis).
	int get_axis_min_pixel() const {
		return is_horizontal() ? x() : y() + h();
	}

	//! Return the width/height of the axis in pixels (either w() for horizontal axis or -h() for vertical axis).
	int get_axis_length_pixels() const {
		return is_horizontal() ? w() : -h();
	}
protected:

	//!\brief Set the tick positions and labels based on the current range and tick spacing.
	virtual void set_ticks() = 0;

	//! \brief Set the grid lines based on the current tick positions.
	virtual void set_grid_lines() = 0;

	//! \brief Draw the line for the axis.
	virtual void draw_axis_line() = 0;
	//! \brief Draw the ticks for the axis.
	virtual void draw_ticks() = 0;
	//! \brief Draw the label for the axis.
	virtual void draw_label() = 0;

		
    // Specified parameters
	range outer_range_;           //!< Absolute minimum and maximum for zooming
	range inner_range_;           //!< Current minimum and maximum for display
	range default_range_;         //!< Default range in the absence of data
	modifier_t modifier_;         //!< Modifier for axis labels
	std::string unit_;            //!< Unit to display on the axis (e.g. "Hz")
	int tick_spacing_pixels_;     //!< Suggested spacing between ticks in pixels
	
	// Calculated parameters
	float scale_;                //!< Scale factor - number of units per pixel
	float inv_scale_;            //!< Inverse scale factor - number of pixels per unit
	int origin_;                 //!< Pixel position of 0 along the axis
	std::string label_;          //!< Actual label to display (base label plus multiplier if appropriate)
                                 //!< The base label is set by the widget label() method.
	// Current state
	range current_range_;         //!< Current range for display (may be zoomed or scrolled)
	//! Upper zoom limit - set by the data range, used to prevent zooming out beyond the data range.
	range zoom_limit_range_;

	//! Tick direction.
	tick_direction_t tick_direction_ = INVALID;

	//! Tick structure to represent a tick mark on the axis.
	struct tick_t {
		int position;           //!< Pixel position of the tick along the axis
		std::string label;      //!< Label to display for the tick
	};

	//! The ticks to display on the axis.
	std::vector<tick_t> ticks_;

	//! Values on the axis at which to draw the grid. These will be drawn on the plot area.
	std::vector<float> grid_values_;

	// Internal methods.


	//! \brief Normalise a number and generate the appropriate multiplier.
	//! For modifier_t values:
	//! - SI_PREFIX: normalise to a value between 0.1 and 100 and generate the appropriate SI prefix (e.g. k for kilo, M for mega).
	//! - POWER_OF_10: normalise to a value between 1 and 10 and generate the appropriate power of 10 multiplier (e.g. x10^4).
	//! \param fin Input number
	//! \param norm Normalised result (mantissa)
	//! \param exp10 Exponent (power of 10)
	//! \param si_prefix SI Prefix (if appropriate) - UTF-8 character
	void normalise(float fin, float& norm, float& exp10, uint32_t& si_prefix) const;



};





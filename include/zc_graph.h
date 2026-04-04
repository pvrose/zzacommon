/*
	Copyright 2017-2026, Philip Rose, GM3ZZA
	
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

#include "zc_line_style.h"

#include <cstdint>
#include <cmath>
#include <cfloat>
#include <map>
#include <string>
#include <vector>

#include <FL/Enumerations.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_Widget.H>

//! \brief Class to display a graph of values.
//! The data is presented as a list of floating point values.
//! How the data is scaled and axes shown \see set_params.
class zc_graph : public Fl_Widget {

public:
	zc_graph(int X, int Y, int W, int H, const char* L = nullptr);
	virtual ~zc_graph();

	//! Draw the display
	virtual void draw();

	//! Resize the widget
	virtual void resize(int X, int Y, int W, int H);

	//! Coordinates
	struct coord {
		float x = 0.0F;     //!< X-coordinate of point
		float y = 0.0F;     //!< Y-coordinate of point
	};

	//! tick definitions
	struct tick_t {
		int pos = 0;
		std::string label = "";
	};

	//! \brief Axis label multiplier
	enum axis_xier_t : uint8_t {
		NONE,                       //!< No multipler - display values as is
		SI_PREFIX,                  //!< Prefix label with SI multiplier, values between 0.1 and 100
		POWER_10,                   //!< Prefix label with 10^N, values between 1 and 10.
	};
	//! \brief Y-axis identifier
	enum y_axis_t : uint8_t {
		Y_LEFT,                     //!< Left Y-axis
		Y_RIGHT                     //!< Right Y-axis
	};
	//! \brief Graph axis options
	struct options_t {
		float minimum = 0.0F;            //!< Minimum  value
		float maximum = 1.0F;            //!< Maximum  value
		const char* base_label = "";     //!< Label (base unit)
		axis_xier_t xier_type = SI_PREFIX;    //!< How to display multipliers
		int suggested_gap = 20;          //!< Suggested gap between ticks
		bool zoomable = false;             //!< Whether to allow zooming and scrolling on this axis
		float absolute_minimum = -FLT_MAX;       //!< Absolute minimum value (for zooming limits)
		float absolute_maximum = FLT_MAX;       //!< Absolute maximum value (for zooming limits)
		bool expandable = true;              //!< Whether to allow expanding the range to fit new data - if false, the range will be fixed and data outside the range will be clipped.

		options_t() {};

		options_t(float minimum, float maximum, bool expandable, const char* label, axis_xier_t xier_type, int suggested_gap, bool zoomable = false, float absolute_minimum = -FLT_MAX, float absolute_maximum = FLT_MAX) {
			set_drawing_options(label, xier_type, suggested_gap);
			set_range(minimum, maximum, expandable);
			set_zoom_options(zoomable, absolute_minimum, absolute_maximum);
		}

		// Set the options for drawing this axis.
		void set_drawing_options(const char* label, axis_xier_t xier_type, int suggested_gap) {
			this->base_label = label;
			this->xier_type = xier_type;
			this->suggested_gap = suggested_gap;
		}

		// Set the range for this axis.
		// If expandable is true, the range may be expanded to include the new minimum and maximum 
		// if they are outside the current range.
		void set_range(float minimum, float maximum, bool expandable = true){
			if (this->expandable) {
				this->minimum = std::min(minimum, this->minimum);
				this->maximum = std::max(maximum, this->maximum);
				this->orig_minimum = this->minimum;
				this->orig_maximum = this->maximum;
				this->expandable = expandable;
			}
		}

		// Set the zooming options for this axis.
		void set_zoom_options(bool zoomable, float absolute_minimum, float absolute_maximum) {
			this->zoomable = zoomable;
			this->absolute_minimum = absolute_minimum;
			this->absolute_maximum = absolute_maximum;
		}

		//! \brief Set the drawing position of the axis.
		void set_origin_length(int origin, int length) {
			this->origin = origin;
			this->length = length;
		}

		int position_0 = 0;              //!< pixel position of 0 along the axis
		float scale = 0.0F;              //!< Scale factor - Number of units per pixel
		float inv_scale = 0.0F;          //!< Inverse scae factor - number of pixels per unit
		std::string label;               //!< Label to display (base label plus multiplier if appropriate)
		std::vector<tick_t> ticks;       //!< Ticks to display
		std::vector<int> lines;          //!< Pixel position of lines to draw every few ticks.

	protected:

		bool in_scrolling = false;       //!< Whether we are currently scrolling - used to track whether to update scroll offset on mouse move
		int scroll_start_pos = 0;        //!< Starting pixel position of scroll - used to calculate scroll offset on mouse move
		float orig_minimum = 0.0F;          //!< Original minimum set before zooming - used to restore when unzooming.
		float orig_maximum = 0.0F;          //!< Original maximum set before zooming - used to restore when unzooming.
		int origin = 0;                   //!< Pixel position to use as the left- or bottom-most along the axis - used to restore when unzooming.
		int length = 0;                   //!< Length of the axis in pixels - used to restore when unzooming.
    public:
		/// \brief Set the scaling factors based on the options and drawing area size
		//! This is called when the options are set and when the widget is resized,
		//! and can be used to restore the scaling factors after zooming or scrolling.
		//! \param origin The pixel position to use as the left- or bottom-most along the axis
		//! \param length The length of the axis in pixels
		//! \param unzoom Whether to reset the zoom factor to 1.0F.
		void set_factors(bool unzoom) {
			if (unzoom) {
				minimum = orig_minimum;
				maximum = orig_maximum;
			}
			float x_range = maximum - minimum;
			// Scale is number of units per pixel - so invert to get pixels per unit.
			// And apply zoom factor - so zoom in is more units per pixel, zoom out is fewer units per pixel.
			scale = x_range / length;
			inv_scale = 1.0F / scale;
			// Set origin and adjust by scroll offset
			position_0 = origin - (minimum * inv_scale);
			set_ticks();
		};

		//! \brief Update zoom factor and scroll offset based on mouse
		//! movement during zooming or scrolling.
		//! \param origin The pixel position to use as the left- or bottom-most along the axis
		//! \param length The length of the axis in pixels
		//! \param mouse_pos The current pixel position of the mouse along the axis
		//! \param delta The change in zoom factor (positive to zoom in, negative to zoom out)
		void update_zoom(int mouse_pos, int delta) {
			// Zoom change is 2^^(delta/10) - so every 10 units of delta doubles the zoom factor, every -10 units halves it.
			float zoom_change = powf(2.0F, (float)delta / 10.0F);
			// Get the value at the mouse position before the zoom change.
			float mouse_value = (mouse_pos - position_0) * scale;
			// Apply the zoom change to the zoomed range.
			minimum = mouse_value - (mouse_value - minimum) * zoom_change;
			maximum = mouse_value + (maximum - mouse_value) * zoom_change;
			// Clamp the zoomed range to the absolute minimum and maximum.
			if (minimum < absolute_minimum) {
				minimum = absolute_minimum;
			}
			if (maximum > absolute_maximum) {
				maximum = absolute_maximum;
			}
			set_factors(false);
		}

		//! \brief Update scroll_offset based on mouse movement during scrolling.
		//! \param origin The pixel position to use as the left- or bottom-most along the axis
		//! \param length The length of the axis in pixels
		//! \param mouse_pos The current pixel position of the mouse along the axis.
		void update_scroll(int mouse_pos) {
			int scroll_offset = mouse_pos - scroll_start_pos;
			maximum += scroll_offset * scale;
			minimum += scroll_offset * scale;
			int range = maximum - minimum;
			// Clamp the zoomed range to the absolute minimum and maximum.
			if (minimum < absolute_minimum) {
				minimum = absolute_minimum;
				maximum = minimum + range;
			}
			if (maximum > absolute_maximum) {
				maximum = absolute_maximum;
				minimum = maximum - range;
			}
			scroll_start_pos = mouse_pos;
			set_factors(false);
		}

		void set_ticks();         //!< Set the ticks and label based on the options

		//! \brief. Normalise number to between 1 and 10 plus exponent
		//! \param fin Input number
		//! \param norm Normalised result
		//! \param exp10 Exponent (power of 10)
		//! \param si_prefix SI Prefix (if appropriate) - UTF-8 character
		void normalise(float fin, float& norm, float& exp10, int& si_prefix);

		//! \brief. Convert data point \p f from float to drawing position
		int float_to_point(float f);

		//! \brief. Set the scrolling start position
		void start_scroll(int mouse_pos) {
			in_scrolling = true;
			scroll_start_pos = mouse_pos;
		}

		//! \brief. End scrolling
		void end_scroll() {
			in_scrolling = false;
			scroll_start_pos = 0;
		}

		//!\brief. In scrolling mode.
		bool is_scrolling() const {
			return in_scrolling;
		}

	};

	//! \brief Structure to describe a set of data points.
	struct data_set_t {
		y_axis_t y_axis = Y_LEFT;          //!< Y-axis to use for this data set
		zc_line_style style;    //!< Line style to use to draw this data set
		std::vector<coord>* data; //!< Data points - by reference to allow manipulation outwith display.
	};

	//! \brief Set parameters
	//! \param x_options Options for displaying X-axis
	//! \param y_options Options for displaying Y-axis
	void set_params(const options_t& x_options, const options_t& y_options);

	//! \brief Set parameters for double Y-axes
	//! \param x_options Options for displaying X-axis
	//! \param y_left_options Options for displaying left Y-axis
	//! \param y_right_options Options for displaying right Y-axis
	void set_params(const options_t& x_options, const options_t& y_left_options, const options_t& y_right_options);

	//! \brief Set value as data to display.
	//! It is intended that the data can be manipulated outwith display
	//! and just use redraw() to update it.
	//! The data may be continually updated at the sample rate (or in
	//! chunks of 64 samples at 22K samples/s) and refreshed at a lower rate
	//! (25 or 30 frames/s).
	void set_data(std::vector<coord>* data);

	//! \brief Set value into specified data set.
	//! \param index The index of the data set to update.
	//! \param data The data to set for this data set.
	void set_data(int index, std::vector<coord>* data);

	//! \brief Add a set of data to display.
	//! \param data_set The data set to add.
	//! \return The index of the data set in the list of data sets.
	int add_data_set(const data_set_t& data_set);

	//! \brief Clear all data sets.
	void clear_data_sets();

	//! \brief. Set the drawing area
	void set_drawing_area();

	//! \brief. Get axis options for X axis
	options_t* get_x_options() {
		return &x_options_;
	}
	//! \brief. Get axis options for Y axis
	options_t* get_y_options(y_axis_t axis) {
		if (y_options_.find(axis) == y_options_.end()) {
			// If we don't have options for this axis yet, add some with default values.
			return nullptr;
		}
		return &y_options_[axis];
	}
	
	//! Overload handle to capture mouse events for zooming and scrolling
	int handle(int event) override;


protected:

	//! \brief Returns true if the pixel at {\p x, \p y} is in the drawimg area
	bool in_drawing_area(int x, int y);

	//! \brief. Draw axes
	void draw_axes();

	//! \brief. Draw points
	void draw_points();

	//! \brief Set the scaling factors etc.
	//! \param unzoom Whether to reset the zoom factors to 1.0F.
	void set_factors(bool unzoom);

	////! \brief. Convert data to points
	//void convert_data_to_points();

	//! \brief The data to display.
	std::vector<data_set_t> data_sets_;

	//! Parameters for drawing the X values and axis.
	options_t x_options_;
	//! Parameters for drawing the Y values and axes.
	std::map<y_axis_t, options_t> y_options_;

		//! Drawing area
	Fl_Rect drawing_area_;
	 
};
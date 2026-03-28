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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <FL/Enumerations.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_Widget.H>

struct zc_graph_line_t {
	Fl_Color colour;        //!< Colour to use to draw this line
	int thickness;          //!< Line width to use to draw this line
	int style;              //!< Line style to use to draw this line
};

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
		friend class zc_graph;
		float minimum = 0.0F;            //!< Minimum  value
		float maximum = 1.0F;            //!< Maximum  value
		const char* base_label = "";     //!< Label (base unit)
		axis_xier_t xier_type = SI_PREFIX;    //!< How to display multipliers
		int suggested_gap = 20;          //!< Suggested gap between ticks

		options_t(float min, float max, const char* label, axis_xier_t xier_type, int suggested_gap) :
			minimum(min), maximum(max), base_label(label), xier_type(xier_type), suggested_gap(suggested_gap) {
		}
		options_t() {};

	protected:
		int position_0 = 0;           //!< pixel position of 0 along the axis
		float scale = 0.0F;              //!< Scale factor - Number of units per pixel
		float inv_scale = 0.0F;          //!< Inverse scae factor - number of pixels per unit
		std::string label;        //!< Label to display (base label plus multiplier if appropriate)
		std::vector<tick_t> ticks; //!< Ticks to display

		/// \brief Set the scaling factors based on the options and drawing area size
		void set_factors(int origin, int length) {
			float x_range = maximum - minimum;
			scale = x_range / length;
			inv_scale = 1.0F / scale;
			// Set origin
			position_0 = origin - (minimum * inv_scale);
			set_ticks();
		};

		void set_ticks();         //!< Set the ticks and label based on the options

		//! \brief. Normalise number to between 1 and 10 plus exponent
		//! \param fin Input number
		//! \param norm Normalised result
		//! \param exp10 Exponent (power of 10)
		//! \param si_prefix SI Prefix (if appropriate) - UTF-8 character
		void normalise(float fin, float& norm, float& exp10, int& si_prefix);

		//! \brief. Convert data point \p f from float to drawing position
		int float_to_point(float f);


	};

	//! \brief Structure to describe a set of data points.
	struct data_set_t {
		y_axis_t y_axis;          //!< Y-axis to use for this data set
		zc_graph_line_t style;    //!< Line style to use to draw this data set
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

	//! \brief Adjust the ranges of the X axis to fit the data.
	void adjust_scale_x();

	//!\brief Adjust the ranges of the Y axis to fit the data.
	void adjust_scale_y(y_axis_t axis);

	//! \brief. Set the drawing area
	void set_drawing_area();


protected:

	//! \brief Returns true if the pixel at {\p x, \p y} is in the drawimg area
	bool in_drawing_area(int x, int y);

	//! \brief. Draw axes
	void draw_axes();

	//! \brief. Draw points
	void draw_points();

	//! \brief Set the scaling factors etc.
	void set_factors();

	////! \brief. Convert data to points
	//void convert_data_to_points();

	//! \brief The data to display.
	std::vector<data_set_t> data_sets_;

	//! Paarmeters for drawing the X values and axis.
	options_t x_options_;
	//! Parameters for drawing the Y values and axes.
	std::map<y_axis_t, options_t> y_options_;

		//! Drawing area
	Fl_Rect drawing_area_;
	 
};
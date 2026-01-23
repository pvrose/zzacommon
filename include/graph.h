#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <FL/Fl_Rect.H>
#include <FL/Fl_Widget.H>

//! \brief Class to display a graph of values.
//! The data is presented as a list of floating point values.
//! How the data is scaled and axes shown \see set_params.
class graph : public Fl_Widget {

public:
	graph(int X, int Y, int W, int H, const char* L = nullptr);
	virtual ~graph();

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
	enum axis_xier_t {
		NONE,                       //!< No multipler - display values as is
		SI_PREFIX,                  //!< Prefix label with SI multiplier, values between 0.1 and 100
		POWER_10,                   //!< Prefix label with 10^N, values between 1 and 10.
	};
	//! \brief Graph axis options
	struct options_t {
		friend class graph;
		float minimum;            //!< Minimum  value
		float maximum;            //!< Maximum  value
		const char* base_label;   //!< Label (base unit)
		axis_xier_t xier_type;    //!< How to display multipliers
		int suggested_gap;        //!< Suggested gap between ticks
	protected:
		int position_0;           //!< pixel position of 0 along the axis
		float scale;              //!< Scale factor - Number of units per pixel
		float inv_scale;          //!< Inverse scae factor - number of pixels per unit
	};
	//! \brief Set parameters
	//! \param x_options Options for displaying X-axis
	//! \param y_options Options for displaying Y-axis
	void set_params(const options_t& x_options, const options_t& y_options);

	//! \brief Set value as data to display.
	//! It is intended that the data can be manipulated outwith display
	//! and just use redraw() to update it.
	//! The data may be continually updated at the sample rate (or in
	//! chunks of 64 samples at 22K samples/s) and refreshed at a lower rate
	//! (25 or 30 frames/s).
	void set_data(std::vector<coord>* data);

protected:
	//! \brief. Convert data point \p f from float to drawing position
	int float_to_point(float f, const options_t& options);

	//! \brief Returns true if the pixel at {\p x, \p y} is in the drawimg area
	bool in_drawing_area(int x, int y);

	//! \brief. Draw axes
	void draw_axes();

	//! \brief. Draw points
	void draw_points();

	//! \brief. Set the drawing area
	void set_drawing_area();

	//! \brief Set the scaling factors etc.
	void set_factors();

	////! \brief. Convert data to points
	//void convert_data_to_points();

	//! \brief. Normalise number to between 1 and 10 plus exponent
	//! \param fin Input number
	//! \param norm Normalised result
	//! \param exp10 Exponent (power of 10)
	//! \param si_prefix SI Prefix (if appropriate) - UTF-8 character
	//! \param xier use of multplier
	void normalise (float fin, float& norm, float& exp10, int& si_prefix, const axis_xier_t& xier);

	//! Set the ticks
	//! \param options - provides range and display options.
	//! \param ticks - array of theticks to draw with their labels
	//! \param label - drawn label
	void set_ticks(const options_t& options, std::vector<tick_t>& ticks, std::string& label);

	//! \brief The data to display.
	std::vector<coord>* data_;

	//! Paarmeters for drawing the X values and axes.
	options_t x_options_;
	//! Parameters for drawing the Y values and axes.
	options_t y_options_;

	//! Drawn X-axis label
	std::string x_label_;
	//! Drawn Y-axis label
	std::string y_label_;

	//! Y-axis ticks
	std::vector<tick_t> y_ticks_;
	//! X-axis ticks
	std::vector<tick_t> x_ticks_;

		//! Drawing area
	Fl_Rect drawing_area_;
	 
};
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

#include <FL/Fl_Slider.H>
#include "zc_range.h"

//! \brief A scroll bar widget that allows zooming and scrolling of a range of values.
//! It is designed to be used with a graph widget, allowing 
//! the user to zoom in and out of the graph and scroll across the data.
//! It is based on Fl_Slider but with the following changes:
//! - The value of the slider is the current position of the range (using zc_range<double>).
//! - The slider button shows the position of the current range within the total range
//! and changes size as the range is zoomed in and out.
//! - The slider button can be dragged to scroll the range.
//! - The mouse wheel can be used to zoom in and out of the range.
//! - Double clicking restores to full range (unzoomed).
//! - Zooming centred outwith the current range changes the range to include the zoom point.
//! The widget can be oriented horizontally or vertically using the Fl_Slider::type() method.

class zc_zoom_scroll_bar : public Fl_Slider {

private:
	//! \brief The current range of values for the scroll bar.
	zc_range<double> value_;

	//! \brief The previous range pre-event.
	zc_range<double> previous_value_;

	//! \brief Validate the current range of values for the scroll bar, ensuring that it is within the bounds of the total range.
	bool validate_range(const zc_range<double>& range) const {
		return bounds().contains(range);
	}

	//! \brief Update the slider button position and size based on the current range of values.
	void update_slider_button();

	//! \brief Convert value to drawn value
	double to_drawn(double v);
	//! \brief Convert value from drawn value
	double from_drawn(double v);

public:

	//! \brief Standard constructor for the zoom scroll bar.
	zc_zoom_scroll_bar(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Set the total range of values for the scroll bar.
	void bounds(const zc_range<double>& range);

	//! \brief Get the total range of values for the scroll bar.
	zc_range<double> bounds() const {
		return zc_range<double>(Fl_Slider::minimum(), Fl_Slider::maximum());
	}

	//! \brief Set the current range of values for the scroll bar.
	void value(const zc_range<double>& range);

	//! \brief Get the current range of values for the scroll bar.
	zc_range<double> value() const {
		return value_;
	}

	//! \brief Handle mouse events for the zoom scroll bar, including dragging and mouse wheel zooming.
	int handle(int event) override;

};

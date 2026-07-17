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
#include "zc_zoom_scroll_bar.h"

#include "zc_range.h"

#include <algorithm>
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Slider.H>

//! \brief Constructor for the zoom scroll bar.
zc_zoom_scroll_bar::zc_zoom_scroll_bar(int X, int Y, int W, int H, const char* L) : Fl_Slider(X, Y, W, H, L), value_(0.0, 1.0) {
}

//! \brief Set the total range of values for the scroll bar.
void zc_zoom_scroll_bar::bounds(const zc_range<double>& range) {
	Fl_Slider::bounds(range.first, range.second);
	// Default value to the full range if the current value is outside the new bounds
	if (!validate_range(value_)) {
		value(range);
	}
	update_slider_button();
}

//! \brief Set the current range of values for the scroll bar.
void zc_zoom_scroll_bar::value(const zc_range<double>& range) {
	if (validate_range(range)) {
		value_ = range;
	} else {
		value_ = range & bounds(); // Intersect with bounds to ensure it's valid
		if (!validate_range(value_)) {
			value_ = bounds(); // If still invalid, reset to full bounds
		}
	}
	// Fl_Slider::draw() appears to start the slider below the value,
    // so we need to adjust it to suit.
	Fl_Slider::value(to_drawn(value_.first)); // Set the slider's value to the first of the range
	update_slider_button();
}

//! \brief Convert value to drawn value (compensates for Fl_Slider adjusting value by slider size)
double zc_zoom_scroll_bar::to_drawn(double v) {
	double draw_value = v - minimum();
	if (bounds().size() - value_.size() > 0) {
		draw_value *= (bounds().size() / (bounds().size() - value_.size()));
	}
	draw_value += minimum();
	return draw_value;
}

//! \brief Convert value from drawn value (uncompensates for Fl_Slider adjusting value)
double zc_zoom_scroll_bar::from_drawn(double v) {
	double set_value = v - minimum();
	if (bounds().size() > 0) {
		set_value *= ((bounds().size() - value_.size()) / bounds().size());
	}
	set_value += minimum();
	return set_value;

}

//! \brief Update the slider button position and size based on the current range of values.
void zc_zoom_scroll_bar::update_slider_button() {
	double total_range = bounds().size();
	double value_range = value_.size();
	if (total_range <= 0) {
		Fl_Slider::slider_size(0.0);
		return;
	}
	double size_ratio = value_range / total_range;
	Fl_Slider::slider_size(size_ratio);
}

//! \brief Handle mouse events for the zoom scroll bar, including dragging and mouse wheel zooming.
int zc_zoom_scroll_bar::handle(int event) {
	// Let the base class handle the event first for scrolling and dragging
	// This will adjust the slider's value based on the mouse position
	// First disable the slider from issuing the callback.
	Fl_When old_when = when();
	when(0); // Temporarily disable callbacks
	if (Fl_Slider::handle(event)) {
		double new_first = from_drawn(Fl_Slider::value());
		if (new_first != value_.first) {
			if (bounds().second - new_first < value_.size()) {
				// Do not allow the first value to go beyond the maximum minus the size of the range
				new_first = bounds().second - value_.size();
				Fl_Slider::value(new_first); // Update the slider's value to the adjusted first value
			}
			double new_second = new_first + value_.size();
			// No need to update the slider value and size here, as the base class already handled it
			value_.first = new_first;
			value_.second = new_second;
			// Reflect the changes in the slider button
			update_slider_button();
		}
		when(old_when); // Restore the original callback setting
		if (when() & FL_WHEN_CHANGED) do_callback(FL_REASON_CHANGED);
		return 1; // Event handled
	}
	else {
		// Handle mouse wheel events for zooming
		if (event == FL_MOUSEWHEEL) {
			int wheel_delta = Fl::event_dy(); // Positive for up, negative for down
			double zoom_factor = 0.1; // 10% zoom per wheel tick
			// Zoom centred on current mouse position within the range
			double mouse_pos;
			if (horizontal()) {
				mouse_pos = static_cast<double>((Fl::event_x() - x() - Fl::box_dx(box()))) / static_cast<double>((w() - Fl::box_dw(box())));
			}
			else {
				mouse_pos = static_cast<double>((Fl::event_y() - y() - Fl::box_dy(box()))) / static_cast<double>((h() - Fl::box_dh(box())));
			}
			double mouse_value = minimum() + mouse_pos * bounds().size();
			// Calculate new range size based on zoom factor and wheel delta
			double range_size = value_.size();
			double new_range_size = range_size * (1.0 - wheel_delta * zoom_factor);
			new_range_size = std::clamp(new_range_size, 0.01, bounds().size()); // Prevent too small or too large
			
			// New first value is calculated to keep the mouse_value at the same relative position in the new range
			// Old delta = mouse_value - value_.first
			double old_delta = mouse_value - value_.first;
			// New delta = old_delta * (new_range_size / range_size)
			double new_delta = old_delta * (new_range_size / range_size);
			double new_first = mouse_value - new_delta;
			double new_second = new_first + new_range_size;
			value(zc_range<double>(new_first, new_second));
			when(old_when); // Restore the original callback setting
			if (when() & FL_WHEN_CHANGED) do_callback(FL_REASON_CHANGED);
			return 1; // Event handled
		}
	}
	when(old_when); // Restore the original callback setting
	return 0; // Event not handled
}
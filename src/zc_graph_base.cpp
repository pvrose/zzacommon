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

#include "zc_graph_base.h"
#include "zc_graph_axis.h"
#include "zc_graph_plot.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>

//! \brief Constructor
zc_graph_base::zc_graph_base(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L) {
}

//! \brief Destructor
zc_graph_base::~zc_graph_base() {
}

//! \brief Add the data to plot.
bool zc_graph_base::add_data_set(data_set_t* ds) {
	// Check that the data types in the data set are supported by this graph.
	if (!data_type_valid(ds->type_a) || !data_type_valid(ds->type_b)) {
		return false;
	}

	data_sets_.push_back(ds);
	convert_data_to_points(data_sets_.back());
	return true;
}

//! \brief Clear all data sets.
void zc_graph_base::clear_data_sets() {
	data_sets_.clear();
}

//! \brief Set the parameters of an axis.
bool zc_graph_base::set_axis_params(const zc_graph_axis::axis_params_t& params) {
	auto it = axes_.find(params.orientation);
	if (it == axes_.end() || !it->second) {
		return false;
	}
	it->second->set_params(params);
	return true;
}

//! Set the range for a data type
bool zc_graph_base::set_data_range(data_type_t type, zc_graph_axis::range new_range) {
	auto axis_type = data_type_to_axis_.find(type);
	if (axis_type == data_type_to_axis_.end()) {
		return false;
	}
	auto axis = axes_.find(axis_type->second);
	if (axis == axes_.end() || !axis->second) {
		return false;
	}
	axis->second->set_range(new_range);
	return true;
}

//! Get the range for the specific axis.
zc_graph_axis::range zc_graph_base::get_data_range(zc_graph_axis::orientation_t orientation) const {
	auto it = axes_.find(orientation);
	if (it == axes_.end() || !it->second) {
		return {0.0F, 1.0F};
	}
	return it->second->get_range();
}

//! \brief override of Fl_Group handle to allow for zooming and scrolling on axes.
int zc_graph_base::handle(int event) {
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

		for (auto& it : axes_) {
			// Continue if the axis is not set up (e.g. for a graph type that doesn't have a Y2 axis).
			if (!it.second) continue;

			// Check if the mouse is over the axis area.
			bool is_x_axis = Fl::belowmouse() == it.second && it.first == zc_graph_axis::X_AXIS;
			bool is_y_axis = Fl::belowmouse() == it.second && 
				(it.first == zc_graph_axis::YL_AXIS || it.first == zc_graph_axis::YR_AXIS);
			if (is_x_axis && is_y_axis) {
				if (shift_pressed) {
					// Scroll by 10 pixels per click if shift is pressed, otherwise zoom.
					it.second->scroll(dy * 10);
				} else {
					int mouse_pos = (it.second->w() > it.second->h()) ? (mouse_x - it.second->x()) : (mouse_y - it.second->y());
					it.second->zoom(mouse_pos, -dy);
				}
				handled = true;

				if (handled) {
					redraw();
					break;
				}
			}
		}
		if (!handled && ctrl_pressed) {
			// If the mouse wheel event was not handled by an axis and Ctrl is pressed, 
			// zoom on all axes.
			for (auto& it : axes_) {
				if (!it.second) continue;
				int mouse_pos = (it.second->w() > it.second->h()) ? (mouse_x - it.second->x()) : (mouse_y - it.second->y());
				it.second->zoom(mouse_pos, -dy);
				handled = true;
			}
			if (handled) {
				redraw();
			}
		}

		if (handled) {
			return 1;
		}
	}

	// Handle double-click on axis to reset zoom.
	else if (event == FL_PUSH && Fl::event_clicks() > 1) {
		for (auto& it : axes_) {
			if (!it.second) continue;
			bool is_x_axis = Fl::belowmouse() == it.second && it.first == zc_graph_axis::X_AXIS;
			bool is_y_axis = Fl::belowmouse() == it.second && 
				(it.first == zc_graph_axis::YL_AXIS || it.first == zc_graph_axis::YR_AXIS);
			if (is_x_axis || is_y_axis) {
				it.second->reset_zoom_and_scroll();
				redraw();
				return 1;
			}
		} 
		// If the double-click was not on an axis, but Ctrl is pressed, reset zoom on all axes.
		if (Fl::event_state() & FL_CTRL) {
			bool handled = false;
			for (auto& it : axes_) {
				if (!it.second) continue;
				it.second->reset_zoom_and_scroll();
				handled = true;
			}
			if (handled) {
				redraw();
				return 1;
			}
	    }
	}

	return Fl_Group::handle(event);
}

// Resize the widget - reset scaling factors
void zc_graph_base::resize(int X, int Y, int W, int H) {
	// If we have actually resized...
	if (X != x() || Y != y() || W != w() || H != h()) {
		// This should resize the group and all child widgets (e.g. axes) according
		// to the resize behaviour. Resizing the axes will update their internal
		// scaling factors which we can now use to update the drawing items.
		Fl_Group::resize(X, Y, W, H);
		redraw();
	}
}

// Draw the graph - override of Fl_Group draw to draw the components of the graph.
void zc_graph_base::draw() {
	// The axes should have been resized by the Fl_Group resize, 
	// so now we can get the new scaling factors from the axes and
	// update the plot data points and grid lines accordingly.
	plot_->clear_data();
	define_plot_xforms();
	// Update the data points for the new scaling factors.
	for (auto& ds : data_sets_) {
		convert_data_to_points(ds);
	}
	// Update the grid for the new scaling factors.
	generate_grid();
	// And redraw the components with the new scaling factors.
	Fl_Group::draw();
}
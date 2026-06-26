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
#include <FL/Fl_Value_Slider.H>

//! A version of FLTK Fl_Value_Slider that allows mouse wheel events to adjust the value.

class zc_wheel_value_slider :
	public Fl_Value_Slider {

public:
	zc_wheel_value_slider(int X, int Y, int W, int H, const char* L = nullptr) :
		Fl_Value_Slider(X, Y, W, H, L)
	{
	}

	int handle(int event) {
		// Handle wheel events.
		// Adjust the value by the wheel dy * step() clamping if necessary.
		if (event == FL_MOUSEWHEEL && Fl::event_inside(this)) {
			double d = static_cast<double>(Fl::event_dy()) * step();
			double pv = value();
			double v = pv + d;
			v = clamp(v);
			value(v);
			redraw();
			if (v != pv) {
				do_callback(FL_REASON_CHANGED);
			}
			return true;
		}
		return Fl_Value_Slider::handle(event);
	}

};
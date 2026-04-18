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

#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

#include <string>

namespace zc {

	//! Create a tip window - data \p tip, position(\p root_x, \p root_y).
	Fl_Window* tip_window(const std::string& tip, int x_root, int y_root);
	//! Returns \p data in upper case.
	std::string to_upper(const std::string& data);
	//! Returns \p data in lower case.
	std::string to_lower(const std::string& data);

	//! Returns the widget of class \p WIDGET that encloses \p w.
	template <class WIDGET>
	WIDGET* ancestor_view(Fl_Widget* w) {
		Fl_Widget* p = w;
		// Keep going up the parent until we found one that casts to WIDGET or we run out of ancestors
		while (p != nullptr && dynamic_cast<WIDGET*>(p) == nullptr) {
			p = (Fl_Widget*)p->parent();
		}
		// Return null if we don't find one, else the one we did
		if (p == nullptr) return nullptr;
		else return dynamic_cast<WIDGET*>(p);
	}

}
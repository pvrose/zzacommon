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
#include "zc_tabs_nonav.h"

#include <FL/Enumerations.H>

zc_tabs_nonav::zc_tabs_nonav(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L) {

}

zc_tabs_nonav::~zc_tabs_nonav() {}

int zc_tabs_nonav::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return Fl_Tabs::handle(event);
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_Left:
		case FL_Right:
			// Deliberately ignore
			return false;
		}
	}
	return Fl_Tabs::handle(event);
};

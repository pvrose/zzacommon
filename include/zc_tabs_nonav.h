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
//! This provides a version of Fl_Tabs without the navigation
//! 
#pragma once

#include <FL/Fl_Tabs.H>

//! \brief This class implements a version of Fl_Tabs with navigation by
//! individual tabs using the arrow buttons inhibited.
class zc_tabs_nonav : public Fl_Tabs {

public:

	//! \brief Constructor.
	//! \param X,Y,W,H,L Standard FLTK widget parameters.
	zc_tabs_nonav(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor.
	virtual ~zc_tabs_nonav();

	//! Override of handle to inhibit navigation by arrow button.
	virtual int handle(int event);

};

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
#include "zc_button_input.h"

//! \brief This class implements a combined input and button.
//! The button chnages the display mode of the input between clear text and hidden text. 
class zc_password_input :
    public zc_button_input
{

public:
    //! \brief Constructor.
    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    zc_password_input(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~zc_password_input() {};

    //! Callback to switch input between FL_NORMAL_INPUT and FL_SECRET_INPUT modes.
    static void cb_button(Fl_Widget* w, void* v);



};


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

pragma once

#include "zc_button_input.h"

//! \brief This class is a widget that combines a button that opens a calendar browser 
//! and input widget.

//! The button opens a calendar widget to select a date.
//! The input allows the date to be typed in (and displays that selected by the calendar).
class zc_calendar_input :
    public zc_button_input
{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    zc_calendar_input(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor.
    ~zc_calendar_input();

    //! Override button_input::callback to intercept any value returned by it.
    virtual void callback(Fl_Callback* cb, void* v);
    //! Override button_input::user_data(void*) to forward user data to components.
    virtual void user_data(void* v);

    //! Callback for the button.
    
    //! When the button is pressed, any data in the input is passed to the
    //! calendar with a default of the current date. The calendar is shown 
    //! at the position of the button
    //! pending any interaction with it.
    static void cb_button(Fl_Widget* w, void* v);

    //! Callback for the calendar
    static void cb_calendar(Fl_Widget* w, void* v);

    //! Set format to \p value.
    void format(const char* value);

    //! Get format.
    const char* format();

protected:
    //! Date display format - defaults to (eg) 20251228.
    const char* format_ = "%Y%m%d";
};


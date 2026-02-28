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

#include <FL/Enumerations.H>

//! \file 
//! This file defines extra symbols  to be used by FLTK

//! Draw an open eye (<B>\@eyeopen</B>) - for password inputs indicating plain text.
void draw_eyeopen(Fl_Color c);
//! Draw a shut eye (<B>\@eyeshut</B>) - for password inputs indicating hidden text.
void draw_eyeshut(Fl_Color c);
//! Draw a calendar (<B>\@calendar</B>) - for calendar input widget.
void draw_calendar(Fl_Color c);
//! Draw a letter image (<B>\@mail</B>) - as a label image for e-Mail.
void draw_mail(Fl_Color c);


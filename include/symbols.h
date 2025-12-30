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


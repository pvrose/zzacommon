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

//! \brief This class provides a line style structure and related dialog.
//! The line style comprises a colour, a width and a pattern (solid, dashed, dotted etc).

#include "zc_button_dialog.h"

#include <FL/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>

// Forward declaration of FLTK classes.
class Fl_Hold_Browser;
class Fl_Color_Chooser;


//! The line style structure.
struct zc_line_style
{
	//! The line colour.
	Fl_Color colour;
	//! The line width.
	int width;
	//! The line pattern.
	int style;
	//! Default constructor. Sets the line style to a solid black line of width 1.
	zc_line_style() :
		colour(FL_BLACK),
		width(1),
		style(FL_SOLID)
	{}
	//! Constructor with parameters.
	zc_line_style(Fl_Color c, int w, int s) :
		colour(c),
		width(w),
		style(s)
	{
	}
};

//! \brief This class provides a dialog for editing a line style.
//! The dialog allows the user to select a colour, width and pattern for a line style.
class zc_line_style_dialog :
	public Fl_Double_Window
{
public:
	//! Constructor. 
	//! The dialog defines its own size and layout.
	zc_line_style_dialog();
	//! Destructor.
	~zc_line_style_dialog();

	//! Set the line style to be edited.
	//! \param ls The line style to be edited.
	void set_data(const zc_line_style& ls);
	//! Get the line style that has been edited.
	const zc_line_style& get_data() const;

	//! Shwo the dialog and wait for OK or Cancel to be clicked. 
	//! Returns true if OK, false if Cancel.
	bool show_dialog();

private:
	//! The line style being edited.
	zc_line_style ls_;

	//! Callback function for the OK button.
	static void cb_ok(Fl_Widget* w, void* data);
	//! Callback function for the Cancel button.
	static void cb_cancel(Fl_Widget* w, void* data);
	//! Callback function for the colour chooser.
	static void cb_colour(Fl_Widget* w, void* data);
	//! Callback function for the width browser.
	static void cb_width(Fl_Widget* w, void* data);
	//! Callback function for the style browser.
	static void cb_style(Fl_Widget* w, void* data);

	//! Update the display of the line style in the dialog.
	void update_display();

	//! Update the browsers to show the current line style.
	void update_browsers();

	//! The output line style display widget.
	zc_button_dialog<zc_line_style_dialog, zc_line_style>* output_;
	//! The colour chooser widget.
	Fl_Color_Chooser* colour_chooser_;
	//! The width drop-down browser widget.
	Fl_Hold_Browser* width_browser_;
	//! The style drop-down browser widget.
	Fl_Hold_Browser* style_browser_;

	//! OK button clicked;
	bool ok_clicked_ = false;

};

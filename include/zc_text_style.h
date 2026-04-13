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

//! \brief This class provides a text style structure and related dialog.
//! The text style comprises a font, a size and a colour.

#include "zc_button_dialog.h"

//! Include FLTK headers for the types used in the text style structure and dialog.
#include <FL/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Widget.H>

//! Include nlohmann/json for JSON serialization of the text style.
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declaration of FLTK classes.
class Fl_Hold_Browser;
class Fl_Color_Chooser;
class Fl_Output;

//! The text style structure, allowing a font, size and colour to be specified for text.
struct zc_text_style
{
	//! The text colour.
	Fl_Color colour;
	//! The text font.
	Fl_Font font;
	//! The text size.
	Fl_Fontsize size;
	//! Default constructor. Sets the text style to a black, default size, default font.
	zc_text_style() :
		colour(FL_BLACK),
		font(FL_HELVETICA),
		size(12)
	{
	}
	//! Constructor with parameters.
	zc_text_style(Fl_Color c, Fl_Font f, Fl_Fontsize s) :
		colour(c),
		font(f),
		size(s)
	{
	}
};

//! \brief This class provides a dialog for editing a text style.
//! 
//! The dialog allows the user to select a font, size and colour for a text style.
class zc_text_style_dialog :
	public Fl_Double_Window
{
public:
	//! Constructor.
	zc_text_style_dialog();
	//! Destructor.
	~zc_text_style_dialog();

	//! Set the text style to be edited.
	void set_data(const zc_text_style& ts);
	//! Get the text style from the dialog.
	const zc_text_style& get_data() const;

	//! Show thw dialog and wait for OK or Cancel to be clicked.
	//! \p return true if OK was clicked, false if Cancel was clicked or the dialog was closed.
	bool show_dialog();

private:
	//! The text style being edited.
	zc_text_style ts_;

	//! Callback - OK button.
	static void cb_bn_ok(Fl_Widget* w, void* v);
	//! Callback - cancel button.
	static void cb_bn_cancel(Fl_Widget* w, void* v);
	//! Callback - font chooser.
	static void cb_font(Fl_Widget* w, void* v);
	//! Callback - size chooser.
	static void cb_size(Fl_Widget* w, void* v);
	//! Callback - colour chooser.
	static void cb_colour(Fl_Widget* w, void* v);

	//! Update the display of the text style in the dialog.
	void update_sample();

	//! Update the browsers to show the current text style.
	void update_browsers();

	//! Populate the font chooser, setting current selection to \p f.
	//! \param w The font chooser widget to populate.
	//! \param ts The text style for which to populate the font chooser, used to set the current selection.
	void populate_font(Fl_Widget* w, const zc_text_style& ts);

	//! Populate the size chooser, for font \p f, setting selection to \p sz.
	//! \param w The size chooser widget to populate.
	//! \param ts The text style for which to populate the size chooser, used to set the current selection.
	void populate_size(Fl_Widget* w, const zc_text_style& ts);

	//! The output widget used to show a sample of the text style.
	zc_button_dialog<zc_text_style_dialog, zc_text_style>* output_;
	//! The browsers and choosers for the font, size and colour.
	Fl_Color_Chooser* colour_chooser_;
	//! A drop-down browser is used for font selection as this allows the user to see the available options.
	Fl_Hold_Browser* font_browser_;
	//! A drop-down browser is used for size selection.
	Fl_Hold_Browser* size_browser_;

	//! OK button clicked flag.
	bool ok_clicked_ = false;
};

//! JSON serialization from zc_text_style.
void to_json(json& j, const zc_text_style& s);
//! JSON deserialization to zc_text_style.
void from_json(const json& j, zc_text_style& s);

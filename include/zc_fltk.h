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

#include <array>
#include <string>

//! Default sizes
extern const int DEFAULT_DEFAULT_SIZE;    //!< Default base size - used to restore default sizes
extern int DEFAULT_SIZE;            //!< Default base size - also used to calculate other sizes
extern Fl_Fontsize FONT_SIZE;       //!< Default font size 
extern Fl_Font FONT;                //!< Default font
extern int MENU_HEIGHT;             //!< Height of menu bar
extern int TOOL_HEIGHT;             //!< Height of tool bar
extern int FOOT_HEIGHT;             //!< Height of footer
extern int TOOL_GAP;                //!< Gap between tools
extern int BORDER_SIZE;             //!< Size of border
extern int TAB_HEIGHT;              //!< Height of tab bar

//! Default sizes for drawing
extern int HBUTTON;                 //!< Height of a normal button
extern int WBUTTON;                 //!< Width of a normal button
extern int GAP;                     //!< Gap between elements
extern int HTEXT;                   //!< Height of text
extern int WRADIO;                  //!< Width of radio button
extern int HRADIO;                  //!< Height of radio button
extern int WLABEL;                  //!< Width of label
extern int WLLABEL;                 //!< Width of large label
extern int HMLIN;                   //!< Height of multiline input
extern int WEDIT;                   //!< Width of edit box
extern int WSMEDIT;                 //!< Width of small edit box
extern int ROW_HEIGHT;              //!< Height of a row

// Colours to use for buttons - defined using FLTK colour palette
const Fl_Color COLOUR_ORANGE = 93;       /*!< R=4/4, B=0/4, G=5/7 */
const Fl_Color COLOUR_APPLE = 87;        /*!< R=3/4, B=0/4, G=7/7 */
const Fl_Color COLOUR_PINK = 170;        /*!< R=4/4, B=2/4, G=2/7 */
const Fl_Color COLOUR_MAUVE = 212;       /*!< R-4/4, B=3/4, G=4/7 */
const Fl_Color COLOUR_NAVY = 136;        /*!< R=0/4, B=2/4, G=0/7 */
const Fl_Color COLOUR_CLARET = 80;       /*!< R=3/4, B=0/4, G=0/4 */
const Fl_Color COLOUR_GREY = fl_color_average(FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR, 0.33F);
//!< One third between fotreground and background colours.
const Fl_Color COLOUR_BROWN = 74;       /*!< R=2/4, B=0/4, G=2/7 */

// Colour codes for the digits 0-9, based on the resistor colour code.
const std::array<Fl_Color, 10> COLOUR_CODE = {
	FL_BLACK, COLOUR_BROWN, FL_RED, COLOUR_ORANGE, FL_YELLOW, FL_GREEN, FL_BLUE, COLOUR_MAUVE, COLOUR_GREY, FL_WHITE };

namespace zc {

	//! Create a tip window - data \p tip, position(\p root_x, \p root_y).
	Fl_Window* tip_window(const std::string& tip, int x_root, int y_root);
	//! Returns \p data in upper case.
	std::string to_upper(const std::string& data);
	//! Returns \p data in lower case.
	std::string to_lower(const std::string& data);

	//! Customise the look and feel of GM3ZZA's FLTK applications.
	void customise_fltk(int base_size = DEFAULT_DEFAULT_SIZE);

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

	//! Change the base size by \p increase (true to increase, false to decrease). Returns true if the size was changed, false if it was already at the limit.
	//! \param increase If true, will increase the size, if false will decrease it.
	//! \param update If true, will update the sizes even if the size was not changed.
	bool change_base_size(bool increase, bool update = true);

	//! Set a new base size. Returns true if the size was changed, false if it was already 
	//! at the limit.
	//! \param new_size The new base size to set. Will be limited to the range [MIN_SIZE, MAX_SIZE].
	//! \param update If true, will update the sizes even if the new size is the same as the current size.
	bool set_base_size(int new_size, bool update = true);

	//! Update all the sizes based on the current base size.
	void update_sizes();

}
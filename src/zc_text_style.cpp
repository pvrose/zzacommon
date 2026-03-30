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

#include "zc_text_style.h"

// Include zzacommon classes.
#include "zc_drawing.h"
#include "zc_button_dialog.h"

// Include FLTK classes.
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>

// Include json
#include <nlohmann/json.hpp>
using json = nlohmann::json;



const int WDLG = 2 * GAP + WEDIT + WBUTTON + 200; //!< Width of the text style dialog.	
const int HDLG = GAP + HTEXT + 94 + GAP + HBUTTON + GAP; //!< Height of the text style dialog.

//! Dialog constructor
zc_text_style_dialog::zc_text_style_dialog() :
	Fl_Double_Window(WDLG, HDLG, "Text Style Dialog")
{
	int curr_x = GAP;
	int curr_y = GAP + HTEXT;

	// Font selection browser
	Fl_Hold_Browser* w01 = new Fl_Hold_Browser(curr_x, curr_y, WEDIT, HMLIN, "Font");
	w01->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	w01->tooltip("Please select the font used for the QSL card field entry");
	w01->callback(cb_font, &ts_.font);
	font_browser_ = w01;

	curr_x += w01->w();
	// Size selection browser
	Fl_Hold_Browser* w02 = new Fl_Hold_Browser(curr_x, curr_y, WBUTTON, HMLIN, "Size");
	w02->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	w02->callback(cb_size, &ts_.size);
	w02->tooltip("Please select the font size used for the QSL card field entry");
	size_browser_ = w02;

	curr_x += w02->w();
	// Colour chooser
	Fl_Color_Chooser* w03 = new Fl_Color_Chooser(curr_x, curr_y, 200, 94, "Colour");
	w03->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	w03->callback(cb_colour);
	w03->tooltip("Please select the colour to be used for the QSL card field entry");
	colour_chooser_ = w03;

	curr_x = GAP + WLABEL;
	curr_y += HMLIN + GAP;

	// Output to display sample text in the chosen font, size and colour
	output_ = new zc_button_dialog<zc_text_style_dialog, zc_text_style>(curr_x, curr_y, WSMEDIT, HBUTTON, "Sample");
	output_->align(FL_ALIGN_INSIDE);
	output_->type(ZC_BUTTON_DIALOG_OUTPUT);
	output_->button()->label("Example Text");
	output_->box(FL_BORDER_BOX);
	output_->color(FL_WHITE);
	output_->tooltip("Selected font, size and colour will be shown here");
	update_sample();

	curr_x = w03->x();
	curr_y = w03->y() + w03->h() + GAP;

	// Save button
	Fl_Button* w04 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Save");
	w04->callback(cb_bn_ok);
	w04->tooltip("Accept these changes");

	curr_x += WBUTTON;
	// Cancel button
	Fl_Button* w05 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Cancel");
	w05->callback(cb_bn_cancel);
	w05->tooltip("Cancel these changes");

	curr_x += WBUTTON;
	curr_y += HBUTTON;
	end();

}

//! Destructor
zc_text_style_dialog::~zc_text_style_dialog() {}

//! Set the text style to be edited.
void zc_text_style_dialog::set_data(const zc_text_style& ts) {
	ts_ = ts;
	update_browsers();
	update_sample();
}

//! Get the text style from the dialog.
const zc_text_style& zc_text_style_dialog::get_data() const {
	return ts_;
}

//! Show thw dialog and wait for OK or Cancel to be clicked.
bool zc_text_style_dialog::show_dialog() {
	// Implementation of show function goes here.
	Fl_Double_Window::show();
	// Wait for the dialog to be closed by the OK or Cancel button.
	while (shown()) { Fl::check(); }
	return ok_clicked_;
}

//! Update the display of the text style in the dialog.
void zc_text_style_dialog::update_sample() {
	output_->value(ts_);
	output_->redraw();
}

//! Update the browsers to show the current text style.
void zc_text_style_dialog::update_browsers() {
	populate_font(font_browser_, ts_);
	populate_size(size_browser_, ts_);
	uchar r, g, b;
	// Fl_Color_Chooser uses RGB values 0->1.0
	Fl::get_color(ts_.colour, r, g, b);
	colour_chooser_->rgb((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);
	redraw();
}

//! Populate the font chooser, setting current selection to \p ts.font.
//! \param w The font chooser widget to populate.
//! \param ts The text style for which to populate the font chooser, used to set the current selection.
void zc_text_style_dialog::populate_font(Fl_Widget* w, const zc_text_style& ts) {
	Fl_Hold_Browser* br = (Fl_Hold_Browser*)w;
	br->clear();
	// Only get FLTK default fonts
	for (int i = 0; i < FL_FREE_FONT; i++) {
		// Contains any combination of FL_BOLD and FL_ITALIC
		const char* name = Fl::get_font_name(Fl_Font(i), nullptr);
		char buffer[128];
		// display in the named font
		sprintf(buffer, "@F%d@.%s", i, name);
		br->add(buffer);
	}
	br->value(ts.font + 1);
}

//! Populate the size chooser, for font \p ts.font, setting selection to \p ts.size.
void zc_text_style_dialog::populate_size(Fl_Widget* w, const zc_text_style& ts) {
	Fl_Hold_Browser* br = (Fl_Hold_Browser*)w;
	br->clear();
	int* sizes;
	// Get the array of sizes available for thsi font
	int num_sizes = Fl::get_font_sizes(ts.font, sizes);
	if (num_sizes) {
		// We have sizes available
		if (sizes[0] == 0) {
			// {0} indicates a scaleable font - so any size available 
			// Add 1 to largest available (limited to 64)
			for (int i = 1; i < std::max(64, sizes[num_sizes - 1]); i++) {
				char buff[20];
				sprintf(buff, "%d", i);
				br->add(buff);
			}
			br->value(ts.size);
		}
		else {
			// The font only comes in defined sizes
			int select = 0;
			for (int i = 0; i < num_sizes; i++) {
				// The selected size will the largest <= the current size 
				if (sizes[i] < ts.size) {
					select = i;
				}
				char buff[20];
				sprintf(buff, "%d", sizes[i]);
				br->add(buff);
			}
			br->value(select);
		}
	}
}

//! Callback - OK button.
void zc_text_style_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	zc_text_style_dialog* dlg = (zc_text_style_dialog*)w->parent();
	dlg->ok_clicked_ = true;
	dlg->hide();
}

//! Callback - cancel button.
void zc_text_style_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	zc_text_style_dialog* dlg = (zc_text_style_dialog*)w->parent();
	dlg->ok_clicked_ = false;
	dlg->hide();
}

//! Callback - font chooser.
void zc_text_style_dialog::cb_font(Fl_Widget* w, void* v) {
	zc_text_style_dialog* dlg = (zc_text_style_dialog*)w->parent();
	Fl_Hold_Browser* b = (Fl_Hold_Browser*)w;
	int index = b->value();
	if (index > 0) {
		dlg->ts_.font = index - 1;
		dlg->update_sample();
		dlg->populate_size(dlg->size_browser_, dlg->ts_);
	}
}

//! Callback - size chooser.
void zc_text_style_dialog::cb_size(Fl_Widget* w, void* v) {
	zc_text_style_dialog* dlg = (zc_text_style_dialog*)w->parent();
	Fl_Hold_Browser* b = (Fl_Hold_Browser*)w;
	int index = b->value();
	if (index > 0) {
		dlg->ts_.size = std::stoi(b->text(index));
		dlg->update_sample();
	}
}

//! Callback - colour chooser.
void zc_text_style_dialog::cb_colour(Fl_Widget* w, void* v) {
	zc_text_style_dialog* dlg = (zc_text_style_dialog*)w->parent();
	Fl_Color_Chooser* cc = (Fl_Color_Chooser*)w;
	// Convert RGB 0->1.0 to 0->255
	dlg->ts_.colour = fl_rgb_color(255 * cc->r(), 255 * cc->g(), 255 * cc->b());
	dlg->update_sample();
}

template<>
void zc_button_dialog<zc_text_style_dialog, zc_text_style>::update_button_label(zc_text_style ts) {
	bn_->labelfont(ts.font);
	bn_->labelsize(ts.size);
	bn_->labelcolor(ts.colour);
}

void to_json(json& j, const zc_text_style& s) {
	j = json{
		{ "Font", s.font },
		{ "Size", s.size },
		{ "Colour", (unsigned)s.colour }
	};
}

void from_json(const json& j, zc_text_style& s) {
	j.at("Font").get_to(s.font);
	j.at("Size").get_to(s.size);
	j.at("Colour").get_to(s.colour);
}
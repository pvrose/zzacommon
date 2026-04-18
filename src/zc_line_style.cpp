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

#include "zc_line_style.h"

#include "zc_button_dialog.h"
#include "zc_drawing.h"

// Include FLTK classes.
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Image_Surface.H>

// Include C++ classes.
#include <cstdint>
#include <cstdio>

struct zc_ls_style_t {
	const char* name;
	int style;
} zc_ls_line_styles[] = {
	{"Solid", FL_SOLID},
	{"Dashed", FL_DASH},
	{"Dotted", FL_DOT},
	{"Dash-Dot", FL_DASHDOT},
	{"Dash-Dot-Dot", FL_DASHDOTDOT},
	{nullptr, 0}
};

const int WDLG = 2 * GAP + WSMEDIT + WBUTTON + 200; //!< Width of the line style dialog.
const int HDLG = GAP + HTEXT + 94 + GAP + HBUTTON + GAP; //!< Height of the line style dialog.
//! Constructor for the line style dialog.
zc_line_style_dialog::zc_line_style_dialog() :
	Fl_Double_Window(WDLG, HDLG, "Line Style Dialog")
{
	int cx = GAP;
	int cy = HTEXT;
	// The line style browser.
	Fl_Hold_Browser* w01 = new Fl_Hold_Browser(cx, cy, WSMEDIT, HMLIN, "Style");
	w01->align(FL_ALIGN_TOP);
	w01->callback(cb_style, this);
	w01->tooltip("Select the line pattern");
	for (int i = 0; zc_ls_line_styles[i].name != nullptr; i++) {
		w01->add(zc_ls_line_styles[i].name);
	}
	style_browser_ = w01;
	cx += WSMEDIT;
	// The line width browser.
	Fl_Hold_Browser* w02 = new Fl_Hold_Browser(cx, cy, WBUTTON, HMLIN, "Width");
	w02->align(FL_ALIGN_TOP);
	w02->callback(cb_width, this);
	w02->tooltip("Select the line width");
	for (int i = 1; i <= 10; i++) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d", i);
		w02->add(buf);
	}
	width_browser_ = w02;
	cx += WBUTTON;
	// The line colour chooser.
	Fl_Color_Chooser* w03 = new Fl_Color_Chooser(cx, cy, 200, 94, "Colour");
	w03->align(FL_ALIGN_TOP);
	w03->callback(cb_colour, this);
	w03->tooltip("Select the line colour");
	colour_chooser_ = w03;

	// The output line style display.
	cx = GAP + WLABEL;
	cy += HMLIN + GAP;
	zc_button_dialog<zc_line_style_dialog, zc_line_style>* w04 =
		new zc_button_dialog<zc_line_style_dialog, zc_line_style>(cx, cy, WBUTTON, HBUTTON, "Output");
	w04->type(zc_button_dialog_type::ZC_BUTTON_DIALOG_OUTPUT);
	output_ = w04;
	// The output OK and Cancel buttons.
	cx = w03->x();
	cy = w03->y() + w03->h() + GAP;
	Fl_Button* w05 = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "OK");
	w05->callback(cb_ok, this);
	w05->tooltip("Accept the line style and close the dialog");
	cx += WBUTTON;
	Fl_Button* w06 = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Cancel");
	w06->callback(cb_cancel, this);
	w06->tooltip("Cancel the changes and close the dialog");

	end();

}
  
//! Destructor for the line style dialog.
zc_line_style_dialog::~zc_line_style_dialog() {
}

//! Set the line style to be edited.
//! \param ls The line style to be edited.
//! This copies the line style into the dialog and updates the display.
void zc_line_style_dialog::set_data(const zc_line_style& ls) {
	ls_ = ls;
	update_browsers();
	update_display();
}

//! Get the line style that has been edited.
//! \return The line style that has been edited.
const zc_line_style& zc_line_style_dialog::get_data() const {
	return ls_;
}

//! Show the dialog and wait for OK or Cancel to be clicked.
//! \return true if OK, false if Cancel.
bool zc_line_style_dialog::show_dialog() {
	// Implementation of show function goes here.
	Fl_Double_Window::show();
	// Wait for the dialog to be closed by the OK or Cancel button.
	while (shown()) { Fl::check(); }
	if (ok_clicked_) {
		return 1;
	}
	else {
		return 0;
	}
}

// Callback function for the OK button.
void zc_line_style_dialog::cb_ok(Fl_Widget* w, void* data) {
	zc_line_style_dialog* dlg = (zc_line_style_dialog*)w->parent();
	dlg->ok_clicked_ = true;
	dlg->hide();
}

// Callback function for the Cancel button.
void zc_line_style_dialog::cb_cancel(Fl_Widget* w, void* data) {
	zc_line_style_dialog* dlg = (zc_line_style_dialog*)w->parent();
	dlg->ok_clicked_ = false;
	dlg->hide();
}

// Callback function for the colour chooser.
void zc_line_style_dialog::cb_colour(Fl_Widget* w, void* data) {
	zc_line_style_dialog* dlg = (zc_line_style_dialog*)w->parent();
	Fl_Color_Chooser* cc = (Fl_Color_Chooser*)w;
	dlg->ls_.colour = fl_rgb_color(255 * cc->r(), 255 * cc->g(), 255 * cc->b());
	dlg->update_display();
}

// Callback function for the width browser.
void zc_line_style_dialog::cb_width(Fl_Widget* w, void* data) {
	zc_line_style_dialog* dlg = (zc_line_style_dialog*)w->parent();
	Fl_Hold_Browser* b = (Fl_Hold_Browser*)w;
	int index = b->value();
	if (index > 0) {
		dlg->ls_.width = index;
		dlg->update_display();
	}
}

// Callback function for the style browser.
void zc_line_style_dialog::cb_style(Fl_Widget* w, void* data) {
	zc_line_style_dialog* dlg = (zc_line_style_dialog*)w->parent();
	Fl_Hold_Browser* b = (Fl_Hold_Browser*)w;
	int index = b->value();
	if (index > 0) {
		dlg->ls_.style = zc_ls_line_styles[index - 1].style;
		dlg->update_display();
	}
}

// Update the browsers to show the current line style.
void zc_line_style_dialog::update_browsers() {
	// Update the style browser.
	for (int i = 0; zc_ls_line_styles[i].name != nullptr; i++) {
		if (ls_.style == zc_ls_line_styles[i].style) {
			style_browser_->value(i + 1);
			break;
		}
	}
	// Update the width browser.
	width_browser_->value(ls_.width);
	// Update the colour chooser.
	uint8_t r, g, b;
	// Fl_Color_Chooser uses RGB values 0->1.0
	Fl::get_color(ls_.colour, r, g, b);
	colour_chooser_->rgb((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);

}

// Update the display of the line style in the dialog.
void zc_line_style_dialog::update_display() {
	// Update the output line style display.
	output_->value(ls_);
}

// The update_button_label function for the output line style display.
template <>
void zc_button_dialog<zc_line_style_dialog, zc_line_style>::update_button_label(zc_line_style ls) {
	// Draw an image of the line style onto the button.
	// Create the drawing surface - origin will be top-left of the widget
	Fl_Image_Surface* image_surface = new Fl_Image_Surface(bn_->w(), bn_->h());
	Fl_Surface_Device::push_current(image_surface);
	// Draw the background
	fl_color(color());
	fl_rectf(0, 0, bn_->w(), bn_->h());
	// Set the line end coordinates.
	int x1 = bn_->w() / 10;
	int y1 = bn_->h() / 2;
	int x2 = bn_->w() - bn_->w() / 10;
	int y2 = bn_->h() / 2;
	// Set the line colour, width and style.
	fl_color(ls.colour);
	fl_line_style(ls.style, ls.width);
	fl_line(x1, y1, x2, y2);
	fl_line_style(0); // reset to default line style
	Fl_RGB_Image* image = image_surface->image();
	Fl_Surface_Device::pop_current();
	delete image_surface;

	bn_->label(nullptr);
	bn_->image(image);
	redraw();
}
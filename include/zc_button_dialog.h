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

// FLTK includes
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

// C++ includes
#include <cstdint>

class Fl_Button;


//! The button can be used either normally ot just to display the current value.
enum zc_button_dialog_type : uint8_t
{
	ZC_BUTTON_DIALOG_INOUT,     //!< Normal behaviour. Opens the dialog.
	ZC_BUTTON_DIALOG_OUTPUT     //!< The button is just used to display the current value.
};

//! \brief This class provides a button that opens a dialog when clicked.
//! This is a template class. 
//! 
//! The template parameters specify the dialog to be opened,
//! and the structure of the data to be passed to the dialog and returned from it.

template <class DIALOG, class DATA>
class zc_button_dialog :
	public Fl_Group
{
protected:
	//! The button widget.
	Fl_Button* bn_;

	//! Callback function for the button. 
	//! This should open the dialog and pass it the data.
	static void cb_button(Fl_Widget* w, void* data)
	{
		zc_button_dialog* bd = (zc_button_dialog*)w->parent();
		DIALOG dlg;
		dlg.set_data(*(DATA*)data);
		if (dlg.show_dialog())
			*(DATA*)data = dlg.get_data();
		bd->do_callback();
	}

	//! The data to be passed to the dialog and returned from it.
	DATA data_;

	zc_button_dialog_type type_;


	void update_button_label(DATA);


public:
	//! Constructor.
	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	zc_button_dialog<DIALOG, DATA>(int X, int Y, int W, int H, const char* L = 0) :
		Fl_Group(X, Y, W, H, L)
	{
		type_ = ZC_BUTTON_DIALOG_INOUT;
		bn_ = new Fl_Button(X, Y, W, H);
		bn_->callback(cb_button, (void*)&data_);
		Fl_Group::end();
	}

	//! Destructor.
	~zc_button_dialog<DIALOG, DATA>()
	{
	}

	//! Forward the button's when condition to the button.
	virtual void when(int w) { bn_->when(w); }
	//! \see when
	virtual int when() const { return bn_->when(); }

	//! Forward the button's user_data to the button.
	virtual void user_data(void* v) { bn_->user_data(v); }
	//! \see user_data
	virtual void* user_data() const { return bn_->user_data(); }

	//! Set the data to be passed to the dialog and returned from it.
	void value(const DATA& d) { 
		data_ = d; 
	    update_button_label(data_);
		redraw();
	}
	//! Get the data to be passed to the dialog and returned from it.
	const DATA& value() const { return data_; }

	//! Set the button type.
	void type(zc_button_dialog_type t) { 
		type_ = t; 
		// If the button is just used to display the current value,
		// then disable the button callback by setting the when
		// condition to FL_WHEN_NEVER.
		if (type_ == ZC_BUTTON_DIALOG_OUTPUT)
			bn_->when(FL_WHEN_NEVER);
		}
	//! Get the button type.
	zc_button_dialog_type type() const { return type_; }

	//! Get at the button widget, for example to set the label.
	Fl_Button* button() { return bn_; }

};

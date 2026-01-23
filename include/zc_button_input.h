#pragma once

#include <FL/Enumerations.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_types.h>
#include <FL/Fl_Widget.H>

class Fl_Button;

//! \brief This class provides an input plus a button combination where the 
//! button affects the behaviour of the overall widget.

//! The button may affect the appearance or bahaviour of the input,
//! or it may open a dialog that provides an alternate way of 
//! specifying the text.
//! Examples include:
//!   calendar_input: The button opens a display of a calendar to allow a date to be selected.
//!   filename_input: The button opens a file dialog to allow the user to search for a file.
//!   password_input: The button toggles the input between a normal input (where the text is
//! visible) and a secret input (where it is replaced by asterisks). 

//! \note This class has only a header file. The default implmentations may be used,
//! but it is generally expected that inheritees implement their own methods.
class zc_button_input :
	public Fl_Group
{
protected:

	//! The Fl_Input widget.
	Fl_Input* ip_;
	//! The Fl_Button widget.
	Fl_Button* bn_;

public:
	//! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
	zc_button_input(int X, int Y, int W, int H, const char* L = nullptr) :
		Fl_Group(X, Y, W, H, L)
	{
		box(FL_FLAT_BOX);
		// Set the button width to H or 0.2 W whichever is less
		int bw = H < W / 5 ? H : W / 5;

		ip_ = new Fl_Input(X, Y, W - bw, H);
		bn_ = new Fl_Button(X + W - bw, Y, bw, H);

		end();

	};
	//! Destructor
	~zc_button_input() {};

	//! The default is to set the callback of the Fl_Input to the specified callback.
	virtual void callback(Fl_Callback* cb, void* v) { ip_->callback(cb, v); }
	//! \see callback
	virtual void callback(Fl_Callback* cb) { ip_->callback(cb); }
	//! \see callback
	virtual Fl_Callback_p callback() { return ip_->callback(); }

	//! The default sets the when condition onto the Fl_Input component.
	virtual void when(uchar i) { ip_->when(i); }
	//! \see when
	virtual Fl_When when() { return ip_->when(); }

	//! The default forwards the user data to the Fl_Input component.
	virtual void user_data(void* v) { ip_->user_data(v); }
	//! \see user_data
	virtual void* user_data() { return ip_->user_data(); }

	//! The default forwards the value to the Fl_Input component.
	void value(const char* d) { ip_->value(d); }
	//! \see value
	const char* value() { return ip_->value(); }

	//! Returns the internal Fl_Input widget
	Fl_Input* input() { return ip_; };
	//! Returns the internal Fl_Button widget.
	Fl_Button* button() { return bn_; };


};


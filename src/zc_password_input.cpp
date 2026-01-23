#include "zc_password_input.h"

#include <zc_button_input.h>

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Widget.H>

zc_password_input::zc_password_input(int X, int Y, int W, int H, const char* L) :
	zc_button_input(X, Y, W, H, L)
{
	ip_->type(FL_SECRET_INPUT);
	bn_->label("@eyeshut");
	bn_->callback(cb_button);
}

void zc_password_input::cb_button(Fl_Widget* w, void* v) {
	zc_password_input* g = (zc_password_input*)w->parent();
	Fl_Input* ip = g->ip_;
	Fl_Button* bn = g->bn_;
	switch (ip->type()) {
	case FL_SECRET_INPUT:
		ip->type(FL_NORMAL_INPUT);
		bn->label("@eyeopen");
		break;
	case FL_NORMAL_INPUT:
		ip->type(FL_SECRET_INPUT);
		bn->label("@eyeshut");
		break;
	}
	ip->redraw();
}

#include "tabs_nonav.h"

#include <FL/Enumerations.H>

tabs_nonav::tabs_nonav(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L) {

}

tabs_nonav::~tabs_nonav() {}

int tabs_nonav::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return Fl_Tabs::handle(event);
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_Left:
		case FL_Right:
			// Deliberately ignore
			return false;
		}
	}
	return Fl_Tabs::handle(event);
};

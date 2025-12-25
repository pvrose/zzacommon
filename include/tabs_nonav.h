//! This provides a version of Fl_Tabs without the navigation
//! 
#pragma once

#include <FL/Fl_Tabs.H>

class tabs_nonav : public Fl_Tabs {

public:

	tabs_nonav(int X, int Y, int W, int H, const char* L = nullptr);

	virtual ~tabs_nonav();

	virtual int handle(int event);

};

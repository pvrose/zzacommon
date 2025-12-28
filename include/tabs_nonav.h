//! This provides a version of Fl_Tabs without the navigation
//! 
#pragma once

#include <FL/Fl_Tabs.H>

//! \brief This class implements a version of Fl_Tabs with navigation by
//! individual tabs using the arrow buttons inhibited.
class tabs_nonav : public Fl_Tabs {

public:

	//! \brief Constructor.
	//! \param X,Y,W,H,L Standard FLTK widget parameters.
	tabs_nonav(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor.
	virtual ~tabs_nonav();

	//! Override of handle to inhibit navigation by arrow button.
	virtual int handle(int event);

};

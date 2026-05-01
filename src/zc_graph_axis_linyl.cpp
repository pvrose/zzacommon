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
#include "zc_graph_axis_linyl.h"
#include "zc_graph_axis_linear.h"
#include "zc_graph_axis.h"
#include "zc_graph_base.h"

#include "zc_fltk.h"

//! \brief Constructor
zc_graph_axis_linyl::zc_graph_axis_linyl(int X, int Y, int W, int H, const char* L) :
	zc_graph_axis_linear(X, Y, W, H, L) {
}

//! \brief Draw the line for the axis.
void zc_graph_axis_linyl::draw_axis_line() {
	// Set the color and line width for the axis line.
	fl_color(FL_FOREGROUND_COLOR);
	fl_line_style(FL_SOLID, 1);
	// Draw along the right side of the widget area.
	fl_line(x() + w(), y(), x() + w(), y() + h());
	// Restore the default line style.
	fl_line_style(0);
}

//! \brief Draw the ticks for the axis.
void zc_graph_axis_linyl::draw_ticks() {
	// Set the color and line width for the ticks.
	fl_color(FL_FOREGROUND_COLOR);
	fl_line_style(FL_SOLID, 1);
	// Get the font and size for the tick labels.
	fl_font(labelfont(), labelsize() - 2);
	// Draw the ticks and labels based on the orientation.
	for (const auto& tick : ticks_) {
		// Get the size of the tick label.
		int tw = 0, th = 0;
		fl_measure(tick.label.c_str(), tw, th);
		// Draw the tick extending left from the axis line.
		fl_line(x() + w(), tick.position, x() + w() - 5, tick.position);
		// Draw the tick label centered to the right of the tick.
		fl_draw(tick.label.c_str(), x() + w() - 5 - tw, tick.position + th / 2);
	}
	// Restore the default line style.
	fl_line_style(0);
}

//! \brief Draw the label for the axis.
void zc_graph_axis_linyl::draw_label() {
	//Centre the label on the axis and draw it.
	// Set the color and font for the label.
	fl_color(FL_FOREGROUND_COLOR);
	fl_font(labelfont(), labelsize());
	// Get the size of the label.
	int tw = 0, th = 0;
	fl_measure(label_.c_str(), tw, th);
	// Draw the label based on the orientation.
	int lx = 0, ly = 0, lw = 0, lh = 0, ltx = 0, lty = 0, angle = 0;
	// Draw centered to the left of the axis line.
	angle = 90;
	lx = x() + w() / 4;
	ly = y() + h() / 2 - tw / 2;
	lw = th;
	lh = tw;
	ltx = lx + th;
	lty = ly + lh;
	// TODO - this ought to be done between drawing the grid lines and drawing the plot data
	// to avoid the label being overwritten by the grid lines but also to avoid
	// overwriting the plot data with the label background. 
	// Applies to X0_AXIS and Y0_AXIS orientations where the label is drawn over the plot area.
	// However, the grid lines and data are both drawn by zc_graph_plot.
	fl_rectf(lx - 1, ly - 1, lw + 2, lh + 2, (zc::ancestor_view<zc_graph_base>(this))->color());
	fl_color(FL_FOREGROUND_COLOR);
	fl_draw(angle, label_.c_str(), ltx, lty);

}




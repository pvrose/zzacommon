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

#include "zc_graph_plot.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>

zc_graph_plot::zc_graph_plot(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L) {
	clear_data();
}

zc_graph_plot::~zc_graph_plot() {
	clear_data();
}

void zc_graph_plot::add_data(plot_data_t* data) {
	data_sets_.push_back(data);
}

void zc_graph_plot::clear_data() {
	data_sets_.clear();
}

void zc_graph_plot::draw() {
	// Clear the background to white
	fl_rectf(x(), y(), w(), h(), FL_WHITE);
	// Draw a border around the widget area
	fl_rect(x(), y(), w(), h(), FL_FOREGROUND_COLOR);
	// Set drawing clip to the widget area as some
	// of the data may be outside the widget area and we don't want to draw this.
	fl_push_clip(x(), y(), w(), h());
	// Draw the data sets - first LINES and ARCS.
	for (auto& ds : data_sets_) {
		fl_color(ds->style.colour);
		fl_line_style(ds->style.style, ds->style.width);
		switch (ds->type) {
		case LINES:
			for (auto& l : ds->lines) {
				// Draw the line if one end is within the drawing
				// area of the widget, or if the line may cross the
				// drawing area of the widget such as a grid line.
				if (is_within_drawing_area(l.x1, l.y1) ||
					is_within_drawing_area(l.x2, l.y2) ||
					crosses_drawing_area(l)) {
					fl_line(l.x1, l.y1, l.x2, l.y2);
				}
			}
			break;
		case ARCS:
			for (auto& a : ds->arcs) {
				// Draw the arc if any of the corners of the bounding box
				// are within the drawing area.
				if (is_within_drawing_area(a.x, a.y) ||
					is_within_drawing_area(a.x + a.w, a.y + a.h) ||
					is_within_drawing_area(a.x, a.y + a.h) ||
					is_within_drawing_area(a.x + a.w, a.y)) {
					fl_arc(a.x, a.y, a.w, a.h, a.a1, a.a2);
				}
			}
			break;
		default:
			break;
		}
		fl_line_style(0); // reset to default line style after drawing lines and arcs
	}
	// Draw the data sets - then POINTS and CONNECTED_POINTS 
	// so they are on top of the lines and arcs.
	for (auto& ds : data_sets_) {
		fl_color(ds->style.colour);
		fl_line_style(ds->style.style, ds->style.width);
		switch (ds->type) {
		case POINTS:
			for (auto& p : ds->points) {
				// Only draw the point if it is within the drawing area of the widget.
				if (is_within_drawing_area(p)) {
					fl_point(p.x, p.y);
				}
			}
			break;
		case CONNECTED_POINTS:
			for (size_t i = 0; ds->points.size() != 0 && i < ds->points.size() - 1; i++) {
				auto& p1 = ds->points[i];
				auto& p2 = ds->points[i + 1];
				// Only draw the line if either of the points is within the
				// drawing area of the widget. Lines are ordered pairs of points, 
				// so if either point is within the drawing area draw it.
				if (is_within_drawing_area(p1) || is_within_drawing_area(p2)) {
					fl_line(p1.x, p1.y, p2.x, p2.y);
				}
			}
			break;
		default:
			break;
		}
		fl_line_style(0); // reset to default line style
	}
	fl_pop_clip();
}


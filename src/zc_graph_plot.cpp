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

void zc_graph_plot::clear_data() {
	// Delete the data in each dataser.
	for (auto& ds : data_sets_) {
		delete ds.second;
	}
	data_sets_.clear();
}

// Apply the transformation schema to the point for plotting.
void zc_graph_plot::apply_transformation(zc_graph_base::data_type_t type) {
	auto it = data_sets_.find(type);
	auto schema = it->second->xform_schema;
	// Calculate the scaling factors and origin for the transformation.
	// I think this is how the transformation should work.
	double scale_x = w() / (schema.x_max_ - schema.x_min_);
	double scale_y = h() / (schema.y_min_ - schema.y_max_);
	double origin_x = x()  - schema.x_min_ * scale_x;
	double origin_y = y() + h() - schema.y_min_ * scale_y;
	fl_translate(origin_x, origin_y);
	fl_scale(scale_x, scale_y);
}

void zc_graph_plot::draw() {
	// Clear the background to white
	fl_rectf(x(), y(), w(), h(), FL_WHITE);
	// Draw a border around the widget area
	fl_rect(x(), y(), w(), h(), FL_FOREGROUND_COLOR);
	// Set drawing clip to the widget area as some
	// of the data may be outside the widget area and we don't want to draw this.
	fl_push_clip(x(), y(), w(), h());
	// Draw the data sets - first the background lines
	// For each data type, apply the transformation schema, then draw the bg lines.
	for (auto& ds : data_sets_) {
		fl_push_matrix();
		apply_transformation(ds.first);
		for (auto& object : ds.second->background_objects) {
			plot_object(object);
		}
		fl_pop_matrix();
	}
	// Then draw the foreground lines
	for (auto& ds : data_sets_) {
		fl_push_matrix();
		apply_transformation(ds.first);
		for (auto& object : ds.second->foreground_objects) {
			plot_object(object);
		}
		fl_pop_matrix();
	}
	fl_pop_clip();
}

// Plot a drawing object.
void zc_graph_plot::plot_object(const plot_object_t& object) {
	switch (object.shape) {
	case POINTS:
		fl_color(object.style.colour);
		fl_begin_points();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_points();
		fl_line_style(0); // reset to default line style
		break;
	case LINE_STRIP:
		fl_color(object.style.colour);
		fl_line_style(object.style.style, object.style.width);
		fl_begin_line();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_line();
		fl_line_style(0); // reset to default line style
		break;
	case LOOP:
		fl_color(object.style.colour);
		fl_line_style(object.style.style, object.style.width);
		fl_begin_loop();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_loop();
		fl_line_style(0); // reset to default line style
		break;
	case POLYGON:
		fl_color(object.style.colour);
		fl_begin_polygon();
		for (auto& seg : object.segments) {
			if (seg.type == plot_segment_t::VERTEX) {
				fl_vertex(seg.v.x, seg.v.y);
			}
			else if (seg.type == plot_segment_t::ARC) {
				fl_arc(seg.a.x, seg.a.y, seg.a.r, seg.a.r, seg.a.a1, seg.a.a2);
			}
		}
		fl_end_polygon();
		fl_line_style(0); // reset to default line style
		break;
	}
}


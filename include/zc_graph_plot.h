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

#include "zc_line_style.h"

#include <FL/Fl_Widget.H>

//! \brief Header for zc_graph_plot.cpp - the main graph plotting widget.
//! This is the main widget for plotting graphs. Data is supplied
//! as a list of pixel coordinates, and the widgets just draws these as points or lines.
//! The widget also handles the drawing of the grids supplied as a list
//! of lines or arcs supplied by the caller.
class zc_graph_plot : public Fl_Widget {

public:

	enum plot_type_t : uint8_t {
		POINTS,                     //!< Data is a list of points to plot
		CONNECTED_POINTS,           //!< Data is a list of points to plot and connect with lines
		LINES,                      //!< Data is a list of lines to plot
		ARCS                        //!< Data is a list of arcs to plot
	};

	struct plot_point_t {
		int x = 0;          //!< X-coordinate of point
		int y = 0;          //!< Y-coordinate of point
	};

	struct plot_line_t {
		int x1 = 0;         //!< X-coordinate of start of line
		int y1 = 0;         //!< Y-coordinate of start of line
		int x2 = 0;         //!< X-coordinate of end of line
		int y2 = 0;         //!< Y-coordinate of end of line
	};

	struct plot_arc_t {
		int x = 0;          //!< X-coordinate of left of arc bounding box
		int y = 0;          //!< Y-coordinate of top of arc bounding box
		int w = 0;          //!< Width of arc bounding box
		int h = 0;          //!< Height of arc bounding box
		double a1 = 0;      //!< Starting angle of arc (in degrees) E=0, N=90, W=180, S=270
		double a2 = 0;      //!< Ending angle of arc (in degrees) E=0, N=90, W=180, S=270
	};

	struct plot_data_t {
		plot_type_t type;                 //!< Type of data to plot
		zc_line_style style;              //!< Line style to use for plotting 
		std::vector<plot_point_t> points; //!< List of points to plot (for POINTS and CONNECTED_POINTS) 
		std::vector<plot_line_t> lines;   //!< List of lines to plot (for LINES)
		std::vector<plot_arc_t> arcs;     //!< List of arcs to plot (for ARCS)
	};

	//! \brief Constructor
	zc_graph_plot(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor
	~zc_graph_plot();

	//! \brief Add the data to plot.
	//! \param data The data to plot
	void add_data(plot_data_t* data);

	//! \brief Clear the data to plot.
	void clear_data();

	//! \brief Draw the widget.
	void draw() override;

private:

	//! Return true if the point is within the drawing area of the widget.
	bool is_within_drawing_area(plot_point_t p) const {
		return p.x >= this->x() && p.x < this->x() + this->w() && p.y >= this->y() && p.y < this->y() + this->h();
	}
	//! Return true if the point is within the drawing area of the widget.
	bool is_within_drawing_area(int x, int y) const {
		return x >= this->x() && x < this->x() + this->w() && y >= this->y() && y < this->y() + this->h();
	}
	//! Return true if line may cross the drawing area of the widget.
	bool crosses_drawing_area(plot_line_t l) const {
		return is_within_drawing_area(l.x1, l.y1) || 
			is_within_drawing_area(l.x2, l.y2) ||
			(l.x1 < this->x() && l.x2 > this->x()) || 
			(l.x1 > this->x() + this->w() && l.x2 < this->x() + this->w()) ||
			(l.y1 < this->y() && l.y2 > this->y()) || 
			(l.y1 > this->y() + this->h() && l.y2 < this->y() + this->h());
	}

	std::vector<plot_data_t*> data_sets_;   //!< List of data sets to plot

};
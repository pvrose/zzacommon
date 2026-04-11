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

#include "zc_graph_base.h"
#include "zc_line_style.h"

#include <FL/Fl_Widget.H>

#include <cstdint>
#include <map>
#include <vector>

//! \brief Header for zc_graph_plot.cpp - the main graph plotting widget.
//! This is the main widget for plotting graphs. Data is supplied
//! as a list of pixel coordinates, and the widgets just draws these as points or lines.
//! The widget also handles the drawing of the grids supplied as a list
//! of lines or arcs supplied by the caller.
class zc_graph_plot : public Fl_Widget {

public:

	struct plot_vertex_t {
		double x = 0;        //!< X-coordinate of point
		double y = 0;        //!< Y-coordinate of point
		plot_vertex_t() : x(0), y(0) {} // Default constructor initializes to (0, 0)
		plot_vertex_t(double x_, double y_) : x(x_), y(y_) {} // Constructor with parameters
		plot_vertex_t(const plot_vertex_t& other) : x(other.x), y(other.y) {} // Copy constructor
		plot_vertex_t(float x_, float y_) : x(x_), y(y_) {} // Constructor with float parameters
	};

	struct plot_arc_t {
		double x = 0;        //!< X-coordinate of center of arc
		double y = 0;        //!< Y-coordinate of center of arc
		double r = 0;        //!< Radius of arc
		double a1 = 0;       //!< Starting angle of arc in degrees (0 is to the right, positive is counter-clockwise)
		double a2 = 0;       //!< Ending angle of arc in degrees (0 is to the right, positive is counter-clockwise)
	};

	struct plot_segment_t {
		enum segment_type_t :uint8_t {
			VERTEX,          //!< Line segment between two points
			ARC              //!< Arc segment defined by center, radius, and angles
		} type;              //!< Type of segment
		union {
			plot_vertex_t v; //!< Vertex for line segment
			plot_arc_t a;    //!< Arc for arc segment
		};
		plot_segment_t() : type(VERTEX), v{ 0.0, 0.0 } {} // Default constructor initializes to a vertex at (0, 0)
		plot_segment_t(const plot_vertex_t& vertex) : type(VERTEX), v(vertex) {} // Constructor for vertex segment
		plot_segment_t(const plot_arc_t& arc) : type(ARC), a(arc) {} // Constructor for arc segment
		
		// Copy constructor
		plot_segment_t(const plot_segment_t& other) : type(other.type) {
			if (type == VERTEX) {
				v = other.v;
			} else {
				a = other.a;
			}
		}
		
		// Copy assignment operator
		plot_segment_t& operator=(const plot_segment_t& other) {
			if (this != &other) {
				type = other.type;
				if (type == VERTEX) {
					v = other.v;
				} else {
					a = other.a;
				}
			}
			return *this;
		}
	};

	//! \brief Type of data to plot for one line.
	struct plot_line_t {
		zc_line_style style;              //!< Line style to use for plotting
		zc_graph_base::data_type_t transform; //!< Data type to use for transforming the line coordinates.
		std::vector<plot_segment_t> segments; //!< List of line segments to plot
	};

	//! \brief Transformation schema for the plot data. 
	struct plot_xform_t {
		double x_min_ = 0;    //!< Minimum X-coordinate of the plot in pixels
		double y_min_ = 0;    //!< Minimum Y-coordinate of the plot in pixels
		double x_max_ = 1;     //!< Maximum X-coordinate of the plot in pixels
		double y_max_ = 1;     //!< Maximum Y-coordinate of the plot in pixels
	};

	//! \brief All the data for specific data type to be plotted.
	struct plot_data_t {
		plot_xform_t xform_schema; //!< Transformation schema to apply to the data points for this data type.
		std::vector<plot_line_t> background_lines; //!< List of background lines to plot (e.g. grid lines)
		std::vector<plot_line_t> foreground_lines; //!< List of foreground lines to plot (e.g. data lines)
	};

	//! \brief Constructor
	zc_graph_plot(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor
	~zc_graph_plot();

	//! \brief Set the transformation schema for the plot data.
	void set_xform_schema(zc_graph_base::data_type_t type, const plot_xform_t& schema) {
		// Check that the data type exists in the data sets map, and if not, create a new plot_data_t for it.
		if (data_sets_.find(type) == data_sets_.end()) {
			data_sets_[type] = new plot_data_t();
		}
		(data_sets_[type])->xform_schema = schema;
	}

	//! \brief Add a line to the plot for a specific data type.
	void add_line(zc_graph_base::data_type_t type, bool fg, const plot_line_t& line) {
		// Check that the data type exists in the data sets map, and if not, create a new plot_data_t for it.
		if (data_sets_.find(type) == data_sets_.end()) {
			data_sets_[type] = new plot_data_t();
		}
		if (fg) {
			data_sets_[type]->foreground_lines.push_back(line);
		} else {
			data_sets_[type]->background_lines.push_back(line);
		}
	}

	//! \brief Clear the data to plot.
	void clear_data();

	//! \brief Draw the widget.
	void draw() override;

private:

	//! \brief Check if a point is within the drawing area of the widget.
	bool is_within_drawing_area(zc_graph_base::data_type_t type, plot_vertex_t v) const {
		auto& it = data_sets_.find(type);
		auto schema = it->second->xform_schema;
		return v.x >= schema.x_min_ && v.x <= schema.x_max_ && v.y >= schema.y_min_ && v.y <= schema.y_max_;
	}

	//! \brief Apply transformation for FLTK complex drawing functions.
	void apply_transformation(zc_graph_base::data_type_t type);

	//! \brief List of data sets to plot, by data type.
	std::map<zc_graph_base::data_type_t, plot_data_t*> data_sets_;

};
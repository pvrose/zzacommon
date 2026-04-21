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

#include "zc_graph_axis.h"
#include "zc_line_style.h"
#include "zc_text_style.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Forward declaration to avoid circular dependency
class zc_graph_plot;

//! \brief Base class for graph widgets. 
//! 
//! This provides a standard API for the various types of graphs.
//! It can only be used as a base class and must be derived to
//! implement the specific graph types and their components.
//! \image html zc_graph_base.png "Example of a graph with axes and plot area"

class zc_graph_base : public Fl_Group {

public:

	//! Graph types.
	enum graph_type_t : uint8_t {
		XY_LINE,                    //!< X-Y line graph (zc_graph_xy)
		X2Y_LINE,                   //!< X-Y line graph with two Y axes (zc_graph_x2y)
		XY_SCATTER,                 //!< X-Y scatter graph (not yet implemented)
		POLAR,                      //!< Polar graph (Radius vs angle) (not yet implemented)
		SMITH,                      //!< Smith chart (not yet implemented)
	};

	//! Data value pair.
	struct coord {
		float a = 0.0F;     //!< First coordinate (X, angle, or real part)
		float b = 0.0F;     //!< Second coordinate (Y, radius, or imaginary part)

		//! \brief Copy constructor
		coord(const coord& other) : a(other.a), b(other.b) {}
		//! \brief Default constructor
		coord() : a(0.0F), b(0.0F) {}
		//! \brief Constructor with parameters
		coord(float a_, float b_) : a(a_), b(b_) {}
	};

	//! \brief Data value type. A data set will typically consist of pairs of these data types.
	//! Derived graph classes will define which data types they support and how they map
	//! to axes or plot components.
	enum data_type_t : uint8_t {
		X_VALUE,                      //!< X-axis data
		Y_VALUE,                      //!< Y-axis data
		Y2_VALUE,                     //!< Y2-axis data (for X2Y_LINE graph)
		RADIUS,                     //!< Radius data (for POLAR graph)
		THETA,                      //!< Angle data (for POLAR graph)
		REAL,                       //!< Real part data (for SMITH graph)
		IMAGINARY,                  //!< Imaginary part data (for SMITH graph)
	};

	//! \brief Structure to describe a set of data points. This includes the data types,
	//! line style, and pointer to the data.
	struct data_set_t {
		data_type_t type_a;     //!< Type of data in this set - first coordinate.
		data_type_t type_b;     //!< Type of data in this set - second coordinate.
		zc_line_style style;  //!< Line style to use to draw this data set
		std::vector<coord>* data; //!< Data points - by reference to allow manipulation outwith display.
	
		//! \brief Default creator adds an empty data vector.
		data_set_t() {
			data = new std::vector<coord>();
		}
	
	};

	//! \brief Overlay markers for the plot, such as vertical lines to indicate specific X values.
	//! Markers can be added to any data type, and comprose either a single value
	//! or a range of values (e.g. for a shaded area on the plot). 
	//! Each marker includes a line style for rendering and a label for the legend, 
	//! plus a range of values. (For a single value, the range will be from that value to itself.)
	struct marker_t {
		data_type_t type;      //!< Type of data this marker is associated with (e.g. X_VALUE for vertical line markers)
		zc_line_style style;  //!< Line style to use to draw this marker
		float value_1;        //!< First value for this marker (e.g. X value for a vertical line, or start of range for a shaded area)
		float value_2;        //!< Second value for this marker (e.g. same as value_1 for a single line, or end of range for a shaded area)
	};

	//! \brief Overlay text labels for the plot, such as annotations at specific coordinates.
	//! Each label includes a text string, a text style for rendering, and the coordinates for the label.
	//! A coordinate of +/-FLT_MAX can be used to indicate that the label should be aligned to the edge of the plot in that dimension.
	struct label_t {
		data_type_t type;      //!< Type of data this label is associated with (e.g. X_VALUE for a label aligned to the X axis)
		std::string text;    //!< Text string for the label
		zc_text_style style; //!< Text style to use to draw this label
		coord position;      //!< Coordinates for the label (e.g. X and Y values for an annotation). Use +/-FLT_MAX to align to edge of plot.
	};
	
	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the graph drawing area
	//! \param Y The Y coordinate of the top-left corner of the graph drawing area
	//! \param W The width of the graph drawing area
	//! \param H The height of the graph drawing area
	//! \param L The label for the graph
	zc_graph_base(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor
	~zc_graph_base();

	//! \brief Create the graph components and define the data types.
	//! This must be implemeneted by derived classes to create the
	//! specific components of the graph: the axes and plot area.
	//!
	//! When a derived class is instantiated, this method must be called.
	virtual void create() = 0;

	//! \brief Define and map the data types for the graph.
	//! This must be implemented by derived classes to define the data types
	//! they support and how they map to axes or plot components.
	virtual void define_data_types() = 0;

	//!\brief Create the components of the graph.
	//! This must be implemented by derived classes to create the specific components of the graph.
	virtual void create_components() = 0;

	//! \brief Add data to plot.
	//! \param ds The data set to add, including the data type, line style, and pointer to the data.
	//! \return True if the data set was added successfully, False if there was an error (e.g. invalid data type).
	bool add_data_set(data_set_t* ds);

	//! \brief Clear all data sets.
	void clear_data_sets();

	//! \brief Define transformation schemata for the plot based on the axis ranges.
	//! This must be implemented by derived classes to define in particular the 
	//! data ranges for each data set, so that the plot widget can transform the data points
	//! to the appropriate coordinates for rendering.
	//! 
	//! Currently this only 
	//! supports linear transformations based on the axis ranges.
	virtual void define_plot_xforms() = 0;

	//! \brief Convert data set to points to plot.
	//! This must be implemented by derived classes to convert the data.
	//! 
	//! Typically this will involve checking the data and chaining the data points
	//! into a line per data set for the plot widget to render.
	//! Add any modification to plot and axis dimensions here.
	virtual void convert_data_to_points(data_set_t* ds) = 0;

	//! \brief Generate background grid lines.
	//! This must be implemented by derived classes to generate the grid lines for the graph.
	virtual void generate_grid() = 0;

	//! \brief Add the markers and labels to the plot.
	//! This must be implemented by derived classes to add the markers and labels to the plot widget.
	virtual void add_markers() = 0;

	//! \brief override of Fl_Group handle to allow for zooming and scrolling on axes.
	//! Mouse actions:-
	//! - Mouse wheel scroll on an axis to zoom in/out on that axis.
	//! - Mouse wheel scroll on the plot to zoom in/out on all axes.
	//! - Ctrl + mouse wheel scroll on an axis to scroll on that axis.
	//! - Drag on an axis to scroll on that axis.
	//! - Drag on the plot with left mouse button to scroll on X and YL axes.
	//! - Drag on the plot with right mouse button to scroll on X and YR axes.
	//! - Double click on an axis to reset the zoom on that axis to the default range.
	//! - Double click on the plot to reset the zoom on all axes to the default range.
	int handle(int event) override;

	//! \brief override of Fl_Group resize to reset scaling factors on resize.
	//! \param X The new X coordinate of the top-left corner of the graph drawing area
	//! \param Y The new Y coordinate of the top-left corner of the graph drawing area
	//! \param W The new width of the graph drawing area
	//! \param H The new height of the graph drawing area
	void resize(int X, int Y, int W, int H) override;

	//! \brief Draw the graph - override of Fl_Group draw to draw the components of the graph.
	//! This should not need to be implemented by derived classes.
	void draw() override;

	//! \brief Define the parameters for an axis.
	//! \param axis_params Parameter structure for one axis, specified by orientation field.
	//! \return True if the parameters were set successfully, False if there was an error (e.g. missing parameters for an axis).
	bool set_axis_params(const zc_graph_axis::axis_params_t& axis_params);

	//! \brief Set the range for the specified data type.
	//! \param type The data type to set the range for.
	//! \param new_range The new range to set for this data type.
	//! \return True if the range was updated, False if not.
	bool set_data_range(data_type_t type, zc_graph_axis::range new_range);

	//! \brief Get the current range supported by the axis.
	zc_graph_axis::range get_data_range(zc_graph_axis::orientation_t orientation) const;

	//! \brief Get the axis for the specified data type.
	zc_graph_axis::orientation_t get_axis(data_type_t type) const {
		return data_type_to_axis_.at(type);
	};

	//! \brief Add a line marker to the plot. 
	//! \param type The data type this marker is associated with (e.g. X_VALUE for a vertical line marker).
	//! \param style The line style for the marker.
	//! \param value_a The data value for the marker.
	void add_marker(data_type_t type, zc_line_style style, float value_a) {
		add_marker(type, style, value_a, value_a);
	}

	//! \brief Add a bar marker to the plot with a range of values.
	//! \param type The data type this marker is associated with (e.g. X_VALUE for a vertical line marker).
	//! \param style The line style for the marker.
	//! \param value_1 The first data value for the marker (e.g. start of range for a shaded area).
	//! \param value_2 The second data value for the marker (e.g. end of range for a shaded area).
	void add_marker(data_type_t type, zc_line_style style, float value_1, float value_2) {
		marker_t marker;
		marker.type = type;
		marker.style = style;
		marker.value_1 = value_1;
		marker.value_2 = value_2;
		markers_.push_back(marker);
	}

	//! \brief Add a text label to the plot.
	void add_label(data_type_t type, const std::string& text, zc_text_style style, coord position) {
		label_t label;
		label.type = type;
		label.text = text;
		label.style = style;
		label.position = coord(position);
		labels_.push_back(label);
	}


protected:

	//! \brief Return true if the data type is valid for this graph.
	bool data_type_valid(const data_type_t& type) const {
		return data_type_to_axis_.find(type) != data_type_to_axis_.end();
	}

	//! \brief Return pointer to the child at the specified position.
	Fl_Widget* get_child_at_position(int x, int y) const {
		for (int i = children() - 1; i >= 0; i--) {
			Fl_Widget* w = child(i);
			if (w->visible() && x >= w->x() && x < w->x() + w->w() && y >= w->y() && y < w->y() + w->h()) {
				return w;
			}
		}
		return nullptr;
	}

	//! \brief The data sets to plot. Each data set includes the data types, line style, and pointer to the data.
	std::vector<data_set_t*> data_sets_;

	//! \brief List of axis widgets for this graph, mapped by orientation.
	//! The presence of an axis in this map indicates that the graph supports the
	//! corresponding orientation and any data types mapped to that orientation.
	std::map<zc_graph_axis::orientation_t, zc_graph_axis*> axes_; 

	//! Pointer to the plot widget for this graph.
	zc_graph_plot* plot_;

	//! \brief Valid data types for this graph, mapped to the axis component
	//! that they should be plotted on - to be defined by derived classes in 
	//! define_data_types()
	std::map<data_type_t, zc_graph_axis::orientation_t> data_type_to_axis_;

	//! \brief Valid combinations of data types for a single data set - 
	//! to be defined by derived classes in define_data_types()
	std::vector<std::pair<data_type_t, data_type_t>> data_type_combos_;

	//! Previous mouse positions
	int prev_mouse_x_ = 0;
	int prev_mouse_y_ = 0;

	//! \brief The vector of markers to add to the plot.
	std::vector<marker_t> markers_;

	//! \brief The vector of labels to add to the plot.
	std::vector<label_t> labels_;


};

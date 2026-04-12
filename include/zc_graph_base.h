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

#include <FL/Fl_Group.H>

#include <cstdint>
#include <map>
#include <vector>

// Forward declaration to avoid circular dependency
class zc_graph_plot;

//! \brief Base class for graph widgets.
//! This provides a standard API for the various types of graphs.

class zc_graph_base : public Fl_Group {

public:

	//! Graph types.
	enum graph_type_t : uint8_t {
		XY_LINE,                    //!< X-Y line graph
		X2Y_LINE,                   //!< X-Y line graph with two Y axes
		XY_SCATTER,                 //!< X-Y scatter graph
		POLAR,                     //!< Polar graph (Radius vs angle)
		COMPLEX,                   //!< Complex graph (imaginary vs real)
		SMITH,                     //!< Smith chart
	};

	//! Data value pair.
	struct coord {
		float a = 0.0F;     //!< First coordinate (X, angle, or real part)
		float b = 0.0F;     //!< Second coordinate (Y, radius, or imaginary part)
	};

	//! Data value type.
	enum data_type_t : uint8_t {
		X_VALUE,                      //!< X-axis data
		Y_VALUE,                      //!< Y-axis data
		Y2_VALUE,                     //!< Y2-axis data (for X2Y_LINE graph)
		RADIUS,                     //!< Radius data (for POLAR graph)
		THETA,                      //!< Angle data (for POLAR graph)
		REAL,                       //!< Real part data (for COMPLEX and SMITH graph)
		IMAGINARY,                  //!< Imaginary part data (for COMPLEX and SMITHgraph)
	};

		//! Structure to describe a set of data points.
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

	//! \brief Constructor
	zc_graph_base(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor
	~zc_graph_base();

	//! \brief Create the graph components and define the data types - to be implemented by derived classes.
	virtual void create() = 0;

	//! \brief Define and map the data types for the graph -
	//! to be implemented by derived classes to define the data types
	//! they support and how they map to axes or plot components.
	virtual void define_data_types() = 0;

	//!\brief Create the components of the graph - to be implemented by derived classes.
	virtual void create_components() = 0;

	//! \brief Add the data to plot.
	//! \param ds The data set to add, including the data type, line style, and pointer to the data.
	//! \return True if the data set was added successfully, False if there was an error (e.g. invalid data type).
	bool add_data_set(data_set_t* ds);

	//! \brief Clear all data sets.
	void clear_data_sets();

	//! \brief Define transformation schemata for the plot based on the axis ranges.
	virtual void define_plot_xforms() = 0;

	//! \brief Convert data set to points to plot - to be implemented by derived classes.
	virtual void convert_data_to_points(data_set_t* ds) = 0;

	//! \brief Generate background grid lines - to be implemented by derived classes.
	virtual void generate_grid() = 0;

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
	void resize(int X, int Y, int W, int H) override;

	//! \brief Draw the graph - override of Fl_Group draw to draw the components of the graph.
	void draw() override;

	//! \brief Define the parameters for an axis.
	//! \param Parameter structure for one axis, specified by prinetation field.
	//! \return True if the parameters were set successfully, False if there was an error (e.g. missing parameters for an axis).
	bool set_axis_params(const zc_graph_axis::axis_params_t& axis_params);

	//! \brief Set the range for the specified dara.
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

	std::vector<data_set_t*> data_sets_;  //!< Data sets to plot

	// Components of the graph - to be created by derived classes in create_components()
	std::map<zc_graph_axis::orientation_t, zc_graph_axis*> axes_;    //!< List of axes on the graph
	zc_graph_plot* plot_;                 //!< Plot area of the graph

	//! Valid data types for this graph, mapped to the axis component
	//! that they should be plotted on - to be defined by derived classes in 
	//! define_data_types()
	std::map<data_type_t, zc_graph_axis::orientation_t> data_type_to_axis_;

	//! Valid combinations of data types for a single data set - 
	//! to be defined by derived classes in define_data_types()
	std::vector<std::pair<data_type_t, data_type_t>> data_type_combos_;

	//! Previous mouse positions
	int prev_mouse_x_;
	int prev_mouse_y_;
};

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
#include "zc_graph_xy.h"

//! \brief Class for X-Y+Y line graph. 
//! 
//! This is a graph with two Y axes, one on the left and one on the right, and a shared X axis.
//! This graph allows plotting two sets of Y data against the same X data, with each Y set having its own axis for scaling and labeling. The left Y axis is typically used for the first Y data set, and the right Y axis is used for the second Y data set. This is useful when the two Y data sets have different ranges or units, allowing them to be displayed on the same graph without distortion.
//! It reuses the components of zc_graph_xy for the X axis and left Y axis, and adds a right Y axis for the second Y data set. The data type mapping is defined to allow one data set to be plotted against the left Y axis and another data set to be plotted against the right Y axis, both sharing the same X data.
//! \image html zc_graph_x2y.png "Example of an X-Y+Y line graph with two Y axes and grid lines"
class zc_graph_x2y : public zc_graph_xy {

public:
	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the plot drawing area
	//! \param Y The Y coordinate of the top-left corner of the plot drawing area
	//! \param W The width of the plot drawing area
	//! \param H The height of the plot drawing area
	//! \param L The label of the plot
	zc_graph_x2y(int X, int Y, int W, int H, const char* L = nullptr);
	//! \brief Destructor
	~zc_graph_x2y();

	//! \brief Define and map the data types for the graph.
	void define_data_types() override;

	//!\brief Create the graph components - axes and plot area.
	void create_components() override;

};
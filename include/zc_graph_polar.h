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

//! \brief Class for X-Y line graph.
//! 
class zc_graph_polar : public zc_graph_base {

public:
	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the plot drawing area
	//! \param Y The Y coordinate of the top-left corner of the plot drawing area
	//! \param W The width of the plot drawing area
	//! \param H The height of the plot drawing area
	//! \param L The label of the plot
	zc_graph_polar(int X, int Y, int W, int H, const char* L = nullptr);
	//! \brief Destructor
	~zc_graph_polar();

	//! \brief Create the graph components and define the data types.
	void create() override;

	//! \brief Define and map the data types for the graph.
	void define_data_types() override;

	//! \brief Create the graph components - axes and plot area.
	void create_components() override;

	//!\brief Define the data type to axis mappings for the graph.
	void define_plot_xforms() override;

	//! \brief Convert the data sets to points for plotting.
	//! \param ds The data set to convert.
	void convert_data_to_points(data_set_t* ds) override;

	//! \brief Generate the grid lines for the graph.
	void generate_grid() override;

	//! \brief Plot the markers for the graph.
	void add_markers() override;

};
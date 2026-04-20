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

//! \brief Class for X-Y line graph with axes drawn at X=0 and Y=0.
//! 
//! \image html zc_graph_xy0.png "Example of an X-Y line graph with axes at X=0 and Y=0"
class zc_graph_xy0 : public zc_graph_xy {

public:
	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the plot drawing area
	//! \param Y The Y coordinate of the top-left corner of the plot drawing area
	//! \param W The width of the plot drawing area
	//! \param H The height of the plot drawing area
	//! \param L The label of the plot
	zc_graph_xy0(int X, int Y, int W, int H, const char* L = nullptr);
	//! \brief Destructor
	~zc_graph_xy0();

	//! \brief Define and map the data types for the graph.
	void define_data_types() override;

	//!\brief Create the graph components - axes and plot area.
	void create_components() override;

	//!\brief Redraw must reposition the axes at X=0 and Y=0.
	void draw() override;

private:
	//! \brief Width of the axes lines in pixels.
	int axis_width_ = 1;

};
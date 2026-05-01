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

#include "zc_graph_axis_linear.h"
#include "zc_graph_axis.h"

#include <set>

//! \brief Class to represent a linear X axis on a graph, including scaling and zooming options.
//! 
class zc_graph_axis_linx0 : public zc_graph_axis_linear {

public:

	//! \brief Constructor
	zc_graph_axis_linx0(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Return true if the axis is horizontal (X-axis) or false if the axis is vertical (Y-axis).
	bool is_horizontal() const override { return true; }

	//! \brief Returns the zoom capability of the axis.
	zc_graph_axis::zoom_capability_t get_zoom_capability() const override { return zc_graph_axis::ZOOM_ON_CURSOR; }

	//! \brief Returns whether the axis can be scrolled.
	bool can_scroll() const override { return true; }

	//! \brief Draw the line for the axis.
	virtual void draw_axis_line() override;
	//! \brief Draw the ticks for the axis.
	virtual void draw_ticks() override;
	//! \brief Draw the label for the axis.
	virtual void draw_label() override;

};



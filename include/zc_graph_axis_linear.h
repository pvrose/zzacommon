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

//! \brief Class to represent a linear axis on a graph, including scaling and zooming options.

class zc_graph_axis_linear : public zc_graph_axis {

public:
	
	//! \brief Constructor
	zc_graph_axis_linear(int X, int Y, int W, int H, const char* L = nullptr);

	//!\brief Set the tick positions and labels based on the current range and tick spacing.
	void set_ticks() override;

	//! \brief Generate the grid lines for the axis based on the current tick positions.
	void set_grid_lines() override;

protected:
	float tick_spacing_;         //!< Spacing between ticks in units

};

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
class zc_graph_x2y : public zc_graph_xy {

public:
	//! \brief Constructor
	zc_graph_x2y(int X, int Y, int W, int H, const char* L = nullptr);
	//! \brief Destructor
	~zc_graph_x2y();

	void define_data_types() override;

	void create_components() override;

};
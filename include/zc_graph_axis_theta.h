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

//! \brief Class for a polar axis (theta).
class zc_graph_axis_theta : public zc_graph_axis {

public:
	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the axis
	//! \param Y The Y coordinate of the top-left corner of the axis
	//! \param W The width of the axis
	//! \param H The height of the axis
	//! \param L The label of the axis
	zc_graph_axis_theta(int X, int Y, int W, int H, const char* L = nullptr);
	//! \brief Destructor
	~zc_graph_axis_theta();

public:
	//! \brief Set the annular width of the axis in pixels.
	void set_annular_width(int p) {
		annular_width_ = p;
	}

	//! \brief Set range to 0-360 degrees.
	virtual void set_range(range new_range) override;

	//!\brief Set the tick positions and labels based on the current range and tick spacing.
	void set_ticks() override;

	//! \brief Generate the grid lines for the axis based on the current tick positions.
	void set_grid_lines() override;

	//! \brief Return true if the axis is horizontal (X-axis) or false if the axis is vertical (Y-axis).
	bool is_horizontal() const override { return false; }

	//! \brief Returns the zoom capability of the axis.
	zc_graph_axis::zoom_capability_t get_zoom_capability() const override { return zc_graph_axis::NO_ZOOM; }

	//! \brief Returns whether the axis can be scrolled.
	bool can_scroll() const override { return false; }

	//! \brief Draw the line for the axis.
	virtual void draw_axis_line() override;
	//! \brief Draw the ticks for the axis.
	virtual void draw_ticks() override;
	//! \brief Draw the label for the axis.
	virtual void draw_label() override {
		// Do nothing
	};

protected:
	float tick_spacing_;         //!< Spacing between ticks in units

	int annular_width_ = 20;   //!< Width of the annular axis area in pixels

};

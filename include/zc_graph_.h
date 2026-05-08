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

#include "zc_line_style.h"
#include "zc_text_style.h"
#include "zc_utils.h"

#include <FL/Enumerations.H>
#include <FL/Fl_Widget.H>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cfloat>
#include <map>
#include <string>
#include <utility>
#include <vector>

//! \brief Base graph class.
//! 
//! To use this and its derived classes:
//! 
//! \code
//! zc_graph_xxx* graph = new zc_graph_xxx(...); // Create an instance of a derived graph class (e.g. zc_graph_cartesian)
//! graph->start_config(); // Start configuration of the graph
//! graph->set_axis_params(...); // Set parameters for the axes (e.g. labels, units, tick spacing)
//! graph->set_axis_ranges(...); // Set the inner and outer ranges for the axes
//! ... repeat for each axis as needed ...
//! graph->add_data_set(...); // Add data sets to plot, specifying the axis number, data points, and line style
//! ... repeat for each data set as needed ...
//! graph->add_marker(...); // Add markers to the plot, specifying the axis number, layer, line style, and value(s) for the marker
//! ... repeat for each marker as needed ...
//! graph->end_config(); // End configuration of the graph.
//! ... Contents of the data buffers may be changed at any time, after which...
//! graph->redraw(); // Redraw the graph to display the data
//! 
//! \endcode
//! The base class zc_graph_ defines the data structures and functions
//! for managing the graph data, axes, and plotting parameters, 
//! but does not implement the layout of the graph,
//! Based on the values set in the layout() function, the base
//! class will implement the drawing of the graph.
//! \todo Add support for logarithmic axes, including appropriate
//! tick spacing and label formatting. This will probably require
//! the set_ticks() function to be overridden.
class zc_graph_ : public Fl_Widget {

public:

	//! \brief Type for a data type to be plotted. 
	//! 
	//! This is an identifier for the type of data being plotted.
	//! It represents a pair of coordinates, the meaning depending on the graph type.
	typedef std::pair<double, double> data_point_t;

	//! \brief Type of coordinates that a data_point_t represents.
	enum graph_type_t : uint8_t {
		NO_DATA,     //!< No data
		CARTESIAN,    //!< Cartesian coordinates (x, y)
		CARTESIAN_2Y,  //!< Cartesian coordinates with a secondary Y axis (x, y1) and (x, y2)
		CART_OVERLAY,   //!< Cartesian coordinates with X and Y axes overlaid on the same plot area (x, y).
		POLAR,        //!< Polar coordinates (r, theta)
	};

	//! \brief Text alignment wrt the specified position for text labels and text boxes.
	//! ALIGN_LEFT/RIGHT can be orred with ALIGN_ABOVE/BELOW to specify the 
	//! alignment in both dimensions.
	typedef uint8_t text_alignment_t;
	const static text_alignment_t ALIGN_CENTRE = 0;  //!< Align text centred on the specified position.
	const static text_alignment_t ALIGN_LEFT = 1;     //!< Align text to the left of the specified position.
	const static text_alignment_t ALIGN_RIGHT = 2;    //!< Align text to the right of the specified position.
	const static text_alignment_t ALIGN_MASK_LR = 3; //!< Mask for left/right alignment bits.
	const static text_alignment_t ALIGN_ABOVE = 4;    //!< Align text above the specified position.
	const static text_alignment_t ALIGN_BELOW = 8;     //!< Align text below the specified position.
	const static text_alignment_t ALIGN_MASK_AB = 12; //!< Mask for above/below alignment bits.

	//! \brief Structure to represent a set of data to be plotted.
	struct data_set_t {
		std::vector<data_point_t>* data; //!< Pointer to a vector of data points to be plotted
		zc_line_style style; //!< Line style to use for plotting this data set
	};

	//! \brief Overlay markers for the plot, such as vertical lines to indicate specific X values.
	//! Markers can be added to any data type, and comprise either a single value
	//! or a range of values (e.g. for a shaded area on the plot). 
	//! Each marker includes a line style for rendering and a label for the legend, 
	//! plus a range of values. (For a single value, the range will be from that value to itself.)
	struct value_marker_t {
		zc_line_style style;  //!< Line style to use to draw this marker
		float value_1;        //!< First value for this marker (e.g. X value for a vertical line, or start of range for a shaded area)
		float value_2;        //!< Second value for this marker (e.g. same as value_1 for a single line, or end of range for a shaded area)
	};

	//! \brief Data required for a point label marker
	struct point_marker_t {
		data_point_t position; //!< Data coordinates of the point to label
		std::string text;      //!< Text to display for the label
		zc_text_style style;    //!< Text style to use for the label
		text_alignment_t alignment; //!< Alignment of the label text relative to the point (e.g. centered, above, below, left, right)
		bool opaque = false;   //!< Whether to draw an opaque background behind the label text for better visibility
	};

	//! \brief Minimum and maximum values for data coordinates for an
	//! individual coordinate.
	//! 
	//! Reset value indicates an empty range, which can be readily expanded.
	struct range_t {
		double min = DBL_MAX; //!< Minimum value for the coordinate
		double max = -(DBL_MAX); //!< Maximum value for the coordinate

		//! \brief Union assignment operator - expands this range to include another range.
		//! \param other The other range to union with this range.
		//! \return A reference to this range after the union operation.
		range_t& operator|=(const range_t& other) {
			min = std::min(min, other.min);
			max = std::max(max, other.max);
			return *this;
		}

		//! \brief Add a single \p value to this range, expanding the range if necessary to include the value.
		range_t& operator|=(double value) {
			min = std::min(min, value);
			max = std::max(max, value);
			return *this;
		}

		//! \brief Intersection assignment operator - narrows this range to the overlap with another range.
		//! \param other The other range to intersect with this range.
		//! \return A reference to this range after the intersection operation.
		range_t& operator&=(const range_t& other) {
			min = std::max(min, other.min);
			max = std::min(max, other.max);
			if (min > max) {
				// No intersection - set to empty range
				min = DBL_MAX;
				max = -DBL_MAX;
			}
			return *this;
		}

		//! \brief Get the union of this range and another range, returning a new range object.
		range_t operator|(const range_t& other) const {
			range_t result = *this;
			result |= other;
			return result;
		}

		//! \brief Get the intersection of this range and another range, returning a new range object.
		range_t operator&(const range_t& other) const {
			range_t result = *this;
			result &= other;
			return result;
		}

		//! \brief Return true if ranges are equal (i.e. min and max are the same).
		bool operator==(const range_t& other) const {
			return min == other.min && max == other.max;
		}

		//! \brief Return true if ranges are not equal (i.e. min or max are different).
		bool operator!=(const range_t& other) const {
			return !(*this == other);
		}

		//! \brief Return true of other range is wholly contained within this range (i.e. this range is a superset of the other range).
		bool contains(const range_t& other) const {
			return min <= other.min && max >= other.max;
		}

		//! \brief Return true if the range contains a specific value.
		bool contains(double value) const {
			return min <= value && max >= value;
		}

		//! \brief Return that the range is valid (i.e. min is less than or equal to max).
		bool is_valid() const {
			return min <= max;
		}
	};

	//! \brief Axis label modifier type. This indicates how the axis labels should be
	//! modified to indicate the scale of the axis.
	enum modifier_t : uint8_t {
		NO_MODIFIER,            //!< No modifier: use the base label and unit as is.
		SI_PREFIX,              //!< SI prefix (e.g. k for kilo, M for mega)
		POWER_OF_10,            //!< Power of 10 (e.g. 10^3) - displayed as "x10^3" in the label.
	};

	//! \brief Zoom capability of the axis.
	enum zoom_capability_t : uint8_t {
		NO_ZOOM,           //!< No zooming allowed - the range is fixed.
		ZOOM_ON_ORIGIN,       //!< Zooming allowed but the origin (0 value) is fixed in place.
		ZOOM_ON_CURSOR,       //!< Zooming allowed and the zoom is centered on the cursor position.
	};

	//! \brief Tick orientation for the axis.
	enum tick_orientation_t : uint8_t {
		NO_TICKS,           //!< No ticks are drawn for this axis.
		TICK_INCREASING,    //!< Ticks are drawn in the direction of increasing values of the other axis.
		TICK_DECREASING,    //!< Ticks are drawn in the direction of decreasing values of the other axis.
	};

	//! \brief Data associated with a tick on an axis.
	struct tick_data_t {
		double value;        //!< Value of the tick in data coordinates
		std::string label;   //!< Label to display for the tick
		bool is_major;       //!< Whether this tick is a major tick (e.g. for grid lines) or a minor tick
	}; 

	//! \brief Data required for each axis.
	struct axis_data_t {
		range_t outer_range;       //!< Range of data values for this axis
		range_t inner_range;       //!< Range of data values currently displayed for this axis (may be zoomed or scrolled)
		range_t default_range;     //!< Default range for this axis in the absence of data
		range_t current_range;     //!< Current range for this axis (may be zoomed or scrolled)
		modifier_t modifier = NO_MODIFIER;  //!< Modifier for axis labels
		std::string unit;          //!< Unit to display on the axis (e.g. "Hz")
		std::string label;         //!< Base label for the axis (e.g. "Frequency")
		data_point_t label_position; //!< Position on the axis to draw the label (in data coordinates)
		std::string modified_label;      //!< Label to display for the axis, including any modifier (e.g. "Frequency (kHz)")
		int label_angle = 0;         //!< Angle to draw the label at (in degrees, where 0 is horizontal and positive is counter-clockwise)
		int tick_spacing_pixels = 0;   //!< Suggested spacing between ticks in pixels
		double position = 0.0;     //!< Position on other axis where this is drawn
		double inv_scale = 1.0;    //!< Inverse scale factor - number of units per pixel	
		tick_orientation_t tick_orientation = NO_TICKS; //!< Orientation of ticks for this axis
		std::vector<tick_data_t> ticks; //!< Data for the ticks on this axis (value and label)
	};

	//! \brief Data required for the data plot area.
	struct data_area_t {
		data_point_t display_min;   //!< Data coordinates of the bottom-left of the display area.
		data_point_t display_max;   //!< Data coordinates of the top-right of the display area.
	};

	//! \brief Structure to represent a vertex (point) in the plot. This maps onto
	//! the parameters of the FLTK function fl_vertex() when plotting
	//! lines or points.
	struct plot_vertex_t {
		double x = 0;        //!< X-coordinate of point
		double y = 0;        //!< Y-coordinate of point
		plot_vertex_t() : x(0), y(0) {} //!< Default constructor initializes to (0, 0)
		plot_vertex_t(double x_, double y_) : x(x_), y(y_) {} //!< Constructor with parameters
		plot_vertex_t(const plot_vertex_t& other) : x(other.x), y(other.y) {} //!< Copy constructor
		plot_vertex_t(data_point_t point) : x(point.first), y(point.second) {} //!< Constructor from data_point_t
	};

	//! \brief Structure to represent an arc segment in the plot. This maps onto
	//! the parameters of the FLTK function fl_arc() when plotting lines or points.
	struct plot_arc_t {
		double x = 0;        //!< X-coordinate of center of arc
		double y = 0;        //!< Y-coordinate of center of arc
		double r = 0;        //!< Radius of arc
		double a1 = 0;       //!< Starting angle of arc in degrees (0 is to the right - 3 o'clock, positive is counter-clockwise)
		double a2 = 0;       //!< Ending angle of arc in degrees (0 is to the right - 3 o'clock, positive is counter-clockwise)
	};

	//! \brief Structure to represent either a vertex or an arc segment in the plot.
	//! This allows a single data structure to represent both types of plot segments.
	struct plot_segment_t {
		//! The type of segment.
		enum segment_type_t :uint8_t {
			VERTEX,          //!< A single vertex (point) defined by X and Y coordinates.
			ARC,             //!< Arc segment defined by center, radius, and angles.
			GAP              //!< Gap in the line strip, used when constructing complex shapes with holes (e.g. FLTK Complex polygon).
		} type;
		//! The data for the segment.
		union {
			plot_vertex_t v; //!< Vertex for line segment
			plot_arc_t a;    //!< Arc for arc segment
		};
		//! Default constructor initializes to a vertex at (0, 0).
		plot_segment_t() : type(VERTEX), v{ 0.0, 0.0 } {}
		//! Constructor for vertex segment.
		plot_segment_t(const plot_vertex_t& vertex) : type(VERTEX), v(vertex) {}
		//! Constructor for arc segment.
		plot_segment_t(const plot_arc_t& arc) : type(ARC), a(arc) {}
		//! Constructor for gap segment.
		plot_segment_t(bool is_gap) : type(GAP) {}

		//! Copy constructor
		plot_segment_t(const plot_segment_t& other) : type(other.type) {
			if (type == VERTEX) {
				v = other.v;
			}
			else if (type == ARC) {
				a = other.a;
			}
		}

		//! Copy assignment operator
		plot_segment_t& operator=(const plot_segment_t& other) {
			if (this != &other) {
				type = other.type;
				if (type == VERTEX) {
					v = other.v;
				}
				else if (type == ARC) {
					a = other.a;
				}
			}
			return *this;
		}
	};


	//! \brief Type of construct to draw.
	//! 
	//! This maps on to the drawing functions: fl_begin_xxx()/fl_end_xxx() where xxx
	//! is the shape type (e.g. line, loop, points, polygon). 
	enum shape_t : uint8_t {
		POINTS,         //!< Points defined by a list of vertices.
		LINE_STRIP,     //!< Line strip defined by a list of vertices and arcs.
		LOOP,           //!< Loop defined by a list of vertices and arcs (i.e. closed line strip).
		POLYGON,        //!< Polygon defined by a list of vertices and arcs (filled).
		COMPLEX,        //!< Complex shape defined by a list of vertices, gaps and arcs - see FLTK Complex polygon.
		TEXT,			//!< Text label defined by a position and string.
		TICK,			//!< Tick mark defined by a position and direction. Includes a text label.
		TEXT_BOX,       //!< Text box with opaque background defined by a position, string, and inclination.
	};

	//! \brief Type of data to plot for one object.
	struct plot_object_t {
		shape_t shape = LINE_STRIP;           //!< The shape of the object
		zc_line_style style;                  //!< Line style to use for plotting
		zc_text_style text_style;             //!< Text style to use for plotting (only used if shape is TEXT, TICK or TEXT_BOX)
		std::vector<plot_segment_t> segments; //!< List of segments to plot
		std::string text;                    //!< Text string to plot (only used if shape is TEXT, TICK or TEXT_BOX)
		int text_angle = 0;                   //!< Angle to draw text at in degrees (only used if shape is TEXT or TEXT_BOX)
		text_alignment_t text_alignment = ALIGN_CENTRE; //!< Alignment of text with respect to the specified position (only used if shape is TICK, TEXT or TEXT_BOX)
	};

	//! \brief Transformation schema for the plot data. 
	//! 
	//! Currently this is a simple linear transformation defined by the cartesian
	//! coordinates of the extremes of the data to be plotted,
	//! mapped onto the pixel coordinates of the widget.
	//! When used with polar coordinates, the supplied data will be conevrted to cartesian
	//! coordinates before applying the transformation, so the transformation schema
	//! will still be defined in terms of cartesian coordinates.
	//! 
	//! This allows the use of FLTK complex shape drawing functions, but limited to linear transformations.
	struct plot_xform_t {
		double x_min_ = 0;    //!< Minimum X-coordinate of the plot in pixels. Maps onto x() of the widget.
		double y_min_ = 0;    //!< Minimum Y-coordinate of the plot in pixels. Maps onto y() + h() of the widget (i.e. Y increases upwards).
		double x_max_ = 1;     //!< Maximum X-coordinate of the plot in pixels. Maps onto x() + w() of the widget.
		double y_max_ = 1;     //!< Maximum Y-coordinate of the plot in pixels. Maps onto y() of the widget.
	};

	//! \brief Drawing layers. Lower number will be drawn first then
	//! the remaining will be drawn on top.
	enum layer_t : uint8_t {
		BACKGROUND = 0,       //!< Items to be drawn behind all others. Background patterns or shading could be drawn here.
		GRID_LINES,           //!< Typically lines of equal value on each axis, drawn behind the axes and data.
		AXES,                 //!< The axes.
		DATA,                 //!< Points or lines representing the data displayed.
		FOREGROUND,           //!< Items, typically markers to indicate specific values, to be drawn on top of the data.
		MASK                  //!< Topmost layer to remove any unwanted artefacts from the plot area.
	};

	//! \brief All the data for specific data type to be plotted by layer.
	//! 
	//! Typically there will be one or two data types to plot. 
	//! This does not preclude multiple sets of data within one data type.
	typedef std::map<layer_t, std::vector<plot_object_t>> plot_layer_data_t;

	//! \brief The graph can maintain multiple sets of data to be plotted.
	//! Each data can be plotted using a different transformation schema, 
	//! allowing for example different data types to be plotted on the 
	//! same graph with different scales 
	//! (e.g. resistive and reactive components of an impedance plot).
	struct plot_data_t {
		plot_xform_t xform_schema; //!< Transformation schema to apply to the data points for this data type.
		data_area_t data_area; //!< Data coordinates corresponding to the plot area (i.e. the corners of the plot area in data coordinates). This is used to define the transformation schema for this data type.
		plot_layer_data_t layer_data; //!< Data to plot, organised by layer.
	};

	//! \brief Layout area tags for the graph. 
	//! 
	//! Used to identify the different areas of the graph for handling zoom and scroll.
	struct layout_area_t {
		bool is_plot_area; //!< Whether this area is the plot area (true) or the axis area (false).
		int axis_number;   //!< If this is an axis area, the number of the axis (starting from 0). Ignored if is_plot_area is true.
	};

public:
	//! \brief Constructor
	//! \param X The X coordinate of the top-left corner of the graph drawing area
	//! \param Y The Y coordinate of the top-left corner of the graph drawing area
	//! \param W The width of the graph drawing area
	//! \param H The height of the graph drawing area
	//! \param L The label for the graph
	zc_graph_(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor
	~zc_graph_();

	//! \brief Define coordinates for the graph.
	//! \param type The type of coordinates to use for the graph (e.g. Cartesian, Polar).
	void set_type(graph_type_t type) {
		graph_type_ = type;
	}

	//! \brief Set the number of supported axes for the graph.
	//! \param num_axes The number of axes to support for the graph. This should be set before setting the parameters for each axis.
	void set_num_axes(int num_axes) {
		num_axes_ = num_axes;
	}

	//! \brief Specify the basic parameters of an axis.
	//! \param axis_number Use 0 for the first and common axis. Use 1, 2 etc for subsequent axes.
	//! \param modifier Modifier for the axis labels (e.g. SI prefix, power of 10).
	//! \param unit Unit to display on the axis (e.g. "Hz").
	//! \param label Base label for the axis (e.g. "Frequency").
	//! \param tick_spacing_pixels Suggested spacing between ticks in pixels.
	//! The actual tick spacing will be determined based on the range of the axis and the size of the graph,
	//! and will be calculated to be a "nice" number (e.g. 1, 2, 5, 10, etc.) that is close to the suggested spacing in pixels.
	void set_axis_params(
		int axis_number,
		modifier_t modifier = NO_MODIFIER,
		const std::string& unit = "",
		const std::string& label = "",
		int tick_spacing_pixels = 30
	);

	//! \brief Specify the inner and outer ranges for an axis.
	//! \param axis_number The number of the axis to set the ranges for (starting from 0).
	//! \param inner_range Cannot zoom closer than this range.
	//! \param outer_range Cannot zoom further than this range.
	//! \param default_range Default range to use for this axis - resetting zoom will reset to this range.
	void set_axis_ranges(
		int axis_number,
		const range_t& inner_range,
		const range_t& outer_range,
		const range_t& default_range
	);

	//! \brief Get the current range for an axis.
	range_t get_axis_range(int axis_number) const;

	//! \brief Add a data set to the graph.
	//! \param axis_number The number of the data set to add (starting from 0). This allows multiple data sets to be plotted against different
	//! axes or with different transformation schemata.
	//! \param data The data points to plot for this data set. A pointer is used so that when refreshing the data for plotting,
	//! the data can be updated without needing to re-add the data set.
	//! \param style The line style to use for this data set.
	void add_data_set(
		int axis_number,
		std::vector<data_point_t>* data,
		zc_line_style style = zc_line_style()
	);

	//! \brief Add a marker to the graph at a specific value or range of values.
	//! \param axis_number The number of the axis to add the marker for (starting from 0).
	//! \param layer The layer to draw the marker on (e.g. foreground, background).
	//! \param style The line style to use for drawing the marker.
	//! \param value_a The value for the marker.
	void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_a
	)
	{
		add_marker(axis_number, layer, style, value_a, value_a);
	};

	//! \brief Add a marker to the graph at a specific value or range of values.
	//! \param axis_number The number of the axis to add the marker for (starting from 0).
	//! \param layer The layer to draw the marker on (e.g. foreground, background).
	//! \param style The line style to use for drawing the marker.
	//! \param value_1 The value for the marker range lower bound.
	//! \param value_2 The value for the marker range upper bound.
	void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_1,
		double value_2
	);

	//! \brief Add a text label to the graph at a specific position.
	//! \param axis_number The number of the axis to add the label for (starting from 0).
	//! \param layer The layer to draw the label on (e.g. foreground, background).
	//! \param text The text string to display for the label.
	//! \param style The text style to use for drawing the label.
	//! \param position The position to draw the label in data coordinates. 
	//! \param alignment The alignment of the label text relative to the specified position (e.g. centred, above, below, left, right).
	//! \param opaque Whether to draw an opaque background behind the label text for better visibility.
	void add_label(
		int axis_number,
		layer_t layer,
		const std::string& text,
		zc_text_style style,
		data_point_t position,
		text_alignment_t alignment = ALIGN_RIGHT | ALIGN_ABOVE,
		bool opaque = false
	);

	//! \brief Start configuration: clear all data sets and markers from the graph.
	//! 
	//! This should be called before reconfiguring the graph with new axes, data sets and markers.
	void start_config();

	//! \brief End configuration.
	//! 
	//! This should be called after setting the axes parameters and ranges, 
	//! and adding the data sets and markers, to initiate the graph for plotting.
	//! It copies the axis, gridline, data and marker data into 
	//! internal structures for plotting.
	void end_config();

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
	//! 
	//! This should be called when the graph drawing area is resized to update the scaling factors and layout.
	//! If the actual size or position is unaltered the graph will NOT be redrawn.
	void resize(int X, int Y, int W, int H) override;

	//! \brief Draw the graph - override of Fl_Group draw to draw the components of the graph.
	void draw() override;

	//! \brief Set the default colour for text and lines for the graph.
	void textcolor(Fl_Color color) {
		default_text_colour_ = color;
	}

	//! \brief Get the default colour for text and lines for the graph.
	Fl_Color textcolor() const {
		return default_text_colour_;
	}

	//! \brief Set the default font for text for the graph.
	void textfont(Fl_Font font) {
		default_text_font_ = font;
	}

	//! \brief Get the default font for text for the graph.
	Fl_Font textfont() const {
		return default_text_font_;
	}

	//! \brief Set the default font size for text for the graph.
	void textsize(int size) {
		default_text_size_ = size;
	}

	//! \brief Get the default font size for text for the graph.
	Fl_Fontsize textsize() const {
		return default_text_size_;
	}

protected:

	//! \brief Place the axes and plot areas. Define transformation schemata.
	//! 
	//! This MUST be implemented by derived classes to define the layout of the graph, including
	//! the placement of the axes and plot area, and the transformation schemata to 
	//! apply to the data points for plotting.
	//! 
	virtual void layout() = 0;

	//! 
	//! \brief Convert given coordinates to Cartesian coordinates for plotting.
	//! 
	//! \param point The data point to convert, in the original coordinates for the graph type.
	//! \todo Convert this to a virtual function if needed for different graph types, when 
	//! the conversion is not just from polar to Cartesian. For example, supporting
	//! logarithmic axes or smith charts.
	const data_point_t convert_point(const data_point_t& point) const {
		switch (graph_type_) {
		case CARTESIAN:
		case CARTESIAN_2Y:
		case CART_OVERLAY:
			return point;
		case POLAR: {
			double r = point.first;
			double theta = point.second;
			double x = r * cos(theta * zc::PI / 180.0);
			double y = r * sin(theta * zc::PI / 180.0);
			return { x, y };
		}
		default:
			return point;
		}
	}

	//! \brief Apply the transformation schema to the data points for plotting.
	//! \param schema The transformation schema to apply.
	void apply_transformations(plot_xform_t schema);

	//! \brief Return the data coordinates corresponding to the given pixel coordinates.
	//! \param axis_number The number of the axis to use for the transformation (starting from 1)
	//! \param x_pixel The x-coordinate in pixels to convert to data coordinates.
	//! \param y_pixel The y-coordinate in pixels to convert to data coordinates.
	data_point_t pixel_to_data(int axis_number,int x_pixel, int y_pixel) const;

	//! \brief Generate axis drawing objects.
	//! \param axis_number The number of the axis to generate the drawing objects for (starting from 0).
	void generate_axis_grid(
		int axis_number
	);

	//! \brief generate the axis line.
	//! \param axis_number The number of the axis to generate the line for (starting from 0).
	void generate_axis_line(
		int axis_number
	);

	//! \brief generate tick marks for the axis.
	//! \param axis_number The number of the axis to generate ticks for (starting from 0).
	void generate_axis_ticks(
		int axis_number
	);

	//! \brief Generate grid lines for the plot.
	//! \param axis_number The number of the axis to generate grid lines for (starting from 0). 
	void generate_grid_lines(
		int axis_number
	);

	//! \brief Generate label for the axis.
	//! \param axis_number The number of the axis to generate the label for (starting from 0).
	void generate_axis_label(
		int axis_number
	);

	//! \brief Generate data lines for the data set associated with the specified axis number.
	//! \param axis_number The number of the data set to generate lines for (starting from 0).
	void generate_data_lines(
		int axis_number
	);

	//! \brief Generate the specfied value marker for the specified axis number.
	//! \param axis_number The number of the axis to generate the marker for (starting from 0).
	//! \param layer The layer to draw the marker on (e.g. foreground, background).
	//! \param marker The marker to generate.
	//! 
	//! This will also be used to generate axis and grid lines and should 
	//! be the only function that needs to be overridden.
	virtual void generate_value_marker(
		int axis_number,
		layer_t layer,
		const value_marker_t& marker
	) = 0;

	//! \brief Generate all the value markers for the specified axis number.
	void generate_value_markers(
		int axis_number
	);

	//! \brief Generate all the point markers for the specified axis number.
	void generate_point_markers(
		int axis_number
	);

	//! \brief Set tick and grid points for the axis
	//! \param axis_number The number of the axis to generate ticks for (starting from 0).
	//! \param tick_spacing_pixels The desired spacing between ticks in pixels.
	//! \param length_pixels The length of the axis in pixels. 
	void set_ticks(
		int axis_number,
		int tick_spacing_pixels,
		double inv_scale
	);

	//! \brief Overridable function to generate a tick for specific derived types.
	//! 
	//! \param axis_number The number of the axis to generate the tick for (starting from 0).
	//! \param tick_data The data for the tick to generate.
	//! \param tick_object The plot object to populate with the data for the tick.
	virtual bool custom_tick(
		int axis_number,
		const tick_data_t& tick_data,
		plot_object_t& tick_object
	) {
		// Default implementation does nothing - override in derived classes as needed.
        return false;
	}

	//! \brief Normalise a number and generate the appropriate multiplier.
	//! For modifier_t values:
	//! - SI_PREFIX: normalise to a value between 0.1 and 100 and generate the appropriate SI prefix (e.g. k for kilo, M for mega).
	//! - POWER_OF_10: normalise to a value between 1 and 10 and generate the appropriate power of 10 multiplier (e.g. x10^4).
	//! \param fin Input number
	//! \param modifier The type of modifier to apply for normalisation.
	//! \param norm Normalised result (mantissa)
	//! \param exp10 Exponent (power of 10)
	//! \param si_prefix SI Prefix (if appropriate) - UTF-8 character
	void normalise(double fin, modifier_t modifier, double& norm, double& exp10, uint32_t& si_prefix) const;

	//! \brief Draw an individual object on the plot.
	//! \param object The object to draw, including its shape, style and segments.
	void draw_plot_object(
		const plot_object_t & object 
	);

	//! \brief Return the layout area for a given set of pixel coordinates.
	virtual layout_area_t get_layout_area(int x, int y) const = 0;

	//! \brief Return whether given axis is horizontal (true) or vertical (false).
	virtual bool is_axis_horizontal(int axis_number) const = 0;

	//! \brief Scroll the specified axis by the specified amount in pixels.
	//! \param axis_number The number of the axis to scroll (starting from 0).
	//! \param pixels The number of pixels to scroll the axis by.
	//! This fuction checks the scrollability of the axis and the scroll
	//! limits based on the inner and outer ranges for the axis.
	void scroll_axis(int axis_number, int pixels);

	//! \brief Zoom the specified axis by the specified factor, centered on the specified pixel coordinates.
	//! \param axis_number The number of the axis to zoom (starting from 0).
	//! \param centre_x The x-coordinate of the zoom centre in pixels.
	//! \param centre_y The y-coordinate of the zoom centre in pixels.
	//! \param factor The number of zoom steps by which to zoom the axis.
	//! This function checks the zoom capability of the axis and the 
	//! zoom limits based on the inner and outer ranges for the axis.
	void zoom_axis(int axis_number, int centre_x, int centre_y, int factor);

	//! \brief Reset the zoom for the specified axis to the default range.
	//! \param axis_number The number of the axis to reset the zoom for (starting from 0).
	void reset_zoom(int axis_number);

	//! \brief The number of axes supported
	int num_axes_ = 0;

	//! \brief The data for the graph. Pointers to application data.
	//! 
	//! This data will be applied to the DATA layer.
	std::map<int, data_set_t> data_sets_;

	//! \brief The graph_type for the graph, which defines the layout of the axes and plot area.
	graph_type_t graph_type_ = NO_DATA;

	//! \brief The axis data for the graph, keyed by axis number (starting from 0).
	//! 
	//! Ths data will be applied to the AXES amd GRIDLINES layers.
	std::map<int, axis_data_t> axes_data_;

	//! \brief The value markers for the graph, keyed by axis number (starting from 0).
	//! 
	//! This data will be applied to either the BACKGROUND or FOREGROUND layer, as required 
	//! by the application.
	std::map<int, std::map<layer_t, std::vector<value_marker_t>>> value_markers_;

	//! \brief The point markers for the graph, keyed by axis number (starting from 0).
	//! 
	//! This data will be applied to either the BACKGROUND or FOREGROUND layer, as required
	//! by the application.
	std::map<int, std::map<layer_t, std::vector<point_marker_t>>> point_markers_;

	//! \brief The data for plotting.
	//! 
	//! This data will be regenerated from the application data and configuration
	//! parameters every time the widget needs to be redrawn.
	std::map<int, plot_data_t> plot_data_; //!< Map of data sets to plot, keyed by axis number (starting from 1).

	//! \brief The width of the axes in pixels. This is used to calculate the dimensions of the plot area and the transformation schema for the data points.
	int axis_width_ = 50;
	int v_axis_width_ = 50;

	//! \brief Zoom capability for the axes.
	zoom_capability_t zoom_capability_ = ZOOM_ON_CURSOR;

	//! \brief Scrollability for the axes.
	bool scrollable_ = true;

	//! \brief Default text size
	Fl_Fontsize default_text_size_;

	//! \brief Default text colour
	Fl_Color default_text_colour_ = FL_BLACK;

	//! \brief Default text font
	Fl_Font default_text_font_ = FL_HELVETICA;

	//! \brief Saved mouse positions for handling dragging to scroll.
	int last_mouse_x_ = 0;
	int last_mouse_y_ = 0;

	//! \brief Data plot area in pixel coordinates. 
	//! Used to limit drawing of data and markers to the plot area.
	int plot_x_ = 0;
	int plot_y_ = 0;
	int plot_w_ = 0;
	int plot_h_ = 0;

};

//! \brief Derived class for Cartesian graphs.
class zc_graph_cartesian : public zc_graph_ {
public:
	//! \brief Constructor
	zc_graph_cartesian(int X, int Y, int W, int H, const char* L = nullptr) : zc_graph_(X, Y, W, H, L) {
		set_type(CARTESIAN);
		set_num_axes(2); // X and Y axes
		zoom_capability_ = ZOOM_ON_CURSOR; // Allow zooming on both axes centered on the cursor position
		scrollable_ = true; // Allow scrolling on both axes
	}

protected:

	//! \brief Layout the axes and plot area for a Cartesian graph.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	virtual void generate_value_marker(
		int axis_number,
		layer_t layer,
		const value_marker_t& marker
	) override;

	virtual layout_area_t get_layout_area(int x, int y) const override;

	virtual bool is_axis_horizontal(int axis_number) const override {
		return axis_number == 0; // X axis is horizontal, Y axis is vertical
	}
};

//! \brief Derived class for Cartesian graphs with a secondary Y axis.
class zc_graph_cartesian_2y : public zc_graph_ {
public:
	//! \brief Constructor
	zc_graph_cartesian_2y(int X, int Y, int W, int H, const char* L = nullptr) : zc_graph_(X, Y, W, H, L) {
		set_type(CARTESIAN_2Y);
		set_num_axes(3); // X, YL and YR axes
		zoom_capability_ = ZOOM_ON_CURSOR; // Allow zooming on both axes centered on the cursor position
		scrollable_ = true; // Allow scrolling on both axes
	}

protected:
	//! \brief Layout the axes and plot area for a Cartesian graph with a secondary Y axis.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	//! \brief Add a marker to the graph at a specific value or range of values.
	virtual void generate_value_marker(
		int axis_number,
		layer_t layer,
		const value_marker_t& marker
	) override;

	virtual layout_area_t get_layout_area(int x, int y) const override;

	virtual bool is_axis_horizontal(int axis_number) const override {
		return axis_number == 0; // X axis is horizontal, Y axis is vertical
	}
};

//! \brief Derived class for Cartesian graphs with overlaid axes.
class zc_graph_cart_overlay : public zc_graph_ {
public:
	//! \brief Constructor
	zc_graph_cart_overlay(int X, int Y, int W, int H, const char* L = nullptr) : zc_graph_(X, Y, W, H, L) {
		set_type(CART_OVERLAY);
		set_num_axes(2); // X and Y axes, but overlaid on the same plot area
		zoom_capability_ = ZOOM_ON_CURSOR; // Allow zooming on both axes centered on the cursor position
		scrollable_ = true; // Allow scrolling on both axes
	}

protected:
	//! \brief Layout the axes and plot area for a Cartesian graph with overlaid axes.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	//! \brief Add a marker to the graph at a specific value or range of values.
	virtual void generate_value_marker(
		int axis_number,
		layer_t layer,
		const value_marker_t& marker
	) override;

	virtual layout_area_t get_layout_area(int x, int y) const override;

	virtual bool is_axis_horizontal(int axis_number) const override {
		return axis_number == 0; // X axis is horizontal, Y axis is vertical
	}
};

//! \brief Derived class for Polar graphs.
class zc_graph_polar : public zc_graph_ {
public:
	//! \brief Constructor
	zc_graph_polar(int X, int Y, int W, int H, const char* L = nullptr) : zc_graph_(X, Y, W, H, L) {
		set_type(POLAR);
		set_num_axes(2); // R and Theta axes
		zoom_capability_ = ZOOM_ON_ORIGIN; // Allow zooming centred on the origin (0, 0) for both axes.
		scrollable_ = false; // Disallow scrolling on both axes
	}

protected:

	//! \brief Layout the axes and plot area for a Polar graph.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	virtual void generate_value_marker(
		int axis_number,
		layer_t layer,
		const value_marker_t& marker
	) override;


	virtual layout_area_t get_layout_area(int x, int y) const override;

	virtual bool custom_tick(
		int axis_number,
		const tick_data_t& tick_data,
		plot_object_t& tick_object
	) override;

	virtual bool is_axis_horizontal(int axis_number) const override {
		return axis_number == 0; // X axis is horizontal, Y axis is vertical
	}

};


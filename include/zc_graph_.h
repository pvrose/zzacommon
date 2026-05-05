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

//! \brief Header for the base class for graph widgets.
//! 
class zc_graph_ : public Fl_Widget {

public:

	//! \brief Type for a data type to be plotted. This is an identifier for the type of data being plotted.
	typedef std::pair<double, double> data_point_t; //!< Type for a data point (x, y)

	//! \brief Type of coordinates that a data_point_t represents.
	enum graph_type_t : uint8_t {
		NO_DATA,     //!< No data
		CARTESIAN,    //!< Cartesian coordinates (x, y)
		CARTESIAN_2Y,  //!< Cartesian coordinates with a secondary Y axis (x, y1) and (x, y2)
		CART_OVERLAY,   //!< Cartesian coordinates with X and Y axes overlaid on the same plot area (x, y).
		POLAR,        //!< Polar coordinates (r, theta)
	};

	//! \brief Structure to represent a set of data to be plotted.
	struct data_set_t {
		std::vector<data_point_t>* data; //!< Pointer to a vector of data points to be plotted
		zc_line_style style; //!< Line style to use for plotting this data set
	};

	//! \brief Identifies which of the pair of coordinates in a data_point_t is to be used.
	enum coordinate_type_t : uint8_t {
		FIRST,  //!< Use the first coordinate (e.g. x in Cartesian, r in Polar)
		SECOND, //!< Use the second coordinate (e.g. y in Cartesian, theta in Polar)
	};

	//! \brief Overlay markers for the plot, such as vertical lines to indicate specific X values.
	//! Markers can be added to any data type, and comprose either a single value
	//! or a range of values (e.g. for a shaded area on the plot). 
	//! Each marker includes a line style for rendering and a label for the legend, 
	//! plus a range of values. (For a single value, the range will be from that value to itself.)
	struct value_marker_t {
		zc_line_style style;  //!< Line style to use to draw this marker
		float value_1;        //!< First value for this marker (e.g. X value for a vertical line, or start of range for a shaded area)
		float value_2;        //!< Second value for this marker (e.g. same as value_1 for a single line, or end of range for a shaded area)
	};

	////! \brief Overlay marker type for a specific position on the plot.
	//enum position_marker_t : uint8_t {
	//	NO_MARKER,  //!< No marker
	//	POINT,      //!< Point marker at a specific coordinate
	//	LABEL,      //!< Text label at a specific coordinate
	//};

	////! \brief Data structure to represent a marker at a specific position on the plot.
	//struct position_marker_data_t {
	//	position_marker_t type; //!< Type of marker (e.g. POINT, LABEL)
	//	data_point_t position;  //!< Position of the marker in data coordinates
	//	std::string text;      //!< Text for the marker (only used if type is LABEL)
	//	zc_line_style line_style; //!< Line style for the marker (only used if type is TICK or POINT)
	//	zc_text_style text_style; //!< Text style for the marker (only used if type is LABEL)
	//	double direction; //!< Direction for the marker (e.g. angle for a tick, or 0 for a point)
	//};

	//! \brief Minimum and maximum values for data coordinates for an
	//! individual coordinate.
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

		//! \brief Add a single value to this range, expanding the range if necessary to include the value.
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

	// \brief data required for each axis.
	struct axis_data_t {
		range_t outer_range;       //!< Range of data values for this axis
		range_t inner_range;       //!< Range of data values currently displayed for this axis (may be zoomed or scrolled)
		range_t default_range;     //!< Default range for this axis in the absence of data
		range_t current_range;     //!< Current range for this axis (may be zoomed or scrolled)
		modifier_t modifier = NO_MODIFIER;  //!< Modifier for axis labels
		std::string unit;          //!< Unit to display on the axis (e.g. "Hz")
		std::string label;         //!< Base label for the axis (e.g. "Frequency")
		data_point_t label_position; //!< Position on the axis to draw the label (in data coordinates)
		int label_angle = 0;         //!< Angle to draw the label at (in degrees, where 0 is horizontal and positive is counter-clockwise)
		int tick_spacing_pixels = 0;   //!< Suggested spacing between ticks in pixels
		double position = 0.0;     //!< Position on other axis where this is drawn
		double inv_scale = 1.0;    //!< Inverse scale factor - number of units per pixel	
		tick_orientation_t tick_orientation = NO_TICKS; //!< Orientation of ticks for this axis
		std::vector<tick_data_t> ticks; //!< Data for the ticks on this axis (value and label)
	};

	//! \brief data required for the data plot area.
	struct data_area_t {
		data_point_t display_min;   //!< Data coordinates to display the axis at (bottom-left)
		data_point_t display_max;   //!< Data coordinates to display the axis at (top-right)
	};

	//! \brief Structure to represent a vertex (point) in the plot. This maps onto
	//! the paarmeters of the FLTK function fl_vertex() when plotting
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
		double a1 = 0;       //!< Starting angle of arc in degrees (0 is to the right, positive is counter-clockwise)
		double a2 = 0;       //!< Ending angle of arc in degrees (0 is to the right, positive is counter-clockwise)
	};

	//! \brief Structure to represent either a vertex or an arc segment in the plot.
	//! This allows a single data structure to represent both types of plot segments.
	struct plot_segment_t {
		//! The type of segment.
		enum segment_type_t :uint8_t {
			VERTEX,          //!< A single vertex (point) defined by X and Y coordinates.
			ARC,             //!< Arc segment defined by center, radius, and angles.
			GAP              //!< Gap in the line strip, used to break lines into segments.
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
	//! This maps on to the drawing functions: fl_begin_xxx()/fl_end_xxx() where xxx
	//! is the shape type (e.g. line, loop, points, polygon)
	enum shape_t : uint8_t {
		POINTS,         //!< Points defined by a list of vertices.
		LINE_STRIP,     //!< Line strip defined by a list of vertices and arcs.
		LOOP,           //!< Loop defined by a list of vertices and arcs (i.e. closed line strip).
		POLYGON,        //!< Polygon defined by a list of vertices and arcs (filled).
		COMPLEX,        //!< Complex shape defined by a list of vertices, gaps and arcs - see FLTK Complex polygon.
		TEXT,			//!< Text label defined by a position and string.
		TICK,			//!< Tick mark defined by a position and direction.
		TEXT_BOX,       //!< Text box with opaque background defined by a position, string, and inclination.
	};

	//! \brief Text alignment wrt the specified position for text labels and text boxes.
	enum text_alignment_t : uint8_t {
		ALIGN_LEFT,     //!< Align text to the left of the specified position.
		ALIGN_CENTRE,   //!< Align text centred on the specified position.
		ALIGN_RIGHT,    //!< Align text to the right of the specified position.
		ALIGN_ABOVE,    //!< Align text above the specified position.
		ALIGN_BELOW     //!< Align text below the specified position.
	};

	//! \brief Type of data to plot for one object.
	//! \todo Add text labels as a type of plot object, with parameters for text string, font, size, and position.
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
	//! Currently this is a simple linear transformation defined by the cartesian
	//! coordinates of the extremes of the data to be plotted,
	//! mapped onto the pixel coordinates of the widget. More than one
	//! such schema can be defined, allowing different data types to be plotted
	//! with different transformations (eg resistive and reactive components
	//! of an impedance plot).
	struct plot_xform_t {
		double x_min_ = 0;    //!< Minimum X-coordinate of the plot in pixels. Maps onto x() of the widget.
		double y_min_ = 0;    //!< Minimum Y-coordinate of the plot in pixels. Maps onto y() + h() of the widget (i.e. Y increases upwards).
		double x_max_ = 1;     //!< Maximum X-coordinate of the plot in pixels. Maps onto x() + w() of the widget.
		double y_max_ = 1;     //!< Maximum Y-coordinate of the plot in pixels. Maps onto y() of the widget.
	};

	//! \brief Drawing layers. Lower number will be drawn first then
	//! the remaining will be drawn on top.
	enum layer_t : uint8_t {
		BACKGROUND = 0,       //!< Items to be drawn behind all others. Background patters,
		GRID_LINES,           //!< Typically lines of equal value related to ticks on the axes.
		AXES,                 //!< The axes.
		DATA,                 //!< Points or lines reqpresenting the data displayed.
		FOREGROUND,           //!< Items, typically markers, to be displayed on top of all else
		MASK                  //!< Topmost layer to remove any unwanted artefacts from the plot area.
	};

	//! \brief All the data for specific data type to be plotted by layer.
	typedef std::map<layer_t, std::vector<plot_object_t>> plot_layer_data_t;

	//! \brief The graph can maintain multiple sets of data to be plotted.
	//! Each data can be plotted using a different transnformation schema, 
	//! allowing for example different data types to be plotted on the 
	//! same graph with different scales 
	//! (e.g. resistive and reactive components of an impedance plot).
	struct plot_data_t {
		plot_xform_t xform_schema; //!< Transformation schema to apply to the data points for this data type.
		data_area_t data_area; //!< Data coordinates corresponding to the plot area (i.e. the corners of the plot area in data coordinates). This is used to define the transformation schema for this data type.
		axis_data_t* first_axis = nullptr; //!< Pointer to the axis data for the first coordinate (e.g. X axis for Cartesian, R axis for Polar)
		axis_data_t* second_axis = nullptr; //!< Pointer to the axis data for the second coordinate (e.g. Y axis for Cartesian, Theta axis for Polar)
		plot_layer_data_t layer_data; //!< Data to plot, organised by layer.
	};

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
	void set_type(graph_type_t type) {
		graph_type_ = type;
	}

	//! \brief Set the number of supported axes for the graph.
	void set_num_axes(int num_axes) {
		num_axes_ = num_axes;
	}

	//! \brief Specify the basic parameters of an axis.
	//! Axis number = 0 for the first coordinate (common for all coordinate types,
	//! e.g. X axis for Cartesian, R axis for Polar), and the parameters for this axis
	//! Axis number > 0 for the second coordinate (separate for each coordinate type).
	void set_axis_params(
		int axis_number,                    //!< 
		modifier_t modifier = NO_MODIFIER,
		const std::string& unit = "",
		const std::string& label = "",
		int tick_spacing_pixels = 30
	);

	//! \brief Specify the inner and outer ranges for an axis.
	void set_axis_ranges(
		int axis_number,                 //!< The number of the axis to set the ranges for (starting from 0).
		const range_t& inner_range,      //!< Cannot zoom closer than this range
		const range_t& outer_range,      //!< Cannot zoom further than this range
		const range_t& default_range     //!< Default range to use for this axis - resetting zoom will reset to this range.
	);

	//! \brief Add a data set to the graph.
	//! \param axis_number The number of the data set to add (starting from 0). This allows multiple data sets to be plotted against different
	//! axes or with different transformation schemata.
	//! \param data The data points to plot for this data set.
	//! \param style The line style to use for this data set.
	void add_data_set(
		int axis_number,
		std::vector<data_point_t>* data,
		zc_line_style style = zc_line_style()
	);

	//! \brief Add a marker to the graph at a specific value or range of values.
	virtual void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_a
		)	
	{
		add_marker(axis_number, layer, style, value_a, value_a);
	};

	//! \brief Add a marker to the graph at a specific value or range of values.
	virtual void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_1,
		double value_2
	) = 0;

	//! \brief Add a text label to the graph at a specific position.
	void add_label(
		int axis_number,
		layer_t layer,
		const std::string& text,
		zc_text_style style,
		data_point_t position
	);

	//! \brief Clear all data sets and markers from the graph.
	void clear();

	//\brief Initiate the graph with data and parameters.
	void initiate();

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

protected:

	//! \brief Place the axes and plot areas. Define transformation schemata.
	//! 
	//! This MUST be implemented by derived classes to define the layout of the graph, including the placement of the axes and plot area, and the transformation schemata to apply to the data points for plotting.
	//! 
	virtual void layout() = 0;

	//! 
	//! \brief Convert given coordinates to Cartesian coordinates for plotting.
	const data_point_t convert_point(const data_point_t& point) const {
		switch (graph_type_) {
		case CARTESIAN:
		case CARTESIAN_2Y:
		case CART_OVERLAY:
			return point;
		case POLAR: {
			double r = point.first;
			// TODO: Decide whether use degrees or radians for theta. For now, assume radians, but may want to add a flag to specify this.
			double theta = point.second;
			double x = r * cos(theta);
			double y = r * sin(theta);
			return { x, y };
		}
		default:
			return point;
		}
	}

	//! \brief Apply the transformation schema to the data points for plotting.
	void apply_transformations(plot_xform_t schema);

	//! \brief Generate axis drawing objects.
	void generate_axis_grid(
		int axis_number              //!< The number of the axis to generate (starting from 0).
	);

	//! \brief generate the axis line.
	void generate_axis_line(
		int axis_number              //!< The number of the axis to generate the line for (starting from 0).
	);

	//! \brief generate tick marks for the axis.
	void generate_axis_ticks(
		int axis_number              //!< The number of the axis to generate ticks for (starting from 0).
	);

	//! \brief Generate grid lines for the plot.
	void generate_grid_lines(
		int axis_number              //!< The number of the axis to generate grid lines for (starting from 0).
	);

	//! \brief Generate label for the axis.
	void generate_axis_label(
		int axis_number              //!< The number of the axis to generate the label for (starting from 0).
	);

	//! \brief Generate data lines for the data set.
	void generate_data_lines(
		int axis_number              //!< The number of the data set to generate lines for (starting from 0).
	);

	//! \brief Set tick and grid points for the axis
	void set_ticks(
		int axis_number,              //!< The number of the axis to generate ticks for (starting from 0).
		int tick_spacing_pixels,      //!< The desired spacing between ticks in pixels. This can be used to calculate the appropriate tick spacing in data coordinates based on the current zoom level and transformation schema.
		int length_pixels             //!< The length of the axis in pixels. This can be used to calculate the number of ticks based on the desired spacing.
	);

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
	void draw_plot_object(
		const plot_object_t & object     //!< The object to draw.
	);

	//! The number of axes supported
	int num_axes_ = 0;

	//! \brief The data for the graph. Pointers to application data.
	std::map<int, data_set_t> data_sets_;

	//! \brief The data
	std::map<int, plot_data_t> plot_data_; //!< Map of data sets to plot, keyed by axis number (starting from 1).

	//! \brief The graph_type for the graph, which defines the layout of the axes and plot area.
	graph_type_t graph_type_ = NO_DATA;

	//! \brief The axis data for the graph, keyed by axis number (starting from 0)
	std::map<int, axis_data_t> axes_data_;

	//! \brief The width of the axes in pixels. This is used to calculate the dimensions of the plot area and the transformation schema for the data points.
	int axis_width_ = 50;

	//! \brief Zoom capability for the axes.
	zoom_capability_t zoom_capability_ = ZOOM_ON_CURSOR;

	//! \brief Scrollability for the axes.
	bool scrollable_ = true;

	//! \brief Default text size
	Fl_Fontsize default_text_size_ = 12;

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

	//! \brief Layout the axes and plot area for a Cartesian graph.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_1,
		double value_2
	) override;
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
	//! \brief Layout the axes and plot area for a Cartesian graph with a secondary Y axis.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_1,
		double value_2
	) override;
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
	//! \brief Layout the axes and plot area for a Cartesian graph with overlaid axes.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_1,
		double value_2
	) override;
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
	//! \brief Layout the axes and plot area for a Polar graph.
	void layout() override;

	//! \brief Add a marker to the graph at a specific value or range of values.
	void add_marker(
		int axis_number,
		layer_t layer,
		zc_line_style style,
		double value_1,
		double value_2
	) override;
};


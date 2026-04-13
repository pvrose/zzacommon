/*
	Copyright 2017-2026, Philip Rose, GM3ZZA
	
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
// To update the timestamp and version string, run 'cmake' in the build directory.
#pragma once

#include <string>

/*!
\mainpage ZZACOMMON API Documentation

ZZACOMMON is a library of useful methods and classes developed for ZZALOG but
that have been generalised for use with other projects.

ZZACOMMON is based in part on the work of the FLTK project <A HREF=https://www.fltk.org>https://www.fltk.org</A>.

It provides a number of FLTK-derived widgets and other object classes for general use:

- zc_banner
  This is a banner window that provides a progress wheel and status message screen. 
- zc_button_dialog
This is a button which opens a dialog when clicked. The dialog can exchange data with the
calling application. The dialog class and data structure are template parameters of this
class. It has been designed to work with zc_line_style and zc_text_style data structures 
and their associated dialogs.
- zc_button_input
This is a combination of Fl_Input and Fl_Button where clicking the button interacts
in some way with the operation of the input. This is only ever intended to be used
as the basis for other widgets: it is not a free-standing widget. 
It has the following derived classes.
  - zc_calendar_input
This is a version of button_input where the button opens a calendat window to allow
a date to be selected that is then placed as the text value of the input.
It uses the following component classes.
    - zc_calendar
The calendar window opened by the button in calendar_input.
    - zc_calendar_table
The calendar object used in calendar.
  - zc_filename_input
  This is a derived class from button_input where the button opens a file selection dialog.
  The file selected by this dialog is then copied into the input widget for use by
  the application.
  - zc_password_input
This is a version of button_input where the button toggles the behaviour of the input between
FL_NORMAL_INPUT (where the text is visible) and FL_SECRET_INPUT (where the text
is masked).
- zc_callback.h
This is a set of standard callbacks that can be used either directly as the callback
for widget operations or used within custom callbacks.
See \ref zc_callback.h.
- zc_drawing.h
This is a set of drawing constants to maintain a consistent "look-and-feel" across
all projects.
- zc_file_holder
This class provides a consistent approach to filestore management used across all
projects.
- zc_graph
This class provides a simple X/Y line graph. 
This is deprecated in favour of zc_graph_xy and zc_graph_x2y, but is retained 
for backward compatibility with older projects.
- zc_graph_base
This is the base class for all graph types.
See \ref zc_graph_base. It has the following components and derived classes.
  - zc_graph_axis
This class provides a graph axis with automatic scaling and labelling for 
use in zc_graph_base and its derived classes.
  - zc_graph_plot
This class provides the plot area for a graph and is used in zc_graph_base and its
derived classes.
  - zc_graph_xy
This class provides an X/Y line graph with a single Y axis and is derived
from zc_graph_base.
  - zc_graph_x2y
This class provides an X/Y line graph with two Y axes and is derived from zc_graph_xy.
- zc_input_hierch
This is an extension of Fl_Input_Choice that uses a hierarchic menu for use where
there are a large number of menu options. The supplied options are split into
groups according to the first letter (or more) of the option.
- zc_line_style
This class and associated dialog zc_line_style_dialog allow an application to provide
its user to select the drawing style, colour and thickness of lines. Drawing styles are 
defined by FLTK such as FL_SOLID.
- zc_rpc_handler
This class provides a protocol handler for an XML-RPC inter-application interface.
It converts between the XML passed over the interface and methods using the 
data structure zc_rpc_data_item.
- zc_serial
Thos class provides a lightweight wrapper around the boost::asio::serial_port library
to provide simple read/write access to a serial port.
- zc_settings
This class provides a JSON based settings file.
- zc_socket_server
This class provides an OS-independent wrapper for handling data transfers over
inter-application sockets.
- zc_status
This class encapsulates banner and is the main user interface for it.
- zc_symbols.h
This is a set of methods that each draw an image to be used in widget labels using the
'\@' construct.
- zc_tabs_nonav
This is an extension of Fl_Tabs that inhibits the use of navigaton keys to switch
between tab entries.
- zc_text_style
This class and associated dialog zc_text_style_dialog allow an application to provide
its user to select the font, size and colour for text. The available fonts are 
listed as generic sans-serif, serif and mono-spaced fonts (with bold and italic variants), 
plus a terminal and a symbol font. These are equivalent to the named fonts FL_HELVETICA
etc provided by FLTK.
- zc_ticker
This class provides an application-wide clock that can send timer callbacks to 
subscribed instances within the application. It, itself, receives a callback
every 100 ms and forwards it to subscribers according to their needs.
- zc_utils.h
This is a set of standard utility methods.

*/

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

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include <FL/Fl_Double_Window.H>

enum status_t : char;

// FLTK classes
class Fl_Box;
class Fl_Button;
class Fl_Choice;
class Fl_Fill_Dial;
class Fl_Output;
class Fl_Text_Display;

//!  \brief This class provides a banner launched at start-up and shows the progress 
//!  of various activities during the execution of the app.
//!
//! This is a separate window from the main window of the app.
//! It works better with the class status to act as an interface
//! and control the look and feel of the banner display.
//! \image html zc_banner.png "Example of the banner display"
//! \image html zc_banner_2.png "Showing banner at minimum size"
class zc_banner :
	public Fl_Double_Window
{
public:
	//! Constructor - see Fl_Double_Window.

	//! \param W width.
	//! \param H height.
	//! \param L label.
	zc_banner(int W, int H, const char* L = nullptr);

	//! Destructor
	virtual ~zc_banner();

	//! This method builds the banner object from the component widgets.
	void create_form();

	//! Enables and/or configures the compoment widgets in response to a change of conditions.
	void enable_widgets();

	//! Load settings.
	void load_settings();

	//! Save settings.
	void save_settings();

	//! Add a message to the banner.

	//! \param type the status of the message. This determines which output widget displays the
	//! message and the colour in which to display it.
	//! \param msg the message to display.
	//! \param ts timestamp string to prefix the message with.
	void add_message(status_t type, const char* msg, const char* ts);

	//! Start a progress clock.

	//! \param max_value the maximum count of object_t items expected.
	//! \param object identifier of the object_t item.
	//! \param colour Colour to be used for the progress wheel.
	//! \param msg message to accompany any display of progress.
	//! \param suffix textual description of the objecy (eg bytes or records).
	void start_progress(uint64_t max_value, const char* object, Fl_Color colour, const char* msg, const char* suffix);

	//! Report on progress - update the clock.

	//! \param value the current progress value. The display is only updated if the 
	//! change in progress in greater than one hundredth of the maximum value.
	void add_progress(uint64_t value);

	//! \brief End a progress report normally. 

	//! However if add_progress indicates 100% complete then
	//! the progress report will indicate complete.
	void end_progress();

	//! Cancel a progress report if an abnormal condition occurred.

	//! \param msg message to indicate the reason for the cancellation.
	void cancel_progress(const char* msg);

	//! \brief The callback is triggered by the system close button.
	//! This action calls status::close_. This in itself is a callback
	//! that the main app has set up to invoke a closure.
	//! \param w The invoking widget.
	//! \param v User data: expected to be nullptr.
	static void cb_close(Fl_Widget* w, void* v);

	//! \brief The callback is triggered by verbosity level choice widget.
	static void cb_verbosity(Fl_Widget* w, void* v);

	//! \brief The callback is triggered by the topic filter button.
	static void cb_filter(Fl_Widget* w, void* v);

	//! Called indirectly through status when the main app is closing.
	void close();

	//! Set font \p f and size \p sz of display text
	void font(Fl_Font f, Fl_Fontsize sz);

	//! Get font 
	Fl_Font font();
	//! Get fontsize
	Fl_Fontsize fontsize();

	//! Update the display after changing verbosity level.
	void update_display();

protected:

	//! Add message to the message history display (with colour)
	void copy_msg_display(status_t type, const char* msg, const char* ts);

	//! Return true if the message of the given type should be
	//! displayed in the banner according to the current verbosity level.
	bool display_message(status_t type);

	//! Return true if the message is for the current topic filter.
	bool message_for_current_topic(const std::string& topic);

	//! Add topic to the topic filter list and update the filter choice widget.
	void add_topic(const std::string& topic);

	// Widgets
	Fl_Box* bx_icon_;                   //!< holder for the ZZALOG icon.
	Fl_Fill_Dial* fd_progress_;         //!< progress "clock".
	Fl_Box* op_app_title_;              //!< output for app title.
	Fl_Box* bx_prog_value_;             //!< display for current progtress value.
	Fl_Text_Display* display_;          //!< display for message history log.
	Fl_Choice* ch_verbosity_;           //!< choice widget for verbosity level of banner display.
	Fl_Choice* ch_filter_;              //!< choice widget for topic filter.

	//! Progress maximum value
	uint64_t max_value_;

	//! Delta value to trigger update of progress. Set to a fixed fraction of the maximum value.
	uint64_t delta_;

	//! \brief Previous progress value.
	//! The progress clock is updated if the new value is greater than
	//! prev_value_ + delta_.
	uint64_t prev_value_;

	//! Progress unit. A word or phrase indicating how progress is being counted.
	const char* prg_unit_;

	//! Progress message.
	const char* prg_msg_;

	//! \brief Progress object - affects the colour of the progress clock.
	const char* prg_object_;

	//! Progress colour
	Fl_Color prg_colour_;

	//! Display "CLOSING" across the window.
	bool closing_ = false;

	//! Verbosity level of the banner display.
	enum verbosity_t {
		VB_MINIMAL,            //!< Only FATAL and SEVERE messages are displayed.
		VB_ERRORS,             //!< Only ERROR and above messages are displayed.
		VB_WARNINGS,           //!< Only WARNING and above messages are displayed.
		VB_INFO,               //!< Only INFO and above messages are displayed.
		VB_FULL                //!< All messages are displayed.
	} verbosity_ = VB_INFO;

	//! \brief The complete history of messages received by the banner.
	struct message {
		status_t type;         //!< The status of the message.
		std::string topic;       //!< The topic of the message - extracted from the first word of the message text.
		std::string text;        //!< The message text.
		std::string style;       //!< The style to display the message in (colour, font, etc).
	};
	std::vector<message> message_history_; //!< The complete history of messages received by the banner.

	//! \brief The set of topics that have been received by the banner.
	std::set<std::string> topics_;

	//! \brief Selected topic for display in the banner. Only messages with this topic will be displayed.
	std::string filter_topic_;
};


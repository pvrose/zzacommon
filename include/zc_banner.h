#pragma once

#include <cstdint>
#include <FL/Fl_Double_Window.H>

enum status_t : char;

// FLTK classes
class Fl_Box;
class Fl_Button;
class Fl_Fill_Dial;
class Fl_Multiline_Output;
class Fl_Output;
class Fl_Text_Display;

//!  \brief This class provides a banner launched at start-up and shows the progress 
//!  of various activities during the execution of the app.
//!
//! This is a separate window from the main window of the app.
//! It works better with the class status to act as an interface
//! and control the look and feel of the banner display.
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

    //! Override Fl_Double_Window::draw().
    
    //! When the main app is closing, the word "CLOSING" is displayed across the banner.
    //! This provides an indication to the user that the app is, in fact, closing as
    //! this can in some cases take a noticeable time.
    virtual void draw();

    //! Called indirectly through status when the main app is closing.
    void close();

	//! Set font \p f and size \p sz of display text
	void font(Fl_Font f, Fl_Fontsize sz);

	//! Get font 
	Fl_Font font();
	//! Get fontsize
	Fl_Fontsize fontsize();

protected:

    //! Add message to the message history display (with colour)
    void copy_msg_display(status_t type, const char* msg, const char* ts);

    // Widgets
    Fl_Box* bx_icon_;                   //!< holder for the ZZALOG icon.
    Fl_Fill_Dial* fd_progress_;         //!< progress "clock".
    Fl_Multiline_Output* op_msg_low_;   //!< output for low cetegory messages (ST_WARNING and below).
    Fl_Multiline_Output* op_msg_high_;  //!< output for high category messages (ST_ERROR and above).
    Fl_Output* op_prog_title_;          //!< output for progress text message.
    Fl_Box* bx_prog_value_;             //!< display for current progtress value.
    Fl_Text_Display* display_;          //!< display for message history log.
    Fl_Box* bx_closing_;                //!< container for "CLOSING" message.

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

};


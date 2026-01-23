#include "zc_button_input.h"

#include <cstdint>

#include <FL/Fl_Widget.H>


//! \brief This class implements a combination of input widget and a button which opens a file browser.
//! The file browser is of type Fl_Native_File_Chooser.
class zc_filename_input :
    public zc_button_input
{

public:

    //! Defines browser type to open.
    enum type_t : uint8_t {
        FILE,            //!< Opens a file browser
        DIRECTORY        //!< Opens a directory browser
    };
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    zc_filename_input(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~zc_filename_input();

    //! Set title to \p value on file chooser dialog
    void title(const char* value);
    //! Set filename filter pattern to \p value.
    void pattern(const char* value);
    //! Set chooser type to \p value
    void type(type_t value);


protected:

    //! \brief Callback from clicking button: opens file or directory browser, 
    //! The selected object's name will be transferred to input widget.
    static void cb_button(Fl_Widget* w, void* v);

    //! Browser label.
    const char* title_;
    //! Pattern to filter filenames
    const char* pattern_;

    //! Browser type (FILE or DIRECTORY)
    type_t type_;
};

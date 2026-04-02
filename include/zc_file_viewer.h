/*
	Copyright 2025-2026, Philip Rose, GM3ZZA
	
    This file is part of ZZACOMMON. Amateur Radio Logging Software.

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

#include <string>

#include <FL/Fl_Window.H>


class Fl_Text_Editor;
class Fl_Text_Buffer;
class Fl_Button;

//! Rudimentary file editor
class zc_file_viewer :
    public Fl_Window
{
public:
    //! Constructor.

    //! \param W width 
    //! \param H height
    //! \param L label
    zc_file_viewer(int W, int H, const char* L = nullptr);
    //! Destructor.
    ~zc_file_viewer();

    //! Instantiate component widgets.
    void create();
    //! Configure widgets after data changes.
    void enable_widgets();

    //! Load  specified file \p name into the editor.
    void load_file(std::string name);

    //! Returns name of displayed file
    std::string file();

    //! File has been changed and not saved.
    bool is_dirty() const;

    //! Callback from window close button.
    static void cb_close(Fl_Widget* w, void* v);
    //! Callback from "Save" button, saves the file.
    static void cb_save(Fl_Widget* w, void* v);
    //! Callback from "Reload" button, clears buffer and loads file again without saving any edits.
    static void cb_reload(Fl_Widget* w, void* v);

    //! Callback when text buffer in the editor has been modified.
    //! 
    //! Only parameters \p inserted and \p deleted are used and indicate that a change 
    //! has been made, so marks the data "dirty".
    static void cb_modified(int pos, int inserted, int deleted, int restyled, const char* deletion, void* arg);

	//! Key binding for CTRL-s - calls save
	static int kf_save(int c, Fl_Text_Editor* editor);
	//! Key binding for CTRL-x - ends window
	static int kf_close(int c, Fl_Text_Editor* editor);

protected:

    //! Save edited data back to the file.
    void save_file();

    Fl_Text_Editor* display_;    //!< Text editor widget.

    Fl_Text_Buffer* buffer_;     //!< Data buffer used in editor.

    Fl_Button* bn_save_;         //!< "Save" button.
    Fl_Button* bn_reload_;       //!< "Reload" button.

    std::string filename_;            //!< Name of file being edited.
    bool dirty_;                 //!< Data is "dirty", ie it has been changed and 
                                 //!< not written to store.

};


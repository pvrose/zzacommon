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
#include "zc_filename_input.h"
#include "zc_button_input.h"

#include "zc_utils.h"

#include <cstdio>
#include <string>

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Widget.H>

zc_filename_input::zc_filename_input(int X, int Y, int W, int H, const char* L) :
    zc_button_input(X, Y, W, H, L) ,
    title_(nullptr) ,
    pattern_(nullptr) ,
    type_(FILE)
{
    bn_->label("@fileopen");
    bn_->callback(cb_button);

}

zc_filename_input::~zc_filename_input() {}

void zc_filename_input::title(const char* value) {
    title_ = value;
}

void zc_filename_input::pattern(const char* value) {
    pattern_ = value;
}

void zc_filename_input::type(zc_filename_input::type_t value) {
    type_ = value;
}

void zc_filename_input::cb_button(Fl_Widget* w, void* v) {
    zc_filename_input* that = zc::ancestor_view<zc_filename_input>(w);
    std::string filename = that->ip_->value();
    Fl_Native_File_Chooser::Type fb_type = Fl_Native_File_Chooser::BROWSE_FILE;
    switch(that->type_) {
        case FILE:
        fb_type = Fl_Native_File_Chooser::BROWSE_FILE;
        break;
        case DIRECTORY:
        fb_type = Fl_Native_File_Chooser::BROWSE_DIRECTORY;
        break;
    }

	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(fb_type);
    chooser->title(that->title_);
    if (that->type_ == FILE) {
        chooser->filter(that->pattern_);
        chooser->directory(zc::directory(filename).c_str());
    } else {
        chooser->directory(filename.c_str());
    }
    chooser->preset_file(zc::terminal(filename).c_str());
    // Now display the dialog
    switch(chooser->show()) {
        case 0: 
        that->ip_->value(chooser->filename());
        break;

        case -1:
        // Error
        printf("ERROR: %s\n", chooser->errmsg());
        break;

    }
    that->ip_->do_callback();


}

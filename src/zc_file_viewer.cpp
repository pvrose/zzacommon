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
#include "zc_file_viewer.h"
#include "zc_status.h"

#include "zc_utils.h"
#include "zc_drawing.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

// Constructor
zc_file_viewer::zc_file_viewer(int W, int H, const char* L) :
	Fl_Window(W, H, L)
{
	dirty_ = false;
	filename_ = "";
	create();
	enable_widgets();
	callback(cb_close);
}

// Destructor
zc_file_viewer::~zc_file_viewer() {}

// Create
void zc_file_viewer::create() {
	buffer_ = new Fl_Text_Buffer();
	buffer_->add_modify_callback(cb_modified, this);

	int avail_height = h() - HBUTTON;
	display_ = new Fl_Text_Editor(0, 0, w(), avail_height);
	display_->box(FL_BORDER_BOX);
	display_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	display_->linenumber_width(WLABEL);
	display_->textfont(FL_COURIER);
	display_->linenumber_font(FL_COURIER);
	display_->textsize(12);
	display_->add_key_binding('s', FL_CTRL, kf_save);
	display_->add_key_binding('x', FL_CTRL, kf_close);

	display_->buffer(buffer_);

	bn_reload_ = new Fl_Button(w() - (2 * WBUTTON), avail_height, WBUTTON, HBUTTON, "Reload");
	bn_reload_->callback(cb_reload, nullptr);
	bn_reload_->tooltip("Restore original file");

	bn_save_ = new Fl_Button(w() - WBUTTON, avail_height, WBUTTON, HBUTTON, "Save");
	bn_save_->callback(cb_save, nullptr);
	bn_save_->tooltip("SAve the file");

	resizable(display_);

	end();
}

// Enable widgets
void zc_file_viewer::enable_widgets() {
	char l[128];
	if (is_dirty()) {
		bn_reload_->activate();
		bn_save_->activate();
		snprintf(l, sizeof(l), "%s %s", filename_.c_str(), "[Modified]");
		copy_label(l);
	} else {
		bn_reload_->deactivate();
		bn_save_->deactivate();
		copy_label(filename_.c_str());
	}
}

// Callback 
void zc_file_viewer::cb_close(Fl_Widget* w, void* v) {
	zc_file_viewer* win = zc::ancestor_view<zc_file_viewer>(w);
	win->hide();
}

// Reload file
void zc_file_viewer::cb_reload(Fl_Widget* w, void* v) {
	zc_file_viewer* that = zc::ancestor_view<zc_file_viewer>(w);
	int len = that->buffer_->length();
	that->buffer_->remove(0, len);
	that->load_file(that->filename_);
}

// Save file
void zc_file_viewer::cb_save(Fl_Widget* w, void* v) {
	zc_file_viewer* that = zc::ancestor_view<zc_file_viewer>(w);
	that->save_file();
}

// Buffer modified
void zc_file_viewer::cb_modified(int pos, int inserted, int deleted, int restyled, const char* deleteion, void* arg) {
	zc_file_viewer* that = (zc_file_viewer*)arg;
	if (inserted || deleted) {
		that->dirty_ = true;
		that->enable_widgets();
	}
}

// Set text
void zc_file_viewer::load_file(std::string name) {
	if (is_dirty()) {
		switch(fl_choice("Existing file has been modified - Save or cancel?", "Save", "Cancel", nullptr)) {
			case 0: {
				save_file();
				break;
			}
			case 1: {
				break;
			}
		}
	}
	filename_ = name;
	char msg[128];
	switch (buffer_->loadfile(filename_.c_str())) {
		case 1:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to open %s", filename_.c_str(), strerror(errno));
		status_->misc_status(ST_ERROR, msg);
		break;
		case 2:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to read completely", filename_.c_str());
		status_->misc_status(ST_WARNING, msg);
		break;
	}
	copy_label(filename_.c_str());
	dirty_ = false;
	enable_widgets();
	show();
}

std::string zc_file_viewer::file() {
	return filename_;
}

// Save file
void zc_file_viewer::save_file() {
	char msg[128];
	switch(buffer_->savefile(filename_.c_str())) {
		case 1:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to open %s", filename_.c_str(), strerror(errno));
		if (status_) status_->misc_status(ST_ERROR, msg);
		break;
		case 2:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to write completely", filename_.c_str());
		if (status_) status_->misc_status(ST_WARNING, msg);
		break;
		case 0:
		snprintf(msg, sizeof(msg), "APPS: File %s written OK", filename_.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
		dirty_ = false;
		enable_widgets();
		break; 
	}
}

// Is dirty
bool zc_file_viewer::is_dirty() const {
	return dirty_;
}

// CTRL-s Entered
int zc_file_viewer::kf_save(int key, Fl_Text_Editor* editor) {
	cb_save(editor, nullptr);
	return 0;
}

// CTRL-x Entered
int zc_file_viewer::kf_close(int key, Fl_Text_Editor* editor) {
	cb_close(editor, nullptr);
	return 0;
}


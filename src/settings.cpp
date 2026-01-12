#include "settings.h"

#include "file_holder.h"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <string>

using json = nlohmann::json;

extern std::string APP_NAME;


json* settings::all_settings_ = nullptr;
int settings::attachments_ = 0;

//! Basic constructor
settings::settings() {
	// Ddfault filename ZZALOG.json
	std::string filename;
	std::ifstream i;
	parent_ = nullptr;
	if (attachments_ == 0) {
		if (file_holder_->get_file(FILE_SETTINGS, i, filename)) {
			// Load in file
			try {
				json j;
				i >> j;
				if (j.find(APP_NAME) != j.end() &&
					!j.at(APP_NAME).is_null()) {
					all_settings_ = new json(j.at(APP_NAME));
				}
				else {
					all_settings_ = new json;
				}
				i.close();
			}
			catch (const json::exception& e) {
				printf("SETTINGS: Reading JSON failed %d (%s)\n",
					e.id, e.what());
				i.close();
				all_settings_ = new json;
			}
		}
		else {
			all_settings_ = new json;
		}
	}
	attachments_++;
	data_ = all_settings_;
}

//! Construct a sub-group of settings
settings::settings(settings* parent, const std::string name) {
	parent_ = parent;
	name_ = name;
	if (parent->data_->find(name) == parent->data_->end()) {
		(*parent_->data_)[name] = json();
	}
	data_ = &parent->data_->at(name);
}

//! DEstructor - copy data back to file
settings::~settings() {
	if (parent_ == nullptr) {
		flush();
		attachments_--;
		if (attachments_ == 0) {
			all_settings_ = nullptr;
		}
	}
}

//! Clear the settings
void settings::clear() {
	// Do nothing for now
}

//! Flush the settings to filestore
void settings::flush() {
	if (parent_ != nullptr) {
		parent_->flush();
	}
	else {
		std::string filename;
		std::ofstream o;
		file_holder_->get_file(FILE_SETTINGS, o, filename);
		parent_ = nullptr;
		json j;
		j[APP_NAME] = *data_;
		o << std::setw(2) << j << '\n';
		o.close();
	}
}
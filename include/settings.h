#pragma once

#include <nlohmann/json.hpp>

#include <string>

using json = nlohmann::json;

//! JSON based settings, a bit based on Fl_Preferences
class settings
{
public:
	//! Basic constructor
	settings();

	//! Construct a sub-group of settings
	settings(settings* parent, std::string name);

	//! DEstructor
	~settings();

	//! Clear the settings and unhook the group
	void clear();

	//! Flush the settings to filestore
	void flush();

protected:

	//! Set of data items
	static json* all_settings_;

	//! Number of attachments
	static int attachments_;

	//! Data relevant to this instance of settings
	json* data_;

	//! Parent settings
	settings* parent_;

	//! The name of this group
	std::string name_;

public:

	//! Get object
	template <class T>
	bool get(std::string name, T& value, const T def) {
		bool exists = true;
		if (data_->find(name.c_str()) == data_->end()) {
			(*data_)[name.c_str()] = def;
			exists = false;
		}
		try {
			value = data_->at(name.c_str()).get<T>();
		}
		catch (const json::exception& e) {
			(*data_)[name.c_str()] = def;
			exists = false;
		}
		return exists;
	}

	//! Set object
	template <class T>
	void set(std::string name, const T value) {
		(*data_)[name.c_str()] = value;
	}


};


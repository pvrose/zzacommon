#pragma once

#include <nlohmann/json.hpp>

#include <string>

using json = nlohmann::json;

//! \brief This class provides access to a JSON-based setting file.
//! The interface is simillar to that of Fl_Preferences in fltk, but simplified
//! for specific usages.
class settings
{
public:
	//! Basic constructor
	settings();

	//! \brief Construct a sub-group of settings.
	//! \param parent Parent settings for which this object will form a group.
	//! \param name Name of the settings group.
	settings(settings* parent, std::string name);

	//! Destructor
	~settings();

	//! Clear the settings and unhook the group.
	void clear();

	//! Flush the settings to filestore.
	void flush();

protected:

	//! The entite contents of the settings file as a JSON object.
	static json* all_settings_;

	//! Number of items currently accessing the settings file.
	static int attachments_;

	//! The contents of this settings group as a JSON object
	json* data_;

	//! Parent settings group.
	settings* parent_;

	//! The name of this group.
	std::string name_;

public:

	//! \brief Get object of type \p T.
    //! \param name The name of the object.
	//! \param value Reference to where the data will be stored.
	//! \param def Default value in the absence of data in the settings file.
	//! \todo Pass def by reference.
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

	//! \brief Set object of type T.
	//! \param name The name of the object.
	//! \param value The value to be written.
	//! \todo pass value by reference.
	template <class T>
	void set(std::string name, const T value) {
		(*data_)[name.c_str()] = value;
	}


};


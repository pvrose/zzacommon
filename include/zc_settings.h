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

#include <nlohmann/json.hpp>

#include <string>

using json = nlohmann::json;

//! \brief This class provides access to a JSON-based setting file.
//! The interface is simillar to that of Fl_Preferences in fltk, but simplified
//! for specific usages.
class zc_settings
{
public:
	//! Basic constructor
	zc_settings();

	//! \brief Construct a sub-group of zc_settings.
	//! \param parent Parent settings for which this object will form a group.
	//! \param name Name of the settings group.
	zc_settings(zc_settings* parent, std::string name);

	//! \brief Construct a sub-group of zc_settings.
	//! \param parent Parent settings for which this object will form a group.
	//! \param index Index of the subgroup to access.
	zc_settings(zc_settings* parent, int index);

	//! Destructor
	~zc_settings();

	//! Clear the settings and unhook the group.
	void clear();

	//! Flush the settings to filestore.
	void flush();
	
	//! Get the settings name.
	std::string name() const { return name_; }

	//! \brief Get number of groups contained within this group.
	//! \return The number of groups contained within this group.
	int group_count() const;

	//! \brief Get the name of the group at the given index.
	//! \param index The index of the group to get the name of.
	//! \return The name of the group at the given index.
	std::string group_name(int index) const;

	//! \brief Get a subgroup of this group at given index.
	//! \param index The index of the subgroup to get.
	//! \return A zc_settings object for the subgroup at the given index.
	zc_settings& get_group(int index) const;

protected:

	//! The entite contents of the settings file as a JSON object.
	static json* all_settings_;

	//! Number of items currently accessing the settings file.
	static int attachments_;

	//! The contents of this settings group as a JSON object
	json* data_;

	//! Parent settings group.
	zc_settings* parent_;

	//! The name of this group.
	std::string name_;

public:

	//! \brief Get object of type \p T.
    //! \param name The name of the object.
	//! \param value Reference to where the data will be stored.
	//! \param def Default value in the absence of data in the settings file.
	//! \return Returns true if item existed.
	template <class T>
	bool get(std::string name, T& value, const T& def) {
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
	template <class T>
	void set(std::string name, const T& value) {
		(*data_)[name.c_str()] = value;
	}


};


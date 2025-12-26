#pragma once

#include <fstream>
#include <istream>
#include <map>
#include <ostream>
#include <string>

//! Reset configuration files
extern uint16_t DEBUG_RESET_CONFIG;

//! File control structore
struct file_control_t {
	std::string filename;                   //!< Filename
	bool reference;                         //!< file is reference - from source
	bool read_only;                         //!< Not written - do not copy from source
	uint16_t reset_mask;                    //!< Reset mask
	bool fatal{ true };                     //!< Fatal error if not present
};

//! File contents
enum file_contents_t : uint8_t {
	FILE_ICON_ZZA,                          //!< Always have an icon
	FILE_STATUS,                            //!< Conditionally have a status log
	FILE_SETTINGS,                          //!< Conditionally have a settings file
	FILE_USER = 16                          //!< Provide some free space for m ore
};

//! Data type for getting directory 
enum file_data_t : uint8_t {
	DATA_WORKING,               //!< Working directory
    DATA_SOURCE,                //!< Original source directory
	DATA_CODEGEN,               //!< Directory for generated code
	DATA_HTML,                  //!< Directory for HTML & PDF (userguide and code docs)
};

//! Forward declaration to self
class file_holder;
extern file_holder* file_holder_;

//! Holder for file naming
class file_holder
{
public:
	//! Constructor
	file_holder(const char* arg0, const std::map<uint8_t, file_control_t>& control);
	//! Destructor
	~file_holder() {};

	//! Get file for input

	//! \param type File contents
	//! \param is Returned input stream
	//! \param filename Returns name of opened file
	//! \returns true if successful
	bool get_file(uint8_t type, std::ifstream& is, std::string& filename);

	//! Get file for output

	//! \param type File contents
	//! \param os Returned input stream
	//! \param filename Returns name of opened file
	//! \returns true if successful
	bool get_file(uint8_t type, std::ofstream& os, std::string& filename) const;

	//! Get filename for data \p type
	std::string get_filename(uint8_t type) {
		const file_control_t ctrl = control_data_.at(type);
		if (ctrl.read_only) return default_source_directory_ + ctrl.filename;
		else return default_data_directory_ + ctrl.filename;
	}

	//! Get directory for data \p type
	std::string get_directory(file_data_t type) {
		switch (type) {
		case DATA_WORKING: return default_data_directory_;
		case DATA_SOURCE: return default_source_directory_;
		case DATA_CODEGEN: return default_code_directory_;
		case DATA_HTML: return default_html_directory_;
		default: return "";
		}
	}

	//! Release working copy to source
	bool copy_working_to_source(uint8_t type) const;

	//! Get file control for type
	file_control_t file_control(uint8_t type) const;

protected:

	//! Copy source to working
	bool copy_source_to_working(file_control_t ctrl) const;
	

	//! Default location for configuration files, and HTML files
	std::string default_data_directory_;

	//! Default location for reference source data
	std::string default_source_directory_;

	//! Default location for auto-generating compile fodder
	std::string default_code_directory_;

	//! Default directory for HTML files
	std::string default_html_directory_;

	//! Control data
	std::map<uint8_t, file_control_t> control_data_;


};

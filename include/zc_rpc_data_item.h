/*
	Copyright 2025-2026, Philip Rose, GM3ZZA
	
    This file is part of ZZALOG. Amateur Radio Logging Software.

    ZZALOG is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZALOG is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZALOG. 
	If not, see <https://www.gnu.org/licenses/>. 

*/
#ifndef __RPC_DATA_ITEM__
#define __RPC_DATA_ITEM__

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>


	//! XML-RPC data types
	enum rpc_data_t {
		XRT_EMPTY,              //!< Empty data set
		XRT_INT,                //!< I4
		XRT_BOOLEAN,            //!< Y/N
		XRT_DOUBLE,             //!< double
		XRT_STRING,             //!< String (ASCII)
		XRT_DEFAULT,            //!< String (ASCII) by default of no type
		XRT_BYTES,              //!< store as string after compacting the data
		XRT_DATETIME,           //!< Date/time - store as string
		XRT_ARRAY,              //!< Array of data types
		XRT_STRUCT              //!< Struct
	};

	//! This class describes an Remote Procedure Call (XML-RPC) data item 
	class zc_rpc_data_item
	{
	public:
		// Compound RPC data-types
		//! RPC Array of data items
		typedef std::vector<zc_rpc_data_item*> rpc_array;
		//! RPC Structure
		typedef std::map<std::string, zc_rpc_data_item*> rpc_struct;
		//! RPC List
		typedef std::list<zc_rpc_data_item*> rpc_list;

	public:
		//! Constructor.
		zc_rpc_data_item();
		//! Destructor.
		~zc_rpc_data_item();

		//! Returns the data type of this data item
		rpc_data_t type() const;
		// Get the data as specific type - returms true if is the correct type
		bool get(int32_t& i) const;        //!< Receives item as a 32-bit integer, returns false if not integer
		bool get(std::string& s) const;         //!< Receives item as a string, returns false if not a string
		bool get(double& d) const;         //!< Receives item as a double, returns false if not floating point
		bool get(rpc_array*& ap);    //!< Receives iten as an array, returns false if not an array.
		bool get(rpc_struct*& mp);   //!< Receives item as a structure, returns false if not a structure.
		// Returns the data as specific type
		int32_t get_int() const;           //!< Returns item as a 32-bit integer
		std::string get_string();         //!< Returns item as a string
		double get_double() const;         //!< Returns item as a double-precision value
		rpc_array* get_array();      //!< Returns item as an array
		rpc_struct* get_struct();    //!< Returns item as a structure

		//! Convert the item to a textual format
		std::string print_item();
		//! Set the data as int or bool
		void set(int32_t i, rpc_data_t Type);
		//! Set the data as a string or byte encoded string or a date/time
		void set(std::string s, rpc_data_t Type);
		void set(double d);          //!< Set the item as a double-precision value.
		void set(rpc_array* ap);     //!< Set the item as an array
		void set(rpc_struct* mp);    //!< Set the item as a structure.

	protected:
		//! The type of data
		rpc_data_t type_;
		//! Its representation as a 32-bit integer
		int32_t i_;
		//! Its representation as a string
		std::string s_;
		//! Its representation as a double-precision value
		double d_;
		//! Its representation as an array
		rpc_array* array_;
		//! Its representation as a structure
		rpc_struct* struct_;

	};
#endif 
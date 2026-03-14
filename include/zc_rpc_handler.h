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
#ifndef __RPC_HANDLER__
#define __RPC_HANDLER__

#include "zc_rpc_data_item.h"

#include "pugixml.hpp"

#include <string>
#include<istream>
#include<ostream>
#include <regex>
#include <map>

class zc_socket_server;
class zc_url_handler;

	//! This class acts as both a server or a client for the XML-RPC interfcae.
	class zc_rpc_handler
	{
	public:
		//! The only HTTP codes supported
		enum http_code {
			OK = 200,           //<! 200 OK
			BAD_REQUEST = 400   //<! 400 Bad Request.
		};
		//! Structure for an entry for specific method call.
		struct method_entry {
			std::string name;        //!< Method name
			std::string signature;   //!< Method signature - ie encoded parameter and response
			std::string help_text;   //!< Brief help text.
		};

		//! Constructor.
		
		//! \param host_address Network address of host
		//! \param port_name Integer port identifier
		//! \param resource_name A name indicating protocol (in this case "RPC2")
		zc_rpc_handler(std::string host_address, int port_name, std::string resource_name);
		//! Destructor
		~zc_rpc_handler();
		//! Generate XML for the request and decode XML for the response
		
		//! \param method_name Name of the method
		//! \param params Parameters for the method
		//! \param response Receives response from the remote procedure call.
		//! \return true if successful.
		bool do_request(std::string method_name, zc_rpc_data_item::rpc_list* params, zc_rpc_data_item* response);
		//! Receive the request from strean \p ss.
		static int rcv_request(void* instance, std::stringstream& ss);
		//! Run server
		void run_server();
		//! Close server
		void close_server();
		//! Returns true if the server is active.
		bool has_server();
		//! Add a method for the server to handle.
		
		//! \param v Pointer to indicate callback instance.
		//! \param method Method entry structire.
		//! \param callback Local method to handle request.
		void add_method(void* v, method_entry method, int(*callback)(void* v, zc_rpc_data_item::rpc_list& params, zc_rpc_data_item& response));

	protected:
		//! Method definition structure
		struct method_def {
			std::string signature;        //!< Method signature (coded form of parameters and response).
			std::string help_text;        //!< Help text
			void* v{ nullptr };                 //!< Instance pointer for callback
			int(*callback)(void* v, zc_rpc_data_item::rpc_list& params, zc_rpc_data_item& response) { nullptr };
			                         //!< Method call - callback(params, response) 
		};


		//! Generate the RPC request XML
		
		//! \param method_name Name of method.
		//! \param params Parameters for the method.
		//! \param request_xml Output stream to send the request out on.
		//! \return true if successful.
		bool generate_request(std::string method_name, zc_rpc_data_item::rpc_list* params, std::ostream& request_xml);
		//! Generate an RPC Response.
		
		//! \param fault true if responding with an error.
		//! \param response The data item as method return.
		//! \param response_xml Output stream to send the response out on.
		bool generate_response(bool fault, zc_rpc_data_item* response, std::ostream& response_xml);
		//! Decode the RPC Request XML
		
		//! \param request_xml Input stream provodong the request
		//! \param method_name Receives the method name
		//! \param params Receives the parameters for the request
		//! \return true if successful.
		bool decode_request(std::istream& request_xml, std::string& method_name, zc_rpc_data_item::rpc_list* params);
		//! Decode the RPC Response XML.
		
		//! \param response_xml Input stream providing the response
		//! \param response Receives the response returned by the remote method.
		//! \param fault Receives true if the response contains an error.
		//! \return true if successful.
		bool decode_response(std::istream& response_xml, zc_rpc_data_item* response, bool& fault);
		//! Write XML for an individual data \p item to XML \p node
		void write_item(zc_rpc_data_item& item, pugi::xml_node& node);
		//! Gate RPC data item from \p item from XML \p node
		void read_item(pugi::xml_node& node, zc_rpc_data_item& item);
		//! Decode the request on the input stream \p ss, perform the action and send response.
		int handle_request(std::stringstream& ss);
		//! Reserved method: List the available methods.
		static int list_methods(void* v, zc_rpc_data_item::rpc_list& params, zc_rpc_data_item& response);
		//! Reseeved method: Send method help message.
		static int method_help(void* v, zc_rpc_data_item::rpc_list& params, zc_rpc_data_item& response);

		//! Generate error \p response for \p code with \p message.
		void generate_error(int code, std::string message, zc_rpc_data_item& response);

		//! Remove header - returns true if successful and \p payload receives the request body.
		bool strip_header(std::stringstream& message, std::stringstream& payload) const;
		//! Add header with result \p code to the \p payload to generate  \p message.
		bool add_header(http_code code, std::stringstream& payload, std::stringstream& message);

		//! The resource name
		std::string resource_;
		//! The host name
		std::string host_name_;
		//! HTML server
		zc_socket_server* server_;
		//! Server port
		int server_port_;
		//! The method definitions
		std::map<std::string, method_def> method_list_;
		//! URL handler for client requests
		zc_url_handler* url_handler_;

	};
#endif

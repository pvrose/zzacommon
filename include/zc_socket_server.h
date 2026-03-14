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
#ifndef __zc_socket_server__
#define __zc_socket_server__

#include "zc_utils.h"

#include <string>
#include <cstdint>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#define SOCKET int
#define SOCKADDR_IN sockaddr_in
#endif



//! This class provides an interface to use a generic socket as a server.
	class zc_socket_server
	{

	public:

		//! Supported protocols
		enum protocol_t {
			HTTP,
			UDP
		};
		//! Constructor
		 
		//! \param protocol Create serverfor this protocol.
		//! \param address Network address for other end of socket.
		//! \param port_num Port number for other end of socket.
		zc_socket_server(protocol_t protocol, const std::string& address, int port_num);
		//! Destructor.
		~zc_socket_server();

		//! Close the socket
		
		//! \param external Waits for interface to quiesce.
		void close_server(bool external);
		//! Start running the server
		void run_server();
		//! Returns true if this server is listening
		bool has_server() const;
		//! Set callback to handle requests. 
		void callback(void* instance, int(*do_request)(void*, std::stringstream&));
		//! Send response
		int send_response(std::istream& response);

	protected:

		//! Response from client.
		enum client_status {
			OK = 0,           //!< Response OK
			NG = 1,           //!< No good.
			BLOCK = 2         //!< Response would block.
		};

		//! Receive a datagram from UDP client
		int rcv_packet();
		//! Accept the client - returns client status.
		client_status accept_client();
		//! Error handler - \p phase indicates the peocess that errored.
		void handle_error(const char* phase);
		//! Send request - set by call-back
		int (*do_request)(void* instance, std::stringstream& request);

		//! Open socket and create server
		int create_server();
		//! Print diagnostic data
		void dump(std::string line);
		//! Callback from server thread to handle packet.
		static void cb_th_packet(void* v);
		//! Start the server thread.
		static void thread_run(zc_socket_server* that);

		//! Server socket
		SOCKET server_;
		//! HTTP Client
		SOCKET client_;
		//! Current client address
		SOCKADDR_IN client_addr_{};
		//! Previous client address
		std::string prev_addr_ = "";
		//! Previous client port number
		int prev_port_ = 0;
		//! Host IP address e.g. 127.0.0.1
		std::string host_id_ = "";
		//! port number
		int port_num_ = 0;
		//! Server address
		std::string address_= "";
		//! protocol
		protocol_t protocol_ = HTTP;
		//! Socket is closing
		std::atomic<bool> closing_ = false;
		//! Socket has closed.
		std::atomic<bool> closed_ = true;
		//! Separate thread to handle socket transfers.
		std::thread* th_socket_ = nullptr;
		//! Packet queue
		std::queue<std::string> q_packet_;
		//! Lock to avoid pushing into the packet queue and pulling from it at the same time.
		std::mutex mu_packet_;
		//! Client request handler
		void* instance_ = nullptr;

	};

#endif


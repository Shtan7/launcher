#pragma once
#include "../server/file_handler.hpp"
#include "network_module.hpp"

namespace launcher
{
	class client
	{
	private:
		asio::io_context ioc;
		file_handler file_module;
		network network_module;
		bool connection_status = false;

	public:
		/// <summary>
		/// Create client object.
		/// </summary>
		/// <param name="raw_ip">Server ip address in string format.</param>
		/// <param name="port_num">Server port number.</param>
		/// <param name="folder_name">Working directory.</param>
		client(std::string raw_ip, uint16_t port_num, std::string folder_name);

		/// <summary>
		/// Connect to the server.
		/// </summary>
		void connect();

		/// <summary>
		/// Disconnect from the server.
		/// </summary>
		void disconnect();

		/// <summary>
		/// Send sign up request through network module.
		/// </summary>
		void sign_up();

		/// <summary>
		/// Send sign in request through network module.
		/// </summary>
		void sign_in();

		/// <summary>
		/// Request files update through network module.
		/// </summary>
		void update();
	};
}
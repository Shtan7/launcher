#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include "../server/common.hpp"

namespace asio = boost::asio;

namespace launcher
{

	class network
	{
	private:
		asio::io_context& ioc;
		asio::ssl::context ssl_context;
		std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> ssl_stream;
		asio::ip::tcp::endpoint ep;
		std::stringstream request_buf;
		asio::streambuf response_buf;
		std::unique_ptr<boost::archive::binary_oarchive> request_stream; // serialized data
		std::unique_ptr<boost::archive::binary_iarchive> input_stream;

	private:
		/// <summary>
		/// Clear buffs after network operation. It is needed
		/// for buffers reusing.
		/// </summary>
		void get_ready_buffs();

		/// <summary>
		/// Perform sequentional send and get operations.
		/// </summary>
		/// <param name="request">Request struct for the server.</param>
		/// <returns>Response from the server.</returns>
		messages::response send_and_get(messages::request request);

		/// <summary>
		/// Perform response getting.
		/// </summary>
		/// <returns>Response struct from the server.</returns>
		messages::response get();

		/// <summary>
		/// Perform request sending.
		/// </summary>
		/// <param name="request">Request struct itself.</param>
		void send(messages::request& request);

	public:
		/// <summary>
		/// Create network module object.
		/// </summary>
		/// <param name="raw_ip">Server ip address in string format.</param>
		/// <param name="port_num">Server port number.</param>
		/// <param name="ioc_">Executor reference.</param>
		network(std::string raw_ip, uint16_t port_num, asio::io_context& ioc_);

		/// <summary>
		/// Sign in request.
		/// </summary>
		/// <param name="login">User's login.</param>
		/// <param name="password">User's password.</param>
		void sign_in(std::string& login, std::string& password);

		/// <summary>
		/// Sign up request.
		/// </summary>
		/// <param name="login">User's login.</param>
		/// <param name="password">User's password.</param>
		void sign_up(std::string& login, std::string& password);

		/// <summary>
		/// Perform connect to the server.
		/// </summary>
		void connect();

		/// <summary>
		/// Perform disconnect from the server.
		/// </summary>
		void disconnect();

		/// <summary>
		/// Perform files update.
		/// Since SSL protocol is too slow for transferring files a regular tcp socket is used for this purpose.
		/// </summary>
		/// <param name="hash_data">Vector with file names and its hashes.</param>
		/// <param name="general_hash">Hash of all files to quickly check the relevance of files.</param>
		/// <param name="folder_name">Working directory.</param>
		void update(std::vector<std::string>& hash_data, std::string general_hash, std::string folder_name);
	};

}
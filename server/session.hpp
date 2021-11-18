#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include "database.hpp"
#include "file_handler.hpp"
#include "common.hpp"
#include <boost/uuid/random_generator.hpp>

namespace asio = boost::asio;

namespace launcher
{
	class session
	{
	private:
		asio::ssl::stream<asio::ip::tcp::socket> ssl_stream;
		std::shared_ptr<db::database> db_ptr;
		std::shared_ptr<file_handler> fh_ptr;
		bool sign_in_status = false;

	public:
		/// <summary>
		/// Construct session object. One session per client.
		/// </summary>
		/// <param name="ioc">Executor reference.</param>
		/// <param name="ssl_context">Required data for the ssl protocol.</param>
		/// <param name="db_ptr_">Pointer to database module.</param>
		/// <param name="fh_ptr_">Pointer to file handler module.</param>
		session(asio::io_context& ioc, asio::ssl::context& ssl_context, std::shared_ptr<db::database> db_ptr_, std::shared_ptr<file_handler> fh_ptr_);

		/// <summary>
		/// Main session event loop. Handles client's requests until disconnection.
		/// </summary>
		/// <param name="this_ptr">Pointer to object of session.</param>
		static asio::awaitable<void> handle_client(std::shared_ptr<session> this_ptr);

		/// <summary>
		/// Handle client authorization request.
		/// Function executes the corresponding database query.
		/// </summary>
		/// <param name="input_data">Client's login and password.</param>
		/// <param name="response_stream">Response to client.</param>
		asio::awaitable<void> handle_authorization(messages::request& input_data, boost::archive::binary_oarchive& response_stream);

		/// <summary>
		/// Handle client sign up request. Function also
		/// performs regex check of login and password.
		/// Function executes the corresponding database query.
		/// </summary>
		/// <param name="input_data">Login and password from client.</param>
		/// <param name="response_stream">Response to client.</param>
		asio::awaitable<void> handle_registration(messages::request& input_data, boost::archive::binary_oarchive& response_stream);

		/// <summary>
		/// Handle client general hash check request.
		/// Functions performs comparison with client hash and server's one.
		/// </summary>
		/// <param name="input_data">Contains general client hash.</param>
		/// <param name="response_stream">Response to client.</param>
		void handle_general_hash_check(messages::request& input_data, boost::archive::binary_oarchive& response_stream);

		/// <summary>
		/// Handle client request update.
		/// Since ssl protocol is too slow for transferring files, 
		/// a regular tcp socket is used for this purpose.
		/// </summary>
		/// <param name="input_data">Hash of files from client.</param>
		/// <param name="response_stream">Final response to client.</param>
		asio::awaitable<void> handle_update(messages::request& input_data, boost::archive::binary_oarchive& response_stream);

		/// <summary>
		/// Get basic_socket object from ssl_stream.
		/// </summary>
		/// <returns>Lowest socket object.</returns>
		auto& return_lowest_layer()
		{
			return ssl_stream.lowest_layer();
		}
	};
}
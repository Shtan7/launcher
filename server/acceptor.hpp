#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "database.hpp"
#include "file_handler.hpp"
#include <string>

namespace asio = boost::asio;

namespace launcher
{
	class acceptor
	{
	private:
		asio::io_context& ioc;
		asio::ip::tcp::acceptor sock_acceptor;
		asio::ssl::context ssl_context;
		std::pair<std::shared_ptr<db::database>, std::shared_ptr<file_handler>>& db_fh;

	private:
		/// <summary>
		/// Stub-like function. Just ignore it.
		/// </summary>
		/// <param name="max_length"></param>
		/// <param name="purpose"></param>
		/// <returns></returns>
		std::string password_callback(uint64_t max_length, asio::ssl::context::password_purpose purpose) const;

	public:
		/// <summary>
		/// Constructs acceptor object that supports ssl protocol and put
		/// it into a listening state.
		/// </summary>
		/// <param name="ioc_">Reference to executor.</param>
		/// <param name="port_num">Port number for the socket.</param>
		/// <param name="db_fh_">Pointer to file handler module object.</param>
		acceptor(asio::io_context& ioc_, uint16_t port_num, std::pair<std::shared_ptr<db::database>, std::shared_ptr<file_handler>>& db_fh_);

		/// <summary>
		/// Start accepting from all available local addresses.
		/// </summary>
		void accept();

		/// <summary>
		/// Acceptor socket getter.
		/// </summary>
		/// <returns>Tcp acceptor socket.</returns>
		asio::ip::tcp::acceptor& get_socket();
	};
}
#include "acceptor.hpp"
#include "session.hpp"

namespace launcher
{
	acceptor::acceptor(asio::io_context& ioc_, uint16_t port_num, std::pair<std::shared_ptr<db::database>, std::shared_ptr<file_handler>>& db_fh_) : ioc{ ioc_ },
		sock_acceptor{ ioc_, asio::ip::tcp::endpoint{ asio::ip::address_v4::any(), port_num } },
		ssl_context{ asio::ssl::context::sslv23_server }, db_fh{ db_fh_ }
	{
		ssl_context.set_options
		(boost::asio::ssl::context::default_workarounds
			| boost::asio::ssl::context::no_sslv2
			| boost::asio::ssl::context::single_dh_use);

		ssl_context.set_password_callback([this](uint64_t max_length, asio::ssl::context::password_purpose purpose) -> std::string
			{
				return password_callback(max_length, purpose);
			});

		/*
		* You must generate your own keys for the SSL protocol.
		* Check the instruction on the main page of the repository.
		*/

		auto source_directory = common::find_source_directory();

		ssl_context.use_certificate_chain_file(source_directory + "/user.crt");
		ssl_context.use_private_key_file(source_directory + "/user.key", boost::asio::ssl::context::pem);
		ssl_context.use_tmp_dh_file(source_directory + "/dh2048.pem");

		sock_acceptor.listen();
	}

	void acceptor::accept()
	{
		auto ssl_session = std::make_shared<session>(ioc, ssl_context, db_fh.first, db_fh.second);
		sock_acceptor.accept(ssl_session->return_lowest_layer());

		asio::co_spawn(ioc, session::handle_client(std::move(ssl_session)), asio::detached);
	}

	std::string acceptor::password_callback(uint64_t max_length, asio::ssl::context::password_purpose purpose) const
	{
		return "";
	}

	asio::ip::tcp::acceptor& acceptor::get_socket()
	{
		return sock_acceptor;
	}
}
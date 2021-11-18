#include "network_module.hpp"
#include <iostream>
#include <openssl/ssl.h>
#include <fstream>

namespace launcher
{
	network::network(std::string raw_ip, uint16_t port_num, asio::io_context& ioc_) : ep{ asio::ip::address::from_string(raw_ip), port_num }, ioc{ ioc_ },
		ssl_context{ asio::ssl::context::sslv23_client }
	{
		auto source_directory = common::find_source_directory();

		// Root cert for the server verification.
		ssl_context.load_verify_file(source_directory + "/rootca.crt");
	}

	void network::connect()
	{
		// It's better to create new ssl_stream on every new connection.
		ssl_stream = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(ioc, ssl_context);
		ssl_stream->set_verify_mode(asio::ssl::verify_peer);
		ssl_stream->set_verify_callback([](bool preverified, asio::ssl::verify_context&) -> bool
			{
				return preverified;
			});

		ssl_stream->lowest_layer().connect(ep);
		ssl_stream->handshake(asio::ssl::stream_base::client);

		// I don't know how to reuse boost archives after disconnection because new server instances cannot read data from client's old ones :/.
		// So I just decided to recreate them on every connection.
		request_stream = std::make_unique<boost::archive::binary_oarchive>(request_buf, boost::archive::no_codecvt | boost::archive::no_header);
		input_stream = std::make_unique<boost::archive::binary_iarchive>(response_buf, boost::archive::no_codecvt | boost::archive::no_header);

		std::cout << "Connection status: \n"
			<< send_and_get({ messages::request_ids::ping });
	}

	void network::sign_in(std::string& login, std::string& password)
	{
		std::cout << "Sign in status: \n"
			<< send_and_get({ messages::request_ids::authorization, { login, password } });
	}

	void network::sign_up(std::string& login, std::string& password)
	{
		std::cout << "Sign up status: \n"
			<< send_and_get({ messages::request_ids::registration, { login, password } });
	}

	void network::get_ready_buffs()
	{
		request_buf.clear();
		request_buf.str("");

		if (response_buf.size())
		{
			response_buf.consume(response_buf.size());
		}
	}

	void network::send(messages::request& request)
	{
		*request_stream << request;

		uint32_t message_size = common::get_stream_size(request_buf);
		auto message_size_buf = asio::buffer(&message_size, 4);

		// Write buff size and then buff itself.
		asio::write(*ssl_stream, message_size_buf);
		asio::write(*ssl_stream, asio::buffer(request_buf.str()));
	}

	messages::response network::get()
	{
		uint32_t message_size;
		auto message_size_buf = asio::buffer(&message_size, 4);

		// Read buff size and then read buff itself.
		asio::read(*ssl_stream, message_size_buf);
		asio::read(*ssl_stream, response_buf.prepare(message_size));

		response_buf.commit(message_size);

		messages::response response;
		*input_stream >> response;

		return response;
	}

	messages::response network::send_and_get(messages::request request)
	{
		send(request);
		auto response = get();
		get_ready_buffs();

		return response;
	}

	void network::disconnect()
	{
		boost::system::error_code e;

		ssl_stream->lowest_layer().cancel();
		ssl_stream->shutdown(e);
		ssl_stream->lowest_layer().close();
		ssl_stream.reset();

		request_stream.reset();
		input_stream.reset();

		std::cout << "Disconnected\n\n";
	}

	void network::update(std::vector<std::string>& hash_data, std::string general_hash, std::string folder_name)
	{
		auto response = send_and_get({ messages::request_ids::check_general_hash, { std::move(general_hash) } });

		if (response.status == messages::status_codes::success)
		{
			std::cout << "Already up-to-date.\n\n";
			return;
		}

		auto request = messages::request{ messages::request_ids::get_update, std::move(hash_data) };
		send(request);

		asio::streambuf input_buf;
		uint32_t chunk_size;

		bool continuator;

		while(true)
		{
			// Firstly we read file name and create filestream.
			asio::read_until(*ssl_stream, input_buf, '\0');
			std::string_view file_name = reinterpret_cast<const char*>(input_buf.data().data());

			// If client receives 'stop' then all files are up-to-date.
			if (!file_name.compare("stop") ? true : false)
			{
				break;
			}

			std::ofstream updated_file{ (folder_name + "\\" + file_name.data()), std::ios::trunc | std::ios::binary};
			std::vector<char> file_buff;
			file_buff.resize(common::consts::MiB);

			// Read data from the server in cycle and write it to file.
			while (true)
			{
				// Read chunk size.
				asio::read(*ssl_stream, asio::buffer(&chunk_size, sizeof(chunk_size)));

				if (!chunk_size)
				{
					updated_file.close();
					break;
				}

				if (chunk_size < file_buff.size())
				{
					file_buff.resize(chunk_size);
				}

				asio::write(*ssl_stream, asio::buffer(&continuator, sizeof(continuator)));
				asio::read(ssl_stream->next_layer(), asio::buffer(file_buff));
				asio::write(*ssl_stream, asio::buffer(&continuator, sizeof(continuator)));
				// These two asio::write lines are the necessary synchronization before and after non ssl transmission.

				updated_file.write(file_buff.data(), chunk_size);
			}

			input_buf.consume(input_buf.size());
		}

		response = get();
		get_ready_buffs();
		
		std::cout << "Update status:" << '\n' << response;
	}
}
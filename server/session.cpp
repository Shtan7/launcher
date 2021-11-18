#include "session.hpp"
#include <iostream>
#include <ranges>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace launcher
{
	session::session(asio::io_context& ioc, asio::ssl::context& ssl_context, std::shared_ptr<db::database> db_ptr_, std::shared_ptr<file_handler> fh_ptr_) :
		ssl_stream{ ioc, ssl_context }, db_ptr{ std::move(db_ptr_) }, fh_ptr{ std::move(fh_ptr_) }
	{}

	asio::awaitable<void> session::handle_client(std::shared_ptr<session> this_ptr)
	{
		try
		{
			co_await this_ptr->ssl_stream.async_handshake(asio::ssl::stream_base::server, asio::use_awaitable);

			asio::streambuf request_buf;
			std::stringstream response_buf;
			messages::request request;

			boost::archive::binary_oarchive response_stream(response_buf, boost::archive::no_codecvt | boost::archive::no_header); // Serialized data.
			boost::archive::binary_iarchive input_data(request_buf, boost::archive::no_codecvt | boost::archive::no_header);

			uint32_t message_size;
			auto message_size_buf = asio::buffer(&message_size, 4);

			// Client processing loop.
			while (true)
			{
				co_await asio::async_read(this_ptr->ssl_stream, message_size_buf, asio::use_awaitable);
				co_await asio::async_read(this_ptr->ssl_stream, request_buf.prepare(message_size), asio::use_awaitable);

				request_buf.commit(message_size);
				input_data >> request;

				switch (request.request_id)
				{
					case messages::request_ids::authorization:
					{
						co_await this_ptr->handle_authorization(request, response_stream);
						break;
					}
					case messages::request_ids::registration:
					{
						co_await this_ptr->handle_registration(request, response_stream);
						break;
					}
					case messages::request_ids::ping:
					{
						response_stream << messages::response{ messages::status_codes::success, "Server response" };
						break;
					}
					case messages::request_ids::check_general_hash:
					{
						this_ptr->handle_general_hash_check(request, response_stream);
						break;
					}
					case messages::request_ids::get_update:
					{
						co_await this_ptr->handle_update(request, response_stream);
						break;
					}
					default:
					{
						response_stream << messages::response{ messages::status_codes::incorrect_input, "Failed to process request: invalid meessage id" };
						break;
					}
				}

				// Send response to the client.
				message_size = common::get_stream_size(response_buf);

				co_await asio::async_write(this_ptr->ssl_stream, message_size_buf, asio::use_awaitable);
				co_await asio::async_write(this_ptr->ssl_stream, asio::buffer(response_buf.str()), asio::use_awaitable);

				// Get ready to process another message.
				response_buf.clear();
				response_buf.str({});

				if (request_buf.size())
				{
					request_buf.consume(request_buf.size());
				}
			}
		}
		catch (std::exception& e)
		{
			std::cout << "Client communication fail: " << e.what() << '\n';
		}
	}

	asio::awaitable<void> session::handle_authorization(messages::request& input_data, boost::archive::binary_oarchive& response_stream)
	{
		try
		{
			if (sign_in_status)
			{
				response_stream << messages::response{ messages::status_codes::already_authorized };
				co_return;
			}

			auto [login, pass_hash] = input_data.get_login_pass();
			pass_hash = common::hash_string(pass_hash);

			bool is_correct = co_await db_ptr->check_password(login, pass_hash);

			if (is_correct)
			{
				sign_in_status = true;
				response_stream << messages::response{ messages::status_codes::success };
			}
			else
			{
				response_stream << messages::response{ messages::status_codes::incorrect_input, "Wrong password or login" };
			}
		}
		catch (std::exception& e)
		{
			response_stream << messages::response{ messages::status_codes::fail, e.what() };
		}
	}

	asio::awaitable<void> session::handle_registration(messages::request& input_data, boost::archive::binary_oarchive& response_stream)
	{
		try
		{
			auto [login, password] = input_data.get_login_pass();

			if (login.size() <= 25 && password.size() <= 255 && std::regex_match(login, common::regex::log_pass_regex)
				&& std::regex_match(password, common::regex::log_pass_regex))
			{
				password = common::hash_string(password);

				co_await db_ptr->add_login_pass(login, password);

				response_stream << messages::response{ messages::status_codes::success };
			}
			else
			{
				response_stream << messages::response{ messages::status_codes::incorrect_input, "Login or password was in incorrect format" };
			}
		}
		catch (std::exception& e)
		{
			response_stream << messages::response{ messages::status_codes::fail, e.what() };
		}
	}

	void session::handle_general_hash_check(messages::request& input_data, boost::archive::binary_oarchive& response_stream)
	{
		bool hash_status = fh_ptr->compare_general_hash(input_data.get_general_hash());
		messages::response response{};

		hash_status ? response.status = messages::status_codes::success : response.status = messages::status_codes::hash_miss;
		response_stream << response;
	}

	asio::awaitable<void> session::handle_update(messages::request& input_data, boost::archive::binary_oarchive& response_stream)
	{
		messages::response response = messages::response{ messages::status_codes::success };

		while (true)
		{
			if (!sign_in_status)
			{
				response = messages::response{ messages::status_codes::not_authorized, "you need to sign in before downloading the update" };
				break;
			}

			// std::map with all file data from file_handler module.
			auto& map_with_files = fh_ptr->get_file_list();

			std::vector<std::pair<std::string, std::string>> file_list_client = input_data.get_files_hash();
			auto file_list_server = std::views::transform(map_with_files, [](auto&& elem) { return std::pair<std::string, std::string>{ elem.first, elem.second.second }; });
			std::vector<std::pair<std::string, std::string>> diff;

			// Get differences between server and client files.
			std::ranges::set_difference(file_list_server, file_list_client, std::back_inserter(diff));

			bool stopper;

			for (auto&& file : diff)
			{
				std::string absolute_path = map_with_files.at(file.first).first;
				uint64_t file_size = std::filesystem::file_size(absolute_path);
				uint32_t chunk_size;

				// Send file in chunks of 1 MiB.
				std::vector<char> file_buff;
				file_buff.resize(common::consts::MiB);

				std::ifstream ifstream(absolute_path, std::ios::binary);
				if (!ifstream)
				{
					throw std::runtime_error{ ("Fatal server error: failed to open file: " + std::filesystem::path{ absolute_path }.filename().string()).c_str() };
				}

				// Write file name.
				co_await asio::async_write(ssl_stream, asio::buffer(file.first.data(), file.first.size() + 1), asio::use_awaitable);

				do
				{
					if (file_size < file_buff.size())
					{
						file_buff.resize(file_size);
					}

					chunk_size = file_buff.size();

					ifstream.read(file_buff.data(), chunk_size);

					// Write current size of chunk and chunk itself.
					co_await asio::async_write(ssl_stream, asio::buffer(&chunk_size, sizeof(chunk_size)), asio::use_awaitable);
					co_await asio::async_read(ssl_stream, asio::buffer(&stopper, sizeof(stopper)), asio::use_awaitable);
					co_await asio::async_write(ssl_stream.next_layer(), asio::buffer(file_buff), asio::use_awaitable);
					co_await asio::async_read(ssl_stream, asio::buffer(&stopper, sizeof(stopper)), asio::use_awaitable);
					// These two asio::async_read lines are the necessary synchronization before and after non ssl transmission.

					file_size -= chunk_size;

				} while (file_size);

				// If current file is over then send 0 and proceed to next file.
				chunk_size = 0;
				co_await asio::async_write(ssl_stream, asio::buffer(&chunk_size, sizeof(chunk_size)), asio::use_awaitable);
			}

			break;
		}

		// Write 'stop' command at the end.
		co_await asio::async_write(ssl_stream, asio::buffer("stop"), asio::use_awaitable);

		response_stream << response;
	}
}
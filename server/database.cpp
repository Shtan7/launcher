#include "database.hpp"
#include <ranges>
#include <algorithm>
#include <async_exec.hpp>
#include "common.hpp"
#include <fstream>

namespace launcher
{
	db::postgre_db::postgre_db(std::string connection_str, std::string table_name_, std::string login_column_name_, std::string password_column_name_, boost::asio::io_context& ioc_)
		: ioc{ ioc_ }, table_name{ std::move(table_name_) }, conn{ connection_str.c_str() },
		login_column_name{ std::move(login_column_name_) }, password_column_name{ std::move(password_column_name_) }
	{}

	asio::awaitable<void> db::postgre_db::add_login_pass(std::string& login, std::string& pass_hash)
	{
		std::string base64 = common::get_base64_from_sha512(pass_hash);

		auto request = std::string{ "INSERT INTO " + table_name + "(" + login_column_name + ", " + password_column_name + ") VALUES('" + login + "', '" + base64 + "');" };
		auto result = co_await postgrespp::async_exec(conn, request.c_str(), asio::use_awaitable);

		if (!result.ok())
		{
			auto exception_str = std::string{ "Failed to add user. Error: " + std::string{ result.error_message() } };
			throw std::runtime_error(exception_str.c_str());
		}
	}

	asio::awaitable<bool> db::postgre_db::check_password(std::string& login, std::string& pass_hash)
	{
		std::string base64 = common::get_base64_from_sha512(pass_hash);

		auto request = std::string{ "SELECT " + password_column_name + " FROM " + table_name + " WHERE " + login_column_name + "='" + login + "';" };
		auto result = co_await postgrespp::async_exec(conn, request.c_str(), asio::use_awaitable);

		if (!result.ok())
		{
			auto exception_str = std::string{ "Failed to get password. Error: " + std::string{ result.error_message() } };
			throw std::runtime_error(exception_str.c_str());
		}

		co_return base64.compare(result[0][0].as<std::string>()) ? false : true;
	}

	asio::awaitable<void> db::postgre_db::update_pass(std::string& login, std::string& new_pass_hash)
	{
		std::string base64 = common::get_base64_from_sha512(new_pass_hash);

		auto request = std::string{ "UPDATE " + table_name + " SET " + password_column_name + "='" + base64 + "' WHERE " + login_column_name + "='" + login + "';" };
		auto result = co_await postgrespp::async_exec(conn, request.c_str(), asio::use_awaitable);

		if (!result.ok())
		{
			auto exception_str = std::string{ "Failed to update password. Error: " + std::string{ result.error_message() } };
			throw std::runtime_error(exception_str.c_str());
		}
	}

	std::tuple<std::string, std::string, std::string, std::string> db::postgre_db::get_database_conn_data()
	{
		auto source_directory = common::find_source_directory();

		std::ifstream input_data(source_directory + "/connection_data.txt");
		if (!input_data)
		{
			throw std::runtime_error{ "Failed to get database connection data." };
		}

		std::string temp;

		for (int j = 0; j < 4; j++)
		{
			// skip 4 lines with comments
			std::getline(input_data, temp);
		}

		std::string connection_str, table_name, login_column_name, password_column_name;

		std::getline(input_data, connection_str);
		std::getline(input_data, table_name);
		std::getline(input_data, login_column_name);
		std::getline(input_data, password_column_name);

		return { connection_str, table_name, login_column_name, password_column_name };
	}
}
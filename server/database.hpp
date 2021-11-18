#pragma once
#include <string>
#include <boost/asio.hpp>
#include <connection.hpp>
#include <tuple>
#include <string>

namespace asio = boost::asio;

namespace launcher
{
	namespace db
	{
		/*
		* I am using postgre database. You can inherit this
		* abstract class and implement interface with your own
		* database if you want.
		*/
		class base_db_class
		{
		public:
			virtual asio::awaitable<void> add_login_pass(std::string& login, std::string& pass_hash) = 0;
			virtual asio::awaitable<bool> check_password(std::string& login, std::string& pass_hash) = 0;
			virtual asio::awaitable<void> update_pass(std::string& login, std::string& new_pass_hash) = 0;
			virtual ~base_db_class() = default;
		};

		using database = base_db_class;

		class postgre_db final : public database
		{
		private:
			std::string connection_str;
			std::string table_name;
			std::string login_column_name;
			std::string password_column_name;
			boost::asio::io_context& ioc;
			postgrespp::connection conn;

		public:
			/// <summary>
			/// Create database module object.
			/// </summary>
			/// <param name="connection_str">Connection string with network params.</param>
			/// <param name="table_name_">Table name of users data.</param>
			/// <param name="login_column_name_">Name of the column containing the username.</param>
			/// <param name="password_column_name_">Name of the column containing the user's password.</param>
			/// <param name="ioc_">Reference to executor.</param>
			postgre_db(std::string connection_str, std::string table_name_, std::string login_column_name_, std::string password_column_name_, boost::asio::io_context& ioc_);

			/// <summary>
			/// Add new record in table.
			/// </summary>
			/// <param name="login">User name.</param>
			/// <param name="pass_hash">Raw password hash.</param>
			/// <returns></returns>
			asio::awaitable<void> add_login_pass(std::string& login, std::string& pass_hash) override;

			/// <summary>
			/// Compare given password with password field of specific record
			/// in database.
			/// </summary>
			/// <param name="login">Login of user.</param>
			/// <param name="pass_hash">The provided raw password hash.</param>
			/// <returns>Result of comparison in bool variable.</returns>
			asio::awaitable<bool> check_password(std::string& login, std::string& pass_hash) override;

			/// <summary>
			/// Update password of record in table.
			/// The feature is currently not used. But you can implement method with password recovery.
			/// </summary>
			/// <param name="login">Login of user.</param>
			/// <param name="new_pass_hash">Raw hash of new password.</param>
			/// <returns></returns>
			asio::awaitable<void> update_pass(std::string& login, std::string& new_pass_hash) override;

			/// <summary>
			/// Read database connection data from the .txt file.
			/// </summary>
			/// <returns>Tuple with parameters.</returns>
			static std::tuple<std::string, std::string, std::string, std::string> get_database_conn_data();
			virtual ~postgre_db() = default;
		};
	}
}
#include "server.hpp"
#include "acceptor.hpp"
#include <iostream>

namespace launcher
{
	server::server(uint32_t number_of_workers_) : ioc{ number_of_workers_ }, stop{ false }, number_of_workers{ number_of_workers_ }
	{
		// Prevent io_context from stopping when there are no tasks in queue.
		work_object = std::make_unique<asio::io_context::work>(ioc);

		auto [conn_str, table_name, login_column_name, password_column_name] = db::postgre_db::get_database_conn_data();

		// Pair with database module and file handler.
		database_file_handler.first = std::shared_ptr<db::database>{ new db::postgre_db{ conn_str, table_name, login_column_name, password_column_name, ioc } };
		database_file_handler.second = std::make_shared<file_handler>("data");

		/*
		* Server uses model with a single io_context object. Client handling
		* occurs simultaneously on different cores. If you want to prevent the 
		* processing of a single client from being split between cores then
		* you need to create an io_context for each logical processor.
		*/
		for (uint32_t j = 0; j < number_of_workers; j++)
		{
			std::thread{ [this]() { ioc.run(); } }.detach();
		}
	}

	void server::stop_server()
	{
		stop.store(true);

		if (acc != nullptr)
		{
			acc->get_socket().cancel();
		}

		ioc.stop();
	}

	void server::run(uint16_t port_num)
	{
		try
		{
			acc = std::make_unique<acceptor>(ioc, port_num, database_file_handler);
		}
		catch (std::exception& e)
		{
			std::cout << "Failed to create acceptor socket: " << e.what() << '\n';
			stop_server();
			return;
		}

		while (!stop.load())
		{
			acc->accept();
		}
	}
}
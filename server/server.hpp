#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include "database.hpp"
#include "acceptor.hpp"
#include "file_handler.hpp"

namespace asio = boost::asio;

namespace launcher
{
	class server
	{
	private:
		std::atomic<bool> stop;
		asio::io_context ioc;
		std::vector<std::shared_ptr<std::thread>> threads;
		const uint32_t number_of_workers;
		std::unique_ptr<asio::io_context::work> work_object;
		std::shared_ptr<db::database> database;
		std::pair<std::shared_ptr<db::database>, std::shared_ptr<file_handler>> database_file_handler;
		std::unique_ptr<acceptor> acc;

	public:
		/// <summary>
		/// Construct server with specific number of threads.
		/// Default value is std::thread::hardware_concurrency.
		/// </summary>
		/// <param name="number_of_workers_">Number of threads for the server object.</param>
		server(uint32_t number_of_workers_ = std::thread::hardware_concurrency());

		/// <summary>
		/// Stop threads, executor and acceptor socket. You should
		/// call it to achive correct exit from the server.
		/// The feature is currently not used. But you can stop
		/// the server anytime you want.
		/// </summary>
		void stop_server();

		/// <summary>
		/// Run server on specific port.
		/// </summary>
		/// <param name="port_num">Port number for the acceptor socket.</param>
		void run(uint16_t port_num);
	};
}
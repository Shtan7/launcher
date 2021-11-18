#include "client.hpp"
#include <iostream>

namespace launcher
{
	client::client(std::string raw_ip, uint16_t port_num, std::string folder_name) : network_module{ raw_ip, port_num, ioc }, file_module{ folder_name }
	{}

	void client::connect()
	{
		if (connection_status)
		{
			std::cout << "Already connected." << "\n\n";
			return;
		}

		network_module.connect();
		connection_status = true;
	}

	void client::sign_up()
	{
		if (!connection_status)
		{
			std::cout << "Not connected to the server." << "\n\n";
			return;
		}

		std::string login;
		std::string password;

		std::cout << "Type your login and password. \n"
			<< "You can use the latin alphabet and the symbols \"_-*()/\\\"). \n"
			<< "The allowed login length is 25 characters, the password length is 255 characters:\n";
		std::cin >> login >> password;

		if (login.size() <= 25 && password.size() <= 255 && std::regex_match(login, common::regex::log_pass_regex)
			&& std::regex_match(password, common::regex::log_pass_regex))
		{
			network_module.sign_up(login, password);
		}
		else
		{
			std::cout << "Login or password was in incorrect format." << "\n\n";
		}
	}

	void client::sign_in()
	{
		if (!connection_status)
		{
			std::cout << "Not connected to the server." << "\n\n";
			return;
		}

		std::string login;
		std::string password;

		std::cout << "Type your login and password: " << '\n';
		std::cin >> login >> password;

		network_module.sign_in(login, password);
	}

	void client::disconnect()
	{
		if (!connection_status)
		{
			std::cout << "Already disconnected." << "\n\n";
			return;
		}

		network_module.disconnect();
		connection_status = false;
	}

	void client::update()
	{
		if (!connection_status)
		{
			std::cout << "Not connected to the server." << "\n\n";
			return;
		}

		std::vector<std::string> hash_data;

		for (const auto& file : file_module.get_file_list())
		{
			hash_data.push_back(file.first); // push file name
			hash_data.push_back(file.second.second); // push file hash
		}

		network_module.update(hash_data, file_module.get_general_hash(), file_module.get_folder_name());
		file_module.perform_hashing();
	}
}
#include "server.hpp"
#include <iostream>
#include "common.hpp"

int main()
{
	try
	{
		uint32_t number_of_threads = 2;
		launcher::server server_obj{ number_of_threads };

		server_obj.run(3333);
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}

	return 0;
}
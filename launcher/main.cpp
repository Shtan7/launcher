#include "client.hpp"
#include <iostream>

int main()
{
  try
  {
    auto client_obj = launcher::client{ "127.0.0.1", 3333, "data" };

    std::string choice;
    std::cout << "Welcome to the launcher menu.\n" << "Type -h to get all options.\n";

    while (true)
    {
      std::cout << "Ready for input:\n";
      std::cin >> choice;
      if (choice.size() != 2)
      {
        std::cout << "Incorrect input. Type -h to get help.\n";
      }

      switch (choice[1])
      {
        case 'h':
        {
          std::cout << "List of available commands:\n"
            << "-c \t try to connect to the server\n"
            << "-r \t create account\n"
            << "-l \t log in to the account\n"
            << "-d \t disconnect from the server\n"
            << "-u \t update your files\n"
            << "-q \t exit program\n";
          break;
        }
        case 'q':
        {
          return 0;
        }
        case 'c':
        {
          client_obj.connect();
          break;
        }
        case 'd':
        {
          client_obj.disconnect();
          break;
        }
        case 'l':
        {
          client_obj.sign_in();
          break;
        }
        case 'r':
        {
          client_obj.sign_up();
          break;
        }
        case 'u':
        {
          client_obj.update();
          break;
        }
        default:
        {
          std::cout << "Incorrect input. Type -h to get help.\n";
          break;
        }
      }
    }
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << '\n';
  }
  
  return 0;
}

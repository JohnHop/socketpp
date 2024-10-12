#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <iostream>
#include <format>
#include <thread>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "socketpp/serversocket.hpp"


int main()
{
  std::chrono::seconds timeout{0};
  
  ServerSocket server("1234");

  auto closer = std::jthread( [&server](std::stop_token stop_token) {
    std::this_thread::sleep_for( std::chrono::seconds{5} );
    server.stop_accept();
  });

  std::cout << std::format("Listening... ") << std::flush;

  auto connection = server.accept(timeout);
  if(!connection) {
    std::cout << "Accept timeout occur";
    std::exit(EXIT_FAILURE);
  }

  std::cout << std::format("connected!\n");

  size_t const size = 50;
  char buffer[size] = "Hello";
  while(true) {
    strcpy(buffer, "Hello");
    std::cout << std::format("Sending {}...", buffer) << std::endl;
    auto length = connection.write( buffer );
    if(length == -1) {
      throw std::system_error( errno, std::system_category() );
    }
    std::this_thread::sleep_for( std::chrono::seconds(1) );
  }

  std::cout << std::format("{}\n", buffer);

  return EXIT_SUCCESS;
}
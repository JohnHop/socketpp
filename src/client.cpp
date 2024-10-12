#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <iostream>
#include <format>
#include <thread>

#include <netdb.h>
#include <sys/socket.h>

#include <unistd.h>

#include "socketpp/socket.hpp"


int main()
{
  Socket sock("localhost", "1234");
  std::cout << "Connected!" << std::endl;

  const size_t size = 100;
  char buffer[size];
  size_t ret = 1;
  while( ret ) {
    std::cout << "Receving..." << std::flush;
    ret = sock.read(buffer);
    std::cout << std::endl;
    if( ret > 0 ) {
      std::cout << std::format("Received {} bytes and msg: {}", ret, buffer) << std::endl;
    }
    else if( ret == 0) {
      std::cout << "EOF detected." << std::endl;
    }
  }

  std::cout << "Connection closed." << std::endl;

  return EXIT_SUCCESS;
}
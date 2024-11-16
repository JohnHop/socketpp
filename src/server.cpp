#include <cstdlib>
#include <iostream>
#include <print>
#include <map>
#include <functional>
#include <string>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "socketpp/serversocket.hpp"


ServerSocket serversock;
Socket client;


void command_start(std::string const& port)
{
  if(port.empty()) {
    std::println("Invalid expression: missing port");
    return;
  }
  
  serversock.open(port);
  std::println("Start listen on port {}", port);
}

void command_stop(std::string const&)
{
  serversock.close();

  std::println("Stopped listening socket");
}

void command_accept(std::string const&)
{
  std::print("Accepting... ");
  std::fflush(stdout);

  client = serversock.accept();

  std::println("client accepted!");
}

void command_read(std::string const&)
{
  std::string buffer(256, 0);
  auto bytes = client.recv(buffer);
  std::println("Read {} bytes: {}", bytes, buffer);
}

void command_write(std::string const& message)
{
  auto bytes = client.send(message);
  std::println("Sent {} bytes", bytes);
}

void command_close(std::string const&)
{
  client.close();

  std::println("Connection closed");
}

void show_prompt()
{
  std::print("server > ");
}


int main()
{
  std::map<std::string, std::function<void(std::string)>> commands = {
    { "start", command_start },
    { "stop", command_stop, },
    { "accept", command_accept },
    { "read", command_read },
    { "write", command_write },
    { "close", command_close }
  };


  std::string input;
  std::println("(ctrl+d to quit)");
  show_prompt();

  while(std::getline(std::cin, input))
  {
    if(input.empty()) // if the user press enter with no command
    {
      show_prompt();
      continue;
    }

    auto spacePos = input.find_first_of(' ');
    auto const command = input.substr(0, spacePos);
    auto const expression = spacePos != std::string::npos ? input.substr(spacePos+1) : std::string{};
    
    auto foundIt = commands.find(command);
    if(foundIt != std::end(commands))
    {
      commands.at(command)( expression );
    }
    else {
      std::println("command {} not found", command);
    }

    show_prompt();
  }
  
  std::print("\n");

  return EXIT_SUCCESS;
}
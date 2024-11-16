#include <cstdlib>
#include <print>
#include <iostream>
#include <map>
#include <functional>
#include <iterator>

#include "socketpp/socket.hpp"


Socket sock;


void command_open(std::string const& expression)
{
  auto const spacePos = expression.find_first_of(' ');
  if(spacePos == std::string::npos)
  {
    std::println("Invalid expression: {}", expression);
  }
  else
  {
    auto const hostname = expression.substr(0, spacePos);
    auto const port = expression.substr(spacePos+1);

    sock.open(hostname, port);
    std::println("Connected!");
  }
}

void command_close(std::string const& expression)
{
  sock.close();

  std::println("Connection closed");
}

void command_write(std::string const& message)
{
  auto bytes = sock.send(message);
  std::println("Sent {} bytes", bytes);
}

void command_read(std::string const&)
{
  std::string buffer(256, 0);
  auto bytes = sock.recv(buffer);
  std::println("Readed {} bytes: {}", bytes, buffer);
}

void show_prompt()
{
  std::print("client > ");
}

int main()
{
  std::map<std::string, std::function<void(std::string)>> commands = {
    { "open", command_open },
    { "close", command_close },
    { "write", command_write },
    { "read", command_read }
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
    auto const expression = (spacePos != std::string::npos) ? input.substr(spacePos+1) : std::string{};
    
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
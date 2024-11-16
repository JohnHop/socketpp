#if !defined(SOCKET_H)
#define SOCKET_H


#include <string>
#include <utility>
#include <chrono>
#include <sstream>
#include <system_error>
#include <span>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <poll.h>

#include "helper.hpp"


class Socket
{
  int socketfd{-1};

public:
  Socket() = default; // non-valid socket
  explicit Socket(int fd);
  Socket(std::string const& hostname, std::string const& port);
  Socket(Socket&&);
  Socket& operator=(Socket&&);
  ~Socket();

  // Prevent copy
  Socket(Socket const&) = delete;
  Socket& operator=(Socket const&) = delete;

  // Operations
  void open(std::string const& hostname, std::string const& port);
  void close() noexcept;
  std::size_t recv(std::span<char> buffer, int flags = 0);
  std::size_t send(std::span<char const> buffer, int flags = 0);

  // Getters
  bool is_open() const { return this->socketfd != -1; };
  int get_fd() const { return this->socketfd; }
  explicit operator bool() const { return this->is_open(); }  // like std::fstream
};


#endif // SOCKET_H

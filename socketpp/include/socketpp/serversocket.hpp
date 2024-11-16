#if !defined(SERVERSOCKET_H)
#define SERVERSOCKET_H

#include <string>
#include <utility>
#include <chrono>

#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>

#include "socket.hpp"


class ServerSocket
{
  int socketfd{-1};
  
public:
  ServerSocket() = default;
  ServerSocket(std::string const&, int max_requests = 5);
  ServerSocket(ServerSocket&& other);
  ServerSocket& operator=(ServerSocket&&);
  ~ServerSocket();

  // Deleted
  ServerSocket(ServerSocket const&) = delete;
  ServerSocket& operator=(ServerSocket const&) = delete;
  
  // Operations
  void open(std::string const&, int max_requests = 5);
  void close() noexcept;
  Socket accept(std::chrono::milliseconds ms = std::chrono::milliseconds{});
  void stop_accept() noexcept;
  
  // Getters
  bool is_open() const { return this->socketfd != -1; };
  int get_fd() const { return this->socketfd; }
  explicit operator bool() const { return this->is_open(); }
};

#endif // SERVERSOCKET_H

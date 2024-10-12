#include "serversocket.hpp"


/**
 * @brief Create a new ServerSocket objects that listen on < port >.
 * @throw See `ServerSocket::open()`.
 */
ServerSocket::ServerSocket(std::string const& port, int max_requests)
{
  this->open(port, max_requests); 
}


ServerSocket::ServerSocket(ServerSocket&& other) {
  this->socketfd = std::exchange(other.socketfd, -1);
}


/**
 * @brief Close the actual listening socket (if open) and get the file descriptor from `other`.
 */
ServerSocket& ServerSocket::operator=(ServerSocket&& other)
{
  //? protezione contro l'auto-assegnamento?

  if( this->is_open() ) {
    this->close();
  }

  this->socketfd = std::exchange(other.socketfd, -1);

  return *this;
}


/**
 * @brief Try to execute `getaddrinfo()`, `socket()`, `bind()` and `listen()` on a < port >.
 * @throw `std::runtime_error` when `getaddrinfo()` fails.
 * @throw `std::runtime_error` when `socket()` fails.
 * @throw `std::runtime_error` when `bind()` fails.
 * @throw `std::runtime_error` when `listen()` fails.
 */
void ServerSocket::open(std::string const& port, int max_requests)
{
  // If the socket is already open, fail
  if( this->is_open() ) {
    throw std::logic_error("ServerSocket::open(): socket already open");
  }

  // 1. Ottengo informazioni sul servizio che voglio diventare
  addrinfo const hints = {
    .ai_flags    = AI_PASSIVE | AI_NUMERICSERV,  // server
    .ai_family   = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM, // tcp
  };
  addrinfo* res_addrinfo;
  int ret;  // valore di ritorno delle varie funzioni invocate

  if( (ret = getaddrinfo(nullptr, port.c_str(), &hints, &res_addrinfo)) != 0) {
    throw std::runtime_error( std::string("getaddrinfo(): ") + gai_strerror(ret) );
  }

  // 2. Try to execute socket() and bind() for each hint found by getaddrinfo()
  addrinfo* found = nullptr;
  std::string errors;
  for(auto ptr = res_addrinfo; (ptr != nullptr) && (found == nullptr); ptr = ptr->ai_next) {
    if(( ret = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol) ) == -1) {
      errors += get_strerror(errno, ptr);
      // continue to iterate through the next hint
    }
    else {
      constexpr int yes { 1 };  // we want reuse the pair < address, port >
      if( ::setsockopt(ret, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1 ) {
        throw std::system_error( errno, std::system_category(), "setsockopt()" );
      }

      if( ::bind(ret, ptr->ai_addr, ptr->ai_addrlen) == -1 ) {
        errors += get_strerror(errno, ptr);
        ::close(ret);
        // close the socket and try with the next hint
      }
      else {  // both socket() and bind() were successful!
        found = ptr;
      }
    }
  }
  if( !found ) {
    throw std::runtime_error( "Unable to socket() or bind() to any addrinfo result. Error for each addrinfo node:\n" + errors );
  }

  this->socketfd = ret;
  freeaddrinfo(res_addrinfo);

  // 3. listen()
  if(( ret = ::listen(this->socketfd, max_requests) ) == -1) {
    throw std::system_error( errno, std::system_category(), "listen()" );
  }
}


/**
 * @brief Close the socket invoking `close()` to his file descriptor.
 */
void ServerSocket::close()
{
  if( ::close(this->socketfd) == -1 ) {
    throw std::system_error( errno, std::system_category(), "close()" );
  }
  this->socketfd = -1;
}

/**
 * @brief   Calls `accept()` on a listening socket and wait for incoming connections until timeout occur.
 * @param timeout Wait time in milliseconds.
 * @returns A connection `Socket` with a client if successes or a invalid Socket.
 */
Socket ServerSocket::accept(std::chrono::milliseconds timeout)
{
  sockaddr_storage addr;
  socklen_t addr_size = sizeof(sockaddr_storage);

  // Setting poll
  struct pollfd pfds[] = { 
    { this->socketfd, POLLIN }     
  };

  int poll_ret = poll( pfds, std::size(pfds), timeout.count() ? timeout.count() : -1 );
  if(poll_ret == -1) {
    throw std::system_error( errno, std::system_category(), "poll()" );
  }

  if( !poll_ret ) {  // timeout occur
    return Socket();  // returns a non valid socket //? è giusto così?
  }

  int client_sockfd = ::accept(this->socketfd, reinterpret_cast<sockaddr*>(&addr), &addr_size);
  if(client_sockfd == -1) {
    throw std::system_error( errno, std::system_category(), "accept()" );
  }

  return Socket(client_sockfd);
}


/**
 * @brief Unlock `accept()` call on listening socket invoking `shutdown()` on it.
 *        Causes the socket to close like calling `ServerSocket::close()`.
 * 
 * @warning This function cause the thread waiting on `ServerSocket::accept()` to throw because of EBADF 
 *          ("Bad file descriptor").
 */
void ServerSocket::stop_accept()
{
  if( shutdown(this->socketfd, SHUT_RDWR) == -1 ) {
    throw std::system_error( errno, std::system_category(), "shutdown()" );
  }

  this->socketfd = -1;
}
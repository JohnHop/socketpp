#include "socket.hpp"

/**
 * @brief Create a Socket object from an already open socket file descriptor.
 */
Socket::Socket(int fd) 
{
  struct stat statbuf;

  if( fstat(fd, &statbuf) == -1 ) {
    throw std::system_error( errno, std::system_category(), "fstat()" );
  }
  if( S_ISSOCK(statbuf.st_mode) == 0) {
    throw std::runtime_error("Socket::Socket(): not a socket file descriptor");
  }

  socketfd = fd;  // success
}


Socket::Socket(Socket&& other)
{
  this->socketfd = std::exchange(other.socketfd, -1);
}


Socket& Socket::operator=(Socket&& other) 
{
  //? Protezione contro l'auto-assegnamento?

  if( this->is_open() ) {
    this->close();
  }

  this->socketfd = std::exchange(other.socketfd, -1);

  return *this;
}


/**
 * @brief Construct a Socket from pair < hostname, port >.
 * @throw See `Socket::open()`.
 */
Socket::Socket(std::string const& hostname, std::string const& port) {
  this->open(hostname, port);
}


/**
 * @brief Create a socket and try to connect to < hostname, port >.
 * @throw std::logic_error when socket is already open.
 * @throw std::logic_error when getaddrinfo() fails.
 * @throw std::logic_error when socket() fails.
 * @throw std::logic_error when connect() fails.
 */
void Socket::open(std::string const& hostname, std::string const& port)
{
  // If the socket is already open, fail
  if( this->is_open() ) {
    throw std::logic_error("Socket::open(): socket already open");
  }

  struct addrinfo const hints = {
    .ai_flags    = AI_NUMERICSERV,
    .ai_family   = AF_UNSPEC, // TODO: fallo template
    .ai_socktype = SOCK_STREAM  // tcp
  };
  addrinfo* res_addrinfo;
  int ret;

  if(( ret = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res_addrinfo)) != 0) {
    throw std::runtime_error( std::string("getaddrinfo(): ") + gai_strerror(ret) );
  }

  addrinfo* ptr;
  std::string errors;
  for(ptr = res_addrinfo; ptr != nullptr; ptr = ptr->ai_next) {
    if(( ret = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1) { // error
      errors += get_strerror(errno, ptr);
      continue; // fail, trying next node
    }
    if( ::connect(ret, ptr->ai_addr, ptr->ai_addrlen) != -1 ) {
      break;  // success!
    }
    errors += get_strerror(errno, ptr);
    ::close(ret);
  }
  if(ptr == nullptr) {
    throw std::runtime_error( "Socket::open(): Unable or bind() to any addrinfo result. Error chain: \n" + errors );
  }

  freeaddrinfo(res_addrinfo);  // cleanining

  this->socketfd = ret; // success
}


/**
 * @brief Close a open socket identified by his file descriptor.
 * @throw std::system_error when close() fails;
 */
void Socket::close()
{
  if( ::close(this->socketfd) == -1 ) {
    throw std::system_error( errno, std::system_category(), "close()" );
  }
  this->socketfd = -1;
}


/**
 * @brief Rimane in ascolto di messaggi sulla socket. La chiamata è bloccante.
 * @return Il numero di byte letti oppure 0 in caso di EOF (chiusura della connessione).
 */
size_t Socket::read(std::span<char> buffer, int flags)
{

  auto count = ::recv( socketfd, buffer.data(), buffer.size(), 0 );
  if(count == -1) {
    throw std::system_error( errno, std::system_category(), "read()" );
  }
  return count;
}


/**
 * @brief Execute a send() on a file descriptor with the default flag MSG_NOSIGNAL.
 * @note  If receiver close stopped execute recv(), the first write() will execute ok //! FIXME 
 */
size_t Socket::write(std::span<char const> buffer, int flags)
{
  auto byte_count = ::send( this->socketfd, buffer.data(), buffer.size(), flags | MSG_NOSIGNAL );
  if(byte_count == -1) {
    throw std::system_error( errno, std::system_category(), "write()" );
  }

  return byte_count;
}
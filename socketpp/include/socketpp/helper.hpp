#if !defined(HELPER_HPP)
#define HELPER_HPP

/**
 * Some helper definitions.
 */

#include <map>
#include <string>
#include <string.h> // per strerror_r()
#include <sstream>
#include <array>

#include <sys/types.h>
#include <bits/socket.h>  // per Address Families e Socket Types
#include <netdb.h>
#include <netinet/in.h>   // per IP protocols


std::map<int,std::string> const address_families = {
  { AF_UNSPEC,    "AF_UNSPEC" },
  { AF_LOCAL,     "AF_LOCAL | AF_UNIX | AF_FILE" },
  { AF_INET,      "AF_INET" },
  { AF_AX25,      "AF_AX25" },
  { AF_IPX,       "AF_IPX"	},
  { AF_APPLETALK, "AF_APPLETALK"	},
  { AF_NETROM,    "AF_NETROM" },
  { AF_BRIDGE,    "AF_BRIDGE" },
  { AF_ATMPVC,    "AF_ATMPVC" },
  { AF_X25,       "AF_X25" },
  { AF_INET6,     "AF_INET6" },
  { AF_ROSE,      "AF_ROSE" },
  { AF_DECnet,    "AF_DECnet" },
  { AF_NETBEUI,   "AF_NETBEUI" },
  { AF_SECURITY,  "AF_SECURITY" },
  { AF_KEY,       "AF_KEY" },
  { AF_NETLINK,   "AF_NETLINK | AF_ROUTE" },
  { AF_PACKET,    "AF_PACKET" },
  { AF_ASH,       "AF_ASH" },
  { AF_ECONET,    "AF_ECONET" },
  { AF_ATMSVC,    "AF_ATMSVC" },
  { AF_RDS,       "AF_RDS" },
  { AF_SNA,       "AF_SNA" },
  { AF_IRDA,      "AF_IRDA" },
  { AF_PPPOX,     "AF_PPPOX" },
  { AF_WANPIPE,   "AF_WANPIPE" },
  { AF_LLC,       "AF_LLC" },
  { AF_IB,        "AF_IB" },
  { AF_MPLS,      "AF_MPLS" },
  { AF_CAN,       "AF_CAN" },
  { AF_TIPC,      "AF_TIPC" },
  { AF_BLUETOOTH, "AF_BLUETOOTH" },
  { AF_IUCV,      "AF_IUCV" },
  { AF_RXRPC,     "AF_RXRPC"}, 
  { AF_ISDN,      "AF_ISDN" },
  { AF_PHONET,    "AF_PHONET" },
  { AF_IEEE802154,"AF_IEEE802154" },
  { AF_CAIF,      "AF_CAIF" },
  { AF_ALG,       "AF_ALG" },
  { AF_NFC,       "AF_NFC" },
  { AF_VSOCK,     "AF_VSOCK" },
  { AF_KCM,       "AF_KCM" },
  { AF_QIPCRTR,   "AF_QIPCRTR" },
  { AF_SMC,       "AF_SMC" },
  { AF_XDP,       "AF_XDP" },
  { AF_MCTP,      "AF_MCTP" },
  { AF_MAX,       "AF_MAX" },
};

// Macro definite in <bits/socket_type.h>
std::map<int, std::string> const socket_type {
  { SOCK_STREAM,    "SOCK_STREAM" },
  { SOCK_DGRAM,     "SOCK_DGRAM" },
  { SOCK_RAW,       "SOCK_RAW" },
  { SOCK_RDM,       "SOCK_RDM" },
  { SOCK_SEQPACKET, "SOCK_SEQPACKET" },
  { SOCK_DCCP,      "SOCK_DCCP" },
  { SOCK_PACKET,    "SOCK_PACKET" },
  { SOCK_CLOEXEC,   "SOCK_CLOEXEC" },
  { SOCK_NONBLOCK,  "SOCK_NONBLOCK" }
};

// Macro definite in <netinet/in.h>
std::map<int, std::string> const ip_protocols {
  { IPPROTO_IP,       "IPPROTO_IP" },
  { IPPROTO_ICMP,     "IPPROTO_ICMP" },
  { IPPROTO_IGMP,     "IPPROTO_IGMP" },
  { IPPROTO_IPIP,     "IPPROTO_IPIP" },
  { IPPROTO_TCP,      "IPPROTO_TCP" },
  { IPPROTO_EGP,      "IPPROTO_EGP" },
  { IPPROTO_PUP,      "IPPROTO_PUP" },
  { IPPROTO_UDP,      "IPPROTO_UDP" },
  { IPPROTO_IDP,      "IPPROTO_IDP" },
  { IPPROTO_TP,       "IPPROTO_TP" },
  { IPPROTO_DCCP,     "IPPROTO_DCCP" },
  { IPPROTO_IPV6,     "IPPROTO_IPV6" },
  { IPPROTO_RSVP,     "IPPROTO_RSVP" },
  { IPPROTO_GRE,      "IPPROTO_GRE" },
  { IPPROTO_ESP,      "IPPROTO_ESP" },
  { IPPROTO_AH,       "IPPROTO_AH" },
  { IPPROTO_MTP,      "IPPROTO_MTP" },
  { IPPROTO_BEETPH,   "IPPROTO_BEETPH" },
  { IPPROTO_ENCAP,    "IPPROTO_ENCAP" },
  { IPPROTO_PIM,      "IPPROTO_PIM" },
  { IPPROTO_COMP,     "IPPROTO_COMP" },
  { IPPROTO_L2TP,     "IPPROTO_L2TP" },
  { IPPROTO_SCTP,     "IPPROTO_SCTP" },
  { IPPROTO_UDPLITE,  "IPPROTO_UDPLITE" },
  { IPPROTO_MPLS,     "IPPROTO_MPLS" },
  { IPPROTO_ETHERNET, "IPPROTO_ETHERNET" },
  { IPPROTO_RAW,      "IPPROTO_RAW" },
  { IPPROTO_MPTCP,    "IPPROTO_MPTCP" },
  { IPPROTO_MPTCP,    "IPPROTO_MPTCP" }
};

/**
 * @brief Let you know why `socket()` or `bind()` were unsuccessful with a hint node retuned by
 *        `getaddrinfo()`.
 * 
 * @param errno_val errno value set when `socket()` or `bind()` fails.
 * @param node_info hint node returned by 
 * 
 * @details
 * This function calls the inverse function of `getaddrinfo()`, storing in `hbuf` and `sbuf` the 
 * hostname and port used. After, `strerror_r()` is used to get the error string related to the errno
 * value.
 * 
 * @return A string containing hostname, port and the associated error string description.
 */
inline
std::string get_strerror(int const errno_val, addrinfo const* node_info) {
  std::stringstream error_ss;
  char hbuf[NI_MAXHOST], // hostname
       sbuf[NI_MAXSERV]; // service (port)
  char error_buf[255];
  
  int ret;
  if( (ret = getnameinfo(node_info->ai_addr, node_info->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
    throw std::runtime_error( std::string("getnameinfo(): ") + gai_strerror(ret) );
  }

  error_ss << "  [" << hbuf << ", " << sbuf << "]: " << strerror_r(errno_val, error_buf, sizeof(error_buf)) << "\n";
  return error_ss.str();
}


#endif // HELPER_HPP

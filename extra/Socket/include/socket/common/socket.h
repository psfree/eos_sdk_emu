/* Copyright (C) Nemirtingas
 * This file is part of Socket.
 *
 * Socket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Socket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Socket.  If not, see <https://www.gnu.org/licenses/>
 */

#pragma once

#include <utils/utils_exports.h>
#include <utils/class_enum.hpp>
#include <utils/endianness.hpp>

#if defined(UTILS_OS_WINDOWS)

    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <WinSock2.h>
    #include <Ws2tcpip.h>

    //#pragma comment(lib, "ws2_32.lib")

#elif defined(UTILS_OS_LINUX)
    #include <unistd.h>
    #include <signal.h>
    #include <netdb.h>

    #include <arpa/inet.h>

    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/ioctl.h>
    #include <sys/poll.h>

    #include <netinet/in.h>

    struct WSAData {};
#elif defined(UTILS_OS_APPLE)
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>

    #include <arpa/inet.h>
    
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/ioctl.h>
    #include <sys/poll.h>
    #include <sys/select.h>
    #include <sys/filio.h>

    #include <netinet/in.h>

    struct WSAData {};
#endif

#include <cstdint>
#include <string>
#include <vector>
#include <exception>
#include <limits>
#include <iostream>

namespace PortableAPI
{
    ////////////
    /// @brief Base socket exception class
    ////////////
    class socket_exception : public std::exception
    {
        std::string mywhat;
    public:
        ////////////
        /// @brief 
        ////////////
        socket_exception();
        ////////////
        /// @brief 
        /// @param[in] mywhat A string about what did throw the exception
        ////////////
        socket_exception(const char* mywhat);
        ////////////
        /// @brief 
        /// @param[in] mywhat A string about what did throw the exception
        ////////////
        socket_exception(std::string const& mywhat);
        ////////////
        /// @brief 
        /// @return Returns the error as string
        ////////////
        virtual const char* what() const noexcept;
    };

#define SOCKET_EXCEPTION_CLASS(x, def_msg) \
class x : public socket_exception \
{\
public:\
    x(const char* mywhat = def_msg):socket_exception(mywhat){}\
    x(std::string const& mywhat):socket_exception(mywhat){}\
}

    // Windows specific exceptions
    SOCKET_EXCEPTION_CLASS(wsa_not_initialised, "A successful WSAStartup call must occur before using this function.");      // WSANOTINITIALISED
    SOCKET_EXCEPTION_CLASS(wsa_net_down, "The network subsystem has failed.");                                               // WSAENETDOWN
    SOCKET_EXCEPTION_CLASS(wsa_sys_not_ready, "The underlying network subsystem is not ready for network communication.");   // WSASYSNOTREADY
    SOCKET_EXCEPTION_CLASS(wsa_version_not_supported, "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation."); // WSAVERNOTSUPPORTED
    SOCKET_EXCEPTION_CLASS(wsa_proclim, "A limit on the number of tasks supported by the Windows Sockets implementation has been reached."); // WSAEPROCLIM
    SOCKET_EXCEPTION_CLASS(wsa_fault, "The lpWSAData parameter is not a valid pointer.");                                    // WSAEFAULT

    // Generic exceptions
    SOCKET_EXCEPTION_CLASS(address_in_use, "Another socket is already listening on the same port.");                                   // EADDRINUSE   - WSAEADDRINUSE
    SOCKET_EXCEPTION_CLASS(connection_refused, "No-one listening on the remote address.");                                             // ECONNREFUSED - WSAECONNREFUSED
    SOCKET_EXCEPTION_CLASS(connection_reset, "Connection reset by peer.");                                                             // ECONNRESET   - WSAECONNRESET
    SOCKET_EXCEPTION_CLASS(connection_abort, "Software caused connection abort.");                                                     // ECONNABORT   - WSAECONNABORT
    SOCKET_EXCEPTION_CLASS(connection_timeout, "A connection attempt failed because the connected party did not properly respond after a period of time.");  //              - WSAETIMEDOUT
    SOCKET_EXCEPTION_CLASS(error_in_value, "Error in value");                                                                          // EINVAL       - WSAEINVAL
    SOCKET_EXCEPTION_CLASS(is_connected, "A connect request was made on an already-connected socket.");                                // EISCONN      - WSAEISCONN
    SOCKET_EXCEPTION_CLASS(network_unreachable, "Network is unreachable.");                                                            // ENETUNREACH  - WSAENETUNREACH
    SOCKET_EXCEPTION_CLASS(not_connected, "The socket is not connected.");                                                             // ENOTCONN     - WSAENOTCONN
    SOCKET_EXCEPTION_CLASS(would_block, "The operation would block");                                                                  // EWOULDBLOCK  - WSAEWOULDBLOCK
    SOCKET_EXCEPTION_CLASS(in_progress, "A blocking operation is in progress.");                                                       // EINPROGRESS  - WSAEINPROGRESS
    SOCKET_EXCEPTION_CLASS(no_acces, "Authorization refused, you don't have the rights to do this. (Firewall blocked?)");              // EACCES  - WSAEACCES

#undef SOCKET_EXCEPTION_CLASS

    ////////////
    /// @brief An abstract class to represent a Network Address, like a sock_addr*
    ////////////
    class basic_addr
    {
    public:
        ////////////
        /// @brief 
        ////////////
        virtual ~basic_addr();
        ////////////
        /// @brief Transforms the address to a human readable string
        /// @param[in] with_port Append the port
        /// @return The string representation of the address
        ////////////
        virtual std::string to_string(bool with_port = false) const = 0;
        ////////////
        /// @brief Transforms the human readable string into an address
        /// @param[in] str The string representation, see specialization for more informations
        /// @return false failed to parse, true succeeded to parse
        ////////////
        virtual bool from_string(std::string const& str) = 0;
        ////////////
        /// @brief Gets the generic sockaddr ref
        /// @return The sockaddr ref
        ////////////
        virtual sockaddr& addr() = 0;
        ////////////
        /// @brief Gets the generic const sockaddr ref
        /// @return The const sockaddr ref
        ////////////
        virtual sockaddr const& addr() const = 0;
        ////////////
        /// @brief Get the sockaddr size
        /// @return sockaddr size
        ////////////
        virtual size_t len() const = 0;
    };

    ////////////
    /// @brief A wrapper class for 'C' network & socket functions
    ////////////
    class Socket
    {
    public:
#if defined(UTILS_OS_WINDOWS)
        using socket_t = SOCKET;
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        using socket_t = int32_t;
#endif
        ////////////
        /// @brief An invalid value for a socket_t type
        ////////////
        static constexpr socket_t invalid_socket = ((socket_t)(-1));

        ////////////
        /// @brief Network address family enum 
        ////////////
        enum class address_family
        {
#if defined(UTILS_OS_WINDOWS)
            implink = AF_IMPLINK,
            pip = AF_PUP,
            chaos = AF_CHAOS,
            ns = AF_NS,
            uso = AF_ISO,
            osi = AF_OSI,
            ecma = AF_ECMA,
            datakit = AF_DATAKIT,
            ccitt = AF_CCITT,
            dli = AF_DLI,
            lat = AF_LAT,
            hylink = AF_HYLINK,
            netbios = AF_NETBIOS,
            voiceview = AF_VOICEVIEW,
            firefox = AF_FIREFOX,
            unknown1 = AF_UNKNOWN1,
            ban = AF_BAN,
            atm = AF_ATM,
            cluster = AF_CLUSTER,
            IEEE1284_4 = AF_12844,
            netdes = AF_NETDES,
            irda = AF_IRDA,
#elif defined(UTILS_OS_LINUX)
            irda = AF_IRDA,
#endif
            unix = AF_UNIX,
            inet = AF_INET,
            ipx = AF_IPX,
            sna = AF_SNA,
            decnet = AF_DECnet,
            appletalk = AF_APPLETALK,
            inet6 = AF_INET6,
            unspec = AF_UNSPEC,
        };

        ////////////
        /// @brief recv, recvfrom, send, sendto flags enum 
        ////////////
        enum class socket_flags
        {
            normal = 0,
            oob = MSG_OOB,// process out-of-band data
            peek = MSG_PEEK,// peek at incoming message
            dontroute = MSG_DONTROUTE,//send without using routing tables
#if(_WIN32_WINNT >= 0x0502)
            waitall = MSG_WAITALL,//do not complete until packet is completely filled
#endif
        };

        ////////////
        /// @brief Socket types enum 
        ////////////
        enum class types
        {
            stream = SOCK_STREAM,
            dgram = SOCK_DGRAM,
            raw = SOCK_RAW,
            rdm = SOCK_RDM,
            seqpacket = SOCK_SEQPACKET,
        };

        ////////////
        /// @brief Network protocols enum
        ////////////
        enum class protocols
        {
            icmp = IPPROTO_ICMP,
            igmp = IPPROTO_IGMP,
            tcp = IPPROTO_TCP,
            pup = IPPROTO_PUP,
            udp = IPPROTO_UDP,
            idp = IPPROTO_IDP,
            raw = IPPROTO_RAW,
            max = IPPROTO_MAX,
#if defined(UTILS_OS_LINUX)
            hopopts = IPPROTO_HOPOPTS,
            routing = IPPROTO_ROUTING,
            fragment = IPPROTO_FRAGMENT,
            icmpv6 = IPPROTO_ICMPV6,
            none = IPPROTO_NONE,
            dstopts = IPPROTO_DSTOPTS,
            //mh = IPPROTO_MH,
            ip = IPPROTO_IP,
            ipip = IPPROTO_IPIP,
            egp = IPPROTO_EGP,
            tp = IPPROTO_TP,
            dccp = IPPROTO_DCCP,
            ipv6 = IPPROTO_IPV6,
            rsvp = IPPROTO_RSVP,
            gre = IPPROTO_GRE,
            esp = IPPROTO_ESP,
            ah = IPPROTO_AH,
            mtp = IPPROTO_MTP,
            //beetph = IPPROTO_BEETPH,
            encap = IPPROTO_ENCAP,
            pim = IPPROTO_PIM,
            comp = IPPROTO_COMP,
            sctp = IPPROTO_SCTP,
            udplite = IPPROTO_UDPLITE,
#elif defined(UTILS_OS_WINDOWS)
            ggp = IPPROTO_GGP,
            nd = IPPROTO_ND,
#if(_WIN32_WINNT >= 0x0501)
            hopopts = IPPROTO_HOPOPTS,
            ipv4 = IPPROTO_IPV4,
            ipv6 = IPPROTO_IPV6,
            routin = IPPROTO_ROUTING,
            fragment = IPPROTO_FRAGMENT,
            esp = IPPROTO_ESP,
            ah = IPPROTO_AH,
            icmpv6 = IPPROTO_ICMPV6,
            none = IPPROTO_NONE,
            dstopts = IPPROTO_DSTOPTS,
            iclfxbm = IPPROTO_ICLFXBM,
#endif//(_WIN32_WINNT >= 0x0501)
#if(_WIN32_WINNT >= 0x0600)
            st = IPPROTO_ST,
            cbt = IPPROTO_CBT,
            egp = IPPROTO_EGP,
            igp = IPPROTO_IGP,
            rdp = IPPROTO_RDP,
            pim = IPPROTO_PIM,
            pgm = IPPROTO_PGM,
            l2tp = IPPROTO_L2TP,
            sctp = IPPROTO_SCTP,
#endif//(_WIN32_WINNT >= 0x0600)
#endif
        };

        ////////////
        /// @brief getsockopt/setsockopt functions enum 
        ////////////
        enum class level
        {
            sol_socket = SOL_SOCKET,
        };

        ////////////
        /// @brief getsockopt/setsockopt functions enum 
        ////////////
        enum class option_name : uint32_t
        {
#if defined(UTILS_OS_WINDOWS)
            so_debug = SO_DEBUG,
            so_acceptconn = SO_ACCEPTCONN,
            so_reuseaddr = SO_REUSEADDR,
            so_keepalive = SO_KEEPALIVE,
            so_dontroute = SO_DONTROUTE,
            so_broadcast = SO_BROADCAST,
            so_useloopback = SO_USELOOPBACK,
            so_linger = SO_LINGER,
            so_oobinline = SO_OOBINLINE,
            so_sndbuf = SO_SNDBUF,
            so_rcvbuf = SO_RCVBUF,
            so_sndlowat = SO_SNDLOWAT,
            so_rcvlowat = SO_RCVLOWAT,
            so_sndtimeo = SO_SNDTIMEO,
            so_rcvtimeo = SO_RCVTIMEO,
            so_error = SO_ERROR,
            so_type = SO_TYPE,
            so_group_id = SO_GROUP_ID,
            so_group_priority = SO_GROUP_PRIORITY,
            so_max_msg_size = SO_MAX_MSG_SIZE,
            #ifdef UNICODE
                so_protocol_info = SO_PROTOCOL_INFOW,
            #else
                so_protocol_info = SO_PROTOCOL_INFOA,
            #endif /* UNICODE */
#elif defined(UTILS_OS_LINUX)
            so_debug = SO_DEBUG,
            so_reuseaddr = SO_REUSEADDR,
            so_keepalive = SO_KEEPALIVE,
            so_dontroute = SO_DONTROUTE,
            so_broadcast = SO_BROADCAST,
            so_linger = SO_LINGER,
            so_oobinline = SO_OOBINLINE,
            so_sndbuf = SO_SNDBUF,
            so_rcvbuf = SO_RCVBUF,
            so_sndlowat = SO_SNDLOWAT,
            so_rcvlowat = SO_RCVLOWAT,
            so_sndtimeo = SO_SNDTIMEO,
            so_rcvtimeo = SO_RCVTIMEO,
            so_error = SO_ERROR,
            so_type = SO_TYPE,
            so_sndbufforce = SO_SNDBUFFORCE,
            so_rcvbufforce = SO_RCVBUFFORCE,
            so_no_check = SO_NO_CHECK,
            so_priority = SO_PRIORITY,
            so_bsdcompat = SO_BSDCOMPAT,
            so_reuseport = SO_REUSEPORT,
            so_passcred = SO_PASSCRED,
            so_peercred = SO_PEERCRED,
#elif defined(UTILS_OS_APPLE)
            so_debug = SO_DEBUG,
            so_reuseaddr = SO_REUSEADDR,
            so_keepalive = SO_KEEPALIVE,
            so_dontroute = SO_DONTROUTE,
            so_broadcast = SO_BROADCAST,
            so_linger = SO_LINGER,
            so_oobinline = SO_OOBINLINE,
            so_sndbuf = SO_SNDBUF,
            so_rcvbuf = SO_RCVBUF,
            so_sndlowat = SO_SNDLOWAT,
            so_rcvlowat = SO_RCVLOWAT,
            so_sndtimeo = SO_SNDTIMEO,
            so_rcvtimeo = SO_RCVTIMEO,
            so_error = SO_ERROR,
            so_type = SO_TYPE,
            so_reuseport = SO_REUSEPORT,
#endif
        };

        ////////////
        /// @brief ioctl function enum 
        ////////////
        enum class cmd_name : uint32_t
        {
            fionread = FIONREAD,
            fionbio  = FIONBIO,
            fioasync = FIOASYNC,
        };

        ////////////
        /// @brief shutdown function enum 
        ////////////
        enum class shutdown_flags
        {
#if defined(UTILS_OS_WINDOWS)
            reveive = SD_RECEIVE,
            send = SD_SEND,
            both = SD_BOTH,
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            reveive = SHUT_RD,
            send = SHUT_WR,
            both = SHUT_RDWR,
#endif
        };

#if _WIN32_WINNT >= 0x0600 || defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        ////////////
        /// @brief poll function enum 
        ////////////
        enum class poll_flags
        {
            none = 0,
            rdnorm = POLLRDNORM,
            rdband = POLLRDBAND,
            in = POLLIN,
            pri = POLLPRI,
            wrnorm = POLLWRNORM,
            out = POLLOUT,
            wrband = POLLWRBAND,
            err = POLLERR,
            hup = POLLHUP,
            nval = POLLNVAL,
            in_hup = in|hup,
            out_hup = out|hup,
        };
#endif

        ////////////
        /// @brief Wrapper for 'C' accept function. Accepts a connection on the socket
        ///        Can throw exception depending on error
        /// @param[in]  s A valid socket that listens for connections
        /// @param[out] addr Accepted connection address infos
        /// @return The accepted Socket::socket_t
        ////////////
        static Socket::socket_t accept(Socket::socket_t s, basic_addr &addr);
        ////////////
        /// @brief Wrapper for 'C' bind function. Binds a socket on an address
        ///        Can throw exception depending on error
        /// @param[in]  s A valid socket to bind to addr
        /// @param[in]  addr An address to bind s to
        /// @return 
        ////////////
        static void bind(Socket::socket_t s, basic_addr const&addr);
        ////////////
        /// @brief Wrapper for 'C' close function. Closes a socket
        /// @param[in]  s A socket to close
        /// @return 
        ////////////
        static void closeSocket(Socket::socket_t s);
        ////////////
        /// @brief Wrapper for 'C' bind function. Connects to a remote peer
        ///        Can throw exception depending on error
        /// @param[in]  s A valid socket to connect to addr
        /// @param[in]  addr Accepted connection address infos
        /// @return 
        ////////////
        static void connect(Socket::socket_t s, basic_addr const&addr);
        ////////////
        /// @brief Wrapper for 'C' ioctlsocket/ioctl function. Changes the behavior of the socket
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[in]  cmd The cmd to apply to socket
        /// @param[in]  arg The cmd argument
        /// @return 0 on success
        ////////////
        static int ioctlsocket(Socket::socket_t s, Socket::cmd_name cmd, unsigned long* arg);
        ////////////
        /// @brief Wrapper for 'C' setsockopt function. Sets socket options
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[in]  level   Socket level
        /// @param[in]  optname Option Name
        /// @param[in]  optval  Option value
        /// @param[in]  optlen  optval size
        /// @return 0 on success
        ////////////
        static int setsockopt(Socket::socket_t s, Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen);
        ////////////
        /// @brief Wrapper for 'C' getsockopt function. Gets socket options
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[in]  level   Socket level
        /// @param[in]  optname Option Name
        /// @param[out] optval  Option value
        /// @param[in]  optlen  optval size
        /// @return 0 on success
        ////////////
        static int getsockopt(Socket::socket_t s, Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen);
        ////////////
        /// @brief Wrapper for 'C' listen function. Listens for incomming connections
        ///        Can throw exception depending on error
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[in]  waiting_connection Number of waiting connections, default 5
        /// @return 
        ////////////
        static void listen(Socket::socket_t s, int waiting_connection = 5);
        ////////////
        /// @brief Wrapper for 'C' recv function for connected sockets. Receives data on the socket
        ///        Can throw exception depending on error
        ///        If the socket is set non-blocking and it would block, 0 is returned as size
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[out] buffer A buffer to hold the received data
        /// @param[in]  len   Buffer maximum length
        /// @param[in]  flags receive flags
        /// @return received size
        ////////////
        static size_t recv(Socket::socket_t s, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
        ////////////
        /// @brief Wrapper for 'C' recvfrom function for unconnected sockets. Receives data on the socket
        ///        Can throw exception depending on error
        ///        If the socket is set non-blocking and it would block, 0 is returned as size
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[out] addr The peer address informations
        /// @param[out] buffer A buffer to hold the received data
        /// @param[in]  len   Buffer maximum length
        /// @param[in]  flags receive flags
        /// @return received size
        ////////////
        static size_t recvfrom(Socket::socket_t s, basic_addr &addr, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
        ////////////
        /// @brief Wrapper for 'C' send function for connected sockets. Sends data on the socket
        ///        Can throw exception depending on error
        ///        If the socket is set non-blocking and it would block, 0 is returned as size
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[in]  buffer A buffer that holds the datas to send
        /// @param[in]  len   Size in Bytes to send
        /// @param[in]  flags send flags
        /// @return Sent size
        ////////////
        static size_t send(Socket::socket_t s, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
        ////////////
        /// @brief Wrapper for 'C' sendto function for unconnected sockets. Sends data on the socket
        ///        Can throw exception depending on error
        ///        If the socket is set non-blocking and it would block, 0 is returned as size
        /// @param[in]  s A socket with a previous successful call to 'socket'
        /// @param[in]  addr The peer to send to address informations
        /// @param[in]  buffer A buffer that holds the datas to send
        /// @param[in]  len   Size in Bytes to send
        /// @param[in]  flags send flags
        /// @return Send size
        ////////////
        static size_t sendto(Socket::socket_t s, basic_addr const&addr, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
        ////////////
        /// @brief Wrapper for 'C' shutdown function. Shutdowns in read and/or write a socket
        /// @param[in]  s   A socket with a previous successful call to 'socket'
        /// @param[in]  how The mode(s) to shutdown, default to both (in & out)
        /// @return 
        ////////////
        static int shutdown(Socket::socket_t s, Socket::shutdown_flags how = Socket::shutdown_flags::both);
        ////////////
        /// @brief Wrapper for 'C' socket function. Creates a new socket
        /// @param[in]  af    The socket address family
        /// @param[in]  type  The socket type family
        /// @param[in]  proto The socket protocol family
        /// @return A newly created socket or Socket::invalid_socket if the function failedd
        ////////////
        static Socket::socket_t socket(Socket::address_family af, Socket::types type, Socket::protocols proto);
        ////////////
        /// @brief Wrapper for 'C' getaddrinfo function.
        /// @param[in]  node    The name of a host, can be nullptr if service is used
        /// @param[in]  service The name of a service, can be nullptr if node is used
        /// @param[in]  hints   A pointer to an addrinfo structure that provides hints about the type of socket the caller supports.
        /// @param[out] res     A pointer to a linked list of one or more addrinfo structures that contains response information about the host.
        /// @return 0 on success
        ////////////
        static int getaddrinfo(const char* node, const char* service, const addrinfo *hints, addrinfo **res);
        ////////////
        /// @brief Wrapper for 'C' freeaddrinfo function.
        /// @param[in]  infos   A pointer to an addrinfo structure previously allocated with getaddrinfo.
        /// @return 
        ////////////
        static void freeaddrinfo(addrinfo* infos);
        ////////////
        /// @brief Wrapper for 'C' getnameinfo function.
        /// @param[in]  addr    A pointer to a socket address structure that contains the address and port number of the socket.
        ///                     For IPv4, the sa parameter points to a sockaddr_in structure. For IPv6, the sa parameter points to a sockaddr_in6 structure.
        /// @param[in]  addrlen The length, in bytes, of the structure pointed to by the sa parameter
        /// @param[out] host    A pointer to an ANSI string used to hold the host name.
        /// @param[in]  hostlen The length, in bytes, of the buffer pointed to by the host parameter
        /// @param[out] serv    A pointer to an ANSI string to hold the service name
        /// @param[in]  servlen The length, in bytes, of the buffer pointed to by the serv parameter
        /// @param[in]  flags   A value used to customize processing of the getnameinfo function
        /// @return 0 on success
        ////////////
        static int getnameinfo(const sockaddr* addr, socklen_t addrlen, char* host, size_t hostlen, char* serv, size_t servlen, int flags);
        ////////////
        /// @brief Wrapper for 'C' inet_pton function. Transforms a string to its binary representation
        /// @param[in]  family   The address family.
        /// @param[in]  str_addr A string that contains the text representation of the address to convert to numeric binary form.
        /// @param[out] out_buf  A pointer to a buffer in which to store the numeric binary representation of the address. The address is returned in network byte order.
        /// @return 1 on success
        ////////////
        static int inet_pton(Socket::address_family family, std::string const& str_addr, void* out_buf);
        ////////////
        /// @brief Wrapper for 'C' inet_pton function. Transforms a binary representation to a human readable string
        /// @param[in]  family   The address family.
        /// @param[in]  addr     A pointer to a buffer in which 
        /// @param[out] str_addr A string in which to store the representation of the address.
        /// @return 
        ////////////
        static void inet_ntop(Socket::address_family family, const void* addr, std::string& str_addr);
        ////////////
        /// @brief Wrapper for 'C' select function. Gets infos on output and/or input socket buffers
        /// @param[in]     nfds     Is the highest-numbered file descriptor in any of the three sets, plus 1. 
        /// @param[inout] readfd   An optional pointer to a set of sockets to be checked for readability.
        /// @param[inout] writefd  An optional pointer to a set of sockets to be checked for writability.
        /// @param[inout] exceptfd An optional pointer to a set of sockets to be checked for errors.
        /// @param[in]     timeout  The maximum time for select to wait, provided in the form of a TIMEVAL structure. Set the timeout parameter to null for blocking operations.
        /// @return The number of file descriptors contained in the three returned descriptor sets
        ////////////
        static int select(int nfds, fd_set *readfd, fd_set *writefd, fd_set *exceptfd, timeval *timeout);
#if(_WIN32_WINNT >= 0x0600) || defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        ////////////
        /// @brief Wrapper for 'C' poll function. Gets infos on output and/or input socket buffers
        ///        Is more efficient than select and should be used if available
        /// @param[in_out] fds     An array of one or more pollfd structures specifying the set of sockets for which status is requested.
        /// @param[in]     nfds    The number of pollfd structures in fds.
        /// @param[in]     timeout <0, block, 0 returns now, >0 The time in milliseconds to wait.
        /// @return <0 error, 0 no socket queried, >0 number of member in fds for which revents field is non-zero
        ////////////
        static int poll(pollfd *fds, size_t nfds, int timeout);
#endif

        ////////////
        /// @brief Initializes the socket library (has no effect on Macos & Linux)
        /// @param[in] version Version of the winsock to use
        /// @return 
        ////////////
        static void InitSocket(uint32_t version = 0x0202);
        ////////////
        /// @brief Sets the last error
        /// @param[in] error The error number
        /// @return 
        ////////////
        static void SetLastError(int error);
        ////////////
        /// @brief Gets the WinSock data (has no effect on Macos & Linux)
        /// @return The winsock data
        ////////////
        static WSAData const& GetWSAData();
        ////////////
        /// @brief Frees automatically the WinSock library (has no effect on Macos & Linux)
        /// @return 
        ////////////
        ~Socket();

        private:
            ////////////
            /// @brief Constructor for the static object that frees Winsock library
            /// @param [in] version Version of the winsock to use
            /// @return The byte-swapped datas
            ////////////
            Socket();
            Socket(Socket const&) = delete;
            Socket(Socket &&) = delete;
            Socket& operator=(Socket const&) = delete;
            Socket& operator=(Socket &&) = delete;
    }; 
}

inline std::ostream& operator<<(std::ostream &os, PortableAPI::basic_addr const& addr)
{
    return os << addr.to_string();
}

UTILS_ENABLE_BITMASK_OPERATORS(PortableAPI::Socket::socket_flags);
UTILS_ENABLE_BITMASK_OPERATORS(PortableAPI::Socket::poll_flags);

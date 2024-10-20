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

#include "socket/common/socket.h"

using namespace PortableAPI;

socket_exception::socket_exception() :mywhat("Socket exception") {}
socket_exception::socket_exception(const char* mywhat) :mywhat(mywhat) {}
socket_exception::socket_exception(std::string const& mywhat) :mywhat(mywhat) {}
const char* socket_exception::what() const noexcept { return mywhat.c_str(); }

basic_addr::~basic_addr()
{}

Socket::socket_t Socket::accept(Socket::socket_t s, basic_addr &addr)
{
    Socket::socket_t res;
    socklen_t len = static_cast<socklen_t>(addr.len());
    if ((res = ::accept(s, &addr.addr(), &len)) == Socket::invalid_socket)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAECONNRESET    : throw connection_reset();
            case WSAEINVAL        : throw error_in_value("The listen function was not invoked prior to accept.");
            case WSAENETDOWN      : throw wsa_net_down();
            case WSAEWOULDBLOCK   : throw would_block();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case ECONNABORTED : throw connection_reset();
            case EINVAL       : throw error_in_value("The listen function was not invoked prior to accept.");
#if EAGAIN != EWOULDBLOCK
            case EAGAIN: 
#endif
            case EWOULDBLOCK : throw would_block();
#endif
            default               : throw socket_exception("accept exception: " + std::to_string(error));
        }
    }

    return res;
}

void Socket::bind(Socket::socket_t s, basic_addr const&addr)
{
    if (::bind(s, &addr.addr(), addr.len()) == -1)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEADDRINUSE: throw address_in_use("The given address is already in use.");
            case WSAEINVAL: throw error_in_value("The socket is already bound to an address.");
            case WSAEACCES: throw no_acces();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case EADDRINUSE: throw address_in_use("The given address is already in use.");
            case EINVAL: throw error_in_value("The socket is already bound to an address.");
            case EACCES: throw no_acces();
#endif
            default: throw socket_exception("bind exception: " + std::to_string(error));
        }
    }
}

void Socket::closeSocket(Socket::socket_t s)
{
#if defined(UTILS_OS_WINDOWS)
    ::closesocket(s);
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
    ::close(s);
#endif
}

void Socket::connect(Socket::socket_t s, basic_addr const&addr)
{
    if (::connect(s, &addr.addr(), addr.len()) == -1)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEADDRINUSE: throw address_in_use("Local address is already in use.");
            case WSAEADDRNOTAVAIL: throw error_in_value("Address is not available.");
            case WSAECONNREFUSED: throw connection_refused();
            case WSAENETUNREACH: case WSAEHOSTUNREACH: throw network_unreachable();
            case WSAETIMEDOUT: throw connection_timeout();
            case WSAEISCONN: throw is_connected();
            case WSAEWOULDBLOCK: throw would_block();
            case WSAEINPROGRESS: throw in_progress();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case EINVAL: throw error_in_value("Address is not available.");
            case EADDRINUSE: throw address_in_use("Local address is already in use.");
            case ECONNREFUSED:  throw connection_refused();
            case ENETUNREACH: throw network_unreachable();
            case EISCONN: throw is_connected();
            case EINPROGRESS: throw in_progress();
    #if EAGAIN != EWOULDBLOCK
            case EAGAIN:
    #endif
            case EWOULDBLOCK: throw would_block();
#endif
            default: throw socket_exception("connect exception: " + std::to_string(error));
        }
    }
}

int Socket::ioctlsocket(Socket::socket_t s, Socket::cmd_name cmd, unsigned long * arg)
{
#if defined(UTILS_OS_WINDOWS)
    return ::ioctlsocket(s, static_cast<long>(cmd), arg);
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
    return ::ioctl(s, static_cast<long>(cmd), arg);
#endif
}

int Socket::setsockopt(Socket::socket_t s, Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen)
{
#if defined(UTILS_OS_WINDOWS)
    return ::setsockopt(s, static_cast<int>(level), static_cast<int>(optname), reinterpret_cast<const char*>(optval), optlen);
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
    return ::setsockopt(s, static_cast<int>(level), static_cast<int>(optname), optval, static_cast<socklen_t>(optlen));
#endif
}

int Socket::getsockopt(Socket::socket_t s, Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen)
{
#if defined(UTILS_OS_WINDOWS)
    return ::getsockopt(s, static_cast<int>(level), static_cast<int>(optname), reinterpret_cast<char*>(optval), optlen);
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
    return ::getsockopt(s, static_cast<int>(level), static_cast<int>(optname), optval, optlen);
#endif
}

void Socket::listen(Socket::socket_t s, int waiting_connection)
{
    if (::listen(s, waiting_connection) == -1)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEADDRINUSE: throw address_in_use();
            case WSAEACCES: throw no_acces();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case EADDRINUSE: throw address_in_use();
            case EACCES: throw no_acces();
#endif
            default: throw socket_exception("listen exception: " + std::to_string(error));
        }
    }
}

size_t Socket::recv(Socket::socket_t s, void* buffer, size_t len, Socket::socket_flags flags)
{
    int res = ::recv(s, reinterpret_cast<char*>(buffer), len, static_cast<int32_t>(flags));
    if (res == 0)
        throw connection_reset();
    else if( res == -1 )
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAENOTCONN: throw not_connected();
            case WSAECONNABORTED: throw connection_abort();
            case WSAEWOULDBLOCK: res = 0; break;
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case ENOTCONN: throw not_connected();
            case ECONNRESET: throw connection_reset();
            case ECONNABORTED: throw connection_abort();
    #if EAGAIN != EWOULDBLOCK
            case EAGAIN:
    #endif
            case EWOULDBLOCK: res = 0; break;
    #endif
            default: throw socket_exception("recv exception: " + std::to_string(error));
        }
    }
    return static_cast<size_t>(res);
}

size_t Socket::recvfrom(Socket::socket_t s, basic_addr &addr, void* buffer, size_t len, Socket::socket_flags flags)
{
    socklen_t slen = addr.len();
    int res = ::recvfrom(s, reinterpret_cast<char*>(buffer), len, static_cast<int32_t>(flags), &addr.addr(), &slen);
    if (res == -1)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEINVAL: throw error_in_value("The socket is not bound to an address.");
            case WSAEWOULDBLOCK: res = 0; break;
            case WSAECONNRESET: throw connection_reset();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case EINVAL: throw error_in_value("The socket is not bound to an address.");
    #if EAGAIN != EWOULDBLOCK
            case EAGAIN:
    #endif
            case EWOULDBLOCK: res = 0; break;
#endif
            default: throw socket_exception("recvfrom exception: " + std::to_string(error));
        }
    }
    return static_cast<size_t>(res);
}

size_t Socket::send(Socket::socket_t s, const void* buffer, size_t len, Socket::socket_flags flags)
{
    int res = ::send(s, reinterpret_cast<char const*>(buffer), len, static_cast<int32_t>(flags));

    if (res == -1)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAENOTCONN: throw not_connected();
            case WSAEWOULDBLOCK: res = 0; break;
            case WSAECONNABORTED: throw connection_reset();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case ENOTCONN: throw not_connected();
    #if EAGAIN != EWOULDBLOCK
            case EAGAIN:
    #endif
            case EWOULDBLOCK: res = 0; break;
#endif
            default: throw socket_exception("send exception: " + std::to_string(error));
        }
    }
    return res;
}

size_t Socket::sendto(Socket::socket_t s, basic_addr const&addr, const void* buffer, size_t len, Socket::socket_flags flags)
{
    int res = ::sendto(s, reinterpret_cast<const char*>(buffer), len, static_cast<int32_t>(flags), &addr.addr(), addr.len());
    if (res == -1)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAENETUNREACH: throw network_unreachable();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case ENETUNREACH: throw network_unreachable();
#endif
            default: throw socket_exception("sendto exception: " + std::to_string(error));
        }
    }
    return res;
}

int Socket::shutdown(Socket::socket_t s, Socket::shutdown_flags how)
{
    return ::shutdown(s, static_cast<int32_t>(how));
}

Socket::socket_t Socket::socket(Socket::address_family af, Socket::types type, Socket::protocols proto)
{
    Socket::socket_t s = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
    if (s == Socket::invalid_socket)
    {
#if defined(UTILS_OS_WINDOWS)
        int32_t error = WSAGetLastError();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(UTILS_OS_WINDOWS)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEACCES: throw no_acces();
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
            case EACCES: throw no_acces();
#endif
            default: throw socket_exception("socket exception: " + std::to_string(error));
        }
    }
    return s;
}

int Socket::getaddrinfo(const char * node, const char * service, const addrinfo * hints, addrinfo ** res)
{
    return ::getaddrinfo(node, service, hints, res);
}

void Socket::freeaddrinfo(addrinfo* infos)
{
    ::freeaddrinfo(infos);
}

int Socket::getnameinfo(const sockaddr * addr, socklen_t addrlen, char * host, size_t hostlen, char * serv, size_t servlen, int flags)
{
    return ::getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
}

int Socket::inet_pton(Socket::address_family family, std::string const& str_addr, void *out_buf)
{
    return ::inet_pton(static_cast<int>(family), str_addr.c_str(), out_buf);
}

void Socket::inet_ntop(Socket::address_family family, const void* addr, std::string& str_addr)
{
    char buff[1024] = {};
    if (::inet_ntop(static_cast<int>(family), addr, buff, sizeof(buff) / sizeof(*buff)) == nullptr)
        throw error_in_value("Error in value, cannot parse addr");
    str_addr = buff;
}

int Socket::select(int nfds, fd_set* readfd, fd_set* writefd, fd_set* exceptfd, timeval* timeout)
{
    return ::select(nfds, readfd, writefd, exceptfd, timeout);
}

#if(_WIN32_WINNT >= 0x0600)
int Socket::poll(pollfd* fds, size_t nfds, int timeout)
{
    return WSAPoll(fds, nfds, timeout);
}
#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
int Socket::poll(pollfd* fds, size_t nfds, int timeout)
{
    return ::poll(fds, nfds, timeout);
}
#endif

void Socket::InitSocket(uint32_t version)
{
#if defined(UTILS_OS_WINDOWS)
    static Socket s;
    int err = WSAStartup(version, &const_cast<WSAData&>(Socket::GetWSAData()));
    switch (err)
    {
        case WSASYSNOTREADY: throw(wsa_sys_not_ready());
        case WSAVERNOTSUPPORTED: throw(wsa_version_not_supported());
        case WSAEINPROGRESS: throw(in_progress());
        case WSAEPROCLIM: throw(wsa_proclim());
        case WSAEFAULT: throw(wsa_fault());
        case 0: break;
        default: throw(socket_exception("Socket initialisation error: " + std::to_string(err)));
    }
#else
    (void)version;
#endif
}

void Socket::SetLastError(int error)
{
#if defined(UTILS_OS_WINDOWS)
    WSASetLastError(error);
#else
    (void)error;
#endif
}

/*
void Socket::FreeSocket()
{
#if defined(UTILS_OS_WINDOWS)
    WSACleanup();
#endif
}
*/

WSAData const& Socket::GetWSAData()
{
    static WSAData datas;
    return datas;
}

Socket::Socket()
{
}

Socket::~Socket() 
{
#if defined(UTILS_OS_WINDOWS)
    WSACleanup();
#endif
}

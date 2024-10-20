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

#include "socket.h"

#include <memory>

namespace PortableAPI
{
    ////////////
    /// @brief Basic socket type that could be used in a future poll class
    ////////////
    class basic_socket
    {
        class SocketDeleter
        {
        public:
            void operator()(Socket::socket_t* s)
            {
                if (s != nullptr)
                {
                    if (*s != Socket::invalid_socket)
                    {
                        Socket::shutdown(*s, Socket::shutdown_flags::both);
                        Socket::closeSocket(*s);
                    }
                    delete s;
                }
            }
        };

        public:
            virtual ~basic_socket() = default;

            ////////////
            /// @brief Changes the behavior of the socket. Can throw exception depending on error
            /// @param[in]  cmd The cmd to apply to socket
            /// @param[in]  arg The cmd argument
            /// @return 
            ////////////
            void ioctlsocket(Socket::cmd_name cmd, unsigned long* arg)
            {
                auto res = Socket::ioctlsocket(*_sock, cmd, arg);
                if (res)
                {
                #if defined(UTILS_OS_WINDOWS)
                    int error = WSAGetLastError();
                #elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
                    int error = errno;
                #endif
                    throw socket_exception("ioctlsocket exception " + std::to_string(error));
                }
            }
            ////////////
            /// @brief Sets socket options. Can throw exception depending on error
            /// @param[in]  level   Socket level
            /// @param[in]  optname Option Name
            /// @param[in]  optval  Option value
            /// @param[in]  optlen  optval size
            /// @return 
            ////////////
            void setsockopt(Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen)
            {
                auto res = Socket::setsockopt(*_sock, level, optname, optval, optlen);
                if (res)
                {
                #if defined(UTILS_OS_WINDOWS)
                    int error = WSAGetLastError();
                #elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
                    int error = errno;
                #endif
                    throw socket_exception("setsockopt exception " + std::to_string(error));
                }
            }
            ////////////
            /// @brief Gets socket options. Can throw exception depending on error
            /// @param[in]  level   Socket level
            /// @param[in]  optname Option Name
            /// @param[out] optval  Option value
            /// @param[in]  optlen  optval size
            /// @return 
            ////////////
            void getsockopt(Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen)
            {
                auto res = Socket::getsockopt(*_sock, level, optname, optval, optlen);
                if (res)
                {
                #if defined(UTILS_OS_WINDOWS)
                    int error = WSAGetLastError();
                #elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
                    int error = errno;
                #endif
                    throw socket_exception("getsockopt exception " + std::to_string(error));
                }
            }
            ////////////
            /// @brief Shutdowns in read and/or write a socket. Can throw exception depending on error
            /// @param[in]  how The mode(s) to shutdown, default to both (in & out)
            /// @return 
            ////////////
            inline void shutdown(Socket::shutdown_flags how = Socket::shutdown_flags::both)
            {
                if (Socket::shutdown(*_sock, how))
                    throw socket_exception("shutdown exception");
            }
            ////////////
            /// @brief Closes a socket
            /// @return 
            ////////////
            inline void close() { Socket::closeSocket(*_sock); }
            ////////////
            /// @brief Sets non-blocking a socket. Can throw exception depending on error
            /// @param[in]  non_blocking defaults to true
            /// @return 
            ////////////
            inline void set_nonblocking(bool non_blocking = true)
            {
                unsigned long mode = (non_blocking ? 1 : 0);
                ioctlsocket(Socket::cmd_name::fionbio, &mode);
            }
            ////////////
            /// @brief Gets the native socket
            /// @return The native socket (can be used in standard C network functions like recv)
            ////////////
            inline Socket::socket_t get_native_socket() const { return *_sock; }
            ////////////
            /// @brief Builds a socket from a native socket type
            /// @param[in]  s The native socket
            /// @return 
            ////////////
            basic_socket(Socket::socket_t s) : _sock(new Socket::socket_t(s), SocketDeleter()) {}
            ////////////
        protected:
            std::shared_ptr<Socket::socket_t> _sock; ///< The socket value used for various network functions
            ////////////
            /// @brief Is the current object valid ?  Closed or not allocated.
            /// @return Is valid
            ////////////
            inline bool isvalid() const { return (_sock.get() != nullptr && *_sock != Socket::invalid_socket); }
            ////////////
            /// @brief 
            ////////////
            basic_socket()                    = default;
            ////////////
            /// @brief 
            ////////////
            basic_socket(basic_socket const&) = default;
            ////////////
            /// @brief 
            ////////////
            basic_socket(basic_socket &&)     = default;
            /// @brief 
            ////////////
            basic_socket& operator =(basic_socket const&) = default;
            ////////////
            /// @brief 
            ////////////
            basic_socket& operator =(basic_socket &&) = default;

            ////////////
            /// @brief Creates a new socket for the current object. Can throw exception depending on error
            /// @param[in]  af    The socket address family
            /// @param[in]  type  The socket type family
            /// @param[in]  proto The socket protocol family
            /// @return 
            ////////////
            inline void socket(Socket::address_family af, Socket::types type, Socket::protocols proto) { _sock.reset(new Socket::socket_t(Socket::socket(af, type, proto)), SocketDeleter()); }
            ////////////
            /// @brief Resets the internal native socket value
            /// @param[in]  s The new native socket value
            /// @return 
            ////////////
            inline void reset_socket(Socket::socket_t s) { _sock.reset(new Socket::socket_t(s), SocketDeleter()); }
    };

    ////////////
    /// @brief Templated connected_socket class, this uses recv & send.
    ////////////
    template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
    class connected_socket : public basic_socket
    {
        public:
            ////////////
            /// @brief The current object address type
            ////////////
            using myaddr_t = Addr;
            ////////////
            /// @brief The current object type
            ////////////
            using mytype_t = connected_socket<Addr, family, type, proto>;

        protected:
            ////////////
            /// @brief Used to build socket after a connect
            ////////////
            connected_socket(Socket::socket_t s);
            myaddr_t _addr; ///< Representing the socket address

        public:
            ////////////
            /// @brief 
            ////////////
            connected_socket();
            ////////////
            /// @brief 
            ////////////
            connected_socket(connected_socket const&)      = default;
            ////////////
            /// @brief 
            ////////////
            connected_socket(connected_socket &&) noexcept = default;
            ////////////
            /// @brief 
            ////////////
            connected_socket& operator=(connected_socket const&)     = default;
            ////////////
            /// @brief 
            ////////////
            connected_socket& operator=(connected_socket&&) noexcept = default;
            ////////////
            /// @brief 
            ////////////
            virtual ~connected_socket() = default;

            ////////////
            /// @brief Gets the address object
            /// @return The address object
            ////////////
            inline myaddr_t const& get_addr() const;
            ////////////
            /// @brief Start listenning for connections, a previous call to bind is needed
            /// @param[in]  waiting_socks Number of waiting connections
            /// @return 
            ////////////
            inline void listen(int waiting_socks = 5);
            ////////////
            /// @brief Connects to a peer
            /// @param[in]  addr The peer address
            /// @return 
            ////////////
            inline void connect(myaddr_t const& addr);
            ////////////
            /// @brief Accepts a connecting peer
            /// @return A new socket object
            ////////////
            inline mytype_t accept();
            ////////////
            /// @brief Creates a new socket, freeing the previous socket
            /// @return 
            ////////////
            inline void socket();
            ////////////
            /// @brief Binds the socket to an address
            /// @param[in]  addr The address to bind to
            /// @return 
            ////////////
            inline void bind(myaddr_t const& addr);
            ////////////
            /// @brief Receives datas
            /// @param[out] buffer Pointer to a buffer that will receive the datas
            /// @param[in]  len    Maximum size that can be written to buffer
            /// @param[in]  flags  Receive flags, defaults to Socket::socket_flags::normal
            /// @return Size of the received data
            ////////////
            size_t recv(void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
            ////////////
            /// @brief Sends datas
            /// @param[in]  buffer Pointer to a buffer that contains the datas
            /// @param[in]  len    Size in bytes to send (buffer must be at least this big)
            /// @param[in]  flags  Send flags, defaults to Socket::socket_flags::normal
            /// @return Size of the received data
            ////////////
            size_t send(const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
    };

    ////////////
    /// @brief Templated unconnected_socket class, this uses recvfrom & sendto.
    ////////////
    template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
    class unconnected_socket : public basic_socket
    {
        public:
            ////////////
            /// @brief The current object address type
            ////////////
            using myaddr_t = Addr;
            ////////////
            /// @brief The current object type
            ////////////
            using mytype_t = unconnected_socket<Addr, family, type, proto>;
            
        protected:
            myaddr_t _addr; ///< Representing the socket address

        public:
            ////////////
            /// @brief 
            ////////////
            unconnected_socket();
            ////////////
            /// @brief 
            ////////////
            unconnected_socket(unconnected_socket const&)      = default;
            ////////////
            /// @brief 
            ////////////
            unconnected_socket(unconnected_socket &&) noexcept = default;
            ////////////
            /// @brief 
            ////////////
            unconnected_socket& operator=(unconnected_socket const&)      = default;
            ////////////
            /// @brief 
            ////////////
            unconnected_socket& operator=(unconnected_socket &&) noexcept = default;
            ////////////
            /// @brief 
            ////////////
            virtual ~unconnected_socket() = default;

            ////////////
            /// @brief Gets the address object
            /// @return The address object
            ////////////
            inline myaddr_t const& get_addr() const;
            ////////////
            /// @brief Creates a new socket, freeing the previous socket
            /// @return 
            ////////////
            inline void socket();
            ////////////
            /// @brief Binds the socket to an address
            /// @param[in]  addr The address to bind to
            /// @return 
            ////////////
            inline void bind(myaddr_t const& addr);
            ////////////
            /// @brief Receives datas from a peer
            /// @param[out] addr   Address of the peer you received from
            /// @param[out] buffer Pointer to a buffer that will receive the datas
            /// @param[in]  len    Maximum size that can be written to buffer
            /// @param[in]  flags  Receive flags, defaults to Socket::socket_flags::normal
            /// @return Size of the received data
            ////////////
            size_t recvfrom(basic_addr& addr, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
            ////////////
            /// @brief Sends datas to a peer
            /// @param[in]  addr   Address of the peer you send to
            /// @param[in]  buffer Pointer to a buffer that contains the datas
            /// @param[in]  len    Size in bytes to send (buffer must be at least this big)
            /// @param[in]  flags  Send flags, defaults to Socket::socket_flags::normal
            /// @return Size of the received data
            ////////////
            size_t sendto(const basic_addr & addr, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
    };

#include "basic_socket.inl"
}

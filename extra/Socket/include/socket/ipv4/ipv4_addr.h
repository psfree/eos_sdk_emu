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

#ifndef _IPV4_ADDR_INCLUDED_
#define _IPV4_ADDR_INCLUDED_

#include "../common/basic_socket.h"

#ifdef UTILS_OS_WINDOWS
  #ifndef s_addr
    #define s_addr S_un.S_addr
  #endif
#endif

namespace PortableAPI
{
    ////////////
    /// @brief An IPV4 sock_addr object
    ////////////
    class ipv4_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_in;

        private:
            my_sockaddr *_sockaddr;

        public:
            inline ipv4_addr() : _sockaddr(new my_sockaddr()) { _sockaddr->sin_family = static_cast<uint16_t>(Socket::address_family::inet); }
            inline ipv4_addr(ipv4_addr const& other) : _sockaddr(new my_sockaddr) { memcpy(_sockaddr, other._sockaddr, len()); }
            inline ipv4_addr(ipv4_addr && other) noexcept : _sockaddr(nullptr) { std::swap(_sockaddr, other._sockaddr); }
            inline ipv4_addr& operator =(ipv4_addr const& other) { memcpy(_sockaddr, other._sockaddr, len()); return *this; }
            inline ipv4_addr& operator =(ipv4_addr&& other) noexcept { std::swap(_sockaddr, other._sockaddr); return *this; }

            inline virtual ~ipv4_addr() { delete _sockaddr; }
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <ip>[:<port>]
            ////////////
            virtual std::string to_string(bool with_port = false) const
            {
                std::string res;
                Socket::inet_ntop(Socket::address_family::inet, &_sockaddr->sin_addr, res);
                if (with_port)
                {
                    res.push_back(':');
                    res += std::to_string(utils::Endian::net_swap(_sockaddr->sin_port));
                }

                return res;
            }
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str Pass in a formated string like <ip>[:<port>]
            /// @return false failed to parse, true succeeded to parse
            ////////////
            virtual bool from_string(std::string const& str)
            {
                size_t pos = str.find(':');

                if (pos != std::string::npos)
                {
                    std::string tmp(str);
                    tmp[pos] = 0;
                    const char* ip = tmp.c_str();
                    const char* port = &tmp[pos + 1];

                    if (Socket::inet_pton(Socket::address_family::inet, ip, &_sockaddr->sin_addr) != 1)
                        return false;

                    set_port(static_cast<uint16_t>(strtoul(port, nullptr, 10)));
                }
                else
                {
                    if (Socket::inet_pton(Socket::address_family::inet, str, &_sockaddr->sin_addr) != 1)
                        return false;
                }

                return true;
            }
            ////////////
            /// @brief Gets the generic sockaddr ref
            /// @return The sockaddr ref
            ////////////
            inline virtual sockaddr& addr(){ return *reinterpret_cast<sockaddr*>(_sockaddr); }
            ////////////
            /// @brief Gets the generic const sockaddr ref
            /// @return The const sockaddr ref
            ////////////
            inline virtual sockaddr const& addr() const { return *reinterpret_cast<sockaddr*>(_sockaddr); }
            ////////////
            /// @brief Get the sockaddr size
            /// @return sockaddr size
            ////////////
            inline virtual size_t len() const { return sizeof(my_sockaddr); }
            ////////////
            /// @brief Sets the IPV4 ip, will be network ordered
            /// @param[in]  addr The IPV4 ip
            /// @return 
            ////////////
            inline void set_ip(uint32_t ip) { _sockaddr->sin_addr.s_addr = utils::Endian::net_swap(ip); }
            ////////////
            /// @brief Sets the IPV4 addr, it will not be re-ordered
            /// @param[in]  addr The IPV4 addr
            /// @return 
            ////////////
            inline void set_addr(in_addr const& addr) { _sockaddr->sin_addr = addr; }
            ////////////
            /// @brief Sets the IPV4 port
            /// @param[in]  port The IPV4 port
            /// @return 
            ////////////
            inline void set_port(uint16_t port){ _sockaddr->sin_port = utils::Endian::net_swap(port); }
            ////////////
            /// @brief Gets the IPV4 ip
            /// @return The IPV4 ip
            ////////////
            inline uint32_t get_ip() const{ return utils::Endian::net_swap(_sockaddr->sin_addr.s_addr); }
            ////////////
            /// @brief Gets the IPV4 addr
            /// @return The IPV4 addr
            ////////////
            inline in_addr get_addr() const { return _sockaddr->sin_addr; }
            ////////////
            /// @brief Gets the IPV4 port
            /// @return The IPV4 port
            ////////////
            inline uint16_t get_port() const { return utils::Endian::net_swap(_sockaddr->sin_port); }
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            inline my_sockaddr& get_native_addr(){ return *_sockaddr; }
            ////////////
            /// @brief Sets the current addr to any
            /// @return 
            ////////////
            inline void set_any_addr() { _sockaddr->sin_addr.s_addr = INADDR_ANY; }
            ////////////
            /// @brief Sets the current addr to loopback
            /// @return 
            ////////////
            inline void set_loopback_addr() { _sockaddr->sin_addr.s_addr = utils::Endian::net_swap(INADDR_LOOPBACK); }
            ////////////
            /// @brief Sets the current addr to broadcast
            /// @return 
            ////////////
            inline void set_broadcast_addr() { _sockaddr->sin_addr.s_addr = INADDR_BROADCAST; }
    };
}
#endif

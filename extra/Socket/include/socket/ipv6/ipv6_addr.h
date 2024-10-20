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

#ifndef _IPV6_ADDR_INCLUDED_
#define _IPV6_ADDR_INCLUDED_

#include "../common/basic_socket.h"

namespace PortableAPI
{
    ////////////
    /// @brief An IPV6 sock_addr object
    ////////////
    class ipv6_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_in6;

        private:
            my_sockaddr *_sockaddr;

        public:
            inline ipv6_addr() : _sockaddr(new my_sockaddr()) { _sockaddr->sin6_family = static_cast<uint16_t>(Socket::address_family::inet6); }
            inline ipv6_addr(ipv6_addr const& other) : _sockaddr(new my_sockaddr) { memcpy(_sockaddr, other._sockaddr, len()); }
            inline ipv6_addr(ipv6_addr&& other) noexcept : _sockaddr(nullptr) { std::swap(_sockaddr, other._sockaddr); }
            inline ipv6_addr& operator=(ipv6_addr const& other) { memcpy(_sockaddr, other._sockaddr, len()); return *this; }
            inline ipv6_addr& operator=(ipv6_addr&& other) noexcept { std::swap(_sockaddr, other._sockaddr); return *this; }

            inline virtual ~ipv6_addr() { delete _sockaddr; }
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <ip> or [<ip>]:port
            ////////////
            virtual std::string to_string(bool with_port = false) const
            {
                std::string res;
                Socket::inet_ntop(Socket::address_family::inet6, &_sockaddr->sin6_addr, res);
                if (with_port)
                {
                    res.insert(res.begin(), '[');
                    res += "]:" + std::to_string(utils::Endian::net_swap(_sockaddr->sin6_port));
                }

                return res;
            }
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str <ip> or [<ip>]:<port> (brackets needed)
            /// @return false failed to parse, true succeeded to parse
            ////////////
            virtual bool from_string(std::string const& str)
            {
                bool success = false;
                addrinfo* info = nullptr;
                addrinfo hints = {};
                hints.ai_family = (int)PortableAPI::Socket::address_family::inet6;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

                size_t sep_pos = 0;
                std::string ip;
                int sep_count = 0;
                for (size_t i = 0; i < str.length(); ++i)
                {
                    if (str[i] == ':')
                    {
                        sep_pos = i;
                        ++sep_count;
                    }
                }
                if (sep_count == 8)
                {
                    ip = std::move(std::string(str.begin(), str.begin() + sep_pos));
                }
                else
                {
                    ip = str;
                }

                if (PortableAPI::Socket::getaddrinfo(ip.c_str(), nullptr, &hints, &info) == 0)
                {
                    my_sockaddr* maddr = (my_sockaddr*)info->ai_addr;

                    size_t pos = str.find(']');
                    std::string str_port("0");

                    if (pos != std::string::npos)
                    {
                        str_port = std::move(std::string(str.begin() + pos + 2, str.end()));
                    }
                    else if (sep_count == 8)
                    {
                        str_port = std::move(std::string(str.begin() + sep_pos + 1, str.end()));
                    }

                    try
                    {
                        int port = std::stoi(str_port);
                        if (port >= 0 && port <= 65535)
                        {
                            set_addr(maddr->sin6_addr);
                            set_port(port);

                            success = true;
                        }
                    }
                    catch (...)
                    {
                    }
                }

                if (info != nullptr)
                    PortableAPI::Socket::freeaddrinfo(info);

                return success;
            }
            ////////////
            /// @brief Gets the generic sockaddr ref
            /// @return The sockaddr ref
            ////////////
            inline virtual sockaddr& addr() { return *reinterpret_cast<sockaddr*>(_sockaddr); }
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
            /// @brief Sets the IPV6 addr, will be network ordered
            /// @param[in]  addr The IPV6 addr
            /// @return 
            ////////////
            inline void set_ip(in6_addr const& addr) { _sockaddr->sin6_addr = addr; }
            ////////////
            /// @brief Sets the IPV6 addr, it will not be re-ordered
            /// @param[in]  addr The IPV6 addr
            /// @return 
            ////////////
            inline void set_addr(in6_addr const& addr) { _sockaddr->sin6_addr = addr; }
            ////////////
            /// @brief Sets the IPV6 port
            /// @param[in]  port The IPV6 port
            /// @return 
            ////////////
            inline void set_port(uint16_t port) { _sockaddr->sin6_port = utils::Endian::net_swap(port); }
            ////////////
            /// @brief Gets the IPV6 ip
            /// @return The IPV6 ip
            ////////////
            inline in6_addr get_ip() const { return _sockaddr->sin6_addr; }
            ////////////
            /// @brief Gets the IPV6 addr
            /// @return The IPV6 addr
            ////////////
            inline in6_addr get_addr() const { return _sockaddr->sin6_addr; }
            ////////////
            /// @brief Gets the IPV6 port
            /// @return The IPV6 port
            ////////////
            inline uint16_t get_port() const { return utils::Endian::net_swap(_sockaddr->sin6_port); }
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            my_sockaddr& get_native_addr() { return *_sockaddr; }
            ////////////
            /// @brief Sets the current addr to any
            /// @return 
            ////////////
            inline void set_any_addr() { _sockaddr->sin6_addr = IN6ADDR_ANY_INIT; }
            ////////////
            /// @brief Sets the current addr to loopback
            /// @return 
            ////////////
            inline void set_loopback_addr() { _sockaddr->sin6_addr = IN6ADDR_LOOPBACK_INIT; }
    };
}
#endif

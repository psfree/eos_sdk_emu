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

#ifndef _UNIX_ADDR_INCLUDED_
#define _UNIX_ADDR_INCLUDED_

#include "../common/basic_socket.h"

#if defined(UTILS_OS_WINDOWS)
    #include <afunix.h>
#endif

#ifndef UNIX_PATH_MAX
  #define UNIX_PATH_MAX 108
#endif

namespace PortableAPI
{
    ////////////
    /// @brief An UNIX sock_addr object
    ////////////
    class unix_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_un;

        private:
            my_sockaddr *_sockaddr;

        public:
            inline unix_addr() : _sockaddr(new my_sockaddr()) { _sockaddr->sun_family = static_cast<uint16_t>(Socket::address_family::unix); }
            inline unix_addr(unix_addr const& other) : _sockaddr(new my_sockaddr) { memcpy(_sockaddr, other._sockaddr, len()); }
            inline unix_addr(unix_addr && other) noexcept : _sockaddr(nullptr) { std::swap(_sockaddr, other._sockaddr); }
            inline unix_addr& operator =(unix_addr const& other) { memcpy(_sockaddr, other._sockaddr, len()); return *this; }
            inline unix_addr& operator =(unix_addr&& other) noexcept { std::swap(_sockaddr, other._sockaddr); return *this; }
                
            inline virtual ~unix_addr() { delete _sockaddr; }
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <path>
            ////////////
            inline virtual std::string to_string(bool with_port = false) const { return get_addr(); }
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str Pass in a formated string like <path>
            /// @return false if failed to parse, true if succeeded to parse
            ////////////
            inline virtual bool from_string(std::string const& str) { set_addr(str); return true; }
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
            /// @brief Sets the path
            /// @param[in]  addr The path
            /// @return 
            ////////////
            inline void set_addr(std::string const& path)
            {
                path.copy(_sockaddr->sun_path, UNIX_PATH_MAX);
                auto i = (path.length() >= UNIX_PATH_MAX ? (UNIX_PATH_MAX - 1) : path.length());
                _sockaddr->sun_path[i] = '\0';
            }
            ////////////
            /// @brief Gets the path
            /// @return The path
            ////////////
            inline std::string get_addr() const { return std::string{ _sockaddr->sun_path }; }
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            inline my_sockaddr& get_native_addr() { return *_sockaddr; }
    };
}
#endif

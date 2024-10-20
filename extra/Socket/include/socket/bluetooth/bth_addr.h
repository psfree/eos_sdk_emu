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

#ifndef _BTH_ADDR_INCLUDED_
#define _BTH_ADDR_INCLUDED_

#include "bluetooth_socket.h"

namespace PortableAPI
{
    ////////////
    /// @brief A bluetooth sock_addr object
    ////////////
    class bth_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_rc;

        private:
            my_sockaddr *_sockaddr;

        public:
            inline bth_addr() : _sockaddr(new my_sockaddr())
            {
            #if defined(UTILS_OS_WINDOWS)
                _sockaddr->addressFamily = static_cast<uint16_t>(BluetoothSocket::address_family::bth);
            #elif defined(UTILS_OS_LINUX)
                _sockaddr->rc_family = static_cast<uint8_t>(BluetoothSocket::address_family::bth);
            #endif
            }
            inline bth_addr(bth_addr const& other) : _sockaddr(new my_sockaddr) { memcpy(_sockaddr, other._sockaddr, len()); }
            inline bth_addr(bth_addr && other) noexcept : _sockaddr(nullptr) { std::swap(_sockaddr, other._sockaddr); }
            inline bth_addr& operator =(bth_addr const& other){ memcpy(_sockaddr, other._sockaddr, len());  return *this; }
            inline bth_addr& operator =(bth_addr&& other) noexcept { std::swap(_sockaddr, other._sockaddr); return *this; }

            inline virtual ~bth_addr() { delete _sockaddr; }
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <addr>[@<channel>]
            ////////////
            virtual std::string to_string(bool with_port = false) const
            {
                std::string res;

            #if defined(UTILS_OS_WINDOWS)
                BluetoothSocket::inet_ntop(BluetoothSocket::address_family::bth, &_sockaddr->btAddr, res);
            #elif defined(UTILS_OS_LINUX)
                BluetoothSocket::inet_ntop(BluetoothSocket::address_family::bth, &_sockaddr->rc_bdaddr, res);
            #endif

                if (with_port)
                {
                    res.push_back('@');
            #if defined(UTILS_OS_WINDOWS)
                    res += std::to_string(_sockaddr->port);
            #elif defined(UTILS_OS_LINUX)
                    res += std::to_string(_sockaddr->rc_channel);
            #endif
                }

                return res;
            }
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str Pass in a formated string like <addr>[@<channel>]
            /// @return false failed to parse, true succeeded to parse
            ////////////
            virtual bool from_string(std::string const& str)
            {
                size_t pos = str.find('@');

                if (pos != std::string::npos)
                {
                    std::string ip = str.substr(0, pos);
                    std::string channel = str.substr(pos + 1);
                #if defined(UTILS_OS_WINDOWS)
                    if (BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, ip, &_sockaddr->btAddr) != 1)
                        return false;
                #elif defined(UTILS_OS_LINUX)
                    if (BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, ip, &_sockaddr->rc_bdaddr) != 1)
                        return false;
                #endif
                    set_channel(static_cast<uint8_t>(stoi(channel)));
                }
                else
                {
                #if defined(UTILS_OS_WINDOWS)
                    if (BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, str, &_sockaddr->btAddr) != 1)
                        return false;
                #elif defined(UTILS_OS_LINUX)
                    if (BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, str, &_sockaddr->rc_bdaddr) != 1)
                        return false;
                #endif
                }

                return true;
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
            /// @brief Sets the addr
            /// @param[in]  addr The bluetooth addr
            /// @return 
            ////////////
            inline void set_addr(bdaddr_t const& addr)
            {
            #if defined(UTILS_OS_WINDOWS)
                _sockaddr->btAddr = addr;
            #elif defined(UTILS_OS_LINUX)
                _sockaddr->rc_bdaddr = addr;
            #endif
            }
            ////////////
            /// @brief Sets the buetooth channel
            /// @param[in]  channel The bluetooth channel
            /// @return 
            ////////////
            inline void set_channel(uint8_t channel)
            {
            #if defined(UTILS_OS_WINDOWS)
                _sockaddr->port = channel;
            #elif defined(UTILS_OS_LINUX)
                _sockaddr->rc_channel = channel;
            #endif
            }
            ////////////
            /// @brief Gets the Bluetooth addr
            /// @return The Bluetooth addr
            ////////////
            inline bdaddr_t get_addr() const
            {
            #if defined(UTILS_OS_WINDOWS)
                return _sockaddr->btAddr;
            #elif defined(UTILS_OS_LINUX)
                return _sockaddr->rc_bdaddr;
            #endif
            }
            ////////////
            /// @brief Gets the Bluetooth channel
            /// @return The Bluetooth channel
            ////////////
            inline uint8_t get_channel() const
            {
            #if defined(UTILS_OS_WINDOWS)
                return static_cast<uint8_t>(_sockaddr->port);
            #elif defined(UTILS_OS_LINUX)
                return _sockaddr->rc_channel;
            #endif
            }
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            inline my_sockaddr& get_native_addr() { return *_sockaddr; }
            ////////////
            /// @brief Sets the current addr to any
            /// @return 
            ////////////
            inline void set_any_addr()
            {
#if defined(UTILS_OS_WINDOWS)
                _sockaddr->btAddr = { 0 };
#elif defined(UTILS_OS_LINUX)
                _sockaddr->rc_bdaddr = { 0 };
#endif
            }
    };
}
#endif

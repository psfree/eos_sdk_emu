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

#include "basic_socket.h"

namespace PortableAPI
{
    class basic_socket;
    ////////////
    /// @brief A class to manage a socket poll. Can handle a poll with index or with socket address
    ////////////
    class Poll
    {
        std::vector<pollfd>::iterator get_poll_from_socket(basic_socket const& sock)
        {
            auto it = _polls.begin();
            Socket::socket_t sockfd = sock.get_native_socket();
            for (; it != _polls.end(); ++it)
            {
                if (sockfd == it->fd)
                    break;
            }

            return it;
        }

        std::vector<pollfd> _polls;

    public:
        ////////////
        /// @brief Adds a socket into the poll
        /// @param[in] sock The socket to add
        /// @param[in] flags new event flags
        /// @return Has the socket been added to the poll
        ////////////
        inline bool add_socket(basic_socket const& sock, Socket::poll_flags flags = Socket::poll_flags::none)
        {
            if(get_poll_from_socket(sock) != _polls.end())
                return false;
            
            _polls.emplace_back(pollfd{ sock.get_native_socket(), static_cast<int16_t>(flags), 0 });
            return true;
        }
        ////////////
        /// @brief Removes a socket from the poll
        /// @param[in] sock The socket to remove
        /// @return Has the socket been removed from the poll 
        ////////////
        inline bool remove_socket(basic_socket const& sock)
        {
            auto it = get_poll_from_socket(sock);
            if (it == _polls.end())
                return false;

            _polls.erase(it);
            return true;
        }
        ////////////
        /// @brief Get the number of sockets currently in the poll
        /// @return Number of sockets in the poll
        ////////////
        inline size_t get_num_polls() const
        {
            return _polls.size();
        }
        ////////////
        /// @brief Set a poll event for the socket
        /// @param[in] sock The socket to change the flags to
        /// @param[in] flags new event flags
        /// @return Has the events been set on the poll's socket
        ////////////
        inline bool set_events(basic_socket const& sock, Socket::poll_flags flags)
        {
            auto it = get_poll_from_socket(sock);
            if (it == _polls.end())
                return false;

            it->events = static_cast<uint16_t>(flags);
            return true;
        }
        ////////////
        /// @brief Get a poll revents for the socket
        /// @param[in] sock The socket to get the flags from
        /// @return The poll revents flags, returns also none is sock is not in the poll
        ////////////
        inline Socket::poll_flags get_revents(basic_socket const& sock)
        {
            auto it = get_poll_from_socket(sock);
            if (it == _polls.end())
                return Socket::poll_flags::none;

            return static_cast<Socket::poll_flags>(it->revents);
        }
        ////////////
        /// @brief Start the socket poll
        /// @param[in] timeout_ms <0, block, 0 returns now, >0 The time in milliseconds to wait.
        /// @return The number of sockets that have revents
        ////////////
        inline int poll(int timeout_ms) { return Socket::poll(_polls.data(), _polls.size(), timeout_ms); }
        ////////////
        /// @brief Clear the poll of its sockets
        /// @return 
        ////////////
        inline void clear() { _polls.clear(); }
    };
}
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

#ifndef _UNIX_SOCKET_INCLUDED_
#define _UNIX_SOCKET_INCLUDED_

#include "unix_addr.h"
#include "../common/basic_socket.h"

namespace PortableAPI
{
    ////////////
    /// @brief Template specialization for unix socket
    ////////////
    using unix_socket = connected_socket<unix_addr, Socket::address_family::unix, Socket::types::stream, static_cast<Socket::protocols>(0)>;
}
#endif

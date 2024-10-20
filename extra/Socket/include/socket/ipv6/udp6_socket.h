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

#ifndef _UDP6_SOCKET_INCLUDED_
#define _UDP6_SOCKET_INCLUDED_

#include "ipv6_addr.h"
#include "../common/basic_socket.h"

namespace PortableAPI
{
    using udp6_socket = unconnected_socket<ipv6_addr, Socket::address_family::inet6, Socket::types::dgram, Socket::protocols::udp>;
}

#endif

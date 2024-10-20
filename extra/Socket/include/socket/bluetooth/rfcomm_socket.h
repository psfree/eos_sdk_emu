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

#ifndef _RFCOMM_SOCKET_INCLUDED_
#define _RFCOMM_SOCKET_INCLUDED_

#include "bth_addr.h"
#include "../common/basic_socket.h"

namespace PortableAPI
{
    // Connect by uuid:
    //PortableAPI::Socket::InitSocket();
    //
    //PortableAPI::bth_addr addr;
    //addr.from_string("00:11:22:33:44:55");
    //
    //PortableAPI::Uuid uuid;
    //uuid.from_string("00112233-4455-6677-8899001122334455");
    //
    //PortableAPI::rfcomm_socket rfsock;
    //addr.set_channel(PortableAPI::BluetoothSocket::scanOpenPortFromUUID(uuid, addr.get_ip()));
    //rfsock.connect(addr);

    // Register SDP service:
    //PortableAPI::Socket::InitSocket();
    //PortableAPI::rfcomm_socket rfsock;
    //PortableAPI::rfcomm_addr addr;
    //PortableAPI::SDPService service;
    //PortableAPI::Uuid uuid;
    //uuid.from_string("00112233-4455-6677-8899001122334455");
    //
    //addr.from_string("00:11:22:33:44:55");
    //int port; // Port 0 does bind but is invalid, start at 1
    //for (port = 1; port < 32; ++port)
    //{
    //    addr.set_channel(port);
    //    try
    //    {
    //        rfsock.bind(addr);
    //        rfsock.listen();
    //        service.registerService(uuid, port, "service name", "service provider", "service description");
    //        break;
    //    }
    //    catch (...)
    //    {
    //    }
    //}

    ////////////
    /// @brief Template specialization for Bluetooth rfcomm socket
    ////////////
    using rfcomm_socket = connected_socket<bth_addr,
        static_cast<Socket::address_family>(BluetoothSocket::address_family::bth),
        Socket::types::stream,
        static_cast<Socket::protocols>(BluetoothSocket::protocols::rfcomm)
    >;
}

#endif

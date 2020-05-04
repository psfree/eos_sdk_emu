/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Epic Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * The Nemirtingas's Epic Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Epic Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common_includes.h"

class LOCAL_API Network
{
public:
    using channel_t = int32_t;

private:
    static constexpr uint16_t network_port = 56789;
    static constexpr uint16_t max_network_port = (network_port + 10);

    PortableAPI::udp_socket _udp_socket;
    PortableAPI::udp_socket _query_socket;

    std::map<uint64_t, PortableAPI::ipv4_addr> _peers_addrs;

    std::map<Network_Message_pb::MessagesCase, std::map<channel_t, std::vector<IRunFrame*>>> _network_listeners;

    std::recursive_mutex local_mutex;
    
    std::atomic_bool _stop_thread;
    std::thread _network_thread;

    void network_thread();

    std::map<channel_t, std::list<Network_Message_pb>> _network_msgs;

    std::map<uint64_t, channel_t> _default_channels;

public:
    std::atomic_bool _query_started;

    Network();
    ~Network();

    //PortableAPI::ipv4_addr const& get_steamid_addr(uint64 steam_id);

    void set_default_channel(uint64_t peerid, channel_t default_channel);

    void register_listener  (IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type);
    void unregister_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type);

    bool CBRunFrame(channel_t channel, Network_Message_pb::MessagesCase MessageFilter = Network_Message_pb::MessagesCase::MESSAGES_NOT_SET);

    bool SendBroadcast(Network_Message_pb& msg);
    std::set<uint64_t> SendToAllPeers(Network_Message_pb& msg);
    bool SendTo(Network_Message_pb& msg);
};
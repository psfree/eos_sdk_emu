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

#include "network.h"
#include "eos_platform.h"

using namespace PortableAPI;

Network::Network():
    _udp_socket(),
    _stop_thread(false),
    _network_thread(&Network::network_thread, this)
{
}

Network::~Network()
{
    _stop_thread = true;
    _network_thread.join();
}

void Network::network_thread()
{
    uint16_t i;
    size_t len;
    ipv4_addr addr;
    int broadcast = 1;
    std::vector<uint8_t> buffer(4096);
    Network_Message_pb msg;

    _udp_socket.setsockopt(Socket::level::sol_socket, Socket::option_name::so_broadcast, &broadcast, sizeof(broadcast));
    _udp_socket.set_nonblocking();

    // TODO: Use a random port when I'll have a steam server running up
    //std::uniform_int_distribution<int64_t> dis;
    //std::mt19937_64& gen = get_gen();
    //dis(gen);

    addr.set_addr(ipv4_addr::any_addr);

    for (i = network_port; i < max_network_port; ++i)
    {
        addr.set_port(i);
        try
        {
            _udp_socket.bind(addr);
            break;
        }
        catch (...)
        {
        }
    }
    if (i == max_network_port)
    {
        LOG(Log::LogLevel::ERR, "Failed to start network");
        _stop_thread = true;
    }
    else
    {
        LOG(Log::LogLevel::INFO, "Network socket started on port: %hu", i);
    }

    while (!_stop_thread)
    {
        try
        {
            len = _udp_socket.recvfrom(addr, buffer.data(), buffer.size());
        }
        catch (socket_exception & e)
        {
            LOG(Log::LogLevel::WARN, "Udp socket exception: %s", e.what());
            len = 0;
        }
        if (len > 0)
        {
            LOG(Log::LogLevel::TRACE, "Received message from: %s", addr.to_string().c_str());
            if (msg.ParseFromArray(buffer.data(), len))
            {
                LOCAL_LOCK();
                //if (msg.relay())
                //{// We are relaying a message from another user
                //    ipv4_addr real_addr;
                //    real_addr.set_ip(msg.real_ip());
                //    real_addr.set_port(msg.real_port());
                //    _peers_addrs[msg.source_id()] = real_addr;
                //}
                //else
                {
                    _peers_addrs[msg.source_id()] = addr;
                }

                if (msg.dest_id() == 0)
                {
                    for(auto &channel : _default_channels)
                        _network_msgs[channel.second].emplace_back(msg);
                }
                else
                {
                    _network_msgs[_default_channels[msg.dest_id()]].emplace_back(std::move(msg));
                }
            }
        }
    }
}

//PortableAPI::ipv4_addr const& Network::get_steamid_addr(uint64 steam_id)
//{
//    LOCAL_LOCK();
//    return _peers_addrs[steam_id];
//}

void Network::set_default_channel(uint64_t peerid, channel_t default_channel)
{
    _default_channels[peerid] = default_channel;
}

void Network::register_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type)
{
    LOCAL_LOCK();
    _network_listeners[type][channel].push_back(listener);
}

void Network::unregister_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type)
{
    LOCAL_LOCK();
    auto& listeners = _network_listeners[type][channel];
    listeners.erase(
        std::remove(listeners.begin(), listeners.end(), listener),
        listeners.end());
}

bool Network::CBRunFrame(channel_t channel, Network_Message_pb::MessagesCase MessageFilter)
{
    LOCAL_LOCK();
    bool rerun = false;

    auto& channel_messages = _network_msgs[channel];
    for (auto it = channel_messages.begin(); it != channel_messages.end(); )
    {
        auto msg_case = it->messages_case();
        if (msg_case != Network_Message_pb::MessagesCase::MESSAGES_NOT_SET)
        {
            if (MessageFilter == Network_Message_pb::MessagesCase::MESSAGES_NOT_SET || MessageFilter == msg_case)
            {
                auto& listeners = _network_listeners[msg_case][channel];
                for (auto& item : listeners)
                    item->RunNetwork(*it);

                it = channel_messages.erase(it);

                rerun = true;
            }
            else
            {
                ++it;
            }
        }
        else
        {// Don't care about invalid message
            it = channel_messages.erase(it);
        }
    }

    return rerun;
}

bool Network::SendBroadcast(Network_Message_pb& msg)
{
    std::vector<ipv4_addr> broadcasts = std::move(get_broadcasts());

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::string buffer = std::move(msg.SerializeAsString());
    for (auto& brd : broadcasts)
    {
        for (uint16_t port = network_port; port < max_network_port; ++port)
        {
            brd.set_port(port);
            try
            {
                _udp_socket.sendto(brd, buffer.data(), buffer.length());
                LOG(Log::LogLevel::TRACE, "Send broadcast");
            }
            catch (socket_exception & e)
            {
                LOG(Log::LogLevel::WARN, "Udp socket exception: %s", e.what());
                return false;
            }
        }
    }

    return true;
}

std::set<uint64_t> Network::SendToAllPeers(Network_Message_pb& msg)
{
    std::set<uint64_t> peers_sent_to;

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::for_each(_peers_addrs.begin(), _peers_addrs.end(), [&](std::pair<uint64_t const, PortableAPI::ipv4_addr>& peer_infos)
    {
        msg.set_dest_id(peer_infos.first);

        std::string buffer = std::move(msg.SerializeAsString());
        try
        {
            _udp_socket.sendto(peer_infos.second, buffer.c_str(), buffer.length());
            peers_sent_to.insert(peer_infos.first);
            LOG(Log::LogLevel::TRACE, "Sent message to %s", peer_infos.second.to_string().c_str());
        }
        catch (socket_exception & e)
        {
            LOG(Log::LogLevel::WARN, "Udp socket exception: %s on %s", e.what(), peer_infos.second.to_string().c_str());
        }
    });

    return peers_sent_to;
}

bool Network::SendTo(Network_Message_pb& msg)
{
    auto it = _peers_addrs.find(msg.dest_id());

    if (it == _peers_addrs.end())
    {
        LOG(Log::LogLevel::ERR, "No route to %llu", msg.dest_id());
        return false;
    }

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::string buffer = std::move(msg.SerializeAsString());
    try
    {
        _udp_socket.sendto(it->second, buffer.c_str(), buffer.length());
        LOG(Log::LogLevel::TRACE, "Sent message to %s", it->second.to_string().c_str());
    }
    catch (socket_exception & e)
    {
        LOG(Log::LogLevel::WARN, "Udp socket exception: %s on %s", e.what(), it->second.to_string().c_str());
        return false;
    }

    return true;
}


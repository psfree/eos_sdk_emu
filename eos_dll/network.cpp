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
#include "eossdk_platform.h"

using namespace PortableAPI;

Network::Network():
    _udp_socket(),
    _tcp_self_recv({})
{
    //LOG(Log::LogLevel::DEBUG, "");
    _network_task.run(&Network::network_thread, this);
}

Network::~Network()
{
    //LOG(Log::LogLevel::DEBUG, "");

    _network_task.stop();
    _network_task.join();

    //LOG(Log::LogLevel::DEBUG, "Network Thread Joined");
}

void Network::start_network()
{
    ipv4_addr addr;
    uint16_t port;
    addr.set_addr(ipv4_addr::any_addr);
    for (port = network_port; port < max_network_port; ++port)
    {
        addr.set_port(port);
        try
        {
            _udp_socket.bind(addr);
            break;
        }
        catch (...)
        {
        }
    }
    if (port == max_network_port)
    {
        //LOG(Log::LogLevel::ERR, "Failed to start udp socket");
        _network_task.stop();
    }
    else
    {
        //LOG(Log::LogLevel::INFO, "UDP socket started on port: %hu", port);
        std::uniform_int_distribution<int64_t> dis;
        std::mt19937_64& gen = get_gen();
        int x;
        for (x = 0, port = (dis(gen) % 30000 + 30000); x < 100; ++x, port = (dis(gen) % 30000 + 30000))
        {
            addr.set_port(port);
            try
            {
                _tcp_socket.bind(addr);
                _tcp_socket.listen(32);
                addr.set_addr(ipv4_addr::loopback_addr);
                _tcp_self_send.connect(addr);
                _tcp_self_recv.socket = std::move(_tcp_socket.accept());
                _tcp_self_recv.buffer.resize(1024);
                break;
            }
            catch (...)
            {
                //LOG(Log::LogLevel::WARN, "Failed to start tcp socket on port %hu", x);
            }
        }
        if (x == 100)
        {
            //LOG(Log::LogLevel::ERR, "Failed to start tcp socket");
            _udp_socket.close();
            _network_task.stop();
        }
        else
        {
            _tcp_port = port;
            //LOG(Log::LogLevel::INFO, "TCP socket started after %hu tries on port: %hu", x, port);
        }
    }
}

void Network::stop_network()
{
    _advertise = false;
    _udp_socket.close();
    _tcp_socket.close();
    _tcp_clients.clear();
    _network_msgs.clear();
    _udp_addrs.clear();
}

void Network::build_advertise_msg(Network_Message_pb& msg)
{
    Network_Advertise_pb* advertise = new Network_Advertise_pb;
    Network_Peer_pb* peer_pb = new Network_Peer_pb;

#ifdef _DEBUG
    std::string log_buff("Advertising with peer ids: '");
#endif
    for (auto& id : _my_peer_ids)
    {
#ifdef _DEBUG
        log_buff += id + "' ";
#endif
        peer_pb->add_peer_ids(id);
    }
#ifdef _DEBUG
    //LOG(Log::LogLevel::DEBUG, "%s", log_buff.c_str());
#endif

    advertise->set_allocated_peer(peer_pb);
    msg.set_allocated_network_advertise(advertise);
    msg.set_source_id(*_my_peer_ids.begin());
}

std::pair<PortableAPI::tcp_socket*, std::vector<Network::peer_t>> Network::get_new_peer_ids(Network_Peer_pb const& peer_msg)
{
    std::pair<tcp_socket*, std::vector<peer_t>> peer_ids_to_add;
    peer_ids_to_add.first = nullptr;
    peer_ids_to_add.second.reserve(peer_msg.peer_ids_size());

    for (auto& peer_id : peer_msg.peer_ids())
    {
        auto it = _tcp_peers.find(peer_id);
        if (it == _tcp_peers.end())
        {
            peer_ids_to_add.second.emplace_back(peer_id);
        }
        else if (peer_ids_to_add.first == nullptr)
        {
            peer_ids_to_add.first = it->second;
        }
    }
    return peer_ids_to_add;
}

void Network::do_advertise()
{
    if ((std::chrono::steady_clock::now() - _last_advertise) < std::chrono::milliseconds(2000))
        return;

    try
    {
        std::lock_guard<std::mutex> lk(local_mutex);
        if (_advertise && !_my_peer_ids.empty())
        {
            Network_Message_pb msg;
            Network_Advertise_pb* network = new Network_Advertise_pb;
            Network_Port_pb* port = new Network_Port_pb;

            port->set_port(_tcp_port);
            network->set_allocated_port(port);
            msg.set_allocated_network_advertise(network);
            msg.set_source_id(*_my_peer_ids.begin());

            SendBroadcast(msg);
        }
    }
    catch (...)
    {
        //LOG(Log::LogLevel::DEBUG, "Advertising, failed");
    }
}

void Network::add_new_tcp_client(PortableAPI::tcp_socket* cli, std::vector<peer_t> const& peer_ids, bool advertise)
{
    _poll.add_socket(*cli); // Add the client to the poll
    _poll.set_events(*cli, Socket::poll_flags::in);
    for (auto& peerid : peer_ids)
    {// Map all clients peerids to the socket
        //LOG(Log::LogLevel::DEBUG, "Adding peer id %llu to client %s", peerid, cli->get_addr().to_string(true).c_str());
        _tcp_peers[peerid] = &(*cli);
    }

    if (advertise)
    {
        Network_Message_pb msg;
        Network_Advertise_pb* adv = new Network_Advertise_pb;
        Network_Peer_Accept_pb* accept_peer = new Network_Peer_Accept_pb;

        adv->set_allocated_accept(accept_peer);
        msg.set_allocated_network_advertise(adv);

        std::string buff = std::move(msg.SerializeAsString());

        cli->send(buff.data(), buff.length());
    }
}

void Network::connect_to_peer(ipv4_addr &addr, peer_t const& peer_id)
{
    try
    {
        if (_waiting_out_tcp_clients.count(peer_id) == 0)
        {
            //LOG(Log::LogLevel::DEBUG, "Connecting to %s : %llu", addr.to_string(true).c_str(), peer_id);
            tcp_socket new_client;
            new_client.connect(addr);

            Network_Message_pb msg;
            build_advertise_msg(msg);

            std::string buff = std::move(msg.SerializeAsString());
            new_client.send(buff.c_str(), buff.length());
            new_client.set_nonblocking(true);
            _waiting_out_tcp_clients.emplace(peer_id, std::move(new_client));
        }
    }
    catch (std::exception & e)
    {
        //LOG(Log::LogLevel::WARN, "Failed to TCP connect to %s: %s", addr.to_string().c_str(), e.what());
    }
}

void Network::process_waiting_out_clients()
{
    if (_waiting_out_tcp_clients.empty())
        return;

    std::array<uint8_t, 2048> buffer;
    Network_Message_pb msg;
    size_t len;
    for (auto it = _waiting_out_tcp_clients.begin(); it != _waiting_out_tcp_clients.end(); )
    {
        try
        {
            if ((len = it->second.recv(buffer.data(), buffer.size())) > 0)
            {
                if (msg.ParseFromArray(buffer.data(), len) && msg.has_network_advertise() && msg.network_advertise().has_accept())
                {
                    it->second.set_nonblocking(false);
                    tcp_buffer_t new_buff({});
                    new_buff.socket = std::move(it->second);
                    _tcp_clients.emplace_back(std::move(new_buff));
                    auto new_client_it = _tcp_clients.rbegin();
                    _tcp_peers[it->first] = &(new_client_it->socket);
                    _poll.add_socket(new_client_it->socket);   
                    _poll.set_events(new_client_it->socket, Socket::poll_flags::in);
                }
                // Dropping outgoing connection if we don't accept it
                it = _waiting_out_tcp_clients.erase(it);
            }
            else
            {// Don't have data to read
                ++it;
            }
        }
        catch (std::exception &e)
        {
            // Error while reading, connection closed ?
            //LOG(Log::LogLevel::WARN, "Failed peer pair: %s", e.what());
            it = _waiting_out_tcp_clients.erase(it);
        }
    }
}

void Network::process_waiting_in_client(tcp_socket &new_client)
{
    Poll p;
    p.add_socket(new_client);
    p.set_events(0, Socket::poll_flags::in);
    if (p.poll(500) == 1)
    {
        std::array<uint8_t, 2048> buff;
        Network_Message_pb msg;
        size_t len = new_client.recv(buff.data(), buff.size());
        if (len > 0 &&
            msg.ParseFromArray(buff.data(), len) &&
            msg.source_id() != peer_t() &&
            msg.has_network_advertise() &&
            msg.network_advertise().has_peer())
        {
            auto const& peer_msg = msg.network_advertise().peer();
            std::pair<tcp_socket*, std::vector<peer_t>> peer_ids_to_add = std::move(get_new_peer_ids(peer_msg));
            
            if (!peer_ids_to_add.second.empty())
            {// We have peer ids to add
                if (peer_ids_to_add.first == nullptr)
                {// Didn't find a matching peer id, its a new peer
                    tcp_buffer_t new_buff({});
                    new_buff.socket = std::move(new_client);
                    _tcp_clients.emplace_back(std::move(new_buff));
                    peer_ids_to_add.first = &(_tcp_clients.rbegin()->socket);
                }
                add_new_tcp_client(peer_ids_to_add.first, peer_ids_to_add.second, true);
            }
        }
    }
}

void Network::process_network_message(Network_Message_pb &msg)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    
    if (msg.dest_id() == peer_t())
    {// If we received a message without a destination, then its a broadcast.
        // Add the message to all listeners queue
        for (auto& channel : _default_channels)
            _pending_network_msgs[channel.second].emplace_back(msg);
    }
    else
    {
        _pending_network_msgs[_default_channels[msg.dest_id()]].emplace_back(std::move(msg));
    }
}

void Network::process_udp()
{
    try
    {
        ipv4_addr addr;
        std::array<uint8_t, 4096> buffer;
        Network_Message_pb msg;
        size_t len;
        len = _udp_socket.recvfrom(addr, buffer.data(), buffer.size());
        if (len > 0 && msg.ParseFromArray(buffer.data(), len) && msg.source_id() != peer_t())
        {
            {
                std::lock_guard<std::mutex> lk(local_mutex);
                _udp_addrs[msg.source_id()] = addr;
            }
            //LOG(Log::LogLevel::TRACE, "Received UDP message from: %s - %s", addr.to_string().c_str(), msg.source_id().c_str());
            if (msg.has_network_advertise())
            {
                auto const& advertise = msg.network_advertise();
                if (advertise.has_port())
                {
                    if (!_my_peer_ids.empty() &&
                        _my_peer_ids.count(msg.source_id()) == 0 &&
                        _tcp_peers.count(msg.source_id()) == 0)
                    {
                        ipv4_addr peer_addr;
                        peer_addr.set_ip(addr.get_ip());
                        peer_addr.set_port(advertise.port().port());
                        connect_to_peer(peer_addr, msg.source_id());
                    }
                }
                else if (advertise.has_peer())
                {
                    std::pair<tcp_socket*, std::vector<peer_t>> peer_ids_to_add = std::move(get_new_peer_ids(advertise.peer()));

                    if (peer_ids_to_add.first != nullptr && !peer_ids_to_add.second.empty())
                    {// We have peer ids to add
                        add_new_tcp_client(peer_ids_to_add.first, peer_ids_to_add.second, false);
                    }
                }
            }
            else
            {
                process_network_message(msg);
            }
        }
    }
    catch (socket_exception & e)
    {
        //LOG(Log::LogLevel::WARN, "Udp socket exception: %s", e.what());
    }
}

void Network::process_tcp_listen()
{
    try
    {
        tcp_socket new_client = std::move(_tcp_socket.accept());
        //LOG(Log::LogLevel::DEBUG, "Accepted TCP client %s", new_client.get_addr().to_string().c_str());
        process_waiting_in_client(new_client);
    }
    catch (socket_exception & e)
    {
        //LOG(Log::LogLevel::WARN, "Tcp socket exception: %s", e.what());
    }
}

void Network::process_tcp_data(tcp_buffer_t& tcp_buffer)
{
    ipv4_addr addr;
    Network_Message_pb msg;
    size_t len;

    if (tcp_buffer.next_packet_size == 0)
    {
        unsigned long count = 0;
        tcp_buffer.socket.ioctlsocket(Socket::cmd_name::fionread, &count);
        if (count >= sizeof(tcp_buffer_t::next_packet_size))
        {
            tcp_buffer.socket.recv(&tcp_buffer.next_packet_size, sizeof(tcp_buffer_t::next_packet_size));
            tcp_buffer.next_packet_size = Socket::net_swap(tcp_buffer.next_packet_size); // Re-order the size
            if(tcp_buffer.buffer.size() < tcp_buffer.next_packet_size)
                tcp_buffer.buffer.resize(tcp_buffer.next_packet_size);

            tcp_buffer.received_size = 0;
        }
    }
    if (tcp_buffer.next_packet_size > 0)
    {
        len = tcp_buffer.socket.recv(tcp_buffer.buffer.data() + tcp_buffer.received_size, tcp_buffer.next_packet_size - tcp_buffer.received_size);
        tcp_buffer.received_size += len;
        assert((tcp_buffer.received_size <= tcp_buffer.next_packet_size && "recevied tcp buffer is bigger than what we're waiting for"));
        if (tcp_buffer.received_size == tcp_buffer.next_packet_size)
        {
            tcp_buffer.next_packet_size = 0;
            if (msg.ParseFromArray(tcp_buffer.buffer.data(), tcp_buffer.received_size))
            {
                process_network_message(msg);
            }
            tcp_buffer.buffer.erase(tcp_buffer.buffer.begin(), tcp_buffer.buffer.begin() + tcp_buffer.received_size);
        }
    }
}

Network::tcp_client_iterator Network::process_tcp_client(tcp_client_iterator client)
{
    try
    {
        process_tcp_data(*client);
        ++client;
    }
    catch (std::exception &e)
    {
        //LOG(Log::LogLevel::DEBUG, "TCP Client %s gone: %s", client->socket.get_addr().to_string().c_str(), e.what());
        _poll.remove_socket(client->socket);
        // Remove the peer mappings
        for (auto it = _tcp_peers.begin(); it != _tcp_peers.end();)
        {
            if (it->second == &(client->socket))
            {
                it = _tcp_peers.erase(it);
            }
            else
                ++it;
        }
        client = _tcp_clients.erase(client);
    }

    return client;
}

void Network::network_thread()
{
    int broadcast = 1;
    std::vector<uint8_t> buffer(4096);
    Network_Message_pb msg;

    start_network();

    _udp_socket.setsockopt(Socket::level::sol_socket, Socket::option_name::so_broadcast, &broadcast, sizeof(broadcast));
    //_udp_socket.set_nonblocking();

    if (!_network_task.want_stop())
    {
        _poll.add_socket(_udp_socket);
        _poll.add_socket(_tcp_socket);
        _poll.add_socket(_tcp_self_recv.socket);
        for(auto i = 0; i < _poll.get_num_polls(); ++i)
            _poll.set_events(i, Socket::poll_flags::in);
    }

    while (!_network_task.want_stop())
    {
        do_advertise();

        auto res = _poll.poll(500);
        if (res == 0)
            continue;

        if ((_poll.get_revents(_udp_socket) & Socket::poll_flags::in_hup) != Socket::poll_flags::none)
            process_udp(); // Process udp datas & advertising

        if ((_poll.get_revents(_tcp_socket) & Socket::poll_flags::in_hup) != Socket::poll_flags::none)
            process_tcp_listen(); // Process the waiting incoming peers
        
        if ((_poll.get_revents(_tcp_self_recv.socket) & Socket::poll_flags::in_hup) != Socket::poll_flags::none)
            process_tcp_data(_tcp_self_recv); // Process our TCP message, we are not considered as a classic client as we have 2 sockets for the same peer id
        
        for (auto it = _tcp_clients.begin(); it != _tcp_clients.end();)
        {// Process the multiple tcp clients we have
            auto reevents = _poll.get_revents(it->socket);
            if ((reevents & Socket::poll_flags::hup) != Socket::poll_flags::none)
            {
                it = _tcp_clients.erase(it);
            }
            else if ((reevents & Socket::poll_flags::in_hup) != Socket::poll_flags::none)
            {
                it = process_tcp_client(it);
            }
            else
                ++it;
        }
        
        // We might have found a peer while he didn't find us yet, so begin the connection procedure
        process_waiting_out_clients();
    }
}

void Network::advertise_peer_id(peer_t const& peerid)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    _my_peer_ids.insert(peerid);
    _tcp_peers[peerid] = &_tcp_self_send;
}

void Network::remove_advertise_peer_id(peer_t const& peerid)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    _my_peer_ids.erase(peerid);
    _tcp_peers.erase(peerid);
}

void Network::advertise(bool doit)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    _advertise = doit;
}

bool Network::is_advertising()
{
    std::lock_guard<std::mutex> lk(local_mutex);
    return _advertise;
}

void Network::set_default_channel(peer_t peerid, channel_t default_channel)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    _default_channels[peerid] = default_channel;
}

void Network::register_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    _network_listeners[type][channel].push_back(listener);
}

void Network::unregister_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    auto& listeners = _network_listeners[type][channel];
    listeners.erase(
        std::remove(listeners.begin(), listeners.end(), listener),
        listeners.end());
}

bool Network::CBRunFrame(channel_t channel, Network_Message_pb::MessagesCase MessageFilter)
{
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

    {
        std::lock_guard<std::mutex> lk(local_mutex);
        auto& pending_channel_messages = _pending_network_msgs[channel];
        if(!pending_channel_messages.empty())
            std::move(pending_channel_messages.begin(), pending_channel_messages.end(), std::back_inserter(channel_messages));
    }

    return rerun;
}

bool Network::SendBroadcast(Network_Message_pb& msg)
{
    std::vector<ipv4_addr> broadcasts = std::move(get_broadcasts());

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

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
                ////LOG(Log::LogLevel::TRACE, "Send broadcast");
            }
            catch (socket_exception & e)
            {
                //LOG(Log::LogLevel::WARN, "Udp socket exception: %s", e.what());
                return false;
            }
        }
    }

    return true;
}

std::set<Network::peer_t> Network::UDPSendToAllPeers(Network_Message_pb& msg)
{
    std::lock_guard<std::mutex> lk(local_mutex);

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    std::set<peer_t> peers_sent_to;

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::for_each(_udp_addrs.begin(), _udp_addrs.end(), [&](std::pair<peer_t const, PortableAPI::ipv4_addr>& peer_infos)
    {
        msg.set_dest_id(peer_infos.first);

        std::string buffer = std::move(msg.SerializeAsString());
        try
        {
            _udp_socket.sendto(peer_infos.second, buffer.c_str(), buffer.length());
            peers_sent_to.insert(peer_infos.first);
            ////LOG(Log::LogLevel::TRACE, "Sent message to %s", peer_infos.second.to_string().c_str());
        }
        catch (socket_exception & e)
        {
            //LOG(Log::LogLevel::WARN, "Udp socket exception: %s on %s", e.what(), peer_infos.second.to_string().c_str());
        }
    });

    return peers_sent_to;
}

bool Network::UDPSendTo(Network_Message_pb& msg)
{
    std::lock_guard<std::mutex> lk(local_mutex);

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    auto it = _udp_addrs.find(msg.dest_id());
    if (it == _udp_addrs.end())
    {
        //LOG(Log::LogLevel::ERR, "No route to %llu", msg.dest_id());
        return false;
    }

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::string buffer = std::move(msg.SerializeAsString());
    try
    {
        _udp_socket.sendto(it->second, buffer.c_str(), buffer.length());
        ////LOG(Log::LogLevel::TRACE, "Sent message to %s", it->second.to_string().c_str());
    }
    catch (socket_exception & e)
    {
        //LOG(Log::LogLevel::WARN, "Udp socket exception: %s on %s", e.what(), it->second.to_string().c_str());
        return false;
    }

    return true;
}

std::set<Network::peer_t> Network::TCPSendToAllPeers(Network_Message_pb& msg)
{
    std::lock_guard<std::mutex> lk(local_mutex);
    std::set<peer_t> peers_sent_to;

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::for_each(_tcp_peers.begin(), _tcp_peers.end(), [&](std::pair<peer_t const, tcp_socket*>& client)
    {
        msg.set_dest_id(client.first);

        std::string buffer(sizeof(uint16_t), 0);
        buffer += std::move(msg.SerializeAsString());
        *reinterpret_cast<uint16_t*>(&buffer[0]) = Socket::net_swap(uint16_t(buffer.length() - sizeof(uint16_t)));
        try
        {
            client.second->send(buffer.c_str(), buffer.length());
            peers_sent_to.insert(client.first);
            ////LOG(Log::LogLevel::TRACE, "Sent message to %s", peer_infos.second.to_string().c_str());
        }
        catch (socket_exception & e)
        {
            //LOG(Log::LogLevel::WARN, "Tcp socket exception: %s on %s", e.what(), client.second->get_addr().to_string().c_str());
        }
    });

    return peers_sent_to;
}

bool Network::TCPSendTo(Network_Message_pb& msg)
{
    std::lock_guard<std::mutex> lk(local_mutex);

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    auto it = _tcp_peers.find(msg.dest_id());
    if (it == _tcp_peers.end())
    {
        //LOG(Log::LogLevel::ERR, "No route to %llu", msg.dest_id());
        return false;
    }

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::string buffer(sizeof(uint16_t), 0);
    buffer += std::move(msg.SerializeAsString());
    *reinterpret_cast<uint16_t*>(&buffer[0]) = Socket::net_swap(uint16_t(buffer.length() - sizeof(uint16_t)));
    try
    {
        it->second->send(buffer.c_str(), buffer.length());
        ////LOG(Log::LogLevel::TRACE, "Sent message to %s", it->second.to_string().c_str());
    }
    catch (socket_exception & e)
    {
        //LOG(Log::LogLevel::WARN, "Tcp socket exception: %s on %s", e.what(), it->second->get_addr().to_string().c_str());
        return false;
    }

    return true;
}

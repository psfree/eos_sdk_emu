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

using namespace PortableAPI;

namespace std
{
    inline std::string to_string(std::string const& str)
    {
        return str;
    }
}

Network::Network():
    _udp_socket(),
    _tcp_self_recv({})
{
    //LOG(Log::LogLevel::DEBUG, "");
#if defined(NETWORK_COMPRESS)
    max_message_size = 0;
    max_compressed_message_size = 0;
    _zstd_ccontext = ZSTD_createCCtx();
    _zstd_dstream = ZSTD_createDStream();
#endif

    _network_task.run(&Network::network_thread, this);
}

Network::~Network()
{
#if defined(NETWORK_COMPRESS)
    LOG(Log::LogLevel::DEBUG, "Shutting down Network, biggest message size was %llu, biggest compressed message size was %llu", max_message_size, max_compressed_message_size);
#else
    LOG(Log::LogLevel::DEBUG, "Shutting down Network");
#endif

    _network_task.stop();
    _network_task.join();

#if defined(NETWORK_COMPRESS)
    ZSTD_freeCCtx(_zstd_ccontext);
    ZSTD_freeDStream(_zstd_dstream);
#endif

    //LOG(Log::LogLevel::DEBUG, "Network Thread Joined");
}

#if defined(NETWORK_COMPRESS)

std::string Network::compress(void const* data, size_t len)
{
    std::string res(ZSTD_compressBound(len), '\0');
    res.resize(ZSTD_compressCCtx(_zstd_ccontext, &res[0], res.length(), data, len, ZSTD_CLEVEL_DEFAULT));
    return res;
}

std::string Network::decompress(void const* data, size_t len)
{
    static size_t decompress_block_size = ZSTD_DStreamOutSize();
    static std::string res;

    res.resize(decompress_block_size);
    ZSTD_inBuffer inbuff{ data, len, 0 };
    ZSTD_outBuffer outbuff{ const_cast<char*>(res.data()), res.length(), 0 };

    while (inbuff.pos < inbuff.size)
    {
        size_t x = 0;
        x = ZSTD_decompressStream(_zstd_dstream, &outbuff, &inbuff);
        if (ZSTD_isError(x))
        {
            auto str_error = ZSTD_getErrorName(x);
            LOG(Log::LogLevel::WARN, "Decompression error: %s", str_error);
            return std::string((char*)data, ((char*)data) + len);
        }
        if (inbuff.pos == res.size())
        {
            res.resize(res.length() + decompress_block_size);
            outbuff.size = res.length();
            outbuff.dst = const_cast<char*>(res.data());
        }
    }

    ZSTD_initDStream(_zstd_dstream);
    res.resize(outbuff.pos);
    return res;
}

#endif

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
        LOG(Log::LogLevel::INFO, "UDP socket started on port: %hu", port);
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
                LOG(Log::LogLevel::WARN, "Failed to start tcp socket on port %hu", x);
            }
        }
        if (x == 100)
        {
            LOG(Log::LogLevel::ERR, "Failed to start tcp socket");
            _udp_socket.close();
            _network_task.stop();
        }
        else
        {
            _tcp_port = port;
            LOG(Log::LogLevel::INFO, "TCP socket started after %hu tries on port: %hu", x, port);
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

inline Network::next_packet_size_t Network::make_next_packet_size(std::string const& buff) const
{
    return Socket::net_swap(next_packet_size_t(buff.length() - sizeof(next_packet_size_t)));
}

void Network::build_advertise_msg(Network_Message_pb& msg)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    Network_Advertise_pb* advertise = new Network_Advertise_pb;
    Network_Peer_pb* peer_pb = new Network_Peer_pb;

    LOG(Log::LogLevel::DEBUG, "Advertising with peer ids: ");
    for (auto& id : _my_peer_ids)
    {
        LOG(Log::LogLevel::DEBUG, "%s", std::to_string(id).c_str());
        peer_pb->add_peer_ids(id);
    }

    advertise->set_allocated_peer(peer_pb);
    msg.set_allocated_network_advertise(advertise);
    msg.set_source_id(*_my_peer_ids.begin());
}

std::pair<PortableAPI::tcp_socket*, std::vector<Network::peer_t>> Network::get_new_peer_ids(Network_Peer_pb const& peer_msg)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

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
    std::lock_guard<std::recursive_mutex> lk(local_mutex);
    if (!_advertise)
        return;

    auto now = std::chrono::steady_clock::now();
    if ((now - _last_advertise) < std::chrono::milliseconds(2000))
        return;

    _last_advertise = now;
    
    try
    {
        if (!_my_peer_ids.empty())
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

void Network::add_new_tcp_client(PortableAPI::tcp_socket* cli, std::vector<peer_t> const& peer_ids, bool advertise_peer)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    _poll.add_socket(*cli); // Add the client to the poll
    _poll.set_events(*cli, Socket::poll_flags::in);

    Network_Message_pb msg;
    Network_Advertise_pb adv;
    Network_Peer_Connect_pb conn;

    adv.set_allocated_peer_connect(&conn);
    msg.set_allocated_network_advertise(&adv);

    for (auto& peerid : peer_ids)
    {// Map all clients peerids to the socket
        LOG(Log::LogLevel::DEBUG, "Adding peer id %s to client %s", std::to_string(peerid).c_str(), cli->get_addr().to_string(true).c_str());
        _tcp_peers[peerid] = cli;

        msg.set_source_id(peerid);

        for (auto& channel : _default_channels)
        {
            _pending_network_msgs[channel.second].emplace_back(msg);
        }
    }

    adv.release_peer_connect();
    msg.release_network_advertise();
    
    if(advertise_peer)
    {
        LOG(Log::LogLevel::DEBUG, "New peer: id %s %s", std::to_string(*peer_ids.begin()).c_str(), cli->get_addr().to_string(true).c_str());

        Network_Message_pb msg;
        Network_Advertise_pb* adv = new Network_Advertise_pb;
        Network_Peer_Accept_pb* accept_peer = new Network_Peer_Accept_pb;

        adv->set_allocated_accept(accept_peer);
        msg.set_allocated_network_advertise(adv);

        std::string buff(sizeof(next_packet_size_t), 0);
        // Don't compress the accept message, its only 4 bytes long
    //#if defined(NETWORK_COMPRESS)
    //    std::string data;
    //    msg.SerializeToString(&data);
    //    buff += std::move(compress(data.data(), data.length()));
    //    
    //    max_message_size = std::max<uint64_t>(max_message_size, data.length());
    //    max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buff.length());
    //#else
        buff += std::move(msg.SerializeAsString());
    //#endif

        *reinterpret_cast<next_packet_size_t*>(&buff[0]) = make_next_packet_size(buff);

        cli->send(buff.data(), buff.length());
    }
}

void Network::remove_tcp_peer(tcp_buffer_t& tcp_buffer)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    LOG(Log::LogLevel::DEBUG, "TCP Client %s gone", tcp_buffer.socket.get_addr().to_string().c_str());
    _poll.remove_socket(tcp_buffer.socket);
    // Remove the peer mappings

    Network_Message_pb msg;
    Network_Advertise_pb adv;
    Network_Peer_Disconnect_pb disc;

    adv.set_allocated_peer_disconnect(&disc);
    msg.set_allocated_network_advertise(&adv);

    for (auto it = _tcp_peers.begin(); it != _tcp_peers.end();)
    {
        if (it->second == &(tcp_buffer.socket))
        {
            msg.set_source_id(it->first);
            it = _tcp_peers.erase(it);

            for (auto& channel : _default_channels)
            {
                _pending_network_msgs[channel.second].emplace_back(msg);
            }
        }
        else
            ++it;
    }

    adv.release_peer_disconnect();
    msg.release_network_advertise();
}

void Network::connect_to_peer(ipv4_addr &addr, peer_t const& peer_id)
{
    if (_waiting_out_tcp_clients.count(peer_id) != 0)
        return;

    bool connected = false;
    auto it = _waiting_connect_tcp_clients.find(peer_id);
    try
    {
        if (it == _waiting_connect_tcp_clients.end())
        {
            LOG(Log::LogLevel::DEBUG, "Connecting to %s : %s", addr.to_string(true).c_str(), std::to_string(peer_id).c_str());
            
            _waiting_connect_tcp_clients.emplace(peer_id, tcp_socket());
            it = _waiting_connect_tcp_clients.find(peer_id);
            it->second.set_nonblocking(true);
        }
        it->second.connect(addr);
        connected = true;
    }
    catch (is_connected &e)
    {
        connected = true;
    }
    catch (would_block &e)
    {}
    catch(in_progress &e)
    {}
    catch (std::exception &e)
    {
        _waiting_connect_tcp_clients.erase(it);
        LOG(Log::LogLevel::WARN, "Failed to TCP connect to %s: %s", addr.to_string().c_str(), e.what());
    }

    if (connected)
    {
        Network_Message_pb msg;
        build_advertise_msg(msg);

        std::string buff(sizeof(next_packet_size_t), 0);
        
    #if defined(NETWORK_COMPRESS)
        std::string data;
        msg.SerializeToString(&data);
        buff += std::move(compress(data.data(), data.length()));

        max_message_size = std::max<uint64_t>(max_message_size, data.length());
        max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buff.length());
    #else
        buff += std::move(msg.SerializeAsString());
    #endif
        *reinterpret_cast<next_packet_size_t*>(&buff[0]) = make_next_packet_size(buff);

        it->second.send(buff.data(), buff.length());

        LOG(Log::LogLevel::DEBUG, "Connected to %s : %s", it->second.get_addr().to_string(true).c_str(), std::to_string(peer_id).c_str());

        tcp_buffer_t tcp_buffer{};
        tcp_buffer.socket = std::move(it->second);
        _waiting_out_tcp_clients.emplace(peer_id, std::move(tcp_buffer));
        _waiting_connect_tcp_clients.erase(it);
    }
}

void Network::process_waiting_out_clients()
{
    if (_waiting_out_tcp_clients.empty())
        return;

    Network_Message_pb msg;
    for (auto it = _waiting_out_tcp_clients.begin(); it != _waiting_out_tcp_clients.end(); )
    {
        try
        {
            unsigned long count = 0;
            it->second.socket.ioctlsocket(Socket::cmd_name::fionread, &count);
            if (count > 0)
            {
                if (it->second.next_packet_size == 0 && count > sizeof(next_packet_size_t))
                {
                    it->second.socket.recv(&it->second.next_packet_size, sizeof(next_packet_size_t));
                    it->second.next_packet_size = Socket::net_swap(it->second.next_packet_size);
                    count -= sizeof(next_packet_size_t);
                }
                if (it->second.next_packet_size > 0 && count >= it->second.next_packet_size)
                {
                    it->second.buffer.resize(it->second.next_packet_size);
                    it->second.socket.recv(it->second.buffer.data(), it->second.next_packet_size);

                    const void* message;
                    int message_size;

                    // Don't compress the accept message, its only 4 bytes long
                //#if defined(NETWORK_COMPRESS)
                //    std::string buff = std::move(decompress(it->second.buffer.data(), it->second.buffer.size()));
                //    message = buff.data();
                //    message_size = buff.length();
                //#else
                    message = it->second.buffer.data();
                    message_size = it->second.buffer.size();
                //#endif
                    
                    if (msg.ParseFromArray(message, message_size) &&
                        msg.has_network_advertise() && 
                        msg.network_advertise().has_accept())
                    {
                        std::lock_guard<std::recursive_mutex> lk(local_mutex);

                        it->second.next_packet_size = 0;
                        it->second.buffer.clear();
                        it->second.socket.set_nonblocking(false);

                        _tcp_clients.emplace_back(std::move(it->second));
                        add_new_tcp_client(&(_tcp_clients.rbegin()->socket), std::vector<peer_t>{it->first}, false);
                    }
                    it = _waiting_out_tcp_clients.erase(it);
                    continue;
                }
            }
            
            ++it;
        }
        catch (std::exception &e)
        {
            // Error while reading, connection closed ?
            LOG(Log::LogLevel::WARN, "Failed peer pair: %s", e.what());
            it = _waiting_out_tcp_clients.erase(it);
        }
    }
}

void Network::process_waiting_in_client()
{
    Network_Message_pb msg;
    for (auto it = _waiting_in_tcp_clients.begin(); it != _waiting_in_tcp_clients.end(); )
    {
        try
        {
            unsigned long count = 0;
            it->socket.ioctlsocket(Socket::cmd_name::fionread, &count);
            if (count > 0)
            {
                if (it->next_packet_size == 0 && count > sizeof(next_packet_size_t))
                {
                    it->socket.recv(&it->next_packet_size, sizeof(next_packet_size_t));
                    it->next_packet_size = Socket::net_swap(it->next_packet_size);
                    count -= sizeof(next_packet_size_t);
                }
                if (it->next_packet_size > 0 && count >= it->next_packet_size)
                {
                    it->buffer.resize(it->next_packet_size);
                    it->socket.recv(it->buffer.data(), it->next_packet_size);

                    const void* message;
                    int message_size;

                #if defined(NETWORK_COMPRESS)
                    std::string buff = std::move(decompress(it->buffer.data(), it->next_packet_size));
                    message = buff.data();
                    message_size = buff.length();
                #else
                    message = it->buffer.data();
                    message_size = it->buffer.size();
                #endif
                    
                    if (msg.ParseFromArray(message, message_size) &&
                        msg.has_network_advertise() && 
                        msg.network_advertise().has_peer())
                    {
                        std::lock_guard<std::recursive_mutex> lk(local_mutex);

                        it->next_packet_size = 0;
                        it->buffer.clear();
                        it->socket.set_nonblocking(false);

                        auto const& peer_msg = msg.network_advertise().peer();
                        std::pair<tcp_socket*, std::vector<peer_t>> peer_ids_to_add = std::move(get_new_peer_ids(peer_msg));

                        if (!peer_ids_to_add.second.empty())
                        {// We have peer ids to add
                            if (peer_ids_to_add.first == nullptr)
                            {// Didn't find a matching peer id, its a new peer
                                _tcp_clients.emplace_back(std::move(*it));
                                peer_ids_to_add.first = &(_tcp_clients.rbegin()->socket);
                            }
                            add_new_tcp_client(peer_ids_to_add.first, peer_ids_to_add.second, true);
                        }
                    }
                    it = _waiting_in_tcp_clients.erase(it);
                    continue;
                }
            }
            
            ++it;
        }
        catch (std::exception &e)
        {
            // Error while reading, connection closed ?
            LOG(Log::LogLevel::WARN, "Failed peer pair: %s", e.what());
            it = _waiting_in_tcp_clients.erase(it);
        }
    }
}

void Network::process_network_message(Network_Message_pb &msg)
{
    std::lock_guard<std::mutex> lk(message_mutex);

    std::chrono::system_clock::time_point msg_time(std::chrono::milliseconds(msg.timestamp()));
    
    //if ((std::chrono::system_clock::now() - msg_time) > std::chrono::milliseconds(1500))
    //{
    //    LOG(Log::LogLevel::WARN, "Message dropped because it was too old");
    //    return;
    //}

    if (msg.dest_id() == peer_t())
    {// If we received a message without a destination, then its a broadcast.
        // Add the message to all listeners queue
        for (auto& channel : _default_channels)
            _pending_network_msgs[channel.second].emplace_back(msg);
    }
    else
    {
        assert(_default_channels.find(msg.dest_id()) != _default_channels.end());
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
        if (len > 0)
        {
            const void* message;
            int message_size;

            #if defined(NETWORK_COMPRESS)
                std::string buff(std::move(decompress(buffer.data(), len)));
                message = buff.data();
                message_size = buff.length();
            #else
                message = buffer.data();
                message_size = buffer.length();
            #endif

            if (msg.ParseFromArray(message, message_size))
            {
                if (msg.source_id() != peer_t())
                {
                    std::lock_guard<std::recursive_mutex> lk(local_mutex);
                    _udp_addrs[msg.source_id()] = addr;

                    //LOG(Log::LogLevel::TRACE, "Received UDP message from: %s - %s", addr.to_string().c_str(), msg.source_id().c_str());
                    if (msg.has_network_advertise())
                    {
                        if (_advertise)
                        {
                            auto const& advertise = msg.network_advertise();
                            if (advertise.has_port())
                            {
                                if (!_my_peer_ids.empty() &&
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
                    }
                    else
                    {
                        //LOG(Log::LogLevel::DEBUG, "Received UDP message from %s type %d", addr.to_string(true).c_str(), msg.messages_case());
                        process_network_message(msg);
                    }
                }
                else
                {
                    LOG(Log::LogLevel::DEBUG, "Dropping UDP data: peer_id is null");
                }
            }
            else
            {
                LOG(Log::LogLevel::DEBUG, "Dropping UDP data: failed to pase protobuf");
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
        tcp_buffer_t tcp_buff({});
        tcp_buff.socket = std::move(_tcp_socket.accept());
        tcp_buff.socket.set_nonblocking(true);
        _waiting_in_tcp_clients.emplace_back(std::move(tcp_buff));
    }
    catch (socket_exception & e)
    {
        LOG(Log::LogLevel::WARN, "TCP Listen exception: %s", e.what());
    }
}

void Network::process_tcp_data(tcp_buffer_t& tcp_buffer)
{
    // Don't lock here, its already locked in network_thread when needed
    Network_Message_pb msg;
    size_t len;

    unsigned long count = 0;
    tcp_buffer.socket.ioctlsocket(Socket::cmd_name::fionread, &count);
    if (count > 0)
    {
        size_t buff_len = tcp_buffer.buffer.size();
        tcp_buffer.buffer.resize(buff_len + count); // We grow to the current size + stream size

        len = tcp_buffer.socket.recv(tcp_buffer.buffer.data() + buff_len, count);

        while(tcp_buffer.buffer.size() > 0)
        {
            if (tcp_buffer.next_packet_size == 0 && tcp_buffer.buffer.size() >= sizeof(next_packet_size_t))
            {
                tcp_buffer.next_packet_size = *reinterpret_cast<next_packet_size_t*>(&tcp_buffer.buffer[0]);
                tcp_buffer.next_packet_size = Socket::net_swap(tcp_buffer.next_packet_size);
                tcp_buffer.buffer.erase(tcp_buffer.buffer.begin(), tcp_buffer.buffer.begin() + sizeof(tcp_buffer.next_packet_size));
            }

            if (tcp_buffer.next_packet_size > 0 && tcp_buffer.buffer.size() >= tcp_buffer.next_packet_size)
            {
                const void* message;
                int message_size;
            #if defined(NETWORK_COMPRESS)
                std::string buff = std::move(decompress(tcp_buffer.buffer.data(), tcp_buffer.next_packet_size));
                message = buff.data();
                message_size = buff.length();
            #else
                message = tcp_buffer.buffer.data();
                message_size = tcp_buffer.received_size;
            #endif

                if (msg.ParseFromArray(message, message_size))
                {
                    //LOG(Log::LogLevel::DEBUG, "Received TCP message from %s type %d", tcp_buffer.socket.get_addr().to_string(true).c_str(), msg.messages_case());
                    process_network_message(msg);
                }
                tcp_buffer.buffer.erase(tcp_buffer.buffer.begin(), tcp_buffer.buffer.begin() + tcp_buffer.next_packet_size);
                tcp_buffer.next_packet_size = 0;
            }
            else
            {
                break;
            }
        }
    }
}

void Network::network_thread()
{
    int broadcast = 1;

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
        {
            try
            {
                process_tcp_data(_tcp_self_recv); // Process our TCP message, we are not considered as a classic client as we have 2 sockets for the same peer id
            }
            catch (...)
            {
                assert(0 == 1 && "The local socket should not fail");
            }
        }
        
        {
            std::lock_guard<std::recursive_mutex> lk(local_mutex);
            for (auto it = _tcp_clients.begin(); it != _tcp_clients.end();)
            {// Process the multiple tcp clients we have
                auto reevents = _poll.get_revents(it->socket);
                if ((reevents & Socket::poll_flags::hup) != Socket::poll_flags::none)
                {
                    remove_tcp_peer(*it);
                    it = _tcp_clients.erase(it);
                }
                else if ((reevents & Socket::poll_flags::in_hup) != Socket::poll_flags::none)
                {
                    try
                    {
                        process_tcp_data(*it);
                        ++it;
                    }
                    catch (std::exception & e)
                    {
                        remove_tcp_peer(*it);
                        it = _tcp_clients.erase(it);
                    }
                }
                else
                    ++it;
            }
        }
        
        process_waiting_in_client();
        // We might have found a peer while he didn't find us yet, so begin the connection procedure
        process_waiting_out_clients();
    }
}

void Network::advertise_peer_id(peer_t const& peerid)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    _my_peer_ids.insert(peerid);
    _tcp_peers[peerid] = &_tcp_self_send;
}

void Network::remove_advertise_peer_id(peer_t const& peerid)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    _my_peer_ids.erase(peerid);
    _tcp_peers.erase(peerid);
}

void Network::advertise(bool doit)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);
    _advertise = doit;
}

bool Network::is_advertising()
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    return _advertise;
}

void Network::set_default_channel(peer_t peerid, channel_t default_channel)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    _default_channels[peerid] = default_channel;
}

void Network::register_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    _network_listeners[type][channel].push_back(listener);
}

void Network::unregister_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    auto& listeners = _network_listeners[type][channel];
    listeners.erase(
        std::remove(listeners.begin(), listeners.end(), listener),
        listeners.end());
}

bool Network::CBRunFrame(channel_t channel, Network_Message_pb::MessagesCase MessageFilter)
{
    bool rerun = false;
    auto& channel_messages = _network_msgs[channel];
    {
        std::lock_guard<std::recursive_mutex> lk(local_mutex);
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
    }

    {
        std::lock_guard<std::mutex> lk(message_mutex);

        auto& pending_channel_messages = _pending_network_msgs[channel];
        if (!pending_channel_messages.empty())
        {
            std::move(pending_channel_messages.begin(), pending_channel_messages.end(), std::back_inserter(channel_messages));
            pending_channel_messages.clear();
        }
    }

    return rerun;
}

bool Network::SendBroadcast(Network_Message_pb& msg)
{
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    std::vector<ipv4_addr> broadcasts = std::move(get_broadcasts());

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));
    assert((msg.dest_id() == peer_t() && "Destination id should be null"));

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    msg.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    std::string buffer;
    msg.SerializeToString(&buffer);
#if defined(NETWORK_COMPRESS)
    max_message_size = std::max<uint64_t>(max_message_size, buffer.length());

    buffer = std::move(compress(buffer.data(), buffer.length()));
    max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buffer.length());
#endif

    for (auto& brd : broadcasts)
    {
        for (uint16_t port = network_port; port < max_network_port; ++port)
        {
            brd.set_port(port);
            try
            {
                _udp_socket.sendto(brd, buffer.data(), buffer.length());
                //LOG(Log::LogLevel::TRACE, "Send broadcast");
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
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    std::set<peer_t> peers_sent_to;

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::for_each(_udp_addrs.begin(), _udp_addrs.end(), [&](std::pair<peer_t const, PortableAPI::ipv4_addr>& peer_infos)
    {
        msg.set_dest_id(peer_infos.first);
        msg.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        std::string buffer;
        msg.SerializeToString(&buffer);

    #if defined(NETWORK_COMPRESS)
        buffer = std::move(compress(buffer.data(), buffer.length()));

        max_message_size = std::max<uint64_t>(max_message_size, buffer.length());
        max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buffer.length());
    #endif

        try
        {
            _udp_socket.sendto(peer_infos.second, buffer.data(), buffer.length());
            peers_sent_to.insert(peer_infos.first);
            //LOG(Log::LogLevel::TRACE, "Sent message to %s", peer_infos.second.to_string().c_str());
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
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    auto it = _udp_addrs.find(msg.dest_id());
    if (it == _udp_addrs.end())
    {
        //LOG(Log::LogLevel::ERR, "No route to %llu", msg.dest_id());
        return false;
    }

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    msg.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    std::string buffer;
    msg.SerializeToString(&buffer);

#if defined(NETWORK_COMPRESS)
    max_message_size = std::max<uint64_t>(max_message_size, buffer.length());

    buffer = std::move(compress(buffer.data(), buffer.length()));
    max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buffer.length());
#endif

    try
    {
        _udp_socket.sendto(it->second, buffer.data(), buffer.length());
        LOG(Log::LogLevel::DEBUG, "Sent message to peer_id: %s, addr: %s", std::to_string(msg.dest_id()).c_str(), it->second.to_string().c_str());
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
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    std::set<peer_t> peers_sent_to;

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    std::for_each(_tcp_peers.begin(), _tcp_peers.end(), [&](std::pair<peer_t const, tcp_socket*>& client)
    {
        msg.set_dest_id(client.first);
        msg.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        std::string buffer(sizeof(next_packet_size_t), 0);

    #if defined(NETWORK_COMPRESS)
        std::string data;
        msg.SerializeToString(&data);

        max_message_size = std::max<uint64_t>(max_message_size, data.length());

        buffer += std::move(compress(data.data(), data.length()));
        max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buffer.length());
    #else
        buffer += std::move(msg.SerializeAsString());
    #endif

        *reinterpret_cast<next_packet_size_t*>(&buffer[0]) = make_next_packet_size(buffer);

        try
        {
            client.second->send(buffer.data(), buffer.length());
            peers_sent_to.insert(client.first);
            //LOG(Log::LogLevel::TRACE, "Sent message to %s", peer_infos.second.to_string().c_str());
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
    std::lock_guard<std::recursive_mutex> lk(local_mutex);

    assert((msg.source_id() != peer_t() && "Source id cannot be null"));

    auto it = _tcp_peers.find(msg.dest_id());
    if (it == _tcp_peers.end())
    {
        //LOG(Log::LogLevel::ERR, "No route to %llu", msg.dest_id());
        return false;
    }

    //if (msg.appid() == 0)
    //    msg.set_appid(Settings::Inst().gameid.AppID());

    msg.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    std::string buffer(sizeof(next_packet_size_t), 0);

#if defined(NETWORK_COMPRESS)
    std::string data;
    msg.SerializeToString(&data);

    max_message_size = std::max<uint64_t>(max_message_size, data.length());

    buffer += std::move(compress(data.data(), data.length()));
    max_compressed_message_size = std::max<uint64_t>(max_compressed_message_size, buffer.length());
#else
    buffer += std::move(msg.SerializeAsString());
#endif

    *reinterpret_cast<next_packet_size_t*>(&buffer[0]) = make_next_packet_size(buffer);

    try
    {
        it->second->send(buffer.data(), buffer.length());
        //LOG(Log::LogLevel::TRACE, "Sent message to %s", it->second.to_string().c_str());
    }
    catch (socket_exception & e)
    {
        //LOG(Log::LogLevel::WARN, "Tcp socket exception: %s on %s", e.what(), it->second->get_addr().to_string().c_str());
        return false;
    }

    return true;
}

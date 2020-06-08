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

#if defined(NETWORK_COMPRESS)
#include <zstd.h>
#endif

#include "common_includes.h"
#include "task.h"

class LOCAL_API Network
{
public:
    using channel_t = int32_t;
    using peer_t = std::string;

    struct tcp_buffer_t
    {
        PortableAPI::tcp_socket socket;
        std::vector<uint8_t> buffer;
        uint32_t next_packet_size;
        uint32_t received_size;
    };

    using tcp_client_iterator = typename std::list<tcp_buffer_t>::iterator;

private:
    static constexpr uint16_t network_port = 55789;
    static constexpr uint16_t max_network_port = (network_port + 10);

#if defined(NETWORK_COMPRESS)
    ZSTD_CCtx* _zstd_ccontext;
    ZSTD_DCtx* _zstd_dcontext;

    std::string compress(void const* data, size_t len);
    std::string decompress(void const* data, size_t len);
#endif

    bool _advertise;
    std::chrono::steady_clock::time_point _last_advertise;
    std::set<peer_t> _my_peer_ids;
    uint16_t _tcp_port;

    PortableAPI::Poll _poll;
    PortableAPI::udp_socket _udp_socket;
    PortableAPI::udp_socket _query_socket;
    std::map<peer_t, PortableAPI::ipv4_addr> _udp_addrs;

    PortableAPI::tcp_socket _tcp_socket;
    std::list<tcp_buffer_t> _tcp_clients;
    std::map<peer_t, PortableAPI::tcp_socket> _waiting_out_tcp_clients;
    std::map<peer_t, PortableAPI::tcp_socket> _waiting_connect_tcp_clients;
    std::list<tcp_buffer_t>                   _waiting_in_tcp_clients;
    PortableAPI::tcp_socket _tcp_self_send;
    tcp_buffer_t _tcp_self_recv;
    std::map<peer_t, PortableAPI::tcp_socket*> _tcp_peers;

    std::map<Network_Message_pb::MessagesCase, std::map<channel_t, std::vector<IRunFrame*>>> _network_listeners;

    // Lock message_mutex when accessing:
    //  _pending_network_msgs
    std::mutex message_mutex;
    // Lock local_mutex when accessing:
    //  _udp_addrs
    //  _tcp_clients
    //  _tcp_peers
    //  _my_peer_ids
    //  _network_listeners
    //  _advertise
    std::recursive_mutex local_mutex;

    void start_network();
    void stop_network();

    void build_advertise_msg(Network_Message_pb& msg);
    std::pair<PortableAPI::tcp_socket*, std::vector<peer_t>> get_new_peer_ids(Network_Peer_pb const& peer_msg);

    void do_advertise();

    void add_new_tcp_client(PortableAPI::tcp_socket* cli, std::vector<peer_t> const& peer_ids, bool advertise);
    void remove_tcp_peer(tcp_buffer_t& tcp_buffer);
    void connect_to_peer(PortableAPI::ipv4_addr& addr, peer_t const& peer_id);
    void process_waiting_out_clients();
    void process_waiting_in_client();

    void process_network_message(Network_Message_pb& msg);
    void process_udp();
    void process_tcp_listen();
    void process_tcp_data(tcp_buffer_t& tcp_buffer);
    void network_thread();
    task _network_task;

    std::map<channel_t, std::list<Network_Message_pb>> _pending_network_msgs;
    std::map<channel_t, std::list<Network_Message_pb>> _network_msgs;

    std::map<peer_t, channel_t> _default_channels;

public:
    std::atomic_bool _query_started;

    Network();
    ~Network();

    void advertise_peer_id(peer_t const& peerid);
    void remove_advertise_peer_id(peer_t const& peerid);
    void advertise(bool doit);
    bool is_advertising();

    void set_default_channel(peer_t peerid, channel_t default_channel);

    void register_listener  (IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type);
    void unregister_listener(IRunFrame* listener, channel_t channel, Network_Message_pb::MessagesCase type);

    bool CBRunFrame(channel_t channel, Network_Message_pb::MessagesCase MessageFilter = Network_Message_pb::MessagesCase::MESSAGES_NOT_SET);

    bool SendBroadcast(Network_Message_pb& msg); // Always UDP
    std::set<peer_t> UDPSendToAllPeers(Network_Message_pb& msg);
    bool UDPSendTo(Network_Message_pb& msg);

    std::set<peer_t> TCPSendToAllPeers(Network_Message_pb& msg);
    bool TCPSendTo(Network_Message_pb& msg);
};
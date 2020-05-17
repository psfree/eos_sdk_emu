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

#include "eossdk_p2p.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

EOSSDK_P2P::EOSSDK_P2P():
    next_requested_channel(-1)
{
    GetCB_Manager().register_callbacks(this);

    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kP2P);
}

EOSSDK_P2P::~EOSSDK_P2P()
{
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kP2P);

    GetCB_Manager().unregister_callbacks(this);
}

/**
  * Send a packet to a peer at the specified address. If there is already an open connection to this peer, it will be
  * sent immediately. If there is no open connection, an attempt to connect to the peer will be made. A EOS_Success
  * result does not guarantee the packet will be delivered to the peer, as data is sent unreliably.
  *
  * @param Options Information about the data being sent, by who, to who
  * @return EOS_EResult::EOS_Success           - If packet was queued to be sent successfully
  *         EOS_EResult::EOS_InvalidParameters - If input was invalid
  */
EOS_EResult EOSSDK_P2P::SendPacket(const EOS_P2P_SendPacketOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    
    if (Options == nullptr || Options->RemoteUserId == nullptr || Options->Data == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    p2p_state_t& p2pstate = _p2p_connections[Options->RemoteUserId];
    P2P_Data_Message_pb data;
    data.set_data(reinterpret_cast<const char*>(Options->Data), Options->DataLengthBytes);
    data.set_channel(Options->Channel);
    data.set_socket_name(Options->SocketId->SocketName);
    data.set_user_id(Options->LocalUserId->to_string());
    if (p2pstate.status != p2p_state_t::status_e::connected)
    {
        // Save the message for later
        p2pstate.p2p_out_messages[Options->RemoteUserId].emplace(std::move(data));

        p2pstate.status = p2p_state_t::status_e::connecting;
        p2pstate.socket_name = Options->SocketId->SocketName;

        P2P_Connect_Request_pb* req = new P2P_Connect_Request_pb;
        req->set_socket_name(p2pstate.socket_name);
        send_p2p_connection_request(Options->RemoteUserId->to_string(), req);
    }
    else
    {
        // We're connected, send the message now
        send_p2p_data(Options->RemoteUserId->to_string(), &data);
    }

    return EOS_EResult::EOS_Success;
}

/**
 * Gets the size of the packet that will be returned by ReceivePacket for a particular user, if there is any available
 * packets to be retrieved.
 *
 * @param Options Information about who is requesting the size of their next packet
 * @param OutPacketSize The amount of bytes required to store the data of the next packet for the requested user
 * @return EOS_EResult::EOS_Success - If OutPacketSize was successfully set and there is data to be received
 *         EOS_EResult::EOS_InvalidParameters - If input was invalid
 *         EOS_EResult::EOS_NotFound  - If there are no packets available for the requesting user
 */
EOS_EResult EOSSDK_P2P::GetNextReceivedPacketSize(const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || OutPacketSizeBytes == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    bool has_packet = false;
    if (Options->RequestedChannel == nullptr)
    {
        for (auto& in_msgs : _p2p_in_messages)
        {
            if (!in_msgs.second.empty())
            {
                auto& front = in_msgs.second.front();
                *OutPacketSizeBytes = front.data().length();
                next_requested_channel = front.channel();
                has_packet = true;
            }
        }
        
    }
    else
    {
        next_requested_channel = *Options->RequestedChannel;
        auto& in_msgs = _p2p_in_messages[next_requested_channel];
        if (!in_msgs.empty())
        {
            *OutPacketSizeBytes = in_msgs.front().data().length();
            has_packet = true;
        }
    }

    if (has_packet)
    {
        return EOS_EResult::EOS_Success;
    }
    
    *OutPacketSizeBytes = 0;
    return EOS_EResult::EOS_NotFound;
}

/**
 * Receive the next packet for the local user, and information associated with this packet, if it exists.
 *
 * @param Options Information about who is requesting the size of their next packet, and how much data can be stored safely
 * @param OutPeerId The Remote User who sent data. Only set if there was a packet to receive.
 * @param OutSocketId The Socket ID of the data that was sent. Only set if there was a packet to receive.
 * @param OutChannel The channel the data was sent on. Only set if there was a packet to receive.
 * @param OutData Buffer to store the data being received. Must be at least EOS_P2P_GetNextReceivedPacketSize in length or data will be truncated
 * @param OutBytesWritten The amount of bytes written to OutData. Only set if there was a packet to receive.
 * @return EOS_EResult::EOS_Success - If the packet was received successfully
 *         EOS_EResult::EOS_InvalidParameters - If input was invalid
 *         EOS_EResult::EOS_NotFound - If there are no packets available for the requesting user
 */
EOS_EResult EOSSDK_P2P::ReceivePacket(const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten)
{
    //TRACE_FUNC();
    GLOBAL_LOCK();
    
    if (Options == nullptr || OutPeerId == nullptr || OutSocketId == nullptr ||
        OutChannel == nullptr || OutData == nullptr || OutBytesWritten == nullptr)
    {
        return EOS_EResult::EOS_InvalidParameters;
    }

    if (Options->RequestedChannel != nullptr)
        next_requested_channel = *Options->RequestedChannel;

    std::list<P2P_Data_Message_pb> *queue = nullptr;
    if (next_requested_channel == -1)
    {// No channel, get the next available message
        auto it = std::find_if(_p2p_in_messages.begin(), _p2p_in_messages.end(), []( std::pair<uint8_t const, std::list<P2P_Data_Message_pb>>& messages_queue)
        {
            return !messages_queue.second.empty();
        });
        if (it != _p2p_in_messages.end())
            queue = &it->second;
    }
    else
    {
        queue = &_p2p_in_messages[next_requested_channel];
    }
    if (queue == nullptr)
        return EOS_EResult::EOS_NotFound;

    auto& msg = queue->front();

    *OutPeerId = GetProductUserId(msg.user_id());
    *OutBytesWritten = msg.data().copy(reinterpret_cast<char*>(OutData), Options->MaxDataSizeBytes);
    msg.socket_name().copy(OutSocketId->SocketName, sizeof(EOS_P2P_SocketId::SocketName));
    OutSocketId->SocketName[32] = 0;
    *OutChannel = msg.channel();
    next_requested_channel = -1;

    queue->pop_front();

    return EOS_EResult::EOS_Success;
}

/**
 * Listen for incoming connection requests on a particular Socket ID, or optionally all Socket IDs. The bound function
 * will only be called if the connection has not already been accepted.
 *
 * @param Options Information about who would like notifications, and (optionally) only for a specific socket
 * @param ClientData This value is returned to the caller when ConnectionRequestHandler is invoked
 * @param ConnectionRequestHandler The callback to be fired when we receive a connection request
 * @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
 */
EOS_NotificationId EOSSDK_P2P::AddNotifyPeerConnectionRequest(const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);
    
    EOS_P2P_OnIncomingConnectionRequestInfo& oicri = res->CreateCallback<EOS_P2P_OnIncomingConnectionRequestInfo>((CallbackFunc)ConnectionRequestHandler);
    oicri.ClientData = ClientData;
    oicri.LocalUserId = GetEOS_Connect()._myself.first;
    oicri.RemoteUserId = GetProductUserId(sdk::NULL_USER_ID);
    oicri.SocketId = new EOS_P2P_SocketId;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Stop listening for connection requests on a previously bound handler
 *
 * @param NotificationId The previously bound notification ID
 */
void EOSSDK_P2P::RemoveNotifyPeerConnectionRequest(EOS_NotificationId NotificationId)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    GetCB_Manager().remove_notification(this, NotificationId);
}

/**
 * Listen for when a previously opened connection is closed
 *
 * @param Options Information about who would like notifications about closed connections, and for which socket
 * @param ClientData This value is returned to the caller when ConnectionClosedHandler is invoked
 * @param ConnectionClosedHandler The callback to be fired when we an open connection has been closed
 * @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
 */
EOS_NotificationId EOSSDK_P2P::AddNotifyPeerConnectionClosed(const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    
    pFrameResult_t res(new FrameResult);

    EOS_P2P_OnRemoteConnectionClosedInfo& orcci = res->CreateCallback<EOS_P2P_OnRemoteConnectionClosedInfo>((CallbackFunc)ConnectionClosedHandler);
    orcci.ClientData = ClientData;
    orcci.LocalUserId = GetEOS_Connect()._myself.first;
    orcci.RemoteUserId = GetProductUserId(sdk::NULL_USER_ID);
    orcci.SocketId = new EOS_P2P_SocketId;
    orcci.Reason = EOS_EConnectionClosedReason::EOS_CCR_Unknown;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Stop notifications for connections being closed on a previously bound handler
 *
 * @param NotificationId The previously bound notification ID
 */
void EOSSDK_P2P::RemoveNotifyPeerConnectionClosed(EOS_NotificationId NotificationId)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    GetCB_Manager().remove_notification(this, NotificationId);
}

/**
 * Accept connections from a specific peer. If this peer has not attempted to connect yet, when they do, they will automatically be accepted.
 *
 * @param Options Information about who would like to accept a connection, and which connection
 * @return EOS_EResult::EOS_Success - if the provided data is valid
 *         EOS_EResult::EOS_InvalidParameters - if the provided data is invalid
 */
EOS_EResult EOSSDK_P2P::AcceptConnection(const EOS_P2P_AcceptConnectionOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->RemoteUserId == nullptr || Options->SocketId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    auto& conn = _p2p_connections[Options->RemoteUserId];

    if (conn.status == p2p_state_t::status_e::requesting)
    {
        P2P_Connect_Response_pb* resp = new P2P_Connect_Response_pb;
        resp->set_accepted(true);
        send_p2p_connection_response(Options->RemoteUserId->to_string(), resp);
    }
    
    conn.status = p2p_state_t::status_e::connected;
    return EOS_EResult::EOS_Success;
}

/**
 * Stop accepting new connections from a specific peer and close any open connections.
 *
 * @param Options Information about who would like to close a connection, and which connection.
 * @return EOS_EResult::EOS_Success - if the provided data is valid
 *         EOS_EResult::EOS_InvalidParameters - if the provided data is invalid
 */
EOS_EResult EOSSDK_P2P::CloseConnection(const EOS_P2P_CloseConnectionOptions* Options)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::DEBUG, "TODO");
    GLOBAL_LOCK();
    
    if (Options == nullptr || Options->RemoteUserId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (Options->SocketId == nullptr)
    {
        auto& conn = _p2p_connections[Options->RemoteUserId];
        conn.p2p_out_messages.clear();
        for (auto& in_msgs : _p2p_in_messages)
        {
            in_msgs.second.erase(std::remove_if(in_msgs.second.begin(), in_msgs.second.end(), [&Options](P2P_Data_Message_pb& msg)
            {
                return msg.user_id() == Options->RemoteUserId->to_string();
            }), in_msgs.second.end());
        }

        if (conn.status != p2p_state_t::status_e::closed)
        {
            conn.status = p2p_state_t::status_e::closed;

            P2P_Connection_Close_pb* close = new P2P_Connection_Close_pb;
            send_p2p_connetion_close(Options->RemoteUserId->to_string(), close);
        }
    }
    else
    {
        std::string target_sock_name = Options->SocketId->SocketName;
        auto& conn = _p2p_connections[Options->RemoteUserId];
        conn.p2p_out_messages.clear();
        for (auto& in_msgs : _p2p_in_messages)
        {
            in_msgs.second.erase(std::remove_if(in_msgs.second.begin(), in_msgs.second.end(), [&Options](P2P_Data_Message_pb& msg)
            {
                return msg.user_id() == Options->RemoteUserId->to_string();
            }), in_msgs.second.end());
        }

        if (conn.status != p2p_state_t::status_e::closed)
        {
            if (conn.socket_name == target_sock_name)
            {
                conn.status = p2p_state_t::status_e::closed;

                P2P_Connection_Close_pb* close = new P2P_Connection_Close_pb;
                send_p2p_connetion_close(Options->RemoteUserId->to_string(), close);
            }
        }
    }
    
    return EOS_EResult::EOS_Success;
}

/**
 * Close any open Connections for a specific Peer Connection ID.
 *
 * @param Options Information about who would like to close connections, and by what socket ID
 * @return EOS_EResult::EOS_Success - if the provided data is valid
 *         EOS_EResult::EOS_InvalidParameters - if the provided data is invalid
 */
EOS_EResult EOSSDK_P2P::CloseConnections(const EOS_P2P_CloseConnectionsOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->SocketId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    std::string target_sock_name = Options->SocketId->SocketName;
    for (auto& conn : _p2p_connections)
    {
        if (conn.second.socket_name == target_sock_name)
        {
            conn.second.status = p2p_state_t::status_e::closed;

            P2P_Connection_Close_pb* close = new P2P_Connection_Close_pb;
            send_p2p_connetion_close(conn.first->to_string(), close);
        }
    }
    
    return EOS_EResult::EOS_Success;
}

/**
 * Query the current NAT-type of our connection.
 *
 * @param Options Information about what version of the EOS_P2P_QueryNATType API they support
 * @param NATTypeQueriedHandler The callback to be fired when we finish querying our NAT type
 */
void EOSSDK_P2P::QueryNATType(const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::DEBUG, "TODO");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);
    EOS_P2P_OnQueryNATTypeCompleteInfo& pqntci = res->CreateCallback<EOS_P2P_OnQueryNATTypeCompleteInfo>((CallbackFunc)NATTypeQueriedHandler, std::chrono::milliseconds(15000));
    pqntci.ClientData = ClientData;
    pqntci.NATType = EOS_ENATType::EOS_NAT_Moderate;

    if (Options == nullptr)
    {
        pqntci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        pqntci.ResultCode = EOS_EResult::EOS_Success;
    }
    
    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Get our last-queried NAT-type, if it has been successfully queried.
 *
 * @param Options Information about what version of the EOS_P2P_GetNATType API they support
 * @param OutNATType The queried NAT Type, or unknown if unknown
 * @return EOS_EResult::EOS_Success - if we have cached data
 *         EOS_EResult::EOS_NotFound - If we do not have queried data cached
 *         EOS_EResult::EOS_IncompatibleVersion - If the provided version is unknown
 */
EOS_EResult EOSSDK_P2P::GetNATType(const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::DEBUG, "TODO");
    GLOBAL_LOCK();

    *OutNATType = EOS_ENATType::EOS_NAT_Moderate;
    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_P2P::send_p2p_connection_request(Network::peer_t const& peerid, P2P_Connect_Request_pb* req) const
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_connect_request(req);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_P2P::send_p2p_connection_response(Network::peer_t const& peerid, P2P_Connect_Response_pb* resp) const
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_connect_response(resp);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_P2P::send_p2p_data(Network::peer_t const& peerid, P2P_Data_Message_pb* data) const
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_data_message(data);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);
    auto res = GetNetwork().UDPSendTo(msg);

    p2p->release_data_message();

    return res;
}

bool EOSSDK_P2P::send_p2p_data_ack(Network::peer_t const& peerid, P2P_Data_Acknowledge_pb* ack) const
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_data_acknowledge(ack);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().UDPSendTo(msg);
}

bool EOSSDK_P2P::send_p2p_connetion_close(Network::peer_t const& peerid, P2P_Connection_Close_pb* close) const
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_connection_close(close);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().TCPSendTo(msg);
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_P2P::on_p2p_connection_request(Network_Message_pb const& msg, P2P_Connect_Request_pb const& req)
{
    GLOBAL_LOCK();
    TRACE_FUNC();

    auto peer_id = GetProductUserId(msg.source_id());
    auto& conn = _p2p_connections[peer_id];
    if (conn.status != p2p_state_t::status_e::connected)
    {
        conn.status = p2p_state_t::status_e::requesting;
        std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(this, EOS_P2P_OnIncomingConnectionRequestInfo::k_iCallback));
        for (auto& notif : notifs)
        {
            EOS_P2P_OnIncomingConnectionRequestInfo& oicrc = notif->GetCallback<EOS_P2P_OnIncomingConnectionRequestInfo>();
            oicrc.RemoteUserId = peer_id;
            strncpy(const_cast<char*>(oicrc.SocketId->SocketName), req.socket_name().c_str(), sizeof(EOS_P2P_SocketId::SocketName));

            notif->res.cb_func(notif->res.data);
        }
    }
    else
    {
        P2P_Connect_Response_pb* resp = new P2P_Connect_Response_pb;
        resp->set_accepted(true);
        send_p2p_connection_response(msg.source_id(), resp);
    }

    return true;
}

bool EOSSDK_P2P::on_p2p_connection_response(Network_Message_pb const& msg, P2P_Connect_Response_pb const& resp)
{
    GLOBAL_LOCK();
    TRACE_FUNC();
    
    if (resp.accepted())
    {
        auto& conn = _p2p_connections[GetProductUserId(msg.source_id())];
        conn.status = p2p_state_t::status_e::connected;

        for (auto& out_msgs : conn.p2p_out_messages)
        {
            while(!out_msgs.second.empty())
            {
                send_p2p_data(msg.source_id(), &out_msgs.second.front());
                out_msgs.second.pop();
            }
        }
    }
    else
    {
        std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(this, EOS_P2P_OnRemoteConnectionClosedInfo::k_iCallback));
        for (auto& notif : notifs)
        {
            EOS_P2P_OnRemoteConnectionClosedInfo& orcci = notif->GetCallback<EOS_P2P_OnRemoteConnectionClosedInfo>();
            orcci.Reason = EOS_EConnectionClosedReason::EOS_CCR_ClosedByPeer;
            orcci.RemoteUserId = GetProductUserId(msg.source_id());

            notif->res.cb_func(notif->res.data);
        }
    }

    return true;
}

bool EOSSDK_P2P::on_p2p_data(Network_Message_pb const& msg, P2P_Data_Message_pb const& data)
{
    GLOBAL_LOCK();
    TRACE_FUNC();

    P2P_Data_Acknowledge_pb* ack = new P2P_Data_Acknowledge_pb;
    ack->set_channel(data.channel());

    _p2p_in_messages[data.channel()].emplace_back(data);

    return send_p2p_data_ack(msg.source_id(), ack);
}

bool EOSSDK_P2P::on_p2p_data_ack(Network_Message_pb const& msg, P2P_Data_Acknowledge_pb const& ack)
{
    GLOBAL_LOCK();
    TRACE_FUNC();

    return true;
}

bool EOSSDK_P2P::on_p2p_connection_close(Network_Message_pb const& msg, P2P_Connection_Close_pb const& close)
{
    GLOBAL_LOCK();
    TRACE_FUNC();

    std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(this, EOS_P2P_OnRemoteConnectionClosedInfo::k_iCallback));
    for (auto& notif : notifs)
    {
        EOS_P2P_OnRemoteConnectionClosedInfo& orcci = notif->GetCallback<EOS_P2P_OnRemoteConnectionClosedInfo>();
        orcci.Reason = EOS_EConnectionClosedReason::EOS_CCR_ClosedByPeer;
        orcci.RemoteUserId = GetProductUserId(msg.source_id());

        notif->res.cb_func(notif->res.data);
    }

    _p2p_connections[GetProductUserId(msg.source_id())].status = p2p_state_t::status_e::closed;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_P2P::CBRunFrame()
{
    GLOBAL_LOCK();

    return true;
}

bool EOSSDK_P2P::RunNetwork(Network_Message_pb const& msg)
{
    if (GetProductUserId(msg.source_id()) == GetEOS_Connect().product_id())
        return true;

    P2P_Message_pb const& p2p = msg.p2p();

    switch (p2p.message_case())
    {
        case P2P_Message_pb::MessageCase::kConnectRequest : return on_p2p_connection_request(msg, p2p.connect_request());
        case P2P_Message_pb::MessageCase::kConnectResponse: return on_p2p_connection_response(msg, p2p.connect_response());
        case P2P_Message_pb::MessageCase::kDataMessage    : return on_p2p_data(msg, p2p.data_message());
        case P2P_Message_pb::MessageCase::kDataAcknowledge: return on_p2p_data_ack(msg, p2p.data_acknowledge());
        case P2P_Message_pb::MessageCase::kConnectionClose: return on_p2p_connection_close(msg, p2p.connection_close());
        default: LOG(Log::LogLevel::WARN, "Unhandled network message %d", p2p.message_case());
    }

    return true;
}

bool EOSSDK_P2P::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_P2P::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case callback_type::k_iCallback:
        //{
        //    callback_type& callback = res->GetCallback<callback_type>();
        //    // Free resources
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        case EOS_P2P_OnIncomingConnectionRequestInfo::k_iCallback:
        {
            EOS_P2P_OnIncomingConnectionRequestInfo& callback = res->GetCallback<EOS_P2P_OnIncomingConnectionRequestInfo>();
            delete callback.SocketId;
        }
        break;
        case EOS_P2P_OnRemoteConnectionClosedInfo::k_iCallback:
        {
            EOS_P2P_OnRemoteConnectionClosedInfo& callback = res->GetCallback<EOS_P2P_OnRemoteConnectionClosedInfo>();
            delete callback.SocketId;
        }
        break;
    }
}

}
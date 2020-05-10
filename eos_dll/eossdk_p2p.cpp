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

EOSSDK_P2P::EOSSDK_P2P()
{
    GetCB_Manager().register_callbacks(this);
    GetCB_Manager().register_frame(this);

    //_p2p_task.run(&EOSSDK_P2P::p2p_task_proc, this);
}

EOSSDK_P2P::~EOSSDK_P2P()
{
    GetCB_Manager().unregister_frame(this);
    GetCB_Manager().unregister_callbacks(this);

    _p2p_task.stop();
    _p2p_task.join();
}

void EOSSDK_P2P::p2p_task_proc()
{
    while (!_p2p_task.want_stop())
    {
        {
            LOCAL_LOCK();
        }
    }
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
    LOG(Log::LogLevel::TRACE, "");
    LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();
    
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
    LOG(Log::LogLevel::TRACE, "");
    LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();
    
    return EOS_EResult::EOS_Success;
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
    //LOG(Log::LogLevel::TRACE, "");
    //LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();
    
    return EOS_EResult::EOS_NotFound;
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
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

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
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

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
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
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
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

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
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    if (Options == nullptr || Options->RemoteUserId == nullptr || Options->SocketId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    auto& conn = _p2p_connections[Options->RemoteUserId];

    if (conn.status == p2p_state_t::status_e::requested)
    {
        P2P_Connect_Response_pb* resp = new P2P_Connect_Response_pb;
        resp->set_accepted(true);
        resp->set_channel(-1);
        send_p2p_connection_response(GetEOS_Connect()._myself.first->to_string(), resp);
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
    LOG(Log::LogLevel::TRACE, "");
    LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();
    
    
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
    LOG(Log::LogLevel::TRACE, "");
    LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();

    
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
    LOG(Log::LogLevel::TRACE, "");
    LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();

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
    LOG(Log::LogLevel::TRACE, "");
    LOG(Log::LogLevel::DEBUG, "TODO");
    LOCAL_LOCK();

    *OutNATType = EOS_ENATType::EOS_NAT_Moderate;
    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_P2P::send_p2p_connection_request(Network::peer_t const& peerid, P2P_Connect_Request_pb* req) const
{
    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_connect_request(req);

    msg.set_source_id(GetEOS_Connect().product_id()->to_string());
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().SendTo(msg);
}

bool EOSSDK_P2P::send_p2p_connection_response(Network::peer_t const& peerid, P2P_Connect_Response_pb* resp) const
{
    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_connect_response(resp);

    msg.set_source_id(GetEOS_Connect().product_id()->to_string());
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().SendTo(msg);
}

bool EOSSDK_P2P::send_p2p_data(Network::peer_t const& peerid, P2P_Data_Message_pb* data) const
{
    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_data_message(data);

    msg.set_source_id(GetEOS_Connect().product_id()->to_string());
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().SendTo(msg);
}

bool EOSSDK_P2P::send_p2p_data_ack(Network::peer_t const& peerid, P2P_Data_Acknowledge_pb* ack) const
{
    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_data_acknowledge(ack);

    msg.set_source_id(GetEOS_Connect().product_id()->to_string());
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().SendTo(msg);
}

bool EOSSDK_P2P::send_p2p_connetion_close(Network::peer_t const& peerid, P2P_Connection_Close_pb* close) const
{
    Network_Message_pb msg;
    P2P_Message_pb* p2p = new P2P_Message_pb;

    p2p->set_allocated_connection_close(close);

    msg.set_source_id(GetEOS_Connect().product_id()->to_string());
    msg.set_dest_id(peerid);

    msg.set_allocated_p2p(p2p);

    return GetNetwork().SendTo(msg);
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_P2P::on_p2p_connection_request(Network_Message_pb const& msg, P2P_Connect_Request_pb const& req)
{
    LOCAL_LOCK();

    return true;
}

bool EOSSDK_P2P::on_p2p_connection_response(Network_Message_pb const& msg, P2P_Connect_Response_pb const& resp)
{
    LOCAL_LOCK();

    return true;
}

bool EOSSDK_P2P::on_p2p_data(Network_Message_pb const& msg, P2P_Data_Message_pb const& data)
{
    LOCAL_LOCK();

    return true;
}

bool EOSSDK_P2P::on_p2p_data_ack(Network_Message_pb const& msg, P2P_Data_Acknowledge_pb const& ack)
{
    LOCAL_LOCK();

    return true;
}

bool EOSSDK_P2P::on_p2p_connection_close(Network_Message_pb const& msg, P2P_Connection_Close_pb const& close)
{
    LOCAL_LOCK();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_P2P::CBRunFrame()
{
    LOCAL_LOCK();

    return true;
}

bool EOSSDK_P2P::RunNetwork(Network_Message_pb const& msg)
{
    if (GetEpicUserId(msg.source_id()) == Settings::Inst().userid)
        return true;

    P2P_Message_pb const& p2p = msg.p2p();

    switch (p2p.message_case())
    {
        case P2P_Message_pb::MessageCase::kConnectRequest : return on_p2p_connection_request(msg, p2p.connect_request());
        case P2P_Message_pb::MessageCase::kConnectResponse: return on_p2p_connection_response(msg, p2p.connect_response());
        case P2P_Message_pb::MessageCase::kDataMessage    : return on_p2p_data(msg, p2p.data_message());
        case P2P_Message_pb::MessageCase::kDataAcknowledge: return on_p2p_data_ack(msg, p2p.data_acknowledge());
        case P2P_Message_pb::MessageCase::kConnectionClose: return on_p2p_connection_close(msg, p2p.connection_close());
    }

    return true;
}

bool EOSSDK_P2P::RunCallbacks(pFrameResult_t res)
{
    LOCAL_LOCK();

    return res->done;
}

void EOSSDK_P2P::FreeCallback(pFrameResult_t res)
{
    LOCAL_LOCK();

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
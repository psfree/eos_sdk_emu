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
#include "network.h"

namespace sdk
{
    struct p2p_state_t
    {
        enum class status_e
        {
            closed,
            requesting,
            connecting,
            connected,
            connection_loss,
        };

        status_e status;
        std::list<P2P_Data_Message_pb> p2p_out_messages;
        std::string socket_name;
        std::chrono::steady_clock::time_point connection_loss_start;

        p2p_state_t() :
            status(status_e::closed)
        {}
    };

    class EOSSDK_P2P :
        public IRunFrame
    {
        constexpr static auto connecting_timeout = std::chrono::milliseconds(5000);
        constexpr static auto connection_timeout = std::chrono::milliseconds(10000);

        std::recursive_mutex local_mutex;

        int32_t next_requested_channel;
        std::unordered_map<uint8_t, std::list<P2P_Data_Message_pb>> _p2p_in_messages;
        std::unordered_map<EOS_ProductUserId, p2p_state_t> _p2p_connections;

        EOS_ERelayControl _relay_control;
        uint16_t _p2p_port;
        uint16_t _max_additional_ports_to_try;

    public:
        EOSSDK_P2P();
        ~EOSSDK_P2P();

        void set_p2p_state_connected(EOS_ProductUserId remote_id, p2p_state_t& state);

        // Send Network messages
        bool send_p2p_connection_request(Network::peer_t const& peerid, P2P_Connect_Request_pb *req) const;
        bool send_p2p_connection_response(Network::peer_t const& peerid, P2P_Connect_Response_pb *resp) const;
        bool send_p2p_data(Network::peer_t const& peerid, P2P_Data_Message_pb *data) const;
        bool send_p2p_data_ack(Network::peer_t const& peerid, P2P_Data_Acknowledge_pb *ack) const;
        bool send_p2p_connetion_close(Network::peer_t const& peerid, P2P_Connection_Close_pb *close) const;

        // Receive Network messages
        bool on_peer_connect(Network_Message_pb const& msg, Network_Peer_Connect_pb const& peer);
        bool on_peer_disconnect(Network_Message_pb const& msg, Network_Peer_Disconnect_pb const& peer);
        bool on_p2p_connection_request(Network_Message_pb const& msg, P2P_Connect_Request_pb const& req);
        bool on_p2p_connection_response(Network_Message_pb const& msg, P2P_Connect_Response_pb const& resp);
        bool on_p2p_data(Network_Message_pb const& msg, P2P_Data_Message_pb const& data);
        bool on_p2p_data_ack(Network_Message_pb const& msg, P2P_Data_Acknowledge_pb const& ack);
        bool on_p2p_connection_close(Network_Message_pb const& msg, P2P_Connection_Close_pb const& close);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        EOS_EResult        SendPacket(const EOS_P2P_SendPacketOptions* Options);
        EOS_EResult        GetNextReceivedPacketSize(const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes);
        EOS_EResult        ReceivePacket(const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten);
        EOS_NotificationId AddNotifyPeerConnectionRequest(const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler);
        void               RemoveNotifyPeerConnectionRequest(EOS_NotificationId NotificationId);
        EOS_NotificationId AddNotifyPeerConnectionClosed(const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler);
        void               RemoveNotifyPeerConnectionClosed(EOS_NotificationId NotificationId);
        EOS_EResult        AcceptConnection(const EOS_P2P_AcceptConnectionOptions* Options);
        EOS_EResult        CloseConnection(const EOS_P2P_CloseConnectionOptions* Options);
        EOS_EResult        CloseConnections(const EOS_P2P_CloseConnectionsOptions* Options);
        void               QueryNATType(const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler);
        EOS_EResult        GetNATType(const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType);
        EOS_EResult        SetRelayControl(const EOS_P2P_SetRelayControlOptions* Options);
        EOS_EResult        GetRelayControl(const EOS_P2P_GetRelayControlOptions* Options, EOS_ERelayControl* OutRelayControl);
        EOS_EResult        SetPortRange(const EOS_P2P_SetPortRangeOptions* Options);
        EOS_EResult        GetPortRange(const EOS_P2P_GetPortRangeOptions* Options, uint16_t* OutPort, uint16_t* OutNumAdditionalPortsToTry);
    };
}
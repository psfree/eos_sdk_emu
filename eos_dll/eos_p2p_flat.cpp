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

using namespace sdk;

/**
 * P2P functions to help manage sending and receiving of messages to peers
 *
 * These functions will attempt to perform NAT-punching, but will fallback to relays if a direct connection cannot be established
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SendPacket(EOS_HP2P Handle, const EOS_P2P_SendPacketOptions* Options)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->SendPacket(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNextReceivedPacketSize(EOS_HP2P Handle, const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->GetNextReceivedPacketSize(Options, OutPacketSizeBytes);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ReceivePacket(EOS_HP2P Handle, const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->ReceivePacket(Options, OutPeerId, OutSocketId, OutChannel, OutData, OutBytesWritten);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionRequest(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->AddNotifyPeerConnectionRequest(Options, ClientData, ConnectionRequestHandler);
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    pInst->RemoveNotifyPeerConnectionRequest(NotificationId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionClosed(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->AddNotifyPeerConnectionClosed(Options, ClientData, ConnectionClosedHandler);
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionClosed(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    pInst->RemoveNotifyPeerConnectionClosed(NotificationId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_AcceptConnection(EOS_HP2P Handle, const EOS_P2P_AcceptConnectionOptions* Options)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->AcceptConnection(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnection(EOS_HP2P Handle, const EOS_P2P_CloseConnectionOptions* Options)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->CloseConnection(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnections(EOS_HP2P Handle, const EOS_P2P_CloseConnectionsOptions* Options)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->CloseConnections(Options);
}

EOS_DECLARE_FUNC(void) EOS_P2P_QueryNATType(EOS_HP2P Handle, const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    pInst->QueryNATType(Options, ClientData, NATTypeQueriedHandler);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNATType(EOS_HP2P Handle, const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType)
{
    auto pInst = reinterpret_cast<EOSSDK_P2P*>(Handle);
    return pInst->GetNATType(Options, OutNATType);
}

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
    class EOSSDK_Presence;

    class EOSSDK_PresenceModification
    {
        friend EOSSDK_Presence;

        Presence_Info_pb infos;

        std::mutex _local_mutex;

    public:
        EOS_EResult SetStatus(const EOS_PresenceModification_SetStatusOptions* Options);
        EOS_EResult SetRawRichText(const EOS_PresenceModification_SetRawRichTextOptions* Options);
        EOS_EResult SetData( const EOS_PresenceModification_SetDataOptions* Options);
        EOS_EResult DeleteData(const EOS_PresenceModification_DeleteDataOptions* Options);
        EOS_EResult SetJoinInfo(const EOS_PresenceModification_SetJoinInfoOptions* Options);
    };

    class EOSSDK_Presence :
        public IRunFrame
    {
        static constexpr auto presence_query_timeout = std::chrono::milliseconds(1000);

        nlohmann::fifo_map<EOS_EpicAccountId, Presence_Info_pb> _presences;
        std::map<EOS_EpicAccountId, std::list<pFrameResult_t>> _presence_queries;

    public:
        EOSSDK_Presence();
        ~EOSSDK_Presence();

        void setup_myself();
        inline Presence_Info_pb& get_myself();
        Presence_Info_pb* get_presence(EOS_EpicAccountId userid);
        void trigger_presence_change(EOS_EpicAccountId userid);
        void set_user_status(EOS_EpicAccountId userid, EOS_Presence_EStatus status); // Will call trigger_presence_change

        // Send Network messages
        bool send_presence_info_request(Network::peer_t const& peerid, Presence_Info_Request_pb* req);
        bool send_my_presence_info(Network::peer_t const& peerid);
        bool send_my_presence_info_to_all_peers();

        // Receive Network messages
        bool on_presence_request(Network_Message_pb const& msg, Presence_Info_Request_pb const& req);
        bool on_presence_infos(Network_Message_pb const& msg, Presence_Info_pb const& infos);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void QueryPresence( const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate);
        EOS_Bool HasPresence( const EOS_Presence_HasPresenceOptions* Options);
        EOS_EResult CopyPresence( const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence);
        EOS_EResult CreatePresenceModification( const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle);
        void SetPresence( const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate);
        EOS_NotificationId AddNotifyOnPresenceChanged( const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler);
        void RemoveNotifyOnPresenceChanged( EOS_NotificationId NotificationId);
        EOS_NotificationId AddNotifyJoinGameAccepted( const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn);
        void RemoveNotifyJoinGameAccepted( EOS_NotificationId InId);
        EOS_EResult GetJoinInfo( const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength);
    };
}
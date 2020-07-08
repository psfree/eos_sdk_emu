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
    class EOSSDK_Sessions;

    class EOSSDK_SessionModification
    {
    public:
        friend class sdk::EOSSDK_Sessions;
        enum class modif_type : uint8_t
        {
            creation = 0,
            update = 1,
        };

    private:

        std::mutex _local_mutex;

        uint32_t _api_version;
        modif_type _type;
        std::string _session_name;
        Session_Infos_pb _infos;

    public:
        
        EOS_EResult SetBucketId(const EOS_SessionModification_SetBucketIdOptions* Options);
        EOS_EResult SetHostAddress(const EOS_SessionModification_SetHostAddressOptions* Options);
        EOS_EResult SetPermissionLevel(const EOS_SessionModification_SetPermissionLevelOptions* Options);
        EOS_EResult SetJoinInProgressAllowed(const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options);
        EOS_EResult SetMaxPlayers(const EOS_SessionModification_SetMaxPlayersOptions* Options);
        EOS_EResult SetInvitesAllowed(const EOS_SessionModification_SetInvitesAllowedOptions* Options);
        EOS_EResult AddAttribute(const EOS_SessionModification_AddAttributeOptions* Options);
        EOS_EResult RemoveAttribute(const EOS_SessionModification_RemoveAttributeOptions* Options);
        void        Release();
    };

    class EOSSDK_SessionSearch:
        public IRunFrame
    {
        friend class sdk::EOSSDK_Sessions;
     
        static constexpr auto search_timeout = std::chrono::milliseconds(5000);
        static std::atomic<uint64_t> search_id;

        std::mutex _local_mutex;

        bool                       _released;
        Sessions_Search_pb         _search_infos;
        std::list<Session_Infos_pb> _results;
        pFrameResult_t             _search_cb;
        std::set<std::string>      _search_peers;

    public:
        EOSSDK_SessionSearch();
        ~EOSSDK_SessionSearch();

        bool released();

        // Send Network messages
        bool send_sessions_search(Sessions_Search_pb *search);

        // Receive Network messages
        bool on_sessions_search_response(Network_Message_pb const& msg, Sessions_Search_response_pb const& resp);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        EOS_EResult SetSessionId(const EOS_SessionSearch_SetSessionIdOptions* Options);
        EOS_EResult SetTargetUserId(const EOS_SessionSearch_SetTargetUserIdOptions* Options);
        EOS_EResult SetParameter(const EOS_SessionSearch_SetParameterOptions* Options);
        EOS_EResult RemoveParameter(const EOS_SessionSearch_RemoveParameterOptions* Options);
        EOS_EResult SetMaxResults(const EOS_SessionSearch_SetMaxResultsOptions* Options);
        void        Find(const EOS_SessionSearch_FindOptions* Options, void* ClientData, const EOS_SessionSearch_OnFindCallback CompletionDelegate);
        uint32_t    GetSearchResultCount(const EOS_SessionSearch_GetSearchResultCountOptions* Options);
        EOS_EResult CopySearchResultByIndex(const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle);
        void        Release();
    };

    class EOSSDK_SessionDetails
    {
        friend class sdk::EOSSDK_Sessions;
        friend class sdk::EOSSDK_SessionSearch;

        std::string _session_name;
        Session_Infos_pb _infos;

    public:
        EOS_EResult CopyInfo(const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info** OutSessionInfo);
        uint32_t    GetSessionAttributeCount(const EOS_SessionDetails_GetSessionAttributeCountOptions* Options);
        EOS_EResult CopySessionAttributeByIndex(const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute);
        EOS_EResult CopySessionAttributeByKey(const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute);
        void        Release();
    };

    class EOSSDK_ActiveSession
    {
        friend class sdk::EOSSDK_Sessions;

        std::string _session_name;
        Session_Infos_pb _infos;

    public:
        EOS_EResult       CopyInfo(const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info** OutActiveSessionInfo);
        uint32_t          GetRegisteredPlayerCount(const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options);
        EOS_ProductUserId GetRegisteredPlayerByIndex(const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options);
        void              Release();
    };

    struct session_state_t
    {
        enum state_e
        {
            created,
            joined,
            joining,
        } state;
        Session_Infos_pb infos;
    };

    struct session_invite_t
    {
        std::string invite_id;
        EOS_ProductUserId peer_id;
        Session_Infos_pb infos;
    };

    class EOSSDK_Sessions :
        public IRunFrame
    {

        static constexpr auto join_timeout = std::chrono::milliseconds(5000);
        // key: session_id
        std::map<std::string, pFrameResult_t> _sessions_join;
        std::list<session_invite_t>           _session_invites;
        std::list<EOSSDK_SessionSearch*>      _session_searchs;

    public:
        EOSSDK_Sessions();
        ~EOSSDK_Sessions();

        std::map<std::string, session_state_t> _sessions;

        session_state_t* get_session_by_name(std::string const& session_name);
        session_state_t* get_session_by_id(std::string const& session_id);
        std::vector<session_state_t*> get_sessions_from_attributes(google::protobuf::Map<std::string, Session_Search_Parameter> const& parameters);
        void add_player_to_session(std::string const& player, session_state_t* session);
        void register_player_to_session(std::string const& player, session_state_t *session);
        void remove_player_from_session(std::string const& player, session_state_t *session);
        void unregister_player_from_session(std::string const& player, session_state_t* session);
        bool is_player_in_session(std::string const& player, session_state_t* session);
        bool is_player_registered(std::string const& player, session_state_t *session);

        // Send Network messages
        bool send_to_all_members(Network_Message_pb & msg, session_state_t *session);
        bool send_session_info_request(Network::peer_t const& peerid, Session_Infos_Request_pb* req);
        bool send_session_info(session_state_t* session);
        bool send_session_destroy(session_state_t* session);
        bool send_sessions_search_response(Network::peer_t const& peerid, Sessions_Search_response_pb* resp);
        bool send_session_join_request(session_state_t* session);
        bool send_session_join_response(Network::peer_t const& peerid, Session_Join_Response_pb* resp);
        bool send_session_invite(Network::peer_t const& peerid, Session_Invite_pb* invite);
        bool send_session_invite_response(Network::peer_t const& peerid, Session_Invite_Response_pb* invite);

        // Receive Network messages
        bool on_peer_disconnect(Network_Message_pb const& msg, Network_Peer_Disconnect_pb const& peer);
        bool on_session_info_request(Network_Message_pb const& msg, Session_Infos_Request_pb const& req);
        bool on_session_info(Network_Message_pb const& msg, Session_Infos_pb const& infos);
        bool on_session_destroy(Network_Message_pb const& msg, Session_Destroy_pb const& destr);
        bool on_sessions_search(Network_Message_pb const& msg, Sessions_Search_pb const& search);
        bool on_session_join_request(Network_Message_pb const& msg, Session_Join_Request_pb const& req);
        bool on_session_join_response(Network_Message_pb const& msg, Session_Join_Response_pb const& resp);
        bool on_session_invite(Network_Message_pb const& msg, Session_Invite_pb const& invite);
        bool on_session_invite_response(Network_Message_pb const& msg, Session_Invite_Response_pb const& resp);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        EOS_EResult        CreateSessionModification(const EOS_Sessions_CreateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle);
        EOS_EResult        UpdateSessionModification(const EOS_Sessions_UpdateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle);
        void               UpdateSession(const EOS_Sessions_UpdateSessionOptions* Options, void* ClientData, const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate);
        void               DestroySession(const EOS_Sessions_DestroySessionOptions* Options, void* ClientData, const EOS_Sessions_OnDestroySessionCallback CompletionDelegate);
        void               JoinSession(const EOS_Sessions_JoinSessionOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionCallback CompletionDelegate);
        void               StartSession(const EOS_Sessions_StartSessionOptions* Options, void* ClientData, const EOS_Sessions_OnStartSessionCallback CompletionDelegate);
        void               EndSession(const EOS_Sessions_EndSessionOptions* Options, void* ClientData, const EOS_Sessions_OnEndSessionCallback CompletionDelegate);
        void               RegisterPlayers(const EOS_Sessions_RegisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate);
        void               UnregisterPlayers(const EOS_Sessions_UnregisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate);
        void               SendInvite(const EOS_Sessions_SendInviteOptions* Options, void* ClientData, const EOS_Sessions_OnSendInviteCallback CompletionDelegate);
        void               RejectInvite(const EOS_Sessions_RejectInviteOptions* Options, void* ClientData, const EOS_Sessions_OnRejectInviteCallback CompletionDelegate);
        void               QueryInvites(const EOS_Sessions_QueryInvitesOptions* Options, void* ClientData, const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate);
        uint32_t           GetInviteCount(const EOS_Sessions_GetInviteCountOptions* Options);
        EOS_EResult        GetInviteIdByIndex(const EOS_Sessions_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength);
        EOS_EResult        CreateSessionSearch(const EOS_Sessions_CreateSessionSearchOptions* Options, EOS_HSessionSearch* OutSessionSearchHandle);
        EOS_EResult        CopyActiveSessionHandle(const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle);
        EOS_NotificationId AddNotifySessionInviteReceived(const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn);
        void               RemoveNotifySessionInviteReceived(EOS_NotificationId InId);
        EOS_NotificationId AddNotifySessionInviteAccepted(const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn);
        void               RemoveNotifySessionInviteAccepted(EOS_NotificationId InId);
        EOS_NotificationId AddNotifyJoinSessionAccepted(const EOS_Sessions_AddNotifyJoinSessionAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionAcceptedCallback NotificationFn);
        void               RemoveNotifyJoinSessionAccepted(EOS_NotificationId InId);
        EOS_EResult        CopySessionHandleByInviteId(const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options, EOS_HSessionDetails* OutSessionHandle);
        EOS_EResult        CopySessionHandleByUiEventId(const EOS_Sessions_CopySessionHandleByUiEventIdOptions* Options, EOS_HSessionDetails* OutSessionHandle);
        EOS_EResult        CopySessionHandleForPresence(const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle);
        EOS_EResult        IsUserInSession(const EOS_Sessions_IsUserInSessionOptions* Options);
        EOS_EResult        DumpSessionState(const EOS_Sessions_DumpSessionStateOptions* Options);
    };
}
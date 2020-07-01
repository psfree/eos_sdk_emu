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

#include "eossdk_lobby.h"
#include "eossdk_platform.h"

namespace sdk
{

decltype(EOSSDK_LobbySearch::search_timeout) EOSSDK_LobbySearch::search_timeout;
decltype(EOSSDK_LobbySearch::search_id)      EOSSDK_LobbySearch::search_id(0);

EOSSDK_LobbySearch::EOSSDK_LobbySearch() :
    _released(false)
{
    GetCB_Manager().register_callbacks(this);

    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kLobbiesSearch);
}

EOSSDK_LobbySearch::~EOSSDK_LobbySearch()
{
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kLobbiesSearch);

    GetCB_Manager().unregister_callbacks(this);
}

bool EOSSDK_LobbySearch::released()
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    bool res = _released;
    return res;
}

/**
 * Class responsible for the creation, setup, and execution of a search query.
 * Search parameters are defined, the query is executed and the search results are returned within this object
 */

 /**
  * Find lobbies matching the search criteria setup via this lobby search handle.
  * When the operation completes, this handle will have the search results that can be parsed
  *
  * @param Options Structure containing information about the search criteria to use
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate A callback that is fired when the search operation completes, either successfully or in error
  *
  * @return EOS_Success if the find operation completes successfully
  *         EOS_NotFound if searching for an individual lobby by lobby id or target user id returns no results
  *         EOS_InvalidParameters if any of the options are incorrect
  */
void EOSSDK_LobbySearch::Find(const EOS_LobbySearch_FindOptions* Options, void* ClientData, const EOS_LobbySearch_OnFindCallback CompletionDelegate)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);
    
    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_LobbySearch_FindCallbackInfo& fci = res->CreateCallback<EOS_LobbySearch_FindCallbackInfo>((CallbackFunc)CompletionDelegate);

    fci.ClientData = ClientData;

    if (_search_cb.get() != nullptr)
    {
        fci.ResultCode = EOS_EResult::EOS_AlreadyPending;
        res->done = true;
    }
    // If the user has set parameters and session_id or target userid, it fails
    // If sessiondid and target userid is set, it fails
    else if (
             (_search_infos.parameters_size() != 0 && 
                 (!_search_infos.target_id().empty() || !_search_infos.lobby_id().empty())) ||
             (!_search_infos.target_id().empty() && !_search_infos.lobby_id().empty())
            )
    {
        fci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        res->done = true;
    }
    else
    {
        _search_cb = res;
        _search_infos.set_search_id(search_id++);
        send_lobbies_search(&_search_infos);
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * Set a lobby id to find and will return at most one search result.  Setting TargetUserId or SearchParameters will result in EOS_LobbySearch_Find failing
 *
 * @param Options A specific lobby id for which to search
 *
 * @return EOS_Success if setting this lobby id was successful
 *         EOS_InvalidParameters if the lobby id is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbySearch::SetLobbyId(const EOS_LobbySearch_SetLobbyIdOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->LobbyId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _search_infos.set_lobby_id(Options->LobbyId);

    return EOS_EResult::EOS_Success;
}

/**
 * Set a target user id to find.  Setting LobbyId or SearchParameters will result in EOS_LobbySearch_Find failing
 * NOTE: a search result will only be found if this user is in a public lobby
 *
 * @param Options a specific target user id to find
 *
 * @return EOS_Success if setting this target user id was successful
 *         EOS_InvalidParameters if the target user id is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbySearch::SetTargetUserId(const EOS_LobbySearch_SetTargetUserIdOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->TargetUserId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _search_infos.set_target_id(Options->TargetUserId->to_string());

    return EOS_EResult::EOS_Success;
}

/** NYI */
EOS_EResult EOSSDK_LobbySearch::SetParameter(const EOS_LobbySearch_SetParameterOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->Parameter == nullptr || Options->Parameter->Key == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto &params = *_search_infos.mutable_parameters();
    auto& param = *params[Options->Parameter->Key].mutable_param();
    Lobby_Attr_Value& value = param[static_cast<int32_t>(Options->ComparisonOp)];

    switch (Options->Parameter->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: value.set_b(Options->Parameter->Value.AsBool)  ; break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : value.set_i(Options->Parameter->Value.AsInt64) ; break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : value.set_d(Options->Parameter->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : value.set_s(Options->Parameter->Value.AsUtf8)  ; break;
    }

    return EOS_EResult::EOS_Success;
}

/** NYI */
EOS_EResult EOSSDK_LobbySearch::RemoveParameter(const EOS_LobbySearch_RemoveParameterOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if(Options == nullptr || Options->Key == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    auto& params = *_search_infos.mutable_parameters();
    auto& param = *params[Options->Key].mutable_param();
    auto it = param.find(static_cast<int32_t>(Options->ComparisonOp));

    if (it == param.end())
    {
        return EOS_EResult::EOS_NotFound;
    }
   
    param.erase(it);
    return EOS_EResult::EOS_Success;
}

/**
 * Set the maximum number of search results to return in the query, can't be more than EOS_LOBBY_MAX_SEARCH_RESULTS
 *
 * @param Options maximum number of search results to return in the query
 *
 * @return EOS_Success if setting the max results was successful
 *         EOS_InvalidParameters if the number of results requested is invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbySearch::SetMaxResults(const EOS_LobbySearch_SetMaxResultsOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->MaxResults == 0)
        return EOS_EResult::EOS_InvalidParameters;

    _max_results = Options->MaxResults;
    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of search results found by the search parameters in this search
 *
 * @param Options Options associated with the search count
 *
 * @return return the number of search results found by the query or 0 if search is not complete
 */
uint32_t EOSSDK_LobbySearch::GetSearchResultCount(const EOS_LobbySearch_GetSearchResultCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr || _search_cb.get() != nullptr)
        return 0;

    return _results.size();
}

/**
 * EOS_LobbySearch_CopySearchResultByIndex is used to immediately retrieve a handle to the lobby information from a given search result.
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutLobbyDetailsHandle out parameter used to receive the lobby details handle
 *
 * @return EOS_Success if the information is available and passed out in OutLobbyDetailsHandle
 *         EOS_InvalidParameters if you pass an invalid index or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_LobbySearch_CopySearchResultByIndexOptions
 * @see EOS_LobbyDetails_Release
 */
EOS_EResult EOSSDK_LobbySearch::CopySearchResultByIndex(const EOS_LobbySearch_CopySearchResultByIndexOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->LobbyIndex >= _results.size() || OutLobbyDetailsHandle == nullptr)
    {
        *OutLobbyDetailsHandle = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOSSDK_LobbyDetails* pLobbyDetails = new EOSSDK_LobbyDetails;

    auto it = _results.begin();
    std::advance(it, Options->LobbyIndex);
    pLobbyDetails->_infos = *it;

    *OutLobbyDetailsHandle = reinterpret_cast<EOS_HLobbyDetails>(pLobbyDetails);
    return EOS_EResult::EOS_Success;
}

/**
 * Release the memory associated with a lobby search. This must be called on data retrieved from EOS_Lobby_CreateLobbySearch.
 *
 * @param LobbySearchHandle - The lobby search handle to release
 *
 * @see EOS_Lobby_CreateLobbySearch
 */
void EOSSDK_LobbySearch::Release()
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    _released = true;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_LobbySearch::send_lobbies_search(Lobbies_Search_pb* search)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Lobbies_Search_Message_pb* search_msg = new Lobbies_Search_Message_pb();

    msg.set_source_id(user_id);

    search_msg->set_allocated_search(search);
    msg.set_allocated_lobbies_search(search_msg);

    if (_search_infos.target_id().empty())
    {
        _search_peers = std::move(GetNetwork().TCPSendToAllPeers(msg));
    }
    else
    {
        msg.set_dest_id(_search_infos.target_id());
        if (GetNetwork().TCPSendTo(msg))
        {
            _search_peers.emplace(_search_infos.target_id());
        }
    }

    search_msg->release_search(); // Don't delete our search infos

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_LobbySearch::on_lobbies_search_response(Network_Message_pb const& msg, Lobbies_Search_response_pb const& resp)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (_search_cb.get() != nullptr && resp.search_id() == _search_infos.search_id())
    {
        _search_peers.erase(msg.source_id());
        if (_results.size() < _max_results)
        {
            for (auto const& lobby : resp.lobbies())
            {
                if (_results.size() < _max_results)
                    _results.emplace_back(lobby);
                else
                    break;
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_LobbySearch::CBRunFrame()
{
    return false;
}

bool EOSSDK_LobbySearch::RunNetwork(Network_Message_pb const& msg)
{
    Lobbies_Search_Message_pb const& search = msg.lobbies_search();

    switch (search.message_case())
    {
       case Lobbies_Search_Message_pb::MessageCase::kSearchResponse: return on_lobbies_search_response(msg, search.search_response());
    }

    return true;
}

bool EOSSDK_LobbySearch::RunCallbacks(pFrameResult_t res)
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    if (_released)
        return res->done;

    switch (res->res.m_iCallback)
    {
        case EOS_LobbySearch_FindCallbackInfo::k_iCallback:
        {
            EOS_LobbySearch_FindCallbackInfo& fci = res->GetCallback<EOS_LobbySearch_FindCallbackInfo>();
            if (_search_peers.empty() || 
                (std::chrono::steady_clock::now() - _search_cb->created_time) > search_timeout)
            {// All peers answered or Search timeout
                if (_results.empty())
                    fci.ResultCode = EOS_EResult::EOS_NotFound;
                else
                    fci.ResultCode = EOS_EResult::EOS_Success;
    
                _search_cb.reset();
                res->done = true;
            }
        }
        break;
    }

    return res->done;
}

void EOSSDK_LobbySearch::FreeCallback(pFrameResult_t res)
{
    std::lock_guard<std::mutex> lk(_local_mutex);

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case EOS_LobbySearch::k_iCallback:
        //{
        //    EOS_SessionSearch_FindCallbackInfo& callback = res->GetCallback<EOS_SessionSearch_FindCallbackInfo>();
        //    delete[]callback.InviteId;
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

}
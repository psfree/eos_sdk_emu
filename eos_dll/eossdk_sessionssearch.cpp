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

#include "eossdk_sessions.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_SessionSearch::search_timeout) EOSSDK_SessionSearch::search_timeout;
decltype(EOSSDK_SessionSearch::search_id) EOSSDK_SessionSearch::search_id(0);

EOSSDK_SessionSearch::EOSSDK_SessionSearch():
    _target_userid(nullptr)
{
    GetCB_Manager().register_callbacks(this);

    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kSessionsSearch);
}

EOSSDK_SessionSearch::~EOSSDK_SessionSearch()
{
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kSessionsSearch);

    GetCB_Manager().unregister_callbacks(this);
}

/**
 * Class responsible for the creation, setup, and execution of a search query.
 * Search parameters are defined, the query is executed and the search results are returned within this object
 */

 /**
  * Set a session id to find and will return at most one search result.  Setting TargetUserId or SearchParameters will result in EOS_SessionSearch_Find failing
  *
  * @param Options A specific session id for which to search
  *
  * @return EOS_Success if setting this session id was successful
  *         EOS_InvalidParameters if the session id is invalid or null
  *         EOS_IncompatibleVersion if the API version passed in is incorrect
  */
EOS_EResult EOSSDK_SessionSearch::SetSessionId(const EOS_SessionSearch_SetSessionIdOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->SessionId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    _search_infos.set_sessionid(Options->SessionId);

    return EOS_EResult::EOS_Success;
}

/**
 * Set a target user id to find and will return at most one search result.  Setting SessionId or SearchParameters will result in EOS_SessionSearch_Find failing
 * NOTE: a search result will only be found if this user is in a public session
 *
 * @param Options a specific target user id to find
 *
 * @return EOS_Success if setting this target user id was successful
 *         EOS_InvalidParameters if the target user id is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionSearch::SetTargetUserId(const EOS_SessionSearch_SetTargetUserIdOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->TargetUserId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _target_userid = Options->TargetUserId;
    
    return EOS_EResult::EOS_Success;
}

/**
 * Add a parameter to an array of search criteria combined via an AND operator.  Setting SessionId or TargetUserId will result in EOS_SessionSearch_Find failing
 *
 * @param Options a search parameter and its comparison op
 *
 * @return EOS_Success if setting this search parameter was successful
 *         EOS_InvalidParameters if the search criteria is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Sessions_AttributeData
 * @see EOS_EOnlineComparisonOp
 */
EOS_EResult EOSSDK_SessionSearch::SetParameter(const EOS_SessionSearch_SetParameterOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->Parameter == nullptr || Options->Parameter->Key == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto &params = *_search_infos.mutable_parameters();
    auto& param = *params[Options->Parameter->Key].mutable_param();
    Session_Attr_Value& value = param[static_cast<int32_t>(Options->ComparisonOp)];

    switch (Options->Parameter->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: value.set_b(Options->Parameter->Value.AsBool)  ; break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : value.set_i(Options->Parameter->Value.AsInt64) ; break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : value.set_d(Options->Parameter->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : value.set_s(Options->Parameter->Value.AsUtf8)  ; break;
    }
    return EOS_EResult::EOS_Success;
}

/**
 * Remove a parameter from the array of search criteria.
 *
 * @params Options a search parameter key name to remove
 *
 * @return EOS_Success if removing this search parameter was successful
 *         EOS_InvalidParameters if the search key is invalid or null
 *		   EOS_NotFound if the parameter was not a part of the search criteria
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionSearch::RemoveParameter(const EOS_SessionSearch_RemoveParameterOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if(Options == nullptr || Options->Key == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    auto& params = *_search_infos.mutable_parameters();
    auto& param = *params[Options->Key].mutable_param();
    auto it = param.find(static_cast<int32_t>(Options->ComparisonOp));

    if (it != param.end())
    {
        param.erase(it);
        return EOS_EResult::EOS_Success;
    }
    
    return EOS_EResult::EOS_NotFound;
}

/**
 * Set the maximum number of search results to return in the query, can't be more than EOS_SESSIONS_MAX_SEARCH_RESULTS
 *
 * @param Options maximum number of search results to return in the query
 *
 * @return EOS_Success if setting the max results was successful
 *         EOS_InvalidParameters if the number of results requested is invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionSearch::SetMaxResults(const EOS_SessionSearch_SetMaxResultsOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if(Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _search_infos.set_max_results(Options->MaxSearchResults);
    
    return EOS_EResult::EOS_Success;
}

/**
 * Find sessions matching the search criteria setup via this session search handle.
 * When the operation completes, this handle will have the search results that can be parsed
 *
 * @param Options Structure containing information about the search criteria to use
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the search operation completes, either successfully or in error
 *
 * @return EOS_Success if the find operation completes successfully
 *         EOS_NotFound if searching for an individual session by sessionid or targetuserid returns no results
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_SessionSearch::Find(const EOS_SessionSearch_FindOptions* Options, void* ClientData, const EOS_SessionSearch_OnFindCallback CompletionDelegate)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);
    
    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_SessionSearch_FindCallbackInfo& fci = res->CreateCallback<EOS_SessionSearch_FindCallbackInfo>((CallbackFunc)CompletionDelegate);

    fci.ClientData = ClientData;

    if (_search_cb.get() != nullptr)
    {
        fci.ResultCode = EOS_EResult::EOS_AlreadyPending;
        res->done = true;
    }
    // If the user has set parameters and sessionid or target userid, it fails
    // If sessiondid and target userid is set, it fails
    else if (
             (_search_infos.parameters_size() != 0 && 
                 (_target_userid != nullptr || !_search_infos.sessionid().empty())) ||
             (_target_userid != nullptr && !_search_infos.sessionid().empty())
            )
    {
        fci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        res->done = true;
    }
    else
    {
        fci.ResultCode = EOS_EResult::EOS_Success;
        _search_cb = res;
        _search_infos.set_search_id(search_id++);
        send_sessions_search(&_search_infos);
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * Get the number of search results found by the search parameters in this search
 *
 * @param Options Options associated with the search count
 *
 * @return return the number of search results found by the query or 0 if search is not complete
 */
uint32_t EOSSDK_SessionSearch::GetSearchResultCount(const EOS_SessionSearch_GetSearchResultCountOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);
    
    return static_cast<uint32_t>(_results.size());
}

/**
 * EOS_SessionSearch_CopySearchResultByIndex is used to immediately retrieve a handle to the session information from a given search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid index or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionSearch_CopySearchResultByIndexOptions
 * @see EOS_SessionDetails_Release
 */
EOS_EResult EOSSDK_SessionSearch::CopySearchResultByIndex(const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || OutSessionHandle == nullptr || Options->SessionIndex >= _results.size())
        return EOS_EResult::EOS_InvalidParameters;

    EOSSDK_SessionDetails* details = new EOSSDK_SessionDetails;

    auto it = _results.begin();
    std::advance(it, Options->SessionIndex);
    details->_infos = *it;

    *OutSessionHandle = reinterpret_cast<EOS_HSessionDetails>(details);
    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_SessionSearch::send_sessions_search(Sessions_Search_pb* search)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Sessions_Search_Message_pb* search_msg = new Sessions_Search_Message_pb();

    msg.set_source_id(user_id);

    search_msg->set_allocated_search(search);
    msg.set_allocated_sessions_search(search_msg);

    _search_peers = std::move(GetNetwork().TCPSendToAllPeers(msg));
    search_msg->release_search(); // Don't delete our search infos

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_SessionSearch::on_sessions_search_response(Network_Message_pb const& msg, Sessions_Search_response_pb const& resp)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (_search_cb.get() != nullptr && resp.search_id() == _search_infos.search_id())
    {
        _search_peers.erase(msg.source_id());
        if (!resp.session_infos().sessionname().empty())
            _results.emplace_back(resp.session_infos());
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_SessionSearch::CBRunFrame()
{
    

    return false;
}

bool EOSSDK_SessionSearch::RunNetwork(Network_Message_pb const& msg)
{
    Sessions_Search_Message_pb const& session = msg.sessions_search();

    switch (session.message_case())
    {
        case Sessions_Search_Message_pb::MessageCase::kSearchResponse: return on_sessions_search_response(msg, session.search_response());
    }

    return true;
}

bool EOSSDK_SessionSearch::RunCallbacks(pFrameResult_t res)
{
    std::lock_guard<std::mutex> lk(_local_mutex);

    switch (res->res.m_iCallback)
    {
        case EOS_SessionSearch_FindCallbackInfo::k_iCallback:
        {
            if (_search_peers.empty())
            {
                _search_cb.reset();
                res->done = true;
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                if ((now - _search_cb->created_time) > search_timeout)
                {
                    _search_cb.reset();
                    res->done = true;
                }
            }
        }
        break;
    }

    return res->done;
}

void EOSSDK_SessionSearch::FreeCallback(pFrameResult_t res)
{
    std::lock_guard<std::mutex> lk(_local_mutex);

    //switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case EOS_Sessions_UpdateSessionCallbackInfo::k_iCallback:
        //{
        //    EOS_Sessions_UpdateSessionCallbackInfo& usci = res->GetCallback<EOS_Sessions_UpdateSessionCallbackInfo>();
        //    delete[]usci.SessionId;
        //    delete[]usci.SessionName;
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
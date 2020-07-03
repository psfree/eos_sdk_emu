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

#include "eossdk_metrics.h"

namespace sdk
{

EOSSDK_Metrics::EOSSDK_Metrics()
{}

EOSSDK_Metrics::~EOSSDK_Metrics()
{}

/**
 * Logs the start of a new game session for a local player.
 *
 * The game client should call this function whenever it joins into a new multiplayer, peer-to-peer or single player game session.
 * Each call to BeginPlayerSession must be matched with a corresponding call to EndPlayerSession.
 *
 * @param Options Structure containing the local player's game account and the game session information.
 *
 * @return Returns EOS_Success on success, or an error code if the input parameters are invalid or an active session for the player already exists.
 */
EOS_EResult EOSSDK_Metrics::BeginPlayerSession(const EOS_Metrics_BeginPlayerSessionOptions* Options)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_Success;
}

/**
 * Logs the end of a game session for a local player.
 *
 * Call once when the game client leaves the active game session.
 * Each call to BeginPlayerSession must be matched with a corresponding call to EndPlayerSession.
 *
 * @param Options Structure containing the account id of the player whose session to end.
 *
 * @return Returns EOS_Success on success, or an error code if the input parameters are invalid or there was no active session for the player.
 */
EOS_EResult EOSSDK_Metrics::EndPlayerSession(const EOS_Metrics_EndPlayerSessionOptions* Options)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Metrics::CBRunFrame()
{
    return false;
}

bool EOSSDK_Metrics::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_Metrics::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Metrics::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    //switch (res->res.m_iCallback)
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
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

}
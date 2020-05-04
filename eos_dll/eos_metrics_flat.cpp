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

#include "eos_metrics.h"

using namespace sdk;

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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_BeginPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_BeginPlayerSessionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Metrics*>(Handle);
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_EndPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_EndPlayerSessionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Metrics*>(Handle);
    return EOS_EResult::EOS_Success;
}

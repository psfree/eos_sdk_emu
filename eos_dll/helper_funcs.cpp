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

#include "helper_funcs.h"

union epicid_t {
    struct {
        uint64_t part1;
        uint64_t part2;
    };
    uint8_t id[16];

    inline std::string to_string() const
    {
        std::stringstream sstr;

        sstr << "0x" << std::hex;
        if (part2 != 0)
            sstr << part2;

        sstr << part1;
        return sstr.str();
    }
};

LOCAL_API std::random_device& get_rd()
{
    // Random device generator
    static std::random_device rd;
    return rd;
}

LOCAL_API std::mt19937_64& get_gen()
{
    static std::mt19937_64 gen(get_rd()());
    return gen;
}

LOCAL_API std::recursive_mutex& global_mutex()
{
    static std::recursive_mutex global_mutex;
    return global_mutex;
}

LOCAL_API static void randombytes(uint8_t* buf, size_t len)
{
    // uniform integer distribution
    std::uniform_int_distribution<int64_t> dis;
    std::mt19937_64& gen = get_gen();

    dis(gen); dis(gen); dis(gen);
    dis(gen); dis(gen); dis(gen);

    // Make sure we can hold our buffer size as int64_t
    size_t rand_buf_len = len / sizeof(int64_t) + (len % sizeof(int64_t) ? 1 : 0);
    int64_t* rand_buf = new int64_t[rand_buf_len];
    // Generate some (pseudo) random numbers
    for (size_t i = 0; i < rand_buf_len; ++i)
        rand_buf[i] = dis(gen);

    // Copy the random bytes to buffer
    memcpy(buf, rand_buf, len);

    // Don't forget to free it
    delete[]rand_buf;
}

LOCAL_API std::string generate_account_id()
{
    epicid_t epicid;
    
    randombytes(epicid.id, sizeof(epicid));

    return epicid.to_string();
}

LOCAL_API std::string generate_account_idd_from_name(std::string const& username)
{
    epicid_t epicid = {};
    epicid_t base = {};
    uint16_t i;

    while (epicid.part1 == 0 && epicid.part2 == 0)
    {
        epicid = base;
        if ((base.part1 + 0x0000001201030307ULL) < base.part1)
            base.part2 += static_cast<uint64_t>(static_cast<double>(std::numeric_limits<uint64_t>::max()) - base.part1 + static_cast<double>(0x0000001201030307));

        i = 0;
        std::for_each(username.begin(), username.end(), [&epicid, &i](const char& c)
        {
            uint8_t b = static_cast<uint8_t>(c);
            reinterpret_cast<uint8_t*>(&epicid)[i++ % sizeof(epicid)] ^= (b + i * 27);
            reinterpret_cast<uint8_t*>(&epicid)[i   % sizeof(epicid)] ^= (b - i * 8);
        });
    }

    return epicid.to_string();
}

LOCAL_API EOS_EpicAccountIdDetails generate_epic_id_user()
{
    epicid_t epicid = {};
    while(epicid.part1 == 0 && epicid.part2 == 0)
        generate_account_id();

    return epicid.to_string();
}

LOCAL_API EOS_EpicAccountIdDetails generate_epic_id_user_from_name(std::string const& username)
{
    epicid_t epicid = {};
    while (epicid.part1 == 0 && epicid.part2 == 0)
        generate_account_idd_from_name(username);

    return epicid.to_string();
}

LOCAL_API void fatal_throw(const char* msg)
{
    LOG(Log::LogLevel::FATAL, "%s", msg);

    throw std::exception();
}

LOCAL_API bool load_json(std::string const& file_path, nlohmann::json &json)
{
    std::ifstream file(file_path);
    if (file)
    {
        file.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        
        std::string buffer(size, '\0');

        file.read(&buffer[0], size);
        file.close();

        try
        {
            json = std::move(nlohmann::json::parse(buffer));

            return true;
        }
        catch (std::exception &e)
        {
            LOG(Log::LogLevel::ERR, "Error while parsing JSON %s: %s", file_path.c_str(), e.what());
        }
    }
    else
    {
        LOG(Log::LogLevel::WARN, "File not found: %s", file_path.c_str());
    }
    return false;
}

LOCAL_API bool save_json(std::string const& file_path, nlohmann::json const& json)
{
    std::ofstream file(file_path, std::ios::trunc | std::ios::out);
    if (!file)
    {
        LOG(Log::LogLevel::ERR, "Failed to save: %s", file_path.c_str());
        return false;
    }
    file << std::setw(2) << json;
    return true;
}

LOCAL_API std::string get_callback_name(int iCallback)
{
    switch (iCallback)
    {
#define I_CALLBACK(TYPE) case TYPE::k_iCallback: return #TYPE
        // Auth
        I_CALLBACK(EOS_Auth_LoginCallbackInfo);
        I_CALLBACK(EOS_Auth_LogoutCallbackInfo);
        I_CALLBACK(EOS_Auth_VerifyUserAuthCallbackInfo);
        I_CALLBACK(EOS_Auth_DeletePersistentAuthCallbackInfo);
        I_CALLBACK(EOS_Auth_LoginStatusChangedCallbackInfo);
        // Achievements
        I_CALLBACK(EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo);
        I_CALLBACK(EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo);
        // Connect
        I_CALLBACK(EOS_Connect_LoginCallbackInfo);
        I_CALLBACK(EOS_Connect_CreateUserCallbackInfo);
        I_CALLBACK(EOS_Connect_LinkAccountCallbackInfo);
        I_CALLBACK(EOS_Connect_CreateDeviceIdCallbackInfo);
        I_CALLBACK(EOS_Connect_DeleteDeviceIdCallbackInfo);
        I_CALLBACK(EOS_Connect_QueryExternalAccountMappingsCallbackInfo);
        I_CALLBACK(EOS_Connect_QueryProductUserIdMappingsCallbackInfo);
        I_CALLBACK(EOS_Connect_AuthExpirationCallbackInfo);
        I_CALLBACK(EOS_Connect_LoginStatusChangedCallbackInfo);
        // Ecom
        I_CALLBACK(EOS_Ecom_QueryOwnershipCallbackInfo);
        I_CALLBACK(EOS_Ecom_QueryOwnershipTokenCallbackInfo);
        I_CALLBACK(EOS_Ecom_QueryEntitlementsCallbackInfo);
        I_CALLBACK(EOS_Ecom_QueryOffersCallbackInfo);
        I_CALLBACK(EOS_Ecom_CheckoutCallbackInfo);
        I_CALLBACK(EOS_Ecom_RedeemEntitlementsCallbackInfo);
        // Friends
        I_CALLBACK(EOS_Friends_QueryFriendsCallbackInfo);
        I_CALLBACK(EOS_Friends_SendInviteCallbackInfo);
        I_CALLBACK(EOS_Friends_AcceptInviteCallbackInfo);
        I_CALLBACK(EOS_Friends_RejectInviteCallbackInfo);
        I_CALLBACK(EOS_Friends_DeleteFriendCallbackInfo);
        I_CALLBACK(EOS_Friends_OnFriendsUpdateInfo);
        // Leaderboards
        I_CALLBACK(EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo);
        I_CALLBACK(EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo);
        I_CALLBACK(EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo);
        // Lobby
        I_CALLBACK(EOS_Lobby_CreateLobbyCallbackInfo);
        I_CALLBACK(EOS_Lobby_DestroyLobbyCallbackInfo);
        I_CALLBACK(EOS_Lobby_JoinLobbyCallbackInfo);
        I_CALLBACK(EOS_Lobby_LeaveLobbyCallbackInfo);
        I_CALLBACK(EOS_Lobby_UpdateLobbyCallbackInfo);
        I_CALLBACK(EOS_Lobby_PromoteMemberCallbackInfo);
        I_CALLBACK(EOS_Lobby_KickMemberCallbackInfo);
        I_CALLBACK(EOS_Lobby_LobbyUpdateReceivedCallbackInfo);
        I_CALLBACK(EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo);
        I_CALLBACK(EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo);
        I_CALLBACK(EOS_Lobby_LobbyInviteReceivedCallbackInfo);
        I_CALLBACK(EOS_Lobby_SendInviteCallbackInfo);
        I_CALLBACK(EOS_Lobby_RejectInviteCallbackInfo);
        I_CALLBACK(EOS_Lobby_QueryInvitesCallbackInfo);
        I_CALLBACK(EOS_LobbySearch_FindCallbackInfo);
        // Metrics
        // P2P
        I_CALLBACK(EOS_P2P_OnIncomingConnectionRequestInfo);
        I_CALLBACK(EOS_P2P_OnRemoteConnectionClosedInfo);
        I_CALLBACK(EOS_P2P_OnQueryNATTypeCompleteInfo);
        // PlayerDataStorage
        I_CALLBACK(EOS_PlayerDataStorage_QueryFileCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_QueryFileListCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_DuplicateFileCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_DeleteFileCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_FileTransferProgressCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_ReadFileDataCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_ReadFileCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_WriteFileDataCallbackInfo);
        I_CALLBACK(EOS_PlayerDataStorage_WriteFileCallbackInfo);
        // Presence
        I_CALLBACK(EOS_Presence_QueryPresenceCallbackInfo);
        I_CALLBACK(EOS_Presence_SetPresenceCallbackInfo);
        I_CALLBACK(EOS_Presence_PresenceChangedCallbackInfo);
        I_CALLBACK(EOS_Presence_JoinGameAcceptedCallbackInfo);
        // Sessions
        I_CALLBACK(EOS_Sessions_SendInviteCallbackInfo);
        I_CALLBACK(EOS_Sessions_RejectInviteCallbackInfo);
        I_CALLBACK(EOS_Sessions_QueryInvitesCallbackInfo);
        I_CALLBACK(EOS_Sessions_UpdateSessionCallbackInfo);
        I_CALLBACK(EOS_Sessions_DestroySessionCallbackInfo);
        I_CALLBACK(EOS_Sessions_JoinSessionCallbackInfo);
        I_CALLBACK(EOS_Sessions_StartSessionCallbackInfo);
        I_CALLBACK(EOS_Sessions_EndSessionCallbackInfo);
        I_CALLBACK(EOS_Sessions_RegisterPlayersCallbackInfo);
        I_CALLBACK(EOS_Sessions_UnregisterPlayersCallbackInfo);
        I_CALLBACK(EOS_SessionSearch_FindCallbackInfo);
        I_CALLBACK(EOS_Sessions_SessionInviteReceivedCallbackInfo);
        I_CALLBACK(EOS_Sessions_SessionInviteAcceptedCallbackInfo);
        // Stats
        I_CALLBACK(EOS_Stats_IngestStatCompleteCallbackInfo);
        I_CALLBACK(EOS_Stats_OnQueryStatsCompleteCallbackInfo);
        // UI
        I_CALLBACK(EOS_UI_ShowFriendsCallbackInfo);
        I_CALLBACK(EOS_UI_HideFriendsCallbackInfo);
        // UserInfo
        I_CALLBACK(EOS_UserInfo_QueryUserInfoCallbackInfo);
        I_CALLBACK(EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo);
#undef I_CALLBACK
    }

    return "";
}

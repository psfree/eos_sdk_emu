// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#pragma pack(push, 8)

/** Handle to the lobby interface */
EXTERN_C typedef struct EOS_LobbyHandle* EOS_HLobby;
/** Handle to a lobby modification object */
EXTERN_C typedef struct EOS_LobbyModificationHandle* EOS_HLobbyModification;
/** Handle to a single lobby */
EXTERN_C typedef struct EOS_LobbyDetailsHandle* EOS_HLobbyDetails;
/** Handle to the calls responsible for creating a search object */
EXTERN_C typedef struct EOS_LobbySearchHandle* EOS_HLobbySearch;

EOS_DECLARE_FUNC(void) EOS_LobbyModification_Release(EOS_HLobbyModification LobbyModificationHandle);

/**
 * Release the memory associated with a single lobby. This must be called on data retrieved from EOS_LobbySearch_CopySearchResultByIndex.
 *
 * @param LobbyHandle - The lobby handle to release
 *
 * @see EOS_LobbySearch_CopySearchResultByIndex
 */
EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Release(EOS_HLobbyDetails LobbyHandle);

/**
 * Release the memory associated with a lobby search. This must be called on data retrieved from EOS_Lobby_CreateLobbySearch.
 *
 * @param LobbySearchHandle - The lobby search handle to release
 *
 * @see EOS_Lobby_CreateLobbySearch
 */
EOS_DECLARE_FUNC(void) EOS_LobbySearch_Release(EOS_HLobbySearch LobbySearchHandle);

/** All lobbies are referenced by a unique lobby id */
EXTERN_C typedef const char* EOS_LobbyId;

#define EOS_LOBBYDETAILS_INFO_API_001 1

EOS_STRUCT(EOS_LobbyDetails_Info001, (
	/** API Version: Set this to EOS_LOBBYDETAILS_INFO_API_LATEST. */
	int32_t ApiVersion;
	/** Lobby ID */
	EOS_LobbyId LobbyId;
	/** The Product User ID of the current owner of the lobby */
	EOS_ProductUserId LobbyOwnerUserId;
	/** Permission level of the lobby */
	EOS_ELobbyPermissionLevel PermissionLevel;
	/** Current available space */
	uint32_t AvailableSlots;
	/** Max allowed members in the lobby */
	uint32_t MaxMembers;
	/** If true, users can invite others to this lobby */
	EOS_Bool bAllowInvites;
));

/** The most recent version of the EOS_Lobby_CreateLobby API. */
#define EOS_LOBBY_CREATELOBBY_API_002 2

/**
 * Input parameters for the EOS_Lobby_CreateLobby function.
 */
EOS_STRUCT(EOS_Lobby_CreateLobbyOptions002, (
	/** API Version: Set this to EOS_LOBBY_CREATELOBBY_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user creating the lobby; this user will automatically join the lobby as its owner */
	EOS_ProductUserId LocalUserId;
	/** The maximum number of users who can be in the lobby at a time */
	uint32_t MaxLobbyMembers;
	/** The initial permission level of the lobby */
	EOS_ELobbyPermissionLevel PermissionLevel;
	/** If true, this lobby will be associated with presence information. A user's presence can only be associated with one lobby at a time.
	 * This affects the ability of the Social Overlay to show game related actions to take in the user's social graph.
	 *
	 * @note The Social Overlay can handle only one of the following three options at a time:
	 * * using the bPresenceEnabled flags within the Sessions interface
	 * * using the bPresenceEnabled flags within the Lobby interface
	 * * using EOS_PresenceModification_SetJoinInfo
	 *
	 * @see EOS_PresenceModification_SetJoinInfoOptions
	 * @see EOS_Lobby_JoinLobbyOptions
	 * @see EOS_Sessions_CreateSessionModificationOptions
	 * @see EOS_Sessions_JoinSessionOptions
	 */
	EOS_Bool bPresenceEnabled;
));


/** The most recent version of the EOS_Lobby_JoinLobby API. */
#define EOS_LOBBY_JOINLOBBY_API_002 2

/**
 * Input parameters for the EOS_Lobby_JoinLobby function.
 */
EOS_STRUCT(EOS_Lobby_JoinLobbyOptions002, (
	/** API Version: Set this to EOS_LOBBY_JOINLOBBY_API_LATEST. */
	int32_t ApiVersion;
	/** The handle of the lobby to join */
	EOS_HLobbyDetails LobbyDetailsHandle;
	/** The Product User ID of the local user joining the lobby */
	EOS_ProductUserId LocalUserId;
	/** If true, this lobby will be associated with the user's presence information. A user can only associate one lobby at a time with their presence information.
	 * This affects the ability of the Social Overlay to show game related actions to take in the user's social graph.
	 *
	 * @note The Social Overlay can handle only one of the following three options at a time:
	 * * using the bPresenceEnabled flags within the Sessions interface
	 * * using the bPresenceEnabled flags within the Lobby interface
	 * * using EOS_PresenceModification_SetJoinInfo
	 *
	 * @see EOS_PresenceModification_SetJoinInfoOptions
	 * @see EOS_Lobby_CreateLobbyOptions
	 * @see EOS_Lobby_JoinLobbyOptions
	 * @see EOS_Sessions_CreateSessionModificationOptions
	 * @see EOS_Sessions_JoinSessionOptions
	 */
	EOS_Bool bPresenceEnabled;
));

/**
 * Function prototype definition for callbacks passed to EOS_Lobby_JoinLobby
 * @param Data A EOS_Lobby_JoinLobby CallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Lobby_OnJoinLobbyCallback, const EOS_Lobby_JoinLobbyCallbackInfo* Data);

/** The most recent version of the EOS_LobbyModification_AddAttribute API. */
#define EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_001 1

/**
 * Input parameters for the EOS_LobbyModification_AddAttribute function.
 */
EOS_STRUCT(EOS_LobbyModification_AddAttributeOptions001, (
	/** API Version: Set this to EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST. */
	int32_t ApiVersion;
	/** Key/Value pair describing the attribute to add to the lobby */
	const EOS_Lobby_AttributeData* Attribute;
	/** Is this attribute public or private to the lobby and its members */
	EOS_ELobbyAttributeVisibility Visibility;
));

/** The most recent version of the EOS_LobbyModification_AddMemberAttribute API. */
#define EOS_LOBBYMODIFICATION_ADDMEMBERATTRIBUTE_API_001 1

/**
 * Input parameters for the EOS_LobbyModification_AddMemberAttribute function.
 */
EOS_STRUCT(EOS_LobbyModification_AddMemberAttributeOptions001, (
	/** API Version: Set this to EOS_LOBBYMODIFICATION_ADDMEMBERATTRIBUTE_API_LATEST. */
	int32_t ApiVersion;
	/** Key/Value pair describing the attribute to add to the lobby member */
	const EOS_Lobby_AttributeData* Attribute;
	/** Is this attribute public or private to the rest of the lobby members */
	EOS_ELobbyAttributeVisibility Visibility;
));
#pragma pack(pop)

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma pack(push, 8)

/** The most recent version of the EOS_Lobby_CreateLobby API. */
#define EOS_LOBBY_CREATELOBBY_API_007 7

/**
 * Input parameters for the EOS_Lobby_CreateLobby function.
 */
EOS_STRUCT(EOS_Lobby_CreateLobbyOptions007, (
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
	/** Are members of the lobby allowed to invite others */
	EOS_Bool bAllowInvites;
	/** Bucket ID associated with the lobby */
	const char* BucketId;
	/**
	 * Is host migration allowed (will the lobby stay open if the original host leaves?)
	 * NOTE: EOS_Lobby_PromoteMember is still allowed regardless of this setting
	 */
	EOS_Bool bDisableHostMigration;
	/**
	 * Creates a real-time communication (RTC) room for all members of this lobby. All members of the lobby will automatically join the RTC
	 * room when they connect to the lobby and they will automatically leave the RTC room when they leave or are removed from the lobby.
	 * While the joining and leaving of the RTC room is automatic, applications will still need to use the EOS RTC interfaces to handle all
	 * other functionality for the room.
	 *
	 * @see EOS_Lobby_GetRTCRoomName
	 * @see EOS_Lobby_AddNotifyRTCRoomConnectionChanged
	 */
	EOS_Bool bEnableRTCRoom;
	/**
	 * (Optional) Allows the local application to set local audio options for the RTC Room if it is enabled. Set this to NULL if the RTC
	 * RTC room is disabled or you would like to use the defaults.
	 */
	const EOS_Lobby_LocalRTCOptions* LocalRTCOptions;
	/**
	 * (Optional) Set to a globally unique value to override the backend assignment
	 * If not specified the backend service will assign one to the lobby.  Do not mix and match override and non override settings.
	 * This value can be of size [EOS_LOBBY_MIN_LOBBYIDOVERRIDE_LENGTH, EOS_LOBBY_MAX_LOBBYIDOVERRIDE_LENGTH]
	 */
	const char* LobbyId;
));


/** The most recent version of the EOS_Lobby_JoinLobby API. */
#define EOS_LOBBY_JOINLOBBY_API_003 3

/**
 * Input parameters for the EOS_Lobby_JoinLobby function.
 */
EOS_STRUCT(EOS_Lobby_JoinLobbyOptions003, (
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
	/**
	 * (Optional) Set this value to override the default local options for the RTC Room, if it is enabled for this lobby. Set this to NULL if
	 * your application does not use the Lobby RTC Rooms feature, or if you would like to use the default settings. This option is ignored if
	 * the specified lobby does not have an RTC Room enabled and will not cause errors.
	 */
	const EOS_Lobby_LocalRTCOptions* LocalRTCOptions;
));

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

// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_PlatformHandle* EOS_HPlatform;

/** Client credentials. */
EOS_STRUCT(EOS_Platform_ClientCredentials, (
	/** Client id of the service permissions entry. Set to NULL if no service permissions are used. */
	const char* ClientId;
	/** Client secret for accessing the set of permissions. Set to NULL if no service permissions are used. */
	const char* ClientSecret;
));


#define EOS_COUNTRYCODE_MAX_LENGTH 4
#define EOS_COUNTRYCODE_MAX_BUFFER_LEN (EOS_COUNTRYCODE_MAX_LENGTH + 1)
#define EOS_LOCALECODE_MAX_LENGTH 9
#define EOS_LOCALECODE_MAX_BUFFER_LEN (EOS_LOCALECODE_MAX_LENGTH + 1)

#define EOS_PLATFORM_OPTIONS_API_006 6

/* Platform Creation Flags used in EOS_Platform_Create */

/** A bit that indicates the SDK is being loaded in a game editor, like Unity or UE4 Play-in-Editor */
#define EOS_PF_LOADING_IN_EDITOR        0x00001
/** A bit that indicates the SDK should skip initialization of the overlay, which is used by the in-app purchase flow and social overlay. This bit is implied by EOS_PF_LOADING_IN_EDITOR */
#define EOS_PF_DISABLE_OVERLAY          0x00002
/** A bit that indicates the SDK should skip initialization of the social overlay, which provides an overlay UI for social features. This bit is implied by EOS_PF_LOADING_IN_EDITOR or EOS_PF_DISABLE_OVERLAY */
#define EOS_PF_DISABLE_SOCIAL_OVERLAY   0x00004

/** Platform options for EOS_Platform_Create. */
EOS_STRUCT(EOS_Platform_Options006, (
	/** API version of EOS_Platform_Create. */
	int32_t ApiVersion;
	/** A reserved field that should always be nulled. */
	void* Reserved;
	/** The product id for the running application, found on the dev portal */
	const char* ProductId;
	/** The sandbox id for the running application, found on the dev portal */
	const char* SandboxId;
	/** Set of service permissions associated with the running application */
	EOS_Platform_ClientCredentials ClientCredentials;
	/** Is this running as a server */
	EOS_Bool bIsServer;
	/** Only used by Player Data Storage. Must be null initialized if unused. 256-bit Encryption Key for file encryption in hexadecimal format (64 hex chars)*/
	const char* EncryptionKey;
	/** The override country code to use for the logged in user. (EOS_COUNTRYCODE_MAX_LEN)*/
	const char* OverrideCountryCode;
	/** The override locale code to use for the logged in user. This follows ISO 639. (EOS_LOCALECODE_MAX_LEN)*/
	const char* OverrideLocaleCode;
	/** The deployment id for the running application, found on the dev portal */
	const char* DeploymentId;
	/** Platform creation flags. This is a bitwise-or union of EOS_PF_... flags defined above */
	uint64_t Flags;
	/** Only used by Player Data Storage. Must be null initialized if unused. Cache directory path. Absolute path to the folder that is going to be used for caching temporary data. The path is created if it's missing. */
	const char* CacheDirectory;
));

#pragma pack(pop)

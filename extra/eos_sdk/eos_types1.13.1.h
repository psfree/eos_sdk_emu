// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#pragma pack(push, 8)


/** The most recent version of the EOS_Platform_RTCOptions API. */
#define EOS_PLATFORM_RTCOPTIONS_API_001 1

/** Platform RTC options. */
EOS_STRUCT(EOS_Platform_RTCOptions001, (
	/** API Version: Set this to EOS_PLATFORM_RTCOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/**
	 * This field is for platform specific initialization if any.
	 *
	 * If provided then the structure will be located in <System>/eos_<System>.h.
	 * The structure will be named EOS_<System>_RTCOptions.
	 */
	void* PlatformSpecificOptions;
));

#define EOS_PLATFORM_OPTIONS_API_011 11

/** Platform options for EOS_Platform_Create. */
EOS_STRUCT(EOS_Platform_Options011, (
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
	/** Set this to EOS_FALSE if the application is running as a client with a local user, otherwise set to EOS_TRUE (e.g. for a dedicated game server) */
	EOS_Bool bIsServer;
	/** Only used by Player Data Storage. Must be null initialized if unused. 256-bit Encryption Key for file encryption in hexadecimal format (64 hex chars)*/
	const char* EncryptionKey;
	/** The override country code to use for the logged in user. (EOS_COUNTRYCODE_MAX_LENGTH)*/
	const char* OverrideCountryCode;
	/** The override locale code to use for the logged in user. This follows ISO 639. (EOS_LOCALECODE_MAX_LENGTH)*/
	const char* OverrideLocaleCode;
	/** The deployment id for the running application, found on the dev portal */
	const char* DeploymentId;
	/** Platform creation flags, e.g. EOS_PF_LOADING_IN_EDITOR. This is a bitwise-or union of the defined flags. */
	uint64_t Flags;
	/** Used by Player Data Storage and Title Storage. Must be null initialized if unused. Cache directory path. Absolute path to the folder that is going to be used for caching temporary data. The path is created if it's missing. */
	const char* CacheDirectory;
	/**
     * A budget, measured in milliseconds, for EOS_Platform_Tick to do its work. When the budget is met or exceeded (or if no work is available), EOS_Platform_Tick will return.
     * This allows your game to amortize the cost of SDK work across multiple frames in the event that a lot of work is queued for processing.
     * Zero is interpreted as "perform all available work"
     */
	uint32_t TickBudgetInMilliseconds;
	/** RTC options. Setting to NULL will disable RTC features (e.g. voice) */
	const EOS_Platform_RTCOptions* RTCOptions;
));

#pragma pack(pop)

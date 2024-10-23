// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#pragma pack(push, 8)

/** The most recent version of the EOS_Initialize_ThreadAffinity API. */
#define EOS_INITIALIZE_THREADAFFINITY_API_001 1

/**
 * Options for initializing defining thread affinity for use by Epic Online Services SDK.
 * Set the affinity to 0 to allow EOS SDK to use a platform specific default value.
 */
EOS_STRUCT(EOS_Initialize_ThreadAffinity001, (
       /** API Version: Set this to EOS_INITIALIZE_THREADAFFINITY_API_LATEST. */
       int32_t ApiVersion;
       /** Any thread related to network management that is not IO. */
       uint64_t NetworkWork;
       /** Any thread that will interact with a storage device. */
       uint64_t StorageIo;
       /** Any thread that will generate web socket IO. */
       uint64_t WebSocketIo;
       /** Any thread that will generate IO related to P2P traffic and mangement. */
       uint64_t P2PIo;
       /** Any thread that will generate http request IO. */
       uint64_t HttpRequestIo;
));

#pragma pack(pop)

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_P2PHandle* EOS_HP2P;

/**
 * A packet's maximum size in bytes
 */
#define EOS_P2P_MAX_PACKET_SIZE 1170

 /**
  * The maximum amount of unique Socket ID connections that can be opened with each remote user. As this limit is only per remote user, you may have more
  * than this number of Socket IDs across multiple remote users.
  */
#define EOS_P2P_MAX_CONNECTIONS 32

/** The most recent version of the EOS_P2P_SocketId structure. */
#define EOS_P2P_SOCKETID_API_001 1

/** The most recent version of the EOS_P2P_SendPacket API. */
#define EOS_P2P_SENDPACKET_API_002 2

/**
 * Structure containing information about the data being sent and to which player
 */
EOS_STRUCT(EOS_P2P_SendPacketOptions002, (
	/** API Version: Set this to EOS_P2P_SENDPACKET_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is sending this packet */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the Peer you would like to send a packet to */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID for data you are sending in this packet */
	const EOS_P2P_SocketId* SocketId;
	/** Channel associated with this data */
	uint8_t Channel;
	/** The size of the data to be sent to the RemoteUser */
	uint32_t DataLengthBytes;
	/** The data to be sent to the RemoteUser */
	const void* Data;
	/** If false and we do not already have an established connection to the peer, this data will be dropped */
	EOS_Bool bAllowDelayedDelivery;
	/** Setting to control the delivery reliability of this packet */
	EOS_EPacketReliability Reliability;
));

/** The most recent version of the EOS_P2P_GetNextReceivedPacketSize API. */
#define EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_002 2

/**
 * Structure containing information about who would like to receive a packet.
 */
EOS_STRUCT(EOS_P2P_GetNextReceivedPacketSizeOptions002, (
	/** API Version: Set this to EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is receiving the packet */
	EOS_ProductUserId LocalUserId;
	/** An optional channel to request the data for. If NULL, we're retrieving the size of the next packet on any channel. */
	const uint8_t* RequestedChannel;

));

/** The most recent version of the EOS_P2P_ReceivePacket API. */
#define EOS_P2P_RECEIVEPACKET_API_002 2

/**
 * Structure containing information about who would like to receive a packet, and how much data can be stored safely.
 */
EOS_STRUCT(EOS_P2P_ReceivePacketOptions002, (
	/** API Version: Set this to EOS_P2P_RECEIVEPACKET_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the user who is receiving the packet */
	EOS_ProductUserId LocalUserId;
	/** The maximum amount of data in bytes that can be safely copied to OutData in the function call */
	uint32_t MaxDataSizeBytes;
	/** An optional channel to request the data for. If NULL, we're retrieving the next packet on any channel */
	const uint8_t* RequestedChannel;
));

/**
 * Callback for information related to incoming connection requests.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnIncomingConnectionRequestCallback, const EOS_P2P_OnIncomingConnectionRequestInfo* Data);


/**
 * Callback for information related to open connections being closed.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnRemoteConnectionClosedCallback, const EOS_P2P_OnRemoteConnectionClosedInfo* Data);

/** The most recent version of the EOS_P2P_AcceptConnection API. */
#define EOS_P2P_ACCEPTCONNECTION_API_001 1

/**
 * Structure containing information about who would like to accept a connection, and which connection.
 */
EOS_STRUCT(EOS_P2P_AcceptConnectionOptions001, (
	/** API Version: Set this to EOS_P2P_ACCEPTCONNECTION_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is accepting any pending or future connections with RemoteUserId */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the remote user who has either sent a connection request or is expected to in the future */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID of the connection to accept on */
	const EOS_P2P_SocketId* SocketId;
));

/** The most recent version of the EOS_P2P_CloseConnection API. */
#define EOS_P2P_CLOSECONNECTION_API_001 1

/**
 * Structure containing information about who would like to close a connection, and which connection.
 */
EOS_STRUCT(EOS_P2P_CloseConnectionOptions001, (
	/** API Version: Set this to EOS_P2P_CLOSECONNECTION_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who would like to close a previously accepted connection (or decline a pending invite) */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the remote user to disconnect from (or to reject a pending invite from) */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID of the connection to close (or optionally NULL to not accept any connection requests from the Remote User) */
	const EOS_P2P_SocketId* SocketId;
));

/** The most recent version of the EOS_P2P_CloseConnections API. */
#define EOS_P2P_CLOSECONNECTIONS_API_001 1

/**
 * Structure containing information about who would like to close connections, and by what socket ID
 */
EOS_STRUCT(EOS_P2P_CloseConnectionsOptions001, (
	/** API Version: Set this to EOS_P2P_CLOSECONNECTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who would like to close all connections that use a particular socket ID */
	EOS_ProductUserId LocalUserId;
	/** The socket ID of the connections to close */
	const EOS_P2P_SocketId* SocketId;
));

/** The most recent version of the EOS_P2P_GetNATType API. */
#define EOS_P2P_GETNATTYPE_API_001 1

/**
 * Structure containing information needed to get perviously queried NAT-types
 */
EOS_STRUCT(EOS_P2P_GetNATTypeOptions001, (
	/** API Version: Set this to EOS_P2P_GETNATTYPE_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Setting for controlling whether relay servers are used
 */
EOS_ENUM(EOS_ERelayControl,
	/** Peer connections will never attempt to use relay servers. Clients with restrictive NATs may not be able to connect to peers. */
	EOS_RC_NoRelays = 0,
	/** Peer connections will attempt to use relay servers, but only after direct connection attempts fail. This is the default value if not changed. */
	EOS_RC_AllowRelays = 1,
	/** Peer connections will only ever use relay servers. This will add latency to all connections, but will hide IP Addresses from peers. */
	EOS_RC_ForceRelays = 2
);

/** The most recent version of the EOS_P2P_SetRelayControl API. */
#define EOS_P2P_SETRELAYCONTROL_API_001 1

/**
 * Structure containing information about new relay configurations.
 */
EOS_STRUCT(EOS_P2P_SetRelayControlOptions001, (
	/** API Version: Set this to EOS_P2P_SETRELAYCONTROL_API_LATEST. */
	int32_t ApiVersion;
	/**
	 * The requested level of relay servers for P2P connections. This setting is only applied to new P2P connections, or when existing P2P connections
	 * reconnect during a temporary connectivity outage. Peers with an incompatible setting to the local setting will not be able to connnect.
	 */
	EOS_ERelayControl RelayControl;
));

/** The most recent version of the EOS_P2P_SetPortRange API. */
#define EOS_P2P_SETPORTRANGE_API_001 1

/**
 * Structure containing information about new port range settings.
 */
EOS_STRUCT(EOS_P2P_SetPortRangeOptions001, (
	/** API Version: Set this to EOS_P2P_SETPORTRANGE_API_LATEST. */
	int32_t ApiVersion;
	/** The ideal port to use for P2P traffic. The default value is 7777. If set to 0, the OS will choose a port. If set to 0, MaxAdditionalPortsToTry must be set to 0. */
	uint16_t Port;
	/**
	 * The maximum amount of additional ports to try if Port is unavailable. Ports will be tried from Port to Port + MaxAdditionalPortsToTry
	 * inclusive, until one is available or we run out of ports. If no ports are available, P2P connections will fail. The default value is 99.
	 */
	uint16_t MaxAdditionalPortsToTry;
));

/** The most recent version of the EOS_P2P_GetPortRange API. */
#define EOS_P2P_GETPORTRANGE_API_001 1

/**
 * Structure containing information about getting the configured port range settings.
 */
EOS_STRUCT(EOS_P2P_GetPortRangeOptions001, (
	/** API Version: Set this to EOS_P2P_GETPORTRANGE_API_LATEST. */
	int32_t ApiVersion;
));

#pragma pack(pop)

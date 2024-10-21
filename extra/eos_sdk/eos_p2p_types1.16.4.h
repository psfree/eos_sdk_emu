// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#pragma pack(push, 8)

EOS_EXTERN_C typedef struct EOS_P2PHandle* EOS_HP2P;

/**
 * A packet's maximum size in bytes
 */
#define EOS_P2P_MAX_PACKET_SIZE 1170

/**
 * The maximum amount of unique Socket ID connections that can be opened with each remote user. As this limit is only per remote user, you may have more
 * than this number of Socket IDs across multiple remote users.
 */
#define EOS_P2P_MAX_CONNECTIONS 32

 /**
  * Categories of NAT strictness.
  */
EOS_ENUM(EOS_ENATType,
	/** NAT type either unknown (remote) or we are unable to determine it (local) */
	EOS_NAT_Unknown = 0,
	/** All peers can directly-connect to you */
	EOS_NAT_Open = 1,
	/** You can directly-connect to other Moderate and Open peers */
	EOS_NAT_Moderate = 2,
	/** You can only directly-connect to Open peers */
	EOS_NAT_Strict = 3
);

/** The most recent version of the EOS_P2P_SocketId structure. */
#define EOS_P2P_SOCKETID_API_001 1

/** The total buffer size of a EOS_P2P_SocketId SocketName, including space for the null-terminator */
#define EOS_P2P_SOCKETID_SOCKETNAME_SIZE 33

/**
 * P2P Socket ID
 *
 * The Socket ID contains an application-defined name for the connection between a local person and another peer.
 *
 * When a remote user receives a connection request from you, they will receive this information.  It can be important
 * to only accept connections with a known socket-name and/or from a known user, to prevent leaking of private
 * information, such as a user's IP address. Using the socket name as a secret key can help prevent such leaks. Shared
 * private data, like a private match's Session ID are good candidates for a socket name.
 */
EOS_STRUCT(EOS_P2P_SocketId001, (
	/** API Version: Set this to EOS_P2P_SOCKETID_API_LATEST. */
	int32_t ApiVersion;
	/** A name for the connection. Must be a NULL-terminated string of between 1-32 alpha-numeric characters (A-Z, a-z, 0-9, '-', '_', ' ', '+', '=', '.') */
	char SocketName[EOS_P2P_SOCKETID_SOCKETNAME_SIZE];
));

/**
 * Types of packet reliability.
 *
 * Ordered packets will only be ordered relative to other ordered packets. Reliable/unreliable and ordered/unordered communication
 * can be sent on the same Socket ID and Channel.
 */
EOS_ENUM(EOS_EPacketReliability,
	/** Packets will only be sent once and may be received out of order */
	EOS_PR_UnreliableUnordered = 0,
	/** Packets may be sent multiple times and may be received out of order */
	EOS_PR_ReliableUnordered = 1,
	/** Packets may be sent multiple times and will be received in order */
	EOS_PR_ReliableOrdered = 2
);

/** The most recent version of the EOS_P2P_SendPacket API. */
#define EOS_P2P_SENDPACKET_API_003 3

/**
 * Structure containing information about the data being sent and to which player
 */
EOS_STRUCT(EOS_P2P_SendPacketOptions003, (
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
	/**
	 * If set to EOS_TRUE, EOS_P2P_SendPacket will not automatically establish a connection with the RemoteUserId and will require explicit calls to
	 * EOS_P2P_AcceptConnection first whenever the connection is closed. If set to EOS_FALSE, EOS_P2P_SendPacket will automatically accept and start
	 * the connection any time it is called and the connection is not already open.
	 */
	EOS_Bool bDisableAutoAcceptConnection;
));

/** The most recent version of the EOS_P2P_AddNotifyPeerConnectionRequest API. */
#define EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_001 1

/**
 * Structure containing information about who would like connection notifications, and about which socket.
 */
EOS_STRUCT(EOS_P2P_AddNotifyPeerConnectionRequestOptions001, (
	/** API Version: Set this to EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the user who is listening for incoming connection requests */
	EOS_ProductUserId LocalUserId;
	/** The optional socket ID to listen for, used as a filter for incoming connection requests; If NULL, incoming connection requests will not be filtered */
	const EOS_P2P_SocketId* SocketId;
));

/**
 * Structure containing information about an incoming connection request.
 */
EOS_STRUCT(EOS_P2P_OnIncomingConnectionRequestInfo, (
	enum { k_iCallback = k_iP2PCallbackBase + 0 };
	/** Client-specified data passed into EOS_Presence_AddNotifyOnPresenceChanged */
	void* ClientData;
	/** The local user who is being requested to open a P2P session with RemoteUserId */
	EOS_ProductUserId LocalUserId;
	/** The remote user who requested a peer connection with the local user */
	EOS_ProductUserId RemoteUserId;
	/** The ID of the socket the Remote User wishes to communicate on */
	const EOS_P2P_SocketId* SocketId;
));

/**
 * Callback for information related to incoming connection requests.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnIncomingConnectionRequestCallback, const EOS_P2P_OnIncomingConnectionRequestInfo* Data);

/** The most recent version of the EOS_P2P_AddNotifyPeerConnectionEstablished API. */
#define EOS_P2P_ADDNOTIFYPEERCONNECTIONESTABLISHED_API_001 1

/**
 * Structure containing information about which connections should be notified
 */
EOS_STRUCT(EOS_P2P_AddNotifyPeerConnectionEstablishedOptions001, (
	/** API Version: Set this to EOS_P2P_ADDNOTIFYPEERCONNECTIONESTABLISHED_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who would like to receive notifications */
	EOS_ProductUserId LocalUserId;
	/** The optional socket ID, used as a filter for established connections. If NULL, this function handler will be called for all sockets */
	const EOS_P2P_SocketId* SocketId;
));

/**
 * Type of established connection
 */
EOS_ENUM(EOS_EConnectionEstablishedType,
	/** The connection is brand new */
	EOS_CET_NewConnection = 0,
	/** The connection is reestablished (reconnection) */
	EOS_CET_Reconnection = 1
);

/**
 * Types of network connections.
 */
EOS_ENUM(EOS_ENetworkConnectionType,
	/** There is no established connection */
	EOS_NCT_NoConnection = 0,
	/** A direct connection to the peer over the Internet or Local Network */
	EOS_NCT_DirectConnection = 1,
	/** A relayed connection using Epic-provided servers to the peer over the Internet */
	EOS_NCT_RelayedConnection = 2
);

/**
 * Structure containing information about a connection being established
 */
EOS_STRUCT(EOS_P2P_OnPeerConnectionEstablishedInfo, (
	enum { k_iCallback = k_iP2PCallbackBase + 3 };
	/** Client-specified data passed into EOS_P2P_AddNotifyPeerConnectionEstablished */
	void* ClientData;
	/** The Product User ID of the local user who is being notified of a connection being established */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the remote user who this connection was with */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID of the connection being established */
	const EOS_P2P_SocketId* SocketId;
	/** Information if this is a new connection or reconnection */
	EOS_EConnectionEstablishedType ConnectionType;
	/** What type of network connection is being used for this connection */
	EOS_ENetworkConnectionType NetworkType;
));

/**
 * Callback for information related to new connections being established
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnPeerConnectionEstablishedCallback, const EOS_P2P_OnPeerConnectionEstablishedInfo* Data);


/** The most recent version of the EOS_P2P_AddNotifyPeerConnectionInterrupted API. */
#define EOS_P2P_ADDNOTIFYPEERCONNECTIONINTERRUPTED_API_001 1

/**
 * Structure containing information about who would like notifications about interrupted connections, and for which socket.
 */
EOS_STRUCT(EOS_P2P_AddNotifyPeerConnectionInterruptedOptions001, (
	/** API Version: Set this to EOS_P2P_ADDNOTIFYPEERCONNECTIONINTERRUPTED_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who would like notifications */
	EOS_ProductUserId LocalUserId;
	/** An optional socket ID to filter interrupted connections on. If NULL, this function handler will be called for all interrupted connections */
	const EOS_P2P_SocketId* SocketId;
));

/**
 * Structure containing information about an connection request that is that was interrupted.
 */
EOS_STRUCT(EOS_P2P_OnPeerConnectionInterruptedInfo, (
	enum { k_iCallback = k_iP2PCallbackBase + 4 };
	/** Client-specified data passed into EOS_Presence_AddNotifyOnPresenceChanged */
	void* ClientData;
	/** The local user who is being notified of a connection that was interrupted */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the remote user who this connection was with */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID of the connection that was interrupted */
	const EOS_P2P_SocketId* SocketId;
));


/**
 * Callback for information related to open connections that are interrupted.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnPeerConnectionInterruptedCallback, const EOS_P2P_OnPeerConnectionInterruptedInfo* Data);

/** The most recent version of the EOS_P2P_AddNotifyPeerConnectionClosed API. */
#define EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_001 1

/**
 * Structure containing information about who would like notifications about closed connections, and for which socket.
 */
EOS_STRUCT(EOS_P2P_AddNotifyPeerConnectionClosedOptions001, (
	/** API Version: Set this to EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who would like notifications */
	EOS_ProductUserId LocalUserId;
	/** The optional socket ID to listen for to be closed. If NULL, this function handler will be called for all closed connections */
	const EOS_P2P_SocketId* SocketId;
));

/**
 * Reasons why a P2P connection was closed
 */
EOS_ENUM(EOS_EConnectionClosedReason,
	/** The connection was closed for unknown reasons. This most notably happens during application shutdown. */
	EOS_CCR_Unknown = 0,
	/** The connection was at least locally accepted, but was closed by the local user via a call to EOS_P2P_CloseConnection / EOS_P2P_CloseConnections. */
	EOS_CCR_ClosedByLocalUser = 1,
	/** The connection was at least locally accepted, but was gracefully closed by the remote user via a call to EOS_P2P_CloseConnection / EOS_P2P_CloseConnections. */
	EOS_CCR_ClosedByPeer = 2,
	/** The connection was at least locally accepted, but was not remotely accepted in time. */
	EOS_CCR_TimedOut = 3,
	/** The connection was accepted, but the connection could not be created due to too many other existing connections */
	EOS_CCR_TooManyConnections = 4,
	/** The connection was accepted, The remote user sent an invalid message */
	EOS_CCR_InvalidMessage = 5,
	/** The connection was accepted, but the remote user sent us invalid data */
	EOS_CCR_InvalidData = 6,
	/** The connection was accepted, but we failed to ever establish a connection with the remote user due to connectivity issues. */
	EOS_CCR_ConnectionFailed = 7,
	/** The connection was accepted and established, but the peer silently went away. */
	EOS_CCR_ConnectionClosed = 8,
	/** The connection was locally accepted, but we failed to negotiate a connection with the remote user. This most commonly occurs if the local user goes offline or is logged-out during the connection process. */
	EOS_CCR_NegotiationFailed = 9,
	/** The connection was accepted, but there was an internal error occurred and the connection cannot be created or continue. */
	EOS_CCR_UnexpectedError = 10,
	/** The connection was ignored because no connection listeners were bound. */
	EOS_CCR_ConnectionIgnored = 11
);


/**
 * Structure containing information about an connection request that is being closed.
 */
EOS_STRUCT(EOS_P2P_OnRemoteConnectionClosedInfo, (
	enum { k_iCallback = k_iP2PCallbackBase + 1 };
	/** Client-specified data passed into EOS_Presence_AddNotifyOnPresenceChanged */
	void* ClientData;
	/** The local user who is being notified of a connection being closed */
	EOS_ProductUserId LocalUserId;
	/** The remote user who this connection was with */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID of the connection being closed */
	const EOS_P2P_SocketId* SocketId;
	/** The reason the connection was closed (if known) */
	EOS_EConnectionClosedReason Reason;
));

/**
 * Callback for information related to open connections being closed.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnRemoteConnectionClosedCallback, const EOS_P2P_OnRemoteConnectionClosedInfo* Data);

/** The most recent version of the EOS_P2P_QueryNATType API. */
#define EOS_P2P_QUERYNATTYPE_API_001 1

/**
 * Structure containing information needed to query NAT-types
 */
EOS_STRUCT(EOS_P2P_QueryNATTypeOptions001, (
	/** API Version: Set this to EOS_P2P_QUERYNATTYPE_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Structure containing information about the local network NAT type
 */
EOS_STRUCT(EOS_P2P_OnQueryNATTypeCompleteInfo, (
	enum { k_iCallback = k_iP2PCallbackBase + 2 };
	/** Result code for the operation. EOS_Success is returned for a successful query, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into EOS_P2P_QueryNATType */
	void* ClientData;
	/** The queried NAT type */
	EOS_ENATType NATType;
));


/**
 * Callback for information related to our NAT type query completing.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnQueryNATTypeCompleteCallback, const EOS_P2P_OnQueryNATTypeCompleteInfo* Data);

/** The most recent version of the EOS_P2P_GetRelayControl API. */
#define EOS_P2P_GETRELAYCONTROL_API_001 1

/**
 * Structure containing information about getting the relay control setting.
 */
EOS_STRUCT(EOS_P2P_GetRelayControlOptions001, (
	/** API Version: Set this to EOS_P2P_GETRELAYCONTROL_API_LATEST. */
	int32_t ApiVersion;
));

/** Helper constant to signify that the packet queue is allowed to grow indefinitely */
#define EOS_P2P_MAX_QUEUE_SIZE_UNLIMITED 0

/** The most recent version of the EOS_P2P_SetPacketQueueSize API. */
#define EOS_P2P_SETPACKETQUEUESIZE_API_001 1

/**
 * Structure containing information about new packet queue size settings.
 */
EOS_STRUCT(EOS_P2P_SetPacketQueueSizeOptions001, (
	/** API Version: Set this to EOS_P2P_SETPACKETQUEUESIZE_API_LATEST. */
	int32_t ApiVersion;
	/** The ideal maximum amount of bytes the Incoming packet queue can consume */
	uint64_t IncomingPacketQueueMaxSizeBytes;
	/** The ideal maximum amount of bytes the Outgoing packet queue can consume */
	uint64_t OutgoingPacketQueueMaxSizeBytes;
));

/** The most recent version of the EOS_P2P_GetPacketQueueInfo API. */
#define EOS_P2P_GETPACKETQUEUEINFO_API_001 1

/**
 * Structure containing information needed to get the current packet queue information.
 */
EOS_STRUCT(EOS_P2P_GetPacketQueueInfoOptions001, (
	/** API Version: Set this to EOS_P2P_GETPACKETQUEUEINFO_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Information related to the current state of the packet queues. It is possible for the current size
 * to be larger than the maximum size if the maximum size changes or if the maximum queue size is
 * set to EOS_P2P_MAX_QUEUE_SIZE_UNLIMITED.
 */
EOS_STRUCT(EOS_P2P_PacketQueueInfo001, (
	/** The maximum size in bytes of the incoming packet queue */
	uint64_t IncomingPacketQueueMaxSizeBytes;
	/** The current size in bytes of the incoming packet queue */
	uint64_t IncomingPacketQueueCurrentSizeBytes;
	/** The current number of queued packets in the incoming packet queue */
	uint64_t IncomingPacketQueueCurrentPacketCount;
	/** The maximum size in bytes of the outgoing packet queue */
	uint64_t OutgoingPacketQueueMaxSizeBytes;
	/** The current size in bytes of the outgoing packet queue */
	uint64_t OutgoingPacketQueueCurrentSizeBytes;
	/** The current amount of queued packets in the outgoing packet queue */
	uint64_t OutgoingPacketQueueCurrentPacketCount;
));

/** The most recent version of the EOS_P2P_AddNotifyIncomingPacketQueueFull API. */
#define EOS_P2P_ADDNOTIFYINCOMINGPACKETQUEUEFULL_API_001 1

/**
 * Structure containing information about what version of the EOS_P2P_AddNotifyIncomingPacketQueueFull function is supported.
 */
EOS_STRUCT(EOS_P2P_AddNotifyIncomingPacketQueueFullOptions001, (
	/** API Version: Set this to EOS_P2P_ADDNOTIFYINCOMINGPACKETQUEUEFULL_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Structure containing information about the packet queue's state and the incoming packet that would overflow the queue
 */
EOS_STRUCT(EOS_P2P_OnIncomingPacketQueueFullInfo, (
	/** Client-specified data passed into AddNotifyIncomingPacketQueueFull */
	void* ClientData;
	/** The maximum size in bytes the incoming packet queue is allowed to use */
	uint64_t PacketQueueMaxSizeBytes;
	/** The current size in bytes the incoming packet queue is currently using */
	uint64_t PacketQueueCurrentSizeBytes;
	/** The Product User ID of the local user who is receiving the packet that would overflow the queue */
	EOS_ProductUserId OverflowPacketLocalUserId;
	/** The channel the incoming packet is for */
	uint8_t OverflowPacketChannel;
	/** The size in bytes of the incoming packet (and related metadata) that would overflow the queue */
	uint32_t OverflowPacketSizeBytes;
));

/**
 * Callback for information related to incoming connection requests.
 */
EOS_DECLARE_CALLBACK(EOS_P2P_OnIncomingPacketQueueFullCallback, const EOS_P2P_OnIncomingPacketQueueFullInfo* Data);

/** The most recent version of the EOS_P2P_ClearPacketQueue API. */
#define EOS_P2P_CLEARPACKETQUEUE_API_001 1

/**
 * Structure containing information about the packet queue to be cleared
 */
EOS_STRUCT(EOS_P2P_ClearPacketQueueOptions, (
	/** API Version: Set this to EOS_P2P_CLEARPACKETQUEUE_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user for whom we want to clear the queued packets */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID to who (outgoing) or from who (incoming) packets are queued */
	EOS_ProductUserId RemoteUserId;
	/** The socket used for packets to be cleared */
	const EOS_P2P_SocketId* SocketId;
));

#pragma pack(pop)

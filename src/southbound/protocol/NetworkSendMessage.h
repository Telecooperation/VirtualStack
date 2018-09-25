#pragma once


#include "NetworkMessageEnum.h"
#include "../model/ISerializeable.h"

/**
 * A container for a ISerializable message to be send. It provides information about the messagetype
 */
class NetworkSendMessage
{
public:
	NetworkSendMessage(size_t sessionId, NetworkMessageEnum messageType, const ISerializeable* message);
	
	void serialize(Storage &storage) const;
	
	size_t sessionId;
	size_t messageSize;
	NetworkMessageEnum messageType;
	const ISerializeable* message;
	
	ALLOW_MOVE_SEMANTICS_ONLY(NetworkSendMessage);
};



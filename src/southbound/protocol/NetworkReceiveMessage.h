#pragma once


#include "../../common/DataStructures/Model/Storage.h"
#include "../model/IReceiveFromNetwork.h"
#include "NetworkMessageEnum.h"

class NetworkReceiveMessage final : public IReceiveFromNetwork
{
public:
	NetworkReceiveMessage(const sockaddr_storage& sourceAddr, StoragePoolPtr&& storage);

    size_t size() const;
	static size_t getContainingReceiveMessageCount(const Storage& storage);

	size_t sessionId;
	size_t messageSize;
	NetworkMessageEnum messageType;
private:
	static const size_t headerSize = sizeof(sessionId) + sizeof(messageType) + sizeof(messageSize);
};



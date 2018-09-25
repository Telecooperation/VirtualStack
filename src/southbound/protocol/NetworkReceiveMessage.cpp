#include "NetworkReceiveMessage.h"

NetworkReceiveMessage::NetworkReceiveMessage(const sockaddr_storage& sourceAddr, StoragePoolPtr&& storage) :
		IReceiveFromNetwork(sourceAddr, std::move(storage))
{
	//get flowid and messageType

	sessionId = _data->toTypeAutomatic<decltype(sessionId)>();
	messageSize = _data->toTypeAutomatic<decltype(messageSize)>(sizeof(messageSize));
	messageType = _data->toTypeAutomatic<decltype(messageType)>(sizeof(messageSize) + sizeof(sessionId));
	_data->incrementStartIndex(headerSize);
}

size_t NetworkReceiveMessage::getContainingReceiveMessageCount(const Storage &storage)
{
	size_t messageCount = 0;
	size_t currentMessageSize = 0;
	while (currentMessageSize < storage.size())
	{
		currentMessageSize += headerSize + storage.toTypeAutomatic<size_t>(currentMessageSize + sizeof(messageSize));
		++messageCount;
	}

	return messageCount;
}

size_t NetworkReceiveMessage::size() const
{
	return headerSize + messageSize;
}

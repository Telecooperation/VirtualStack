#include "NetworkSendMessage.h"

NetworkSendMessage::NetworkSendMessage(size_t pSessionId, NetworkMessageEnum pMessageType, const ISerializeable* pMessage)
		: sessionId(pSessionId),
		  messageSize(!pMessage ? 0 : pMessage->size()),
		  messageType(pMessageType),
		  message(pMessage)
{}

void NetworkSendMessage::serialize(Storage &storage) const
{
	if(message == nullptr)
		return;
	
	storage.appendDataAutomaticAfterEnd(&sessionId);
	storage.appendDataAutomaticAfterEnd(&messageSize);
	storage.appendDataAutomaticAfterEnd(&messageType);
	message->serialize(storage);
}

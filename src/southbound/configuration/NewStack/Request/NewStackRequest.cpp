#include "NewStackRequest.h"

void NewStackRequest::serialize(Storage &storage) const
{
	storage.appendDataAutomaticAfterEnd(&internetProtocol);
	storage.appendDataAutomaticAfterEnd(&northboundProtocol);

	storage.appendDataAutomaticAfterEnd(&northboundSourcePort);
	storage.appendDataAutomaticAfterEnd(&northboundDestinationPort);
	storage.appendDataAutomaticAfterEnd(&stackPort);

	storage.appendDataAutomaticAfterEnd(&stack);
	storage.appendDataAutomaticAfterEnd(&routed);

    storage.appendDataAutomaticAfterEnd(&origin);
    storage.appendDataAutomaticAfterEnd(&destination);

    storage.appendDataAutomaticAfterEnd(&overrideOriginFlowId);
    storage.appendDataAutomaticAfterEnd(&customOriginFlowId);
}

void NewStackRequest::deserialize(const Storage &storage)
{
    size_t tmpIndex = 0;
    internetProtocol = storage.toTypeAutomatic<InternetProtocolEnum>(tmpIndex);
    tmpIndex += sizeof(InternetProtocolEnum);

    northboundProtocol = storage.toTypeAutomatic<TransportProtocolEnum>(tmpIndex);
    tmpIndex += sizeof(TransportProtocolEnum);

    northboundSourcePort = storage.toTypeAutomatic<uint16_t>(tmpIndex);
    tmpIndex += sizeof(uint16_t);

    northboundDestinationPort = storage.toTypeAutomatic<uint16_t>(tmpIndex);
    tmpIndex += sizeof(uint16_t);

    stackPort = storage.toTypeAutomatic<uint16_t>(tmpIndex);
    tmpIndex += sizeof(uint16_t);

    stack = storage.toTypeAutomatic<StackEnum>(tmpIndex);
    tmpIndex += sizeof(StackEnum);

    routed = storage.toTypeAutomatic<bool>(tmpIndex);
    tmpIndex += sizeof(bool);

    origin = storage.toTypeAutomatic<sockaddr_storage>(tmpIndex);
    tmpIndex += sizeof(sockaddr_storage);

    destination = storage.toTypeAutomatic<sockaddr_storage>(tmpIndex);
    tmpIndex += sizeof(sockaddr_storage);

    overrideOriginFlowId = storage.toTypeAutomatic<bool>(tmpIndex);
    tmpIndex += sizeof(bool);

    customOriginFlowId = storage.toTypeAutomatic<uint64_t>(tmpIndex);
    tmpIndex += sizeof(uint64_t);
}

size_t NewStackRequest::size() const
{
	return sizeof(InternetProtocolEnum) +
           sizeof(TransportProtocolEnum) +
            (3 * sizeof(uint16_t)) +
            sizeof(StackEnum) +
			sizeof(bool) +
            (2 * sizeof(sockaddr_storage)) +
            sizeof(bool) +
            sizeof(uint64_t);
}

std::unique_ptr<NewStackRequest> NewStackRequest::copy() const
{
	auto item = std::make_unique<NewStackRequest>();
	item->internetProtocol = this->internetProtocol;
	item->northboundProtocol = this->northboundProtocol;
	item->northboundSourcePort = this->northboundSourcePort;
	item->northboundDestinationPort = this->northboundDestinationPort;
	item->stackPort = this->stackPort;
	item->stack = this->stack;
	item->routed = this->routed;

	item->origin = this->origin;
	item->destination = this->destination;

	item->overrideOriginFlowId = this->overrideOriginFlowId;
	item->customOriginFlowId = this->customOriginFlowId;

	return item;
}

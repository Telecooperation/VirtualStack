#include "NewStackResponse.h"

NewStackResponse::NewStackResponse(uint16_t pRemotePort, const sockaddr_storage& pSource) :
        remotePort(pRemotePort),
        source(pSource)
{}

NewStackResponse::NewStackResponse() : remotePort(0), source()
{}

void NewStackResponse::serialize(Storage &storage) const
{
	storage.appendDataAutomaticAfterEnd(&remotePort);
	storage.appendDataAutomaticAfterEnd(&source);
}

void NewStackResponse::deserialize(const Storage &storage)
{
	remotePort = storage.toTypeAutomatic<uint16_t>();
	source = storage.toTypeAutomatic<sockaddr_storage>(sizeof(uint16_t));
}

size_t NewStackResponse::size() const
{
	return sizeof(remotePort) + sizeof(source);
}

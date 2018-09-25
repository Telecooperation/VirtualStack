#pragma once


#include <sys/socket.h>
#include "../../../model/ISerializeable.h"

class NewStackResponse final : public ISerializeable
{
public:
	NewStackResponse(uint16_t pRemotePort, const sockaddr_storage& pSource);
	explicit NewStackResponse();
	
	void serialize(Storage &storage) const override;
	
	void deserialize(const Storage &storage) override;

	size_t size() const override;

	uint16_t remotePort;
	sockaddr_storage source;
};



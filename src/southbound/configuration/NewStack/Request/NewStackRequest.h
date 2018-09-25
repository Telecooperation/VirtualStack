#pragma once

#include "../../../../common/DataStructures/Model/Storage.h"
#include "../../../../model/InspectionStruct.h"
#include "../../../../model/InternetProtocolEnum.h"
#include "../../../../model/TransportProtocolEnum.h"
#include "../../../../stacks/StackEnum.h"
#include "../../../model/ISerializeable.h"
#include <sys/socket.h>

struct NewStackRequest : public ISerializeable
{
public:
	NewStackRequest() = default;

	void serialize(Storage &storage) const override;
	
	void deserialize(const Storage &storage) override;

	size_t size() const override;

	std::unique_ptr<NewStackRequest> copy() const;
	
	InternetProtocolEnum internetProtocol = InternetProtocolEnum::IPv4;
	TransportProtocolEnum northboundProtocol = TransportProtocolEnum::NONE;
	uint16_t northboundSourcePort = 0;
	uint16_t northboundDestinationPort = 0;
	uint16_t stackPort = 0;
	StackEnum stack = StackEnum::Invalid;
	bool routed = false;

    sockaddr_storage origin{};
    sockaddr_storage destination{};

    bool overrideOriginFlowId = false;
    uint64_t customOriginFlowId{};

	ALLOW_MOVE_SEMANTICS_ONLY(NewStackRequest);
};
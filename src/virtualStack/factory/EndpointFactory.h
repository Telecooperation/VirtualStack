#pragma once

#include "../../interface/IEndpoint.h"
#include "../../model/TransportProtocolEnum.h"

class EndpointFactory
{
	public:
	static std::unique_ptr<IEndpoint> createEndpoint(const flowid_t& flowid, const flowid_t& partnerFlowId, TransportProtocolEnum endpointType,
													 const sockaddr_storage &northboundSource,
													 const sockaddr_storage &northboundDest);
};



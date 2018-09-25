#include "../stackEngine/endpoints/RouterEndpoint.h"
#include "../stackEngine/endpoints/UdpEndpoint.h"
#include "EndpointFactory.h"

std::unique_ptr<IEndpoint>
EndpointFactory::createEndpoint(const flowid_t& flowid, const flowid_t& partnerFlowId, TransportProtocolEnum endpointType,
								const sockaddr_storage &northboundSource,
								const sockaddr_storage &northboundDest)
{
	switch(endpointType)
	{
		case TransportProtocolEnum::UDP:
		{
			return std::make_unique<UdpEndpoint>(flowid, partnerFlowId, northboundSource, northboundDest);
		}
		case TransportProtocolEnum::ROUTE:
		{
			return std::make_unique<RouterEndpoint>(flowid, partnerFlowId);
		}
        case TransportProtocolEnum::UDPLITE:
		case TransportProtocolEnum::TCP:
        case TransportProtocolEnum::DCCP:
		case TransportProtocolEnum::SCTP:
        case TransportProtocolEnum::NONE:
		case TransportProtocolEnum::RAW:
		{
			Logger::Log(Logger::ERROR, "Unknown Endpoint not implemented yet");
			break;
		}
	}
	return nullptr;
}

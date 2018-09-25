#include "../../common/Helper/DomainExtensions.h"
#include "../../model/InspectionStruct.h"
#include "FlowIdGenerator.h"
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

void FlowIdGenerator::process(StoragePoolPtr &storage)
{
	//generate ID and store in inspectionStruct

	//wir wissen das protocoll
	auto instStructPtr = InspectionStruct::getInspectionStruct(*storage);
	auto constDataPtr = storage->constData();

	uint16_t destPort = 0;
	uint16_t sourcePort = 0;

	switch (instStructPtr->northboundTransportProtocol)
	{
		case TransportProtocolEnum::UDP:
		case TransportProtocolEnum::UDPLITE:
		{
			//betrachte sourcePort
			auto udpHeader = reinterpret_cast<const udphdr *>(constDataPtr +
															  instStructPtr->transportProtocolStartIndex);
			destPort = udpHeader->dest;
			sourcePort = udpHeader->source;
			break;
		}
		case TransportProtocolEnum::TCP:
		{
			auto tcpHeader = reinterpret_cast<const tcphdr *>(constDataPtr +
															  instStructPtr->transportProtocolStartIndex);
			destPort = tcpHeader->dest;
			sourcePort = tcpHeader->source;

			break;
		}
		case TransportProtocolEnum::ROUTE:
		{
			//FlowId steht schon im InspectionStruct
			return;
		}
		case TransportProtocolEnum::SCTP:
		case TransportProtocolEnum::DCCP:
        case TransportProtocolEnum::NONE:
		case TransportProtocolEnum::RAW:
		{
			//wir können mit dem paket nichts anfangen
			return;
		}
	}

	uint64_t dest = getDestIp(instStructPtr, constDataPtr);

	//für ipv4, die untere hälfte ist die src die eigentlich immer gleich sein sollte. daher zerstören wir wenig informationen per xor mit demn port
	instStructPtr->flowId = createFlowId(instStructPtr->northboundTransportProtocol,
										 dest,
										 ntohs(static_cast<uint16_t>(sourcePort)),
										 ntohs(static_cast<uint16_t>(destPort)));
}

uint64_t FlowIdGenerator::getDestIp(const InspectionStruct *inspectionStruct, const uint8_t *data)
{
	uint64_t ip = 0;
	
	switch(inspectionStruct->internetProtocol)
	{
		case InternetProtocolEnum::IPv4:
		{
			auto ipv4Header = reinterpret_cast<const iphdr*>(data);
			ip = static_cast<uint64_t>(ntohl(ipv4Header->daddr));
			break;
		}
		case InternetProtocolEnum::IPv6:
		{
			auto ipv6Header = reinterpret_cast<const ip6_hdr*>(data);
			ip = foldToUint64(ipv6Header->ip6_dst);
			break;
		}
		case InternetProtocolEnum::Route:
		{
			break;
		}
	}
	
	return ip;
}

flowid_t FlowIdGenerator::createFlowId(const TransportProtocolEnum transportProtocol,
										const sockaddr_storage& destination,
										const uint16_t sourcePort, const uint16_t destPort)
{
	uint64_t dest = getIp(destination);

	return createFlowId(transportProtocol, dest, sourcePort, destPort);
}

flowid_t FlowIdGenerator::createFlowIdForRouter(const TransportProtocolEnum transportProtocol,
												const sockaddr_storage &destination, const sockaddr_storage &source,
												const uint16_t sourcePort, const uint16_t destPort)
{
	auto flowId = createFlowId(transportProtocol, destination, sourcePort, destPort);
	flowId.source = getIp(source);

	return flowId;
}

flowid_t FlowIdGenerator::createFlowId(const TransportProtocolEnum transportProtocol,
									   const uint64_t destination,
									   const uint16_t sourcePort,
									   const uint16_t destPort)
{
	flowid_t flowId{};
	flowId.transportProtocol = transportProtocol;
	flowId.sourcePort = sourcePort;
	flowId.destinationPort = destPort;
	flowId.destination = destination;

	return flowId;
}

uint64_t FlowIdGenerator::foldToUint64(const struct in6_addr& addr)
{
	const uint32_t* addrArray = addr.__in6_u.__u6_addr32;

	auto upperLong = static_cast<uint64_t>(addrArray[2] ^ addrArray[3]) << 32;
	auto lowerLong = static_cast<uint64_t>(addrArray[1] ^ addrArray[0]);
	
	return upperLong | lowerLong;
}

flowid_t FlowIdGenerator::convertToCatchAllFlowId(const flowid_t flowid) {
	//remove the ports from the flowId
	auto copy = flowid;
	copy.transportProtocol = static_cast<TransportProtocolEnum>(0);
	copy.sourcePort = 0;
	copy.destinationPort = 0;
	copy.source = 0;

	return copy;
}

flowid_t FlowIdGenerator::generateCatchAllSubFlowId(const sockaddr_storage &destination, uint8_t id)
{
	flowid_t flowId{};
	flowId.subFlow = id;
	flowId.destination = getIp(destination);

	return flowId;
}

flowid_t FlowIdGenerator::generateCatchAllFlowId(const sockaddr_storage &destination) {
	return generateCatchAllSubFlowId(destination, 0);
}

uint64_t FlowIdGenerator::getIp(const sockaddr_storage &addr) {
	auto internetProtocol = DomainExtensions::getInternetProtocol(addr);
	switch(internetProtocol)
	{
		case InternetProtocolEnum::IPv4:
		{
			auto addrIpv4 = reinterpret_cast<const sockaddr_in*>(&addr);
			return static_cast<uint64_t>(ntohl(addrIpv4->sin_addr.s_addr)); //transform to hostOrder as its not a sockaddr_storage anymore but a plain integer
		}
		case InternetProtocolEnum::IPv6:
		{
			auto addrIpv6 = reinterpret_cast<const sockaddr_in6*>(&addr);
			return foldToUint64(addrIpv6->sin6_addr);
		}
		case InternetProtocolEnum::Route:
		{
			break;
		}
	}
	return 0;
}

void FlowIdGenerator::addSourceToFlowId(flowid_t &flowid, const sockaddr_storage &source)
{
	flowid.source = getIp(source);
}

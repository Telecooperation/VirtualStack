
#include "UdpEndpoint.h"

#include "../payloadRepacker/UDPPayloadRepacker.h"
#include "common/UDPHeaderFactory.h"
#include <netinet/ip.h>
#include <netinet/ip6.h>

UdpEndpoint::UdpEndpoint(const flowid_t& flowId, const flowid_t& partnerFlowId, const sockaddr_storage &northboundSource,
						 const sockaddr_storage &northboundDest)
		: IEndpoint(flowId, partnerFlowId, TransportProtocolEnum::UDP, northboundSource, northboundDest)
{
//    Logger::Log(Logger::DEBUG, "UDP-Endpoint-> flowID: ", flowId, ", partnerFlowID: ", partnerFlowId,
//                ", source: ", NetworkExtensions::getAddress(_northboundSourceAddr), ", port: ", _northboundSourcePort,
//                ", dest: ", NetworkExtensions::getAddress(_northboundDestinationAddr), ", port: ", _northboundDestinationPort);
}

void UdpEndpoint::processNBIToStack(Storage &packet) const
{
	size_t ipHeaderSize = 0u;
	switch(_northboundSourceAddr.ss_family)
	{
		case AF_INET:
		{
			ipHeaderSize = reinterpret_cast<iphdr *>(packet.data())->ihl * 4u; //ihl: wieviele 32bit dinge stehen da drinne
			break;
		}
		case AF_INET6:
		{
			uint8_t nextHeader = reinterpret_cast<ip6_hdr *>(packet.data())->ip6_ctlun.ip6_un1.ip6_un1_nxt;
			
			const uint8_t IPv6_NoNxt = 59;
			const uint8_t UDP = 17;
			
			if(nextHeader != IPv6_NoNxt)
			{
				if(nextHeader == UDP) //udp
					Logger::Log(Logger::INFO, "UDPEndpoint: Wenn nextHeader des IPv6Headers 17 ist, dann liegt UDP drinne, also entspricht es dem Protokollfeld des IPv4Headers");
				else
				{
					Logger::Log(Logger::ERROR, "UDPEndpoint: Unknown ipv6 next header value ", nextHeader);
					return;
				}
			}
			ipHeaderSize = sizeof(ip6_hdr);
			break;
		}
		default:
		{
			Logger::Log(Logger::ERROR, "UDPEndpoint: Unknown socketAddr family ", _northboundSourceAddr.ss_family);
			return;
		}
	}
	packet.incrementStartIndex(ipHeaderSize + sizeof(udphdr));
}

void UdpEndpoint::processStackToNBI(Storage &packet) const
{
    //for packing swap source and destination
	switch(_northboundSourceAddr.ss_family)
	{
		case AF_INET:
		{
			UDPPayloadRepacker::repackIpv4(_northboundSourceAddr, _northboundDestinationAddr, packet);
			break;
		}
		case AF_INET6:
		{
			UDPPayloadRepacker::repackIpv6(_northboundSourceAddr, _northboundDestinationAddr, packet);
			break;
		}
		default:
		{
			Logger::Log(Logger::ERROR, "UDPEndpoint: Unknown socketAddr family ", _northboundSourceAddr.ss_family);
			return;
		}
	}
}

UdpEndpoint::~UdpEndpoint()
{
	
}

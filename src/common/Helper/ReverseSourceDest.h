#pragma once

#include "../../model/InspectionStruct.h"
#include "../DataStructures/Model/Storage.h"
#include "DomainExtensions.h"
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

class ReverseSourceDest
{
public:
	static bool process(StoragePoolPtr& storagePoolPtr)
	{
		auto dataPtr = storagePoolPtr->data();
		auto ipVersion = reinterpret_cast<struct iphdr *>(dataPtr)->version;
		
		TransportProtocolEnum transportProtocol;
		size_t transportProtocolStartIndex = 0;
		
		//wenn es ipv4 ist, trage ipv4 ein, ansonsten nehem ipv6 an
		//size_t internetProtocolHeaderSize = 0;
		if (ipVersion == IPVERSION)
		{
			auto ipv4Header = reinterpret_cast<struct iphdr *>(dataPtr);
			transportProtocol = DomainExtensions::getTransportProtocol(ipv4Header->protocol);
			transportProtocolStartIndex = ipv4Header->ihl * 4u; //ihl ist anzahl 32bit werten
			
			std::swap(ipv4Header->saddr, ipv4Header->daddr);
		}
		else
		{
			auto ipv6Header = reinterpret_cast<struct ip6_hdr *>(dataPtr);
			transportProtocol = DomainExtensions::getTransportProtocol(ipv6Header->ip6_ctlun.ip6_un1.ip6_un1_nxt);
			transportProtocolStartIndex = sizeof(ip6_hdr);
			
			std::swap(ipv6Header->ip6_src, ipv6Header->ip6_dst);
		}
		
		return swapTransportPorts(storagePoolPtr, transportProtocol, transportProtocolStartIndex);
	}
	
	static bool swapTransportPorts(StoragePoolPtr &storage, TransportProtocolEnum transportProtocol,
								   size_t transportProtocolStartIndex)
	{
		auto dataPtr = storage->data();
		
		switch(transportProtocol)
		{
			case TransportProtocolEnum::UDP:
			case TransportProtocolEnum::UDPLITE:
			{
				//betrachte sourcePort
				auto udpHeader = reinterpret_cast<struct udphdr *>(dataPtr +
																  transportProtocolStartIndex);
				
				std::swap(udpHeader->source, udpHeader->dest);
				break;
			}
			case TransportProtocolEnum::TCP:
			{
				auto tcpHeader = reinterpret_cast<struct tcphdr *>(dataPtr +
																  transportProtocolStartIndex);
				std::swap(tcpHeader->source, tcpHeader->dest);
				break;
			}
			case TransportProtocolEnum::ROUTE:
			case TransportProtocolEnum::SCTP:
			case TransportProtocolEnum::DCCP:
            case TransportProtocolEnum::NONE:
			case TransportProtocolEnum::RAW:
			{
				//wir können mit dem paket nichts anfangen
				return false;
			}
		}
		return true;
	}
};
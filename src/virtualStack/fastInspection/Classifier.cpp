#include "../../common/Helper/DomainExtensions.h"
#include "Classifier.h"
#include <netinet/ip.h>
#include <netinet/ip6.h>

InspectionStruct* Classifier::process(StoragePoolPtr &storagePoolPtr)
{
	//annahme ip-packet
	
	//wir wollen uns die ip version, also ob ipv4 oder ipv6
	
	auto constDataPtr = storagePoolPtr->constData();
	auto inspectionStruct = InspectionStruct::getInspectionStruct(*storagePoolPtr);
	auto ipVersion = reinterpret_cast<const iphdr*>(constDataPtr)->version;
	
	//wenn es ipv4 ist, trage ipv4 ein, ansonsten nehem ipv6 an
	//size_t internetProtocolHeaderSize = 0;
    auto internetProtocol = DomainExtensions::convertFromSystem(static_cast<uint8_t>(ipVersion));
    switch (internetProtocol)
    {
        case InternetProtocolEnum::IPv4:
        {
            inspectionStruct->internetProtocol = InternetProtocolEnum::IPv4;
            const auto ipv4Header = reinterpret_cast<const iphdr *>(constDataPtr);
            inspectionStruct->northboundTransportProtocol = DomainExtensions::getTransportProtocol(ipv4Header->protocol);
            inspectionStruct->transportProtocolStartIndex = ipv4Header->ihl * 4u; //ihl ist anzahl 32bit werten
            break;
        }
        case InternetProtocolEnum::IPv6:
        {
            inspectionStruct->internetProtocol = InternetProtocolEnum::IPv6;
            const auto ipv6Header = reinterpret_cast<const ip6_hdr *>(constDataPtr);
            inspectionStruct->northboundTransportProtocol = DomainExtensions::getTransportProtocol(ipv6Header->ip6_ctlun.ip6_un1.ip6_un1_nxt);
            inspectionStruct->transportProtocolStartIndex = sizeof(ip6_hdr);
            break;
        }
        case InternetProtocolEnum::Route:
        {
            inspectionStruct->transportProtocolStartIndex = sizeof(InternetProtocolEnum::Route);
            break;
        }
    }
	return inspectionStruct;
}




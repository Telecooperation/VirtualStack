
#include <netinet/ip.h>
#include <sys/socket.h>

#include "DomainExtensions.h"
#include "../../stacks/StackEnumHelper.h"

#define ROUTE_PROTOCOL 254

uint8_t DomainExtensions::convertToSystem(InternetProtocolEnum item)
{
	switch (item)
	{
		case InternetProtocolEnum::IPv4:
			return AF_INET;
		case InternetProtocolEnum::IPv6:
			return AF_INET6;
		case InternetProtocolEnum::Route:
			return AF_UNSPEC;
	}
	return AF_INET;
}

uint8_t DomainExtensions::convertToSystem(TransportProtocolEnum item)
{
	switch (item)
	{
		case TransportProtocolEnum::TCP:
			return IPPROTO_TCP;
		case TransportProtocolEnum::UDP:
			return IPPROTO_UDP;
		case TransportProtocolEnum::UDPLITE:
			return IPPROTO_UDPLITE;
		case TransportProtocolEnum::SCTP:
            return IPPROTO_SCTP;
        case TransportProtocolEnum::DCCP:
            return IPPROTO_DCCP;
		case TransportProtocolEnum::ROUTE:
			return ROUTE_PROTOCOL;
        case TransportProtocolEnum::NONE:
		case TransportProtocolEnum::RAW:
			return IPPROTO_RAW;
	}
	return IPPROTO_RAW;
}

InternetProtocolEnum DomainExtensions::getInternetProtocol(const sockaddr_storage &item)
{
	return convertFromSystem(static_cast<uint8_t>(item.ss_family));
}


TransportProtocolEnum DomainExtensions::getTransportProtocol(uint8_t data)
{
	switch(data)
	{
		case IPPROTO_UDP:
			return TransportProtocolEnum::UDP;
        case IPPROTO_UDPLITE:
			return TransportProtocolEnum::UDPLITE;
		case IPPROTO_TCP:
			return TransportProtocolEnum::TCP;
        case IPPROTO_SCTP:
            return TransportProtocolEnum::SCTP;
        case IPPROTO_DCCP:
            return TransportProtocolEnum::DCCP;
		case ROUTE_PROTOCOL:
			return TransportProtocolEnum::ROUTE;
		default:
			return TransportProtocolEnum::RAW;
	}
}

TransportProtocolEnum DomainExtensions::getTransportProtocol(StackEnum stack) {
    return StackEnumHelper::getInfo(stack).TransportProtocol;
}

InternetProtocolEnum DomainExtensions::convertFromSystem(uint16_t addressFamily)
{
	switch (addressFamily)
	{
		case AF_INET:
			return InternetProtocolEnum::IPv4;
		case AF_INET6:
			return InternetProtocolEnum::IPv6;
		case AF_UNSPEC:
			return InternetProtocolEnum::Route;
		default:
			return InternetProtocolEnum::IPv4;
	}
}

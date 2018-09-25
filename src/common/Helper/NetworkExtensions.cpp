
#include "NetworkExtensions.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>

uint32_t NetworkExtensions::convertIpv4(const std::string &ip)
{
	return ntohl(convertIpv4NetworkOrder(ip));
}

uint32_t NetworkExtensions::convertIpv4NetworkOrder(const std::string &ip)
{
    in_addr addr{};
    if (inet_aton(ip.c_str(), &addr) == 1)
    {
        return addr.s_addr;
    }
    return 0;
}

in6_addr NetworkExtensions::convertIpv6(const std::string &ip)
{
	in6_addr addr{};
	if (inet_pton(AF_INET6, ip.c_str(), &addr) == 1)
	{
		return addr;
	}
	
	return in6_addr{};
}

std::string NetworkExtensions::getAddress(const sockaddr_storage& addr)
{
	if(addr.ss_family == AF_INET)
	{
		auto addrIpv4 = reinterpret_cast<const sockaddr_in*>(&addr);
		
		return inet_ntoa(addrIpv4->sin_addr);
	}
	
	auto addrIpv6 = reinterpret_cast<const sockaddr_in6*>(&addr);
	char buffer[INET6_ADDRSTRLEN];
	
	return inet_ntop(AF_INET6, &addrIpv6->sin6_addr, buffer, INET6_ADDRSTRLEN);
}


uint32_t NetworkExtensions::getAddressIpv4(const sockaddr_storage &addr)
{
	if(addr.ss_family != AF_INET)
		return 0;

	auto addrIpv4 = reinterpret_cast<const sockaddr_in*>(&addr);
	return addrIpv4->sin_addr.s_addr;
}

bool NetworkExtensions::isSameSenderAddress(const sockaddr_storage& first, const sockaddr_storage& second)
{
	if(first.ss_family != second.ss_family)
	{
		Logger::Log(Logger::WARNING, "Tried to compare two socket sender addresses of different networkFamily");
		return false;
	}
	
	if(first.ss_family == AF_INET)
	{
		auto firstIpv4 = reinterpret_cast<const sockaddr_in*>(&first);
		auto secondIpv4 = reinterpret_cast<const sockaddr_in*>(&second);
		
		return firstIpv4->sin_addr.s_addr == secondIpv4->sin_addr.s_addr;
	}
	
	auto firstIpv6 = reinterpret_cast<const sockaddr_in6*>(&first);
	auto secondIpv6 = reinterpret_cast<const sockaddr_in6*>(&second);
	
	return memcmp(&firstIpv6->sin6_addr, &secondIpv6->sin6_addr, sizeof(firstIpv6->sin6_addr)) == 0;
}


/**
	 * Creates a (IPv4) sockaddr_in-struct for socket connection using the given ip and port
	 * @param ipAddress The IPv4 address to use
	 * @param port The port to use
	 * @return The created struct
	 */
sockaddr_storage NetworkExtensions::getIPv4SockAddr(uint32_t ipAddress, const uint16_t port)
{
	sockaddr_storage sockInfo{};
	memset(&sockInfo, 0x00, sizeof(struct sockaddr_storage));

	auto ipv4Info = reinterpret_cast<sockaddr_in*>(&sockInfo);
	ipv4Info->sin_family = AF_INET;
	ipv4Info->sin_port = htons(port);
	ipv4Info->sin_addr.s_addr = htonl(ipAddress);

	return sockInfo;
}

/**
	 * Creates a (IPv4) sockaddr_in-struct for socket connection using the given ip and port
	 * @param ipAddress The IPv4 address to use
	 * @param port The port to use
	 * @return The created struct
	 */
sockaddr_storage NetworkExtensions::getIPv4SockAddr(const std::string& ipAddress, const uint16_t port)
{
	sockaddr_storage sockInfo{};
	memset(&sockInfo, 0x00, sizeof(struct sockaddr_storage));

	auto ipv4Info = reinterpret_cast<sockaddr_in*>(&sockInfo);
	ipv4Info->sin_family = AF_INET;
	ipv4Info->sin_port = htons(port);
	ipv4Info->sin_addr.s_addr = inet_addr(ipAddress.c_str());

	return sockInfo;
}

/**
	 * Creates a (IPv4) sockaddr_in-struct for socket connection using the given ip and port
	 * @param ipAddress The IPv4 address to use
	 * @param port The port to use
	 * @return The created struct
	 */
sockaddr_storage NetworkExtensions::getIPv6SockAddr(const std::string& ipAddress, const uint16_t port)
{
	sockaddr_storage sockInfo{};
	memset(&sockInfo, 0x00, sizeof(struct sockaddr_storage));

	auto ipv6Info = reinterpret_cast<sockaddr_in6*>(&sockInfo);
	ipv6Info->sin6_family = AF_INET6;
	ipv6Info->sin6_port = htons(port);
	inet_pton(AF_INET6, ipAddress.c_str(), &(ipv6Info->sin6_addr));

	return sockInfo;
}

void NetworkExtensions::setPort(sockaddr_storage &sockAddr, uint16_t port)
{
	if(sockAddr.ss_family == AF_INET)
	{
		auto addrIpv4 = reinterpret_cast<sockaddr_in*>(&sockAddr);
		addrIpv4->sin_port = htons(port);
		return;
	}

	auto addrIpv6 = reinterpret_cast<sockaddr_in6*>(&sockAddr);
	addrIpv6->sin6_port = htons(port);
}

std::pair<sockaddr_storage, sockaddr_storage> NetworkExtensions::getSourceAndDestAddr(const InspectionStruct& inspectionStruct, const Storage &storage)
{
	std::pair<sockaddr_storage, sockaddr_storage> result{};

	switch(inspectionStruct.internetProtocol)
	{
		case InternetProtocolEnum::IPv4:
		{
			const auto ipv4Header = reinterpret_cast<const iphdr *>(storage.constData());
			auto * ipv4Source = reinterpret_cast<sockaddr_in*>(&result.first);
			auto * ipv4Dest = reinterpret_cast<sockaddr_in*>(&result.second);

			ipv4Source->sin_family = AF_INET;
			ipv4Dest->sin_family = AF_INET;

			ipv4Source->sin_addr.s_addr = ipv4Header->saddr;
			ipv4Dest->sin_addr.s_addr = ipv4Header->daddr;

			auto udpHdr = reinterpret_cast<const udphdr*>(storage.constData() + inspectionStruct.transportProtocolStartIndex); //ports liegen bei tcp an der selben stelle
			ipv4Source->sin_port = static_cast<uint16_t>(udpHdr->source); //hs to be in networkorder so just leave it
			ipv4Dest->sin_port = static_cast<uint16_t>(udpHdr->dest);
			break;
		}
		case InternetProtocolEnum::IPv6:
		{
			const auto ipv6Header = reinterpret_cast<const ip6_hdr *>(storage.constData());
			auto * ipv6Source = reinterpret_cast<sockaddr_in6*>(&result.first);
			auto * ipv6Dest = reinterpret_cast<sockaddr_in6*>(&result.second);

			ipv6Source->sin6_family = AF_INET6;
			ipv6Dest->sin6_family = AF_INET6;

			ipv6Source->sin6_addr = ipv6Header->ip6_src;
			ipv6Dest->sin6_addr = ipv6Header->ip6_dst;

			auto udpHdr = reinterpret_cast<const udphdr*>(storage.constData(inspectionStruct.transportProtocolStartIndex)); //ports liegen bei tcp an der selben stelle
			ipv6Source->sin6_port = static_cast<uint16_t>(udpHdr->source); //nthos not necessary
			ipv6Dest->sin6_port = static_cast<uint16_t>(udpHdr->dest);
			break;
		}
        case InternetProtocolEnum::Route:
            break;
	}

	return result;
}

uint16_t NetworkExtensions::getPort(const sockaddr_storage &sockAddr, bool toHostOrder) {
	if (sockAddr.ss_family == AF_INET) {
		auto addrIpv4 = reinterpret_cast<const sockaddr_in *>(&sockAddr);
		if (toHostOrder)
			return ntohs(addrIpv4->sin_port);
		return addrIpv4->sin_port;
	}

	auto addrIpv6 = reinterpret_cast<const sockaddr_in6 *>(&sockAddr);
	if (toHostOrder)
		return ntohs(addrIpv6->sin6_port);
	return addrIpv6->sin6_port;
}

bool NetworkExtensions::OverwriteIPv4(sockaddr_storage &destination, const std::string& sourceIp, const uint32_t bitmask)
{
	if(destination.ss_family != AF_INET)
		return false;

	uint32_t convertedBitmask = htonl(bitmask);
	uint32_t ipv4Source = inet_addr(sourceIp.c_str());
	auto * ipv4Dest = reinterpret_cast<sockaddr_in*>(&destination);

	auto firstPart = (ipv4Dest->sin_addr.s_addr & (~convertedBitmask));
	auto secondPart = (ipv4Source & convertedBitmask);
	ipv4Dest->sin_addr.s_addr = firstPart | secondPart;
	return true;
}

bool NetworkExtensions::SetIPv4Address(sockaddr_storage &item, const std::string &ip)
{
	if(item.ss_family != AF_INET)
		return false;

	uint32_t ipv4Source = inet_addr(ip.c_str());
	auto * ipv4Item = reinterpret_cast<sockaddr_in*>(&item);
	ipv4Item->sin_addr.s_addr = ipv4Source;

	return true;
}

sockaddr_storage NetworkExtensions::getSockAddr(InternetProtocolEnum internetProtocol, const std::string &ipAddress,
                                                const uint16_t port) {
    switch (internetProtocol)
	{
		case InternetProtocolEnum::IPv4:
			return getIPv4SockAddr(ipAddress, port);
		case InternetProtocolEnum::IPv6:
			return getIPv6SockAddr(ipAddress, port);
		case InternetProtocolEnum::Route:
			return sockaddr_storage{};
	}
	return sockaddr_storage{};
}

bool NetworkExtensions::isLocalhostAddress(const sockaddr_storage &addr)
{
    if(addr.ss_family == AF_INET)
    {
        auto addrIpv4 = reinterpret_cast<const sockaddr_in*>(&addr);

        //Check if s_addr starts with 127 which means localhost
        return (addrIpv4->sin_addr.s_addr & 0xFF) == 127;
    }

//    auto addrIpv6 = reinterpret_cast<const sockaddr_in6*>(&addr);
//    return IN6_IS_ADDR_LOOPBACK(&addrIpv6->sin6_addr);

    Logger::Log(Logger::ERROR, "NetworkExtensions::isLocalhostAddress is not implemented for IPv6");
    return true; //NOT_IMPLEMENTED
}

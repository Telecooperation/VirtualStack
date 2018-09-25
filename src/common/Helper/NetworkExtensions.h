
#pragma once

#include "../../model/InspectionStruct.h"
#include "../DataStructures/Model/Storage.h"
#include <cstdint>
#include <string>
#include <sys/socket.h>

/**
 * Provides commonly used methods used for network usage
 */
class NetworkExtensions
{
public:
	static uint32_t convertIpv4(const std::string &ip);
	static uint32_t convertIpv4NetworkOrder(const std::string &ip);
	static struct in6_addr convertIpv6(const std::string &ip);

    static bool isLocalhostAddress(const sockaddr_storage &addr);
	
	static std::pair<sockaddr_storage, sockaddr_storage> getSourceAndDestAddr(const InspectionStruct& inspectionStruct, const Storage &storage);
	
	static std::string getAddress(const sockaddr_storage &addr);

	static uint32_t getAddressIpv4(const sockaddr_storage &addr);

	static bool isSameSenderAddress(const sockaddr_storage &first, const sockaddr_storage &second);

	static sockaddr_storage getIPv4SockAddr(uint32_t ipAddress, const uint16_t port);
	
	static sockaddr_storage getIPv4SockAddr(const std::string& ipAddress, const uint16_t port);
	
	static sockaddr_storage getIPv6SockAddr(const std::string& ipAddress, const uint16_t port);

	static sockaddr_storage getSockAddr(InternetProtocolEnum internetProtocol, const std::string& ipAddress, const uint16_t port);

	static void setPort(sockaddr_storage& sockAddr, uint16_t port);
	
	static uint16_t getPort(const sockaddr_storage& sockAddr, bool toHostOrder = true);
	static bool OverwriteIPv4(sockaddr_storage &destination, const std::string& sourceIp, const uint32_t bitmask);
	static bool SetIPv4Address(sockaddr_storage &item, const std::string &ip);
};

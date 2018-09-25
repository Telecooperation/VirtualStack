#pragma once
#include <vector>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <cstdint>
#include <string>
#include "../../../../common/Helper/NetworkExtensions.h"
#include "../../../../common/DataStructures/Model/Storage.h"
#include "../../../../common/Helper/ChecksumExtensions.h"
#include "../../../../common/Helper/DomainExtensions.h"

/**
 * A factory class to construct IPv4 Headers and to validate them
 */
class IPHeaderFactory
{
public:
	static const uint8_t DEFAULT_TTL = 128;
	
	/**
	 * Create a valid ipv4 header with checksum from a given payload-size and metadata
	 * @param payloadSize The size of the payload for this header
	 * @param sourceAddress The source IP address for the header
	 * @param destAddress The destination IP address to the header
	 * @return A valid ipv4 header
	 */
	static struct iphdr BuildIpv4Header(const TransportProtocolEnum transportProtocol, const size_t payloadSize,
										const std::string &sourceAddress,const std::string &destAddress)
	{
		return BuildIpv4Header(transportProtocol,
							   payloadSize,
							   NetworkExtensions::convertIpv4(sourceAddress),
							   NetworkExtensions::convertIpv4(destAddress));
	}
	
	static struct iphdr BuildIpv4Header(const TransportProtocolEnum transportProtocol, const size_t payloadSize,
										const uint32_t sourceAddress,const uint32_t destAddress)
	{
		iphdr ipHeader;
		memset(&ipHeader, 0, sizeof(ipHeader));
		
		ipHeader.version = static_cast<uint8_t >(4);
		ipHeader.ihl = static_cast<uint8_t>(getIpv4HeaderSize() / sizeof(uint32_t));
		ipHeader.tos = 0;
		ipHeader.tot_len = htons(static_cast<uint16_t>(payloadSize + getIpv4HeaderSize()));
		ipHeader.id = htons(1);
		ipHeader.frag_off = 0;
		ipHeader.ttl = DEFAULT_TTL;
		ipHeader.protocol = DomainExtensions::convertToSystem(transportProtocol);
		ipHeader.saddr = htonl(sourceAddress);
		ipHeader.daddr = htonl(destAddress);
		
		ipHeader.check = 0;
		ipHeader.check = generateIpv4Checksum(ipHeader);
		return ipHeader;
	}
	
	/**
	 * Create a valid ipv6 header with checksum from a given payload-size and metadata
	 * @param payloadSize The size of the payload for this header
	 * @param sourceAddress The source IP address for the header
	 * @param destAddress The destination IP address to the header
	 * @return A valid ipv4 header
	 */
	static struct ip6_hdr BuildIpv6Header(const TransportProtocolEnum transportProtocol, const size_t payloadSize,
										const std::string &sourceAddress,const std::string &destAddress)
	{
		return BuildIpv6Header(transportProtocol,
							   payloadSize,
							   NetworkExtensions::convertIpv6(sourceAddress),
							   NetworkExtensions::convertIpv6(destAddress));
	}
	
	static struct ip6_hdr BuildIpv6Header(const TransportProtocolEnum transportProtocol, const size_t payloadSize,
										const in6_addr sourceAddress,const in6_addr destAddress)
	{
		ip6_hdr ipHeader;
		memset(&ipHeader, 0, sizeof(ipHeader));
		
		ipHeader.ip6_vfc = 6 << 4;
		ipHeader.ip6_src = sourceAddress; //htonl???
		ipHeader.ip6_dst = destAddress;
		ipHeader.ip6_nxt = DomainExtensions::convertToSystem(transportProtocol);
		ipHeader.ip6_hlim = DEFAULT_TTL;
		ipHeader.ip6_plen = htons(static_cast<uint16_t>(payloadSize + getIpv6HeaderSize()));
		
		
		return ipHeader;
	}
	
	/**
	 * Reads the ipv4 header from a given storage and validate it by its checksum
	 * @param payload The payload with a preceeding ipv4 header
	 * @return True if the header is valid based on its given checksum, false otherwise
	 */
	static bool Ipv4ChecksumMatch(Storage &payload)
	{
		auto tmpHeader = arrayToIpv4Hdr(payload, false); //checksumme wird auf header in networkOrder generiert, daher hier keine konvertierung inst hostOrder
		return generateIpv4Checksum(tmpHeader) == 0;
	}
	
	/**
	 * Generate the checksum for a ipv4 header. If you dont want do validate the checksum but create a new one you have to set header.check=0 before calling
	 * @param header The ipv4 header
	 * @return The checksum for the header
	 */
	static uint16_t generateIpv4Checksum(struct iphdr header)
	{
		auto headerChecksum = ChecksumExtensions::generateChecksumWithoutWrap(reinterpret_cast<const uint16_t *>(&header),
																			  sizeof(iphdr) / sizeof(uint8_t));
		while (headerChecksum >> 16)
			headerChecksum = (headerChecksum & 0xffff) + (headerChecksum >> 16);
		return static_cast<uint16_t>(~headerChecksum);
	}
	
	/**
	 * Converts a header into a storage
	 * @param header The header to convert
	 * @return The storage containing the header
	 */
	static Storage hdrToArray(struct iphdr header)
	{
		return Storage::toStorage(header);
	}
	
	/**
	 * Converts a header into a storage
	 * @param header The header to convert
	 * @return The storage containing the header
	 */
	static Storage hdrToArray(struct ip6_hdr header)
	{
		return Storage::toStorage(header);
	}
	
	/**
	 * Converts a storage into an ipv4 header. The Storage can be a regular one or one from a socket (Network-to-Host byte-order)
	 * @param data The storage containing a ipv4 header
	 * @param isFromSocket Indicates if the Storage came from an socket, so it will convert the bytes from network to host byte order
	 * @return The ipv4 header of the storage
	 */
	static struct iphdr arrayToIpv4Hdr(const Storage& data, bool isFromSocket = false)
	{
		auto tmpHeader = data.toType<struct iphdr>(getIpv4HeaderSize());
		if(isFromSocket)
		{
			tmpHeader.tot_len = ntohs(tmpHeader.tot_len);
			tmpHeader.id = ntohs(tmpHeader.id);
			tmpHeader.saddr = ntohl(tmpHeader.saddr);
			tmpHeader.daddr = ntohl(tmpHeader.daddr);
			
		}
		return tmpHeader;
	}
	
	/**
	 * Converts a storage into an ipv4 header. The Storage can be a regular one or one from a socket (Network-to-Host byte-order)
	 * @param data The storage containing a ipv4 header
	 * @param isFromSocket Indicates if the Storage came from an socket, so it will convert the bytes from network to host byte order
	 * @return The ipv4 header of the storage
	 */
	static struct ip6_hdr arrayToIpv6Hdr(const Storage& data, bool isFromSocket = false)
	{
		auto tmpHeader = data.toType<struct ip6_hdr>(getIpv6HeaderSize());
		if(isFromSocket)
		{
			tmpHeader.ip6_plen = ntohs(tmpHeader.ip6_plen);
			
		}
		return tmpHeader;
	}
	
	/**
	 * Get the size in bytes for an ipv4 header
	 * @return Size of ipv4 header
	 */
	static size_t getIpv4HeaderSize()
    {
		return sizeof(iphdr);
	}
	
	/**
	 * Get the size in bytes for an ipv6 header
	 * @return Size of ipv6 header
	 */
	static size_t getIpv6HeaderSize()
    {
		return sizeof(ip6_hdr);
	}
};

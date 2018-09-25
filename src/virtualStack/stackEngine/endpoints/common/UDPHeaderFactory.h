
#pragma once

#include <netinet/udp.h>
#include <netinet/in.h>
#include "../../../../common/DataStructures/Model/Storage.h"
#include "../../../../common/Helper/NetworkExtensions.h"
#include "../../../../common/Helper/ChecksumExtensions.h"

/**
 * A factory class to construct UDP Headers and to validate them
 */
class UDPHeaderFactory
{
public:
    /**
     * Create a valid UDP header with checksum from a given payload and metadata
     * @param payload The payload for the header
     * @param sport UDP-Source port
     * @param dport UDP-Destination port
     * @param sourceAddress The source IP address for the header
     * @param destAddress The destination IP address for the header
     * @return A valid ipv4 header
     */
    static struct udphdr BuildHeader(const Storage &payload, const uint16_t sport,
                                     const uint16_t dport, const std::string &sourceAddress,
                                     const std::string &destAddress, bool ignoreChecksum = false)
    {
       return BuildHeader(payload, sport, dport,
                          NetworkExtensions::convertIpv4(sourceAddress),
                          NetworkExtensions::convertIpv4(destAddress), ignoreChecksum);
                                          
    }
    
    
    static struct udphdr BuildHeader(const Storage &payload, const uint16_t sport,
                                     const uint16_t dport, const uint32_t sourceAddress,
                                     const uint32_t destAddress, bool ignoreChecksum = false)
    {
        uint16_t tmpLength = static_cast<uint16_t>(payload.size() + getHeaderSize());
        struct udphdr hdr;
        hdr.len = htons(tmpLength);
        hdr.source = htons(sport);
        hdr.dest = htons(dport);
        
        hdr.check = 0;
        if(!ignoreChecksum)
        {
            hdr.check = generateChecksumForIPv4(hdr,
                                                sourceAddress,
                                                destAddress,
                                                payload);
        }
        return hdr;
    }

    /**
     * Calculates the checksum of the UDP-Header with the payload
     * @param header The UDP Header
     * @param payload The payload
     * @param payloadStartIndex The startIndex of the payload if the payload contains an udp header
     */
    static uint32_t getChecksum(udphdr header, const Storage &payload, size_t payloadStartIndex = 0)
    {
        auto headerChecksum = ChecksumExtensions::generateChecksumWithoutWrap(reinterpret_cast<const uint16_t *>(&header),
                                                                   sizeof(udphdr) / sizeof(uint8_t));

        auto payloadChecksum = ChecksumExtensions::generateChecksumWithoutWrap(
                reinterpret_cast<const uint16_t *>(payload.constData() + payloadStartIndex),
                payload.size() - payloadStartIndex);
        return headerChecksum + payloadChecksum;
    }

    /**
     * Reads the UDP header from a given storage and validate it by its checksum
     * @param payload The payload with a preceding UDP header
     * @param sourceAddress The source IP address for the header
     * @param destAddress The destination IP address for the header
     * @return True if the header is valid based on its given checksum, false otherwise
     */
    static bool ChecksumMatch(const std::string &sourceAddress, const std::string &destAddress,
                              const Storage &payload)
    {
        auto tmpHeader = arrayToHdr(payload, false);
        auto tmpChecksum = generateChecksumForIPv4(tmpHeader,
                                                   NetworkExtensions::convertIpv4(sourceAddress),
                                                   NetworkExtensions::convertIpv4(destAddress),
                                                   payload, getHeaderSize());
        return tmpChecksum == 0;
    }

    /**
     * Generate a pseudoChecksum which is needed for UDP-Checksum generation. Its a fake ip checksum
     * @param source_ip The source IP address for the header as number @see NetworkExtensions
     * @param dest_ip The destination IP address for the header as number @see NetworkExtensions
     * @param len The payload size
     * @param flag A flag saying which type of pseudoHeader it is, for example: IPPROTO_IDP = 22
     * @return The pseudoChecksum
     */
    static uint32_t getPseudoheaderChecksum(uint32_t source_ip, uint32_t dest_ip, uint16_t len, uint8_t flag)
    {
        uint16_t *ip_src = reinterpret_cast<uint16_t *>(&source_ip);
        uint16_t *ip_dst = reinterpret_cast<uint16_t *>(&dest_ip);

        uint32_t checksum = 0;
        checksum += htons(*(ip_src++));
        checksum += htons(*ip_src);

        checksum += htons(*(ip_dst++));
        checksum += htons(*ip_dst);

        checksum += htons(flag);
        checksum += htons(len);
        return checksum;
    }

    /**
     * Generate the checksum for a UDP header. If you dont want do validate the checksum but create a new one you have to set header.check=0 before calling
     * @param udp The UDP header
     * @param src_addr The source IP address for the header as number @see NetworkExtensions
     * @param dst_addr The destination IP address for the header as number @see NetworkExtensions
     * @param payload The payload
     * @param payloadStartIndex The startIndex of the payload if the payload contains an udp header
     * @return The checksum for the header
     */
    static uint16_t generateChecksumForIPv4(udphdr udp, uint32_t src_addr, uint32_t dst_addr, const Storage &payload,
                                            size_t payloadStartIndex = 0)
    {
        uint32_t checksum = getPseudoheaderChecksum(
                src_addr,
                dst_addr,
                static_cast<uint16_t>(payload.size() + getHeaderSize() - payloadStartIndex),
                IPPROTO_UDP
        ) + getChecksum(udp, payload, payloadStartIndex);

        while (checksum >> 16)
            checksum = (checksum & 0xffff) + (checksum >> 16);
        return static_cast<uint16_t>(~checksum);
    }

    /**
     * Converts a header into a storage
     * @param header The header to convert
     * @return The storage containing the header
     */
    static Storage hdrToArray(const struct udphdr header)
    {
        return Storage::toStorage(header);
    }

    /**
	* Converts a storage into an UDP header. The Storage can be a regular one or one from a socket (Network-to-Host byte-order)
	* @param data The storage containing a UDP header
	* @param isFromSocket Indicates if the Storage came from an socket, so it will convert the bytes from network to host byte order
	* @return The UDP header of the storage
	*/
    static struct udphdr arrayToHdr(const Storage &data, bool isFromSocket = false)
    {
        auto tmpHeader = data.toType<struct udphdr>(getHeaderSize());
        if (isFromSocket)
        {
            tmpHeader.source = ntohs(tmpHeader.source);
            tmpHeader.dest = ntohs(tmpHeader.dest);
            tmpHeader.len = ntohs(tmpHeader.len);
        }
        return tmpHeader;
    }

    /**
     * Get the size in bytes for an UDP header
     * @return Size of UDP header
     */
    static inline size_t getHeaderSize()
    {
        return sizeof(udphdr);
    }
};

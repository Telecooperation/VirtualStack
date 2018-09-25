#pragma once

#include "../../../common/DataStructures/Model/Storage.h"
#include <cstdint>

/*
 * Paket:
 * Type: FEC, Other
 * Header: .... (ACK, Data, WindowUpdate), Size, SeqNum
 * Content
 */

struct UdpPlusHeader
{
    explicit UdpPlusHeader(bool pIsData = true) : isData(pIsData ? 1u: 0u)
    {}

    const uint8_t isData;

    bool isDataPacket()
    {
        return isData > 0;
    }

    inline static size_t getHeaderSize()
    {
        return sizeof(UdpPlusHeader);
    }
} __attribute__((packed));

struct UdpPlusFecHeader
{
    explicit UdpPlusFecHeader(bool pIsFec = true) : fecGroup(0),
                                                   isFec(pIsFec ? 1u : 0u)
    {

    }

    size_t fecGroup;
    const uint8_t isFec;

    bool isFecPacket()
    {
        return isFec > 0;
    }

    inline static size_t getHeaderSize()
    {
        return sizeof(UdpPlusFecHeader);
    }
} __attribute__((packed));

struct UdpPlusDataHeader
{
    UdpPlusDataHeader() : size(0),
                          seqNum(0)
    {}
    uint16_t size;
    size_t seqNum;

    inline static size_t getHeaderSize()
    {
        return sizeof(UdpPlusDataHeader);
    }
} __attribute__((packed));


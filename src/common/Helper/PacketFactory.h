#pragma once

#include <virtualStack/stackEngine/endpoints/common/UDPHeaderFactory.h>
#include <virtualStack/stackEngine/endpoints/common/IPHeaderFactory.h>

class PacketFactory final
{
public:
	static StoragePoolPtr createUdpPacket(Pool<Storage>& pool, const sockaddr_storage& source, const sockaddr_storage& destination) {
		auto tmpStorage = pool.request();

		auto tmpSource = reinterpret_cast<const sockaddr_in *>(&source);
		auto tmpDest = reinterpret_cast<const sockaddr_in *>(&destination);

		auto tmpUdpHeader = UDPHeaderFactory::BuildHeader(*tmpStorage,
														  ntohs(tmpSource->sin_port), ntohs(tmpDest->sin_port),
														  ntohl(tmpSource->sin_addr.s_addr),
														  ntohl(tmpDest->sin_addr.s_addr), true);
		tmpStorage->prependDataAutomaticBeforeStart(&tmpUdpHeader);

		auto tmpHeader = IPHeaderFactory::BuildIpv4Header(TransportProtocolEnum::UDP, tmpStorage->size(),
														  ntohl(tmpSource->sin_addr.s_addr),
														  ntohl(tmpDest->sin_addr.s_addr));

		tmpStorage->prependDataAutomaticBeforeStart(&tmpHeader);

		return tmpStorage;
	}

    static size_t getUdpPacketHeaderSize()
    {
        return UDPHeaderFactory::getHeaderSize() + IPHeaderFactory::getIpv4HeaderSize();
    }
};

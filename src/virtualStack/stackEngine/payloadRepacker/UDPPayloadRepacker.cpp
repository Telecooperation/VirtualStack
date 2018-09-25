#include "UDPPayloadRepacker.h"
#include "../endpoints/common/UDPHeaderFactory.h"
#include "../../../common/Helper/DomainExtensions.h"
#include "../endpoints/common/IPHeaderFactory.h"

void UDPPayloadRepacker::repack(const sockaddr_storage& northboundSource,
								const sockaddr_storage& northboundDestination,
								Storage &storage)
{
	//füge udp header an
	//füge ip header an
	//ignoriere checksumme denn wir gehen von datenkonsitenz aus
	
	
	if(DomainExtensions::getInternetProtocol(northboundSource) == InternetProtocolEnum::IPv4)
	{
		repackIpv4(northboundSource, northboundDestination, storage);
		return;
	}
	
	repackIpv6(northboundSource, northboundDestination, storage);
}

void
UDPPayloadRepacker::repackIpv4(const sockaddr_storage &northboundSource, const sockaddr_storage &northboundDestination,
							   Storage &storage) {
	repackUDPHeaderOnly(northboundSource, northboundDestination, storage);
	auto tmpSource = reinterpret_cast<const sockaddr_in *>(&northboundDestination);
	auto tmpDest = reinterpret_cast<const sockaddr_in *>(&northboundSource);

	auto tmpHeader = IPHeaderFactory::BuildIpv4Header(TransportProtocolEnum::UDP,
													  storage.size(),
													  ntohl(tmpSource->sin_addr.s_addr),
													  ntohl(tmpDest->sin_addr.s_addr));
	storage.prependDataAutomaticBeforeStart(&tmpHeader);
}

void
UDPPayloadRepacker::repackIpv6(const sockaddr_storage &northboundSource, const sockaddr_storage &northboundDestination,
							   Storage &storage)
{
	repackUDPHeaderOnly(northboundSource, northboundDestination, storage);
	auto tmpSource = reinterpret_cast<const sockaddr_in6*>(&northboundDestination);
	auto tmpDest = reinterpret_cast<const sockaddr_in6*>(&northboundSource);
	
	auto tmpHeader = IPHeaderFactory::BuildIpv6Header(TransportProtocolEnum::UDP,
													  storage.size(),
													  tmpSource->sin6_addr,
													  tmpDest->sin6_addr);
	storage.prependDataAutomaticBeforeStart(&tmpHeader);
}

void UDPPayloadRepacker::repackUDPHeaderOnly(const sockaddr_storage &northboundSource,
											 const sockaddr_storage &northboundDestination, Storage &storage)
{
	//wir müssen source und destination der ports vertauschen
	//zudem können wir die source und destination ip ignorieren, da wir keine checksumme berechnen
	auto tmpUdpHeader = UDPHeaderFactory::BuildHeader(storage,
													  NetworkExtensions::getPort(northboundDestination),
													  NetworkExtensions::getPort(northboundSource),
													  0, 0,
													  true);
	storage.prependDataAutomaticBeforeStart(&tmpUdpHeader);
}

#pragma once


#include "../../../common/DataStructures/Model/Storage.h"
#include "../../../interface/IEndpoint.h"

class UDPPayloadRepacker final
{
public:
	static void repackIpv4(const sockaddr_storage& northboundSource,
						   const sockaddr_storage& northboundDestination,
						   Storage& storage);
	static void repackIpv6(const sockaddr_storage& northboundSource,
						   const sockaddr_storage& northboundDestination,
						   Storage& storage);
	static void repack(const sockaddr_storage& northboundSource,
					   const sockaddr_storage& northboundDestination,
					   Storage& storage);
	static void repackUDPHeaderOnly(const sockaddr_storage& northboundSource,
							  const sockaddr_storage& northboundDestination,
									Storage& storage);
};



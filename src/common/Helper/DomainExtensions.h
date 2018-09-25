#pragma once

#include "../../model/InternetProtocolEnum.h"
#include "../../model/TransportProtocolEnum.h"
#include "../../stacks/StackEnum.h"
#include <sys/socket.h>

class DomainExtensions final
{
public:
	static uint8_t convertToSystem(InternetProtocolEnum item);
	
	static uint8_t convertToSystem(TransportProtocolEnum item);

	static InternetProtocolEnum convertFromSystem(uint16_t addressFamily);
	
	static InternetProtocolEnum getInternetProtocol(const sockaddr_storage &item);
	
	static TransportProtocolEnum getTransportProtocol(uint8_t data);

	static TransportProtocolEnum getTransportProtocol(StackEnum stack);
};
#pragma once

#include "../../../src/common/DataStructures/Container/unique_fd.h"
#include "../../../src/model/InternetProtocolEnum.h"
#include "UdpBasedSouthboundDevice.h"

namespace ipv6Runner
{
	void Run()
	{
		//2 udp verbindungen auf selben server, ereknnen ob die soc kets auch getrennt verstanden werden
		
		//accept auf lo, port 10000, any_address
		UdpBasedSouthboundDevice dev{InternetProtocolEnum::IPv4, "tun2", 10000};
		if(!dev.deviceCreated())
		{
			return;
		}
		unique_fd tmpSocket = dev.listenAndAccept();
		
	}
}
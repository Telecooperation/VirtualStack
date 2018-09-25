#pragma once

#include <common/DataStructures/Container/unique_fd.h>
#include <model/InternetProtocolEnum.h>
#include <netinet/in.h>
#include <string>

class UdpBasedSouthboundDevice

{
public:
	UdpBasedSouthboundDevice(InternetProtocolEnum internetProtocol, std::string deviceName, uint16_t listenPort);
	unique_fd listenAndAccept();
	
	unique_fd createSocket(int family, int type, int protocol) const;
	bool deviceCreated() const { return _deviceCreated; }
private:
	unique_fd _deviceFd;
	
//	uint16_t _deviceId;
	std::string _deviceName;
	bool _deviceCreated;
	
	
	static sockaddr_storage getSocketListenConfiguration(InternetProtocolEnum internetProtocol, uint16_t listenPort);
	static bool configureSocketAsListener(unique_fd& deviceFd, InternetProtocolEnum internetProtocol, std::string deviceName, uint16_t listenPort);
};



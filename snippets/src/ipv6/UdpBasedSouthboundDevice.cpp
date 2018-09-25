#include "UdpBasedSouthboundDevice.h"
#include <common/Helper/DomainExtensions.h>
#include <common/Helper/Logger.h>
#include <cstring>
#include <net/if.h>


UdpBasedSouthboundDevice::UdpBasedSouthboundDevice(InternetProtocolEnum internetProtocol, std::string deviceName, uint16_t listenPort) {
	_deviceName = deviceName;
	_deviceFd = createSocket(DomainExtensions::convertToSystem(internetProtocol), SOCK_DGRAM, IPPROTO_UDP);
	_deviceCreated = configureSocketAsListener(_deviceFd, internetProtocol, deviceName, listenPort);
}

unique_fd UdpBasedSouthboundDevice::listenAndAccept()
{
	if(listen(_deviceFd, 5) < 0)
	{
		Logger::Log(Logger::ERROR, "Error listen socket on southbound device: \"", _deviceName ,"\" with error: ", strerror(errno));
		return unique_fd();
	}
	
	sockaddr_storage sockInfo;
	socklen_t sockInfoSize = sizeof(sockInfo);
	unique_fd tmpNewClient = unique_fd(accept(_deviceFd, reinterpret_cast<sockaddr *>(&sockInfo), &sockInfoSize));
	
	//konfigurieren, dass udp header selbst geleistet wird und ip header ebenfalls?
	
	if(!tmpNewClient){
		Logger::Log(Logger::ERROR, "Error accepting socket on southbound device: \"", _deviceName ,"\" with error: ", strerror(errno));
		return unique_fd();
	}
	
	return tmpNewClient;
}

unique_fd UdpBasedSouthboundDevice::createSocket(int family, int type, int protocol) const
{
	unique_fd tmpSocket = unique_fd(socket(family, type, protocol));
	if(tmpSocket < 0)
	{
		Logger::Log(Logger::ERROR, "Error creating socket for southbound device: \"", _deviceName ,"\" with error: ", strerror(errno));
		return unique_fd();
	}
	
	int result = setsockopt(tmpSocket, SOL_SOCKET, SO_BINDTODEVICE, _deviceName.c_str(), static_cast<socklen_t>(_deviceName.length()));
	if(result < 0)
	{
		Logger::Log(Logger::ERROR, "Error binding socket to southbound device: \"", _deviceName ,"\" with error: ", strerror(errno));
		return unique_fd();
	}
	return tmpSocket;
}

bool UdpBasedSouthboundDevice::configureSocketAsListener(unique_fd& deviceFd, InternetProtocolEnum internetProtocol, std::string deviceName, uint16_t listenPort)
{
	auto listenConfiguration = getSocketListenConfiguration(internetProtocol, listenPort);
	if (bind(deviceFd, reinterpret_cast<struct sockaddr *>(&listenConfiguration), sizeof(sockaddr_storage)) < 0)
	{
		Logger::Log(Logger::LogLevel::ERROR, "bind() call to configure socket as listen socket failed -> Error: ", errno, ", Message: ",
					strerror(errno));
		return false;
	}
	
	ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", deviceName.c_str());

	if (setsockopt(deviceFd, SOL_SOCKET, SO_BINDTODEVICE, reinterpret_cast<void *>(&ifr), sizeof(ifr)) < 0) {
		Logger::Log(Logger::LogLevel::ERROR, "setsockopt() call to bind socket to device failed -> Error: ", errno, ", Message: ",
					strerror(errno));
		return false;
	}
	
	return true;
}

sockaddr_storage UdpBasedSouthboundDevice::getSocketListenConfiguration(InternetProtocolEnum internetProtocol, uint16_t listenPort)
{
	sockaddr_storage sockInfo;
	memset(&sockInfo, 0x00, sizeof(struct sockaddr_storage));
	
	if(internetProtocol == InternetProtocolEnum::IPv4)
	{
		auto sockInfoIPv4 = reinterpret_cast<sockaddr_in*>(&sockInfo);
		sockInfoIPv4->sin_family = AF_INET;
		sockInfoIPv4->sin_port = htons(listenPort);
		sockInfoIPv4->sin_addr.s_addr = INADDR_ANY;
	}else
	{
		auto sockInfoIPv6 = reinterpret_cast<sockaddr_in6*>(&sockInfo);
		sockInfoIPv6->sin6_family = AF_INET6;
		sockInfoIPv6->sin6_port = htons(listenPort);
		sockInfoIPv6->sin6_addr = IN6ADDR_ANY_INIT;
	}
	
	return sockInfo;
}

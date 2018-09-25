#pragma once

#include "../../../src/common/DataStructures/Container/unique_fd.h"
#include "../../../src/common/Helper/Logger.h"
#include "../../../src/common/Helper/NetworkExtensions.h"
#include "../../../src/common/Helper/socket/SocketExtensions.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

namespace UdpSocketSnippet
{
	void bind(unique_fd& fd);
	void Run()
	{
		std::string _deviceName = "lo";
		unique_fd tmpSocket = unique_fd(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
		unique_fd tmpSocket2 = unique_fd(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
		if(tmpSocket < 0)
		{
			Logger::Log(Logger::ERROR, "Error creating socket for southbound device: \"", _deviceName ,"\" with error: ", strerror(errno));
			return; //unique_fd();
		}
		
//		SocketExtensions::bindToNetworkDevice(tmpSocket, _deviceName);
		
		//aus sicht northobund -> flow, wenn southrboud nicht die src sond dst port
		
		uint8_t buffer[3] = {0,1,2};
		//bind(tmpSocket);
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(13423);
		addr.sin_addr.s_addr = htonl(NetworkExtensions::convertIpv4("127.0.0.1"));

		sendto(tmpSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		sendto(tmpSocket2, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	}
	
	void bind(unique_fd& fd)
	{
		struct sockaddr_in serv_addr;
		bzero(reinterpret_cast<char *>(&serv_addr), sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = 0;
		if (bind(fd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
			if(errno == EADDRINUSE) {
				printf("the port is not available. already to other process\n");
				return;
			} else {
				printf("could not bind to process (%d) %s\n", errno, strerror(errno));
				return;
			}
		}
		
		socklen_t len = sizeof(serv_addr);
		if (getsockname(fd, reinterpret_cast<sockaddr *>(&serv_addr), &len) == -1) {
			perror("getsockname");
			return;
		}
		
		printf("port number %d\n", ntohs(serv_addr.sin_port));
	}
}
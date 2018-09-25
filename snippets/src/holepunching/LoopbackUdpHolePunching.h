#pragma once


#include <common/Helper/NetworkExtensions.h>
#include <common/Helper/StorageExtensions.h>
#include <common/Helper/socket/SocketExtensions.h>
#include <common/Helper/socket/UDPSocketExtensions.h>
#include <future>
#include <iostream>
#include <kernel/GenericKernel.h>
#include <sys/socket.h>

namespace LoopbackUdpHolePunching
{
	unique_fd connect(uint16_t sourcePort, uint16_t destinationPort, bool isServer);
	void run();

	unique_fd connect(uint16_t sourcePort, uint16_t destinationPort, bool isServer)
	{
		sockaddr_storage sockInfoLokal = NetworkExtensions::getIPv4SockAddr("127.0.0.1", sourcePort);;
		sockaddr_storage sockInfoRemote = NetworkExtensions::getIPv4SockAddr("127.0.0.11", destinationPort);;
		auto fd = UDPSocketExtensions::createUDPSocket(AF_INET);
		SocketExtensions::setSocketReuse(fd);
		//me
		if(!isServer)
		{
			std::cout << "Client" << std::endl;
			std::swap(sockInfoLokal, sockInfoRemote);
		}
		
		SocketExtensions::bindSocket(fd, sockInfoLokal);
		SocketExtensions::bindToNetworkDevice(fd, "lo");
		sockaddr_storage tmpStorage{};
		bzero(&tmpStorage, sizeof(tmpStorage));
		
		tmpStorage = sockInfoRemote;
		while (!SocketExtensions::connectSocket(fd, tmpStorage, true))
			std::this_thread::sleep_for(std::chrono::seconds(2));
		
		
		Logger::Log(Logger::DEBUG, "RemoteInfo: ",
					NetworkExtensions::getAddress(tmpStorage),
					", Port: ",
					NetworkExtensions::getPort(tmpStorage));
		return fd;
	}
	
	
	void run()
	{
        VsObjectFactory vsObjectFactory{1024, 1400, 0};
		
		PoolRef sendPool = vsObjectFactory.getStoragePool(1, "");
		
		auto tmpFirstFuture = std::async(std::launch::async, &connect, 11000, 10000, true);
		auto tmpSecond = connect(11000, 10000, false);
		auto tmpFirst = tmpFirstFuture.get();
		
		
		GenericKernel firstKernel{std::move(tmpFirst), vsObjectFactory, 1024};
		GenericKernel secondKernel{std::move(tmpSecond), vsObjectFactory, 1024};
		
		//while(true)
		for (uint16_t j = 0; j < 1; ++j)
		{
			auto tmpSendBuffer = sendPool.request();
			
			time_t tmpTime = time(nullptr);
			tmpSendBuffer->appendDataAfterEnd(&tmpTime, sizeof(time_t));
			
			for (size_t i = 0; i < 5; ++i)
			{
				firstKernel.sendPacket(*tmpSendBuffer);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				if(secondKernel.dataAvailable())
				{
					auto tmpReceived = secondKernel.receivePacket();
					auto tmpData = StorageExtensions::StorageToType<time_t>(*tmpReceived, sizeof(time_t));
					std::cout << "Server responded current time: " << ctime(&tmpData) << std::endl;
				}
			}
			
			for (size_t i = 0; i < 5; ++i)
			{
				secondKernel.sendPacket(*tmpSendBuffer);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				if(firstKernel.dataAvailable())
				{
					auto tmpReceived = firstKernel.receivePacket();
					auto tmpData = StorageExtensions::StorageToType<time_t>(*tmpReceived, sizeof(time_t));
					std::cout << "Server responded current time: " << ctime(&tmpData) << std::endl;
				}
			}
		}
		
	}
}
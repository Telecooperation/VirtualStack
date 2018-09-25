#pragma once


#include <common/Helper/NetworkExtensions.h>
#include <common/Helper/StorageExtensions.h>
#include <common/Helper/socket/SocketExtensions.h>
#include <common/Helper/socket/TCPSocketExtensions.h>
#include <iostream>
#include <kernel/GenericKernel.h>
#include <sys/socket.h>

namespace UdpHolePunching
{
	unique_fd connect(uint16_t sourcePort, uint16_t destinationPort, bool isServer, bool accept);
	void run(int argc) _Noreturn;

	unique_fd connect(uint16_t sourcePort, uint16_t destinationPort, bool isServer, bool accept)
	{
		sockaddr_storage sockInfoLokal{};
		sockaddr_storage sockInfoRemote{};
		auto fd = TCPSocketExtensions::createTcpSocket(AF_INET);
		SocketExtensions::setSocketReuse(fd);
		//me
		if(isServer)
		{
			std::cout << "ich" << std::endl;
			sockInfoLokal = NetworkExtensions::getIPv4SockAddr("127.0.0.1", sourcePort);
			sockInfoRemote = NetworkExtensions::getIPv4SockAddr("127.0.0.1", destinationPort);
//			sockInfoLokal = NetworkExtensions::getIPv4SockAddr("192.168.178.43", sourcePort);
//			sockInfoRemote = NetworkExtensions::getIPv4SockAddr("80.145.228.107", destinationPort);
		}else
		{
			sockInfoLokal = NetworkExtensions::getIPv4SockAddr("127.0.0.1", destinationPort);
			sockInfoRemote = NetworkExtensions::getIPv4SockAddr("127.0.0.1", sourcePort);
//			sockInfoLokal = NetworkExtensions::getIPv4SockAddr("192.168.178.85", destinationPort);
//			sockInfoRemote = NetworkExtensions::getIPv4SockAddr("77.13.126.89", sourcePort);
		}
		
		//SocketExtensions::setAsListenSocket(fd);
		
		SocketExtensions::bindSocket(fd, sockInfoLokal);
		sockaddr_storage tmpStorage{};
		bzero(&tmpStorage, sizeof(tmpStorage));
		if(accept && isServer)
			SocketExtensions::listenAndAcceptSocket(fd, tmpStorage);
		else
		{
			tmpStorage = sockInfoRemote;
			while (!SocketExtensions::connectSocket(fd, tmpStorage, true))
				std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		
		Logger::Log(Logger::DEBUG, "RemoteInfo: ",
					NetworkExtensions::getAddress(tmpStorage),
					", Port: ",
					NetworkExtensions::getPort(tmpStorage));
		return fd;
	}
	

	void run(int argc)
	{
		VsObjectFactory vsObjectFactory{1024, 1400, 0};
		bool isM = argc > 1;
		
		PoolRef sendPool = vsObjectFactory.getStoragePool(1, "");
		
		//mi: 77.13.102.141
//		//er: 87.165.222.191
//		auto sockInfoM = NetworkExtensions::getIPv4SockAddr("77.13.102.141", 11000);
//		auto sockInfoE = NetworkExtensions::getIPv4SockAddr("87.165.222.191", 12000);
		
		
		auto tmpManagement = connect(3000, 4000, isM, true);
		
		while(true)
		{
			for (uint16_t j = 0; j < 5; ++j)
			{
				auto tmpfd = connect(11000 + j, 12000, isM, false);
				GenericKernel kernel{std::move(tmpfd), vsObjectFactory, 1024};

				auto tmpSendBuffer = sendPool.request();

				time_t tmpTime = time(nullptr);
				tmpSendBuffer->appendDataAfterEnd(&tmpTime, sizeof(time_t));

				for (size_t i = 0; i < 5; ++i)
				{
					if (kernel.dataAvailable())
					{
						auto tmpReceived = kernel.receivePacket();
						time_t tmpData = StorageExtensions::StorageToType<time_t>(*tmpReceived, sizeof(time_t));
						std::cout << "Server responded current time: " << ctime(&tmpData) << std::endl;
					}
					kernel.sendPacket(*tmpSendBuffer);
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
		}
	}
}
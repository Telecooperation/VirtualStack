#pragma once

#include <common/Helper/NetworkExtensions.h>
#include <common/Helper/StorageExtensions.h>
#include <common/Helper/socket/TCPSocketExtensions.h>
#include <iostream>
#include <kernel/GenericKernel.h>

namespace AcceptSnippet
{
	unique_fd connect(uint16_t sourcePort, uint16_t destinationPort, bool isServer, bool accept);
	void run(int argc);

	unique_fd connect(uint16_t sourcePort, uint16_t destinationPort, bool isServer, bool accept)
	{
		sockaddr_storage sockInfoLokal;
		sockaddr_storage sockInfoRemote;
		auto fd = TCPSocketExtensions::createTcpSocket(AF_INET);
		SocketExtensions::setSocketReuse(fd);
		//me
		if(isServer)
		{
			std::cout << "ich" << std::endl;
			sockInfoLokal = NetworkExtensions::getIPv4SockAddr("192.168.178.43", sourcePort);
			sockInfoRemote = NetworkExtensions::getIPv4SockAddr("80.145.232.209", destinationPort);
		}else
		{
			sockInfoLokal = NetworkExtensions::getIPv4SockAddr("192.168.178.85", destinationPort);
			sockInfoRemote = NetworkExtensions::getIPv4SockAddr("77.13.231.159", sourcePort);
		}
		
		SocketExtensions::bindSocket(fd, sockInfoLokal);
		
		if(accept && isServer)
		{
			fd_set tmpFdSet;
			FD_ZERO(&tmpFdSet);
			FD_SET(fd.get(), &tmpFdSet);

//			timeval tv;
//			tv.tv_sec = 0;
//			tv.tv_usec = 100 * 1000;
			
			listen(fd, 4);
			//auto tmpSelectInfo = select(fd.get() + 1, &tmpFdSet, 0, 0, nullptr);

			socklen_t connectAddressSize;
			unique_fd clientSocket{::accept(fd, reinterpret_cast<sockaddr *>(&sockInfoRemote), &connectAddressSize)};
			if (clientSocket < 0)
			{
				std::string tmpError(strerror(errno));
				Logger::Log(Logger::LogLevel::ERROR, "accept() for Socket was not successful -> Error: ", errno, ", Message: ",
							tmpError);
			}
			Logger::Log(Logger::INFO, "accept() successful");
			return clientSocket;
//			return SocketExtensions::listenAndAcceptSocket(fd, sockInfoRemote);
		}
		else
		{
			while (!SocketExtensions::connectSocket(fd, sockInfoRemote))
				std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		
		Logger::Log(Logger::DEBUG, "RemoteInfo: ",
					NetworkExtensions::getAddress(sockInfoRemote),
					", Port: ",
					NetworkExtensions::getPort(sockInfoRemote));
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
		auto tmpManagement = connect(14000, 14000, isM, true);
		
		
		GenericKernel kernel{std::move(tmpManagement), vsObjectFactory, 1024};
		
		auto tmpSendBuffer = sendPool.request();
		
		time_t tmpTime = time(nullptr);
		tmpSendBuffer->appendDataAfterEnd(&tmpTime, sizeof(time_t));
		
		for (size_t i = 0; i < 5; ++i)
		{
			if(kernel.dataAvailable())
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
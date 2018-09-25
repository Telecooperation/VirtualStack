#pragma once

#include <DefaultVirtualStackSettings.h>
#include <common/Allocator/VsObjectFactory.h>
#include <common/DataStructures/Container/unique_fd.h>
#include <common/Helper/NetworkExtensions.h>
#include <iostream>
#include <kernel/StreamKernel.h>
#include <model/PosixSocketFactory.h>

namespace TcpHolePunching
{
	UniqueSocket connect(ISocketFactory& socketFactory, uint16_t sourcePort, uint16_t destinationPort, bool isServer)
    {
        sockaddr_storage sockInfoLokal;
        sockaddr_storage sockInfoRemote;
        auto fd = socketFactory.createTCPSocket(InternetProtocolEnum::IPv4);
        fd->setSocketReuse();
        //me
        if (isServer)
        {
            std::cout << "ich" << std::endl;
            sockInfoLokal = NetworkExtensions::getIPv4SockAddr("192.168.178.28", sourcePort);
            sockInfoRemote = NetworkExtensions::getIPv4SockAddr("217.231.95.141", destinationPort);
        } else
        {
            sockInfoLokal = NetworkExtensions::getIPv4SockAddr("0.0.0.0", destinationPort);
            sockInfoRemote = NetworkExtensions::getIPv4SockAddr("79.205.43.19", sourcePort);
        }

        fd->bindSocket(sockInfoLokal);

        while (!fd->connectSocket(sockInfoRemote))
            std::this_thread::sleep_for(std::chrono::milliseconds(2));

        Logger::Log(Logger::DEBUG, "RemoteInfo: ",
                    NetworkExtensions::getAddress(sockInfoRemote),
                    ", Port: ",
                    NetworkExtensions::getPort(sockInfoRemote));
        return fd;
    }
	
	
	void run(int argc)
	{
        auto settings = DefaultVirtualStackSettings::Default();
        VsObjectFactory vsObjectFactory{std::make_unique<PosixSocketFactory>(), 1400, 0, 0};
		bool isM = argc > 1;
		
		PoolRef sendPool = vsObjectFactory.getStorageSendPool(512, "");
		
		//mi: 77.13.102.141
//		//er: 87.165.222.191
//		auto sockInfoM = NetworkExtensions::getIPv4SockAddr("77.13.102.141", 11000);
//		auto sockInfoE = NetworkExtensions::getIPv4SockAddr("87.165.222.191", 12000);
		
		
		//while(true)
		//for (uint16_t j = 0; j < 5; ++j)
		{
			auto tmpfd = connect(*vsObjectFactory.socketFactory, 11002, 12002, isM);
			StreamKernel kernel{std::move(tmpfd), *settings, vsObjectFactory};
			
			auto tmpSendBuffer = sendPool->request();
			
			time_t tmpTime = time(nullptr);
			tmpSendBuffer->appendDataAfterEnd(&tmpTime, sizeof(time_t));
			
			for (size_t i = 0; i < 5; ++i)
			{
				if(kernel.dataAvailable())
				{
					auto tmpReceived = kernel.receivePacket();
					time_t tmpData = tmpReceived->toTypeAutomatic<time_t>();
					std::cout << "Server responded current time: " << ctime(&tmpData) << std::endl;
				}
				kernel.sendPacket(*tmpSendBuffer);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
		
	}
}
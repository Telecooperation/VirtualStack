#pragma once

#include <DefaultVirtualStackSettings.h>
#include <LatencyMeter.h>
#include <VirtualStack.h>
#include <VirtualStackLoader.h>
#include <algorithm>
#include <chrono>
#include <climits>
#include <common/Allocator/VsObjectFactory.h>
#include <common/DataStructures/VS/SettingsProvider.h>
#include <common/Helper/BandwidthLimiter.h>
#include <common/Helper/PacketFactory.h>
#include <common/Helper/ReverseSourceDest.h>
#include <common/Helper/SettingsStreamFactory.h>
#include <common/Helper/StopWatch.h>
#include <fstream>
#include <iostream>
#include <stacks/StackEnum.h>
#include <stacks/updPlus/model/UdpPlusHeader.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/factory/StackFactory.h>

class RawServerClient
{
public:
    static void storeTime(Storage& storage)
    {
        auto currTime = StopWatch::getHighResolutionTime();
        storage.decrementEndIndex(sizeof(long));
        storage.appendDataScalarAfterEnd(currTime);
    }

    static long readTime(Storage& storage)
    {
       return storage.toTypeAutomatic<long>(storage.size() - sizeof(long));
    }

    static void sender(const std::string& senderIp,
                       const std::string& recvIp,
                       const size_t timeInSeconds,
                       const double dataRateInMbps,
                       const size_t payloadSize,
                       TransportProtocolEnum transportProtocol)
    {
        auto source = NetworkExtensions::getIPv4SockAddr(senderIp, 0);
        auto destination = NetworkExtensions::getIPv4SockAddr(recvIp, 0);
        PosixSocketFactory socketFactory;

        auto socket = socketFactory.createSocket(InternetProtocolEnum::IPv4, transportProtocol);

        //Associate a free port for both sockets
        socket->bindSocket(source);
        auto socketPort = socket->getPort();

        std::cout << socketPort << std::endl;
        uint16_t destinationPort = 0;
        std::cin >> destinationPort;

        NetworkExtensions::setPort(source, socketPort);
        NetworkExtensions::setPort(destination, destinationPort);

        socket = socketFactory.createConnection(std::move(socket), true,
                                                destination, nullptr, nullptr);

        if(!socket)
        {
            Logger::Log(Logger::ERROR, "Connection not established, abort");
            return;
        }

        const size_t packetSize = payloadSize >= sizeof(long) ? payloadSize : 1400;
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};
        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        // ## Begin: Initial
        Storage initialPacket(sizeof(size_t) + sizeof(long));
        initialPacket.incrementStartIndex(sizeof(size_t));
        initialPacket.prependDataAutomaticBeforeStart(&packetSize);
        storeTime(initialPacket);

        Logger::Log(Logger::INFO, "Send first packet with packetSize");
        socket->sendBlocking(initialPacket.data(), initialPacket.size());
        size_t receivedPacketSize = 0;
        socket->receiveBlocking(reinterpret_cast<uint8_t*>(&receivedPacketSize), sizeof(packetSize), transportProtocol == TransportProtocolEnum::TCP);
        Logger::Log(Logger::INFO, "Received packetSize from receiver");

        if(packetSize != receivedPacketSize)
            return;
        Logger::Log(Logger::INFO, "Connection Established");

        Storage packet(packetSize);
        StopWatch tim;
        while(!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            if(bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                storeTime(packet);
                socket->sendBlocking(packet.data(), packet.size());
            }
        }
    }

	static std::unique_ptr<LatencyResult> receiver(const std::string& senderIp,
                                                   const std::string& recvIp,
                                                   const size_t timeInSeconds,
                                                   TransportProtocolEnum transportProtocol)
	{
        std::vector<int64_t> packetDelay{};
        packetDelay.reserve(50000000);

        auto source = NetworkExtensions::getIPv4SockAddr(recvIp, 0);
        auto destination = NetworkExtensions::getIPv4SockAddr(senderIp, 0);

        PosixSocketFactory socketFactory;
        auto socket = socketFactory.createSocket(InternetProtocolEnum::IPv4, transportProtocol);

        //Associate a free port for both sockets
        socket->bindSocket(source);
        auto socketPort = socket->getPort();

        std::cout << socketPort << std::endl;
        uint16_t destinationPort = 0;
        std::cin >> destinationPort;

        NetworkExtensions::setPort(source, socketPort);
        NetworkExtensions::setPort(destination, destinationPort);

        socket = socketFactory.createConnection(std::move(socket), false,
                                        destination, nullptr, nullptr);

        if(!socket)
        {
            Logger::Log(Logger::ERROR, "Connection not established, abort");
            return nullptr;
        }

        Logger::Log(Logger::INFO, "Waiting for first packet");
        Storage initialPacket(sizeof(size_t) + sizeof(long));
        socket->receiveBlocking(initialPacket.data(), initialPacket.size(), transportProtocol == TransportProtocolEnum::TCP);
        Logger::Log(Logger::INFO, "Received first packet");

        const size_t packetSize = initialPacket.toTypeAutomatic<size_t>();

		auto connectionPacketRecvTime = StopWatch::getHighResolutionTime();
        auto connectionPacketSendTime = readTime(initialPacket);
        const size_t connectionEstablishment = static_cast<size_t>(connectionPacketRecvTime - connectionPacketSendTime);

        //remove time
        initialPacket.decrementEndIndex(sizeof(long));
        socket->sendBlocking(initialPacket.data(), initialPacket.size());

        Logger::Log(Logger::INFO, "Sent first packet back");
        Logger::Log(Logger::INFO, "Connection Established");

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        Storage packet(packetSize);
        StopWatch tim;
		tim.start();
        while(!tim.stop().hasElapsed(timeInSecondsHighRes))
		{
            if(socket->getBytesAvailable() >= packet.size())
            {
                socket->receiveBlocking(packet.data(), packet.size(), transportProtocol == TransportProtocolEnum::TCP);

                auto sentTimePoint = readTime(packet);
                auto currentTime = StopWatch::getHighResolutionTime();
                auto sendRecvDelay = currentTime - sentTimePoint;
                packetDelay.push_back(sendRecvDelay);
            }
		};

        auto analyseResult = LatencyResult::analyse(0, "runner", connectionEstablishment,
                                                                  timeInSeconds, packetSize, std::move(packetDelay));
        return analyseResult;
	}
};
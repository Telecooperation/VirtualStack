#pragma once

#include <DefaultVirtualStackSettings.h>
#include <LatencyMeter.h>
#include <VirtualStack.h>
#include <VirtualStackLoader.h>
#include <climits>
#include <common/Allocator/VsObjectFactory.h>
#include <common/DataStructures/VS/SettingsProvider.h>
#include <common/Helper/BandwidthLimiter.h>
#include <common/Helper/PacketFactory.h>
#include <common/Helper/SettingsStreamFactory.h>
#include <common/Helper/StopWatch.h>
#include <fstream>
#include <iostream>
#include <stacks/StackEnum.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/factory/StackFactory.h>

namespace RawSocketTestCase
{
    std::unique_ptr<LatencyMeter> runPerf(const size_t timeInSeconds,
                 const double dataRateInMbps,
                                          const size_t payloadSize,
                 Pool<Storage> &pool,
                 UniqueSocket &sendSocket,
                 UniqueSocket &recvSocket)
    {
        Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", timeInSeconds);
        auto tmpPaketWarmup = pool.request();
        tmpPaketWarmup->expandEndIndex();

        auto sendSize = sendSocket->sendBlocking(tmpPaketWarmup->data(), tmpPaketWarmup->size());
        while (recvSocket->getBytesAvailable() < sendSize)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto recvPacketWarmup = pool.request();
        recvPacketWarmup->expandEndIndex();
        recvSocket->receiveBlocking(recvPacketWarmup->data(), recvPacketWarmup->size(), true);

        StopWatch tim;

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        const size_t packetSize = payloadSize >= sizeof(size_t) ? payloadSize : pool.request()->freeSpaceForAppend();
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};

        auto meter = std::make_unique<LatencyMeter>(packetSize);
        meter->clearInTime();
        auto sendKey = meter->getNewInTime("toNorthbound");

        tim.start();

        while (!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            if (pool.canRequest() && bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                auto tmpPaket = pool.request();
                tmpPaket->setSize(packetSize);

                LatencyMeter::addInTime(sendKey);
                sendSocket->sendBlocking(tmpPaket->data(), tmpPaket->size());
            }

            while (recvSocket->getBytesAvailable() >= packetSize)
            {
                auto recvPacket = pool.request();
                recvSocket->receiveBlocking(recvPacket->data(), packetSize, true);
                meter->addOutTime();
            }
        }

        return meter;
    }

    std::unique_ptr<LatencyMeter> run(size_t runtimeInSeconds,
                                      const double dataRateInMbps,
                                      const size_t payloadSize,
                                      TransportProtocolEnum transportProtocol)
    {
        Logger::setMinLogLevel(Logger::DEBUG);
        std::string sendIp = "127.0.0.2";
        std::string recvIp = "127.0.0.3";
        size_t sendMTU = 1500;

        //create send and recv socket
        //connect both per accept and connect
        //has to be done by async
        auto source = NetworkExtensions::getIPv4SockAddr(sendIp, 0);
        auto destination = NetworkExtensions::getIPv4SockAddr(recvIp, 0);
        auto socketFactory = std::make_unique<PosixSocketFactory>();

        StopWatch connectionEstablishmentWatch;
        connectionEstablishmentWatch.start();

        auto sendSocket = socketFactory->createSocket(InternetProtocolEnum::IPv4, transportProtocol);
        auto recvSocket = socketFactory->createSocket(InternetProtocolEnum::IPv4, transportProtocol);

        //Associate a free port for both sockets
        sendSocket->bindSocket(source);
        recvSocket->bindSocket(destination);

        NetworkExtensions::setPort(source, sendSocket->getPort());
        NetworkExtensions::setPort(destination, recvSocket->getPort());

        auto sendSocketTask = std::async(std::launch::async, [&]()
        {
            std::unique_ptr<ISocket> innerSendSocket{sendSocket.release()};
            return socketFactory->createConnection(std::move(innerSendSocket), true,
                                                  destination, nullptr, nullptr);
        });

        recvSocket = socketFactory->createConnection(std::move(recvSocket), false,
                                                    source, nullptr, nullptr);
        sendSocket = sendSocketTask.get();

        if (!recvSocket || !sendSocket)
        {
            Logger::Log(Logger::ERROR, "Send or receive socket failed to build");
            return nullptr;
        }
        connectionEstablishmentWatch.stop();
        Logger::Log(Logger::INFO, "Sockets build");

        VsObjectFactory factory{std::move(socketFactory), sendMTU, 100, 0};
        auto pool = factory.getStorageSendPool(2048, "Pool");

        auto latencyResult = runPerf(runtimeInSeconds, dataRateInMbps, payloadSize,
                                     *pool, sendSocket, recvSocket);

        latencyResult->connectionEstablishmentWatch = std::move(connectionEstablishmentWatch);
        return latencyResult;
    }
}
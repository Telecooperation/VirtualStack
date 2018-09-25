#pragma once

#include <DefaultVirtualStackSettings.h>
#include <LatencyMeter.h>
#include <VirtualStack.h>
#include <VirtualStackLoader.h>
#include <climits>
#include <common/Allocator/VsObjectFactory.h>
#include <common/DataStructures/VS/SettingsProvider.h>
#include <common/Helper/PacketFactory.h>
#include <common/Helper/SettingsStreamFactory.h>
#include <common/Helper/StopWatch.h>
#include <fstream>
#include <iostream>
#include <stacks/GenericStack.h>
#include <stacks/StackEnum.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/factory/StackFactory.h>
#include <common/Helper/BandwidthLimiter.h>

namespace StackTestCase
{
    std::unique_ptr<LatencyMeter> runPerf(const size_t timeInSeconds,
                 const double dataRateInMbps,
                 const size_t payloadSize,
                 Pool<Storage> &pool,
                 IStack &sendStack,
                 IStack &recvStack)
    {
        Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", timeInSeconds);
        auto tmpPaketWarmup = pool.request();
        tmpPaketWarmup->appendDataScalarAfterEnd(1);

        sendStack.push(std::move(tmpPaketWarmup));
        while (!recvStack.available())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        recvStack.pop();

        size_t recvPacketCount = 0;
        size_t sendPacketCount = 0;

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);
        StopWatch tim;
        tim.start();

        size_t bytesSent = 0;
        size_t bytesReceived = 0;

        const size_t packetSize = payloadSize >= sizeof(size_t) ? payloadSize : pool.request()->freeSpaceForAppend();
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};

        auto meter = std::make_unique<LatencyMeter>(packetSize);
        meter->clearInTime();
        auto sendKey = meter->getNewInTime("toNorthbound");

        tim.start();

        while (!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            if(!sendStack.isFull() && pool.canRequest() && bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                sendPacketCount++;

                auto tmpPaket = pool.request();
                tmpPaket->setSize(packetSize);

                bytesSent += tmpPaket->size();
                LatencyMeter::addInTime(sendKey);
                sendStack.push(std::move(tmpPaket));
            }

            while (recvStack.available())
            {
                recvPacketCount++;

                auto recvPacket = recvStack.pop();
                if (!recvPacket)
                    continue;
                bytesReceived += recvPacket->size();
                meter->addOutTime();
            }
        }

        return meter;
    }

    template <typename TStack=GenericStack>
    std::unique_ptr<LatencyMeter> run(size_t runtimeInSeconds,
             const double dataRateInMbps,
             const size_t payloadSize,
             StackEnum stackEnum, size_t stackSendBufferSize)
    {
        Logger::setMinLogLevel(Logger::DEBUG);
        std::string sendIp = "127.0.0.2";
        std::string recvIp = "127.0.0.3";

        //create send and recv socket
        //connect both per accept and connect
        //has to be done by async
        auto source = NetworkExtensions::getIPv4SockAddr(sendIp, 0);
        auto destination = NetworkExtensions::getIPv4SockAddr(recvIp, 0);

        auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(stackEnum, true);
        settingsStream.AddSizeT("SizeOfStackBuffer", stackSendBufferSize)
                .AddSizeT("SizeOfFromNorthboundBuffer", stackSendBufferSize);
        auto vsSettings = DefaultVirtualStackSettings::Default(settingsStream);

        auto vsObjectFactory = VirtualStackLoader<DummyNorthboundDevice>::createVSObjectFactory(*vsSettings);
        auto pool = vsObjectFactory->getStorageSendPool(vsSettings->SizeOfStackBuffer, "sendPool");
        auto &socketFactory = vsObjectFactory->socketFactory;

        auto transportProtocol = DomainExtensions::getTransportProtocol(stackEnum);

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
        Logger::Log(Logger::INFO, "Sockets build");

        auto sendStack = StackFactory::createStack(std::move(sendSocket), vsSettings->DefaultStack, *vsSettings, *vsObjectFactory);
        auto recvStack = StackFactory::createStack(std::move(recvSocket), vsSettings->DefaultStack, *vsSettings, *vsObjectFactory);

        recvStack->start(0);
        sendStack->start(0);

        return runPerf(runtimeInSeconds, dataRateInMbps, payloadSize, *pool, *sendStack, *recvStack);
    }
}
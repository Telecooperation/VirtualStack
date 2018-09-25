#pragma once

#include <DefaultVirtualStackSettings.h>
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
#include <stacks/StackEnum.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/factory/StackFactory.h>
#include <stacks/GenericStack.h>
#include <virtualStack/stackEngine/endpoints/UdpEndpoint.h>

namespace StackWithRecvControllerTestCase
{
    void runPerf(const size_t timeInSeconds,
                 Pool<Storage> &pool,
                 IStack &sendStack,
                 SequenceNumberMiddleware& sequenceNumberMiddleware,
                 DummyNorthboundDevice &recvStack)
    {
        Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", timeInSeconds);
        auto tmpPaketWarmup = pool.request();
        tmpPaketWarmup->appendDataScalarAfterEnd(1);

        if(sendStack.stackInfo.IsReliable)
            sequenceNumberMiddleware.addSequenceNumber(tmpPaketWarmup);

        sendStack.push(std::move(tmpPaketWarmup));
        while (!recvStack.availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        recvStack.externalOutOfNorthbound();

        size_t recvPacketCount = 0;
        size_t sendPacketCount = 0;

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);
        StopWatch tim;
        tim.start();

        size_t bytesSent = 0;
        size_t bytesReceived = 0;
        size_t packetDelay = 0;

        while (!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            bool poolRequestable = pool.canRequest();

            if (!sendStack.isFull() && poolRequestable)
            {
                sendPacketCount++;

                auto tmpPaket = pool.request();
                tmpPaket->expandEndIndex();

                auto currTime = StopWatch::getHighResolutionTime();
                tmpPaket->decrementEndIndex(sizeof(long));
                tmpPaket->appendDataScalarAfterEnd(currTime);

                if(sendStack.stackInfo.IsReliable)
                    sequenceNumberMiddleware.addSequenceNumber(tmpPaket);

                bytesSent += tmpPaket->size();
                sendStack.push(std::move(tmpPaket));
            }

            if (recvStack.availableExternal())
            {
                recvPacketCount++;

                auto recvPacket = recvStack.externalOutOfNorthbound();
                if (!recvPacket)
                    continue;
                bytesReceived += recvPacket->size();

                auto sentTimePoint = recvPacket->toTypeAutomatic<long>(recvPacket->size() - sizeof(long));
                auto currentTime = StopWatch::getHighResolutionTime();
                auto sendRecvDelay = currentTime - sentTimePoint;
                if (sendRecvDelay < 0)
                    Logger::Log(Logger::DEBUG, "VSSnippetSocket: sendRecvDelay < 0");
                packetDelay += static_cast<size_t>(sendRecvDelay);
            }
        }

        tim.stop();

        Logger::Log(Logger::INFO, bytesSent >> 20, " MByte sent (",
                    (bytesSent / tim.getDifferenceInSeconds()) / (1 << 20), " MByte/s), ",
                    bytesReceived >> 20, " MByte received (",
                    (bytesReceived / tim.getDifferenceInSeconds()) / (1 << 20), " MBytes/s)");

        if (recvPacketCount > 0)
            Logger::Log(Logger::INFO, "AVG Roundriptime in ns: ", (packetDelay / recvPacketCount));
    }

    template <typename TStack=GenericStack>
    std::unique_ptr<LatencyMeter> run(size_t runtimeInSeconds, StackEnum stackEnum, size_t stackSendBufferSize)
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
        auto pool = vsObjectFactory->getStorageSendPool(vsSettings->SizeOfNorthboundPoolBuffer, "sendPool");
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

        SequenceNumberMiddleware sequenceNumberMiddleware{*vsSettings};
        sockaddr_storage dummyAddr = NetworkExtensions::getIPv4SockAddr(sendIp, 10000);
        DummyNorthboundDevice dummyNorthboundDevice{*vsSettings, *vsObjectFactory};
        flowid_t flowid{};
        UdpEndpoint udpEndpoint(flowid, flowid, dummyAddr, dummyAddr);

        StackEngineReceiveController receiveController{*vsSettings, *vsObjectFactory, sequenceNumberMiddleware, udpEndpoint, dummyNorthboundDevice};
        receiveController.addStack(*recvStack);

        runPerf(runtimeInSeconds, *pool, *sendStack, sequenceNumberMiddleware, dummyNorthboundDevice);
        return nullptr;
    }
}
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
#include <common/Helper/BandwidthLimiter.h>

namespace VSRouterOnlyStack
{
    std::unique_ptr<LatencyMeter> runPerf(const size_t timeInSeconds,
                         const double dataRateInMbps,
                         const size_t payloadSize,
                         Pool<Storage> &pool,
                         IStack &sendStack,
                         IStack &recvStack)
    {
        Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", timeInSeconds);

//        const bool sendIsNotReliable = !sendStack.stackInfo.IsReliable;
//        const bool recvIsNotReliable = !recvStack.stackInfo.IsReliable;

//        size_t recvPacketCount = 0;
//        size_t sendPacketCount = 0;

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        const size_t packetSize = payloadSize >= sizeof(size_t) ? payloadSize : pool.request()->freeSpaceForAppend();
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};

        auto meter = std::make_unique<LatencyMeter>(packetSize);
        meter->clearInTime();
        auto sendKey = meter->getNewInTime("AppToAppRouter");

        StopWatch tim;
        tim.start();

        while (!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            if (!sendStack.isFull() && pool.canRequest() && bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                auto tmpPaket = pool.request();
                //tmpPaket->appendDataScalarAfterEnd(sendPacketCount); //add packetNumber for sequenceNumberMiddleware
                tmpPaket->setSize(packetSize);

                //sendPacketCount++;

                LatencyMeter::addInTime(sendKey);
                sendStack.push(std::move(tmpPaket));
            }

            while(recvStack.available())
            {
                auto recvPacket = recvStack.pop();
                if(!recvPacket)
                    continue;

                meter->addOutTime();

//                auto dataIntegrityValue = recvPacket->toTypeAutomatic<size_t>();
//
//                if(!sendIsNotReliable && !recvIsNotReliable && recvPacketCount != dataIntegrityValue)
//                    Logger::Log(Logger::ERROR, "DataIntegrity failed, expected: ", recvPacketCount, " but was: ", dataIntegrityValue);
//
//                recvPacketCount++;
            }
        }

        return meter;
    }

    std::tuple<UniqueSocket, UniqueSocket> createConnection(ISocketFactory& socketFactory,
                                                            sockaddr_storage source, sockaddr_storage destination,
                                                            TransportProtocolEnum transportProtocol)
    {
        auto sendSocket = socketFactory.createSocket(InternetProtocolEnum::IPv4, transportProtocol);
        auto recvSocket = socketFactory.createSocket(InternetProtocolEnum::IPv4, transportProtocol);

        //Associate a free port for both sockets
        sendSocket->bindSocket(source);
        recvSocket->bindSocket(destination);

        NetworkExtensions::setPort(source, sendSocket->getPort());
        NetworkExtensions::setPort(destination, recvSocket->getPort());

        auto sendSocketTask = std::async(std::launch::async, [&]()
        {
            std::unique_ptr<ISocket> innerSendSocket{sendSocket.release()};
            return socketFactory.createConnection(std::move(innerSendSocket), true,
                                                  destination, nullptr, nullptr);
        });

        recvSocket = socketFactory.createConnection(std::move(recvSocket), false,
                                                    source, nullptr, nullptr);
        sendSocket = sendSocketTask.get();
        if (!recvSocket || !sendSocket)
        {
            Logger::Log(Logger::ERROR, "Send or receive socket failed to build");
            return std::make_tuple(UniqueSocket(), UniqueSocket());
        }
        Logger::Log(Logger::INFO, "Sockets build");
        return std::make_tuple(std::move(sendSocket), std::move(recvSocket));
    }

    void addStacksToRouter(VirtualStackLoader<LoopbackNorthboundDevice>& vsLoader,
                           StackEnum sendStack,
                           StackEnum receiveStack,
                           UniqueSocket&& routerInSocket,
                           UniqueSocket&& routerOutSocket,
                           sockaddr_storage& routerOutSockAddr, sockaddr_storage& recvSockAddr)
    {
        flowid_t routerInFlowId;
        flowid_t routerOutFlowId;

        routerInFlowId.destination = 1;
        routerOutFlowId.destination = 2;

        auto routerInNewStackRequest = std::make_unique<NewStackRequest>();
        auto routerOutNewStackRequest = std::make_unique<NewStackRequest>();

        routerInNewStackRequest->northboundProtocol = TransportProtocolEnum::ROUTE;
        routerInNewStackRequest->northboundSourcePort = 10000;
        routerInNewStackRequest->northboundDestinationPort = 11000;

        routerOutNewStackRequest->northboundProtocol = TransportProtocolEnum::ROUTE;
        routerOutNewStackRequest->northboundSourcePort = 10000;
        routerOutNewStackRequest->northboundDestinationPort = 11000;

        auto routerInNewStackResult = std::make_unique<NewStackResult>(NetworkMessageEnum::NewStack, ConfigurationState::Ok, UniqueSocket(), routerOutSockAddr, std::move(routerInNewStackRequest));
        auto routerOutNewStackResult = std::make_unique<NewStackResult>(NetworkMessageEnum::NewStack, ConfigurationState::Ok, UniqueSocket(), recvSockAddr, std::move(routerOutNewStackRequest));

        auto routerInStack = StackFactory::createStack(std::move(routerInSocket), sendStack, *vsLoader.virtualStackSettings, *vsLoader.vsObjectFactory);
        auto routerOutStack = StackFactory::createStack(std::move(routerOutSocket), receiveStack, *vsLoader.virtualStackSettings, *vsLoader.vsObjectFactory);

        auto routerInCreation = std::make_unique<StackCreationResult>(false, std::move(routerInStack), nullptr, std::move(routerInNewStackResult));
        auto routerOutCreation = std::make_unique<StackCreationResult>(true, std::move(routerOutStack), nullptr, std::move(routerOutNewStackResult));

        routerInCreation->flowId = routerInFlowId;
        routerInCreation->partnerFlowId = routerOutFlowId;

        routerOutCreation->flowId = routerOutFlowId;
        routerOutCreation->partnerFlowId = routerInFlowId;

        vsLoader.virtualStack->start();

        //call .get() so we know its in the virtualStack
        auto& remoteControl = vsLoader.virtualStack->getRemoteControl();
        remoteControl.addStack(std::move(routerInCreation)).get();
        remoteControl.addStack(std::move(routerOutCreation)).get();
    }

    std::unique_ptr<LatencyMeter> run(size_t runtimeInSeconds,
             const double dataRateInMbps,
             const size_t payloadSize,
             StackEnum sendStackEnum,
             StackEnum receiveStackEnum,
             size_t stackSendBufferSize)
    {
        Logger::setMinLogLevel(Logger::DEBUG);
        std::string sendIp = "127.0.0.2";
        std::string routerInIp = "127.0.0.3";
        std::string routerOutIp = "127.0.0.4";
        std::string recvIp = "127.0.0.5";

        auto sendSockaddr = NetworkExtensions::getIPv4SockAddr(sendIp, 0);
        auto routerInSockAddr = NetworkExtensions::getIPv4SockAddr(routerInIp, 0);
        auto routerOutSockAddr = NetworkExtensions::getIPv4SockAddr(routerOutIp, 0);
        auto recvSockAddr = NetworkExtensions::getIPv4SockAddr(recvIp, 0);

        UniqueSocket sendSocket;
        UniqueSocket routerInSocket;
        UniqueSocket routerOutSocket;
        UniqueSocket recvSocket;

        auto sendTransportProtocol = DomainExtensions::getTransportProtocol(sendStackEnum);
        auto recvTransportProtocol = DomainExtensions::getTransportProtocol(receiveStackEnum);

        auto socketFactory = std::make_unique<PosixSocketFactory>();
        std::tie(sendSocket, routerInSocket) = createConnection(*socketFactory, sendSockaddr, routerInSockAddr, sendTransportProtocol);
        std::tie(routerOutSocket, recvSocket) = createConnection(*socketFactory, routerOutSockAddr, recvSockAddr, recvTransportProtocol);

        VirtualStackLoader<LoopbackNorthboundDevice> vsLoader;

        auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true, true);
        settingsStream.AddSizeT("SizeOfStackBuffer", stackSendBufferSize)
                .AddSizeT("SizeOfFromNorthboundBuffer", stackSendBufferSize);
        auto vsSettings = DefaultVirtualStackSettings::Default(settingsStream);

        if (!vsLoader.Initialize(std::move(vsSettings)))
            return nullptr;

        addStacksToRouter(vsLoader, sendStackEnum, receiveStackEnum, std::move(routerInSocket), std::move(routerOutSocket), routerOutSockAddr, recvSockAddr);

        auto pool = vsLoader.vsObjectFactory->getStorageSendPool(2048, "SendPool");

        auto sendStack = StackFactory::createStack(std::move(sendSocket), sendStackEnum, *vsLoader.virtualStackSettings, *vsLoader.vsObjectFactory);
        auto recvStack = StackFactory::createStack(std::move(recvSocket), receiveStackEnum, *vsLoader.virtualStackSettings, *vsLoader.vsObjectFactory);

        sendStack->start(0, false);
        recvStack->start(0, false);

        return runPerf(runtimeInSeconds, dataRateInMbps, payloadSize, *pool, *sendStack, *recvStack);
    }
}
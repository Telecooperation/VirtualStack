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
#include <virtualStack/device/northboundDevices/LoopbackNorthboundDevice.h>
#include <virtualStack/factory/StackFactory.h>
#include <common/Helper/BandwidthLimiter.h>
#include <LatencyMeter.h>

namespace VSSnippetMultiSocketRouted
{
	std::unique_ptr<LatencyMeter> run(size_t runtimeInSeconds,
                                      const double dataRateInMbps,
                                      const size_t payloadSize,
                                      StackEnum stack,
                                      StackEnum secondStack)
	{
		Logger::setMinLogLevel(Logger::DEBUG);

		auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(stack, true);

        settingsStream.AddBool("IsRouter", false);
        settingsStream.AddStringVector("RoutingTable", {"127.0.0.0", "127.0.0.2"});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.1"});
        settingsStream.AddString("ManagementBindAddress", "127.0.0.1");
        auto s1Settings = DefaultVirtualStackSettings::Default(settingsStream);

        settingsStream.AddBool("IsRouter", true);
        settingsStream.AddStringVector("RoutingTable", {});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.2"});
        settingsStream.AddString("ManagementBindAddress", "127.0.0.2");
        auto r1Settings = DefaultVirtualStackSettings::Default(settingsStream);

        settingsStream.AddBool("IsRouter", false);
        settingsStream.AddStringVector("RoutingTable", {"127.0.0.0", "127.0.0.2"});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.3"});
        settingsStream.AddString("ManagementBindAddress", "127.0.0.3");
        auto s2Settings = DefaultVirtualStackSettings::Default(settingsStream);

        const sockaddr_storage source = NetworkExtensions::getIPv4SockAddr(s1Settings->getDefaultIPv4(), 11000);
        const sockaddr_storage destination = NetworkExtensions::getIPv4SockAddr(s2Settings->getDefaultIPv4(), 12000);

        VirtualStackLoader<DummyNorthboundDevice> s2{};
        VirtualStackLoader<LoopbackNorthboundDevice> r1{};
        VirtualStackLoader<DummyNorthboundDevice> s1{};

        s2.Initialize(std::move(s2Settings));
        r1.Initialize(std::move(r1Settings));
        s1.Initialize(std::move(s1Settings));

        auto& s1Northbound = *s1.northboundDevice;
        auto& s2Northbound = *s2.northboundDevice;

        Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", runtimeInSeconds);

        const auto tmpTemplatePacket = PacketFactory::createUdpPacket(s1Northbound.getPool(), source, destination);
        const size_t udpHeaderSize = PacketFactory::getUdpPacketHeaderSize();
        tmpTemplatePacket->appendDataScalarAfterEnd(1);

        auto tmpPaket = s1Northbound.getPool().request();
        tmpTemplatePacket->copyInto(*tmpPaket);

        s1Northbound.externalIntoNorthbound(std::move(tmpPaket));
        while(!s2Northbound.availableExternal())
            std::this_thread::yield();

        // #### Add additional stack
        auto& remoteControl = s1.virtualStack->getRemoteControl();
        auto flowIdsFuture = remoteControl.getFlowIds();
        auto flowIds = flowIdsFuture.get();

        auto newStackFuture = remoteControl.createStack(flowIds.message[0], secondStack);
        auto newStackRequestResult = newStackFuture.get();
        if(newStackRequestResult.result != RemoteControlResultEnum::Ok)
        {
            Logger::Log(Logger::ERROR, "Failed to create second stack in stackCreationHandler");
            return nullptr;
        }
        auto newStackResult = newStackRequestResult.message.get();
        if(newStackResult != ConfigurationState::Ok)
        {
            Logger::Log(Logger::ERROR, "Failed to create second stack in southbound");
            return nullptr;
        }

        s2Northbound.externalOutOfNorthbound();

        auto timeInSecondsHighRes = std::chrono::seconds(runtimeInSeconds);


        StopWatch tim;
        const size_t packetSize = payloadSize >= (udpHeaderSize + sizeof(size_t)) ? payloadSize : s1Northbound.getPool().request()->freeSpaceForAppend();
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};

        auto meter = std::make_unique<LatencyMeter>(packetSize);
        meter->clearInTime();
        auto sendKey = meter->getNewInTime("toNorthbound");

        tim.start();

        while(!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            if(s1Northbound.canSend() && s1Northbound.getPool().canRequest() && bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                tmpPaket = s1Northbound.getPool().request();
                tmpTemplatePacket->copyInto(*tmpPaket);
                tmpPaket->setSize(packetSize);

                LatencyMeter::addInTime(sendKey);
                s1Northbound.externalIntoNorthbound(std::move(tmpPaket));
            }

            if(s2Northbound.availableExternal())
            {
                s2Northbound.externalOutOfNorthbound();
                meter->addOutTime();
            }
        }

        s2.virtualStack->stop();
        r1.virtualStack->stop();
        s1.virtualStack->stop();

        return meter;
	}
}
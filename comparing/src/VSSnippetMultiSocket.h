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
#include <common/Helper/SettingsStreamFactory.h>
#include <common/Helper/StopWatch.h>
#include <fstream>
#include <iostream>
#include <stacks/StackEnum.h>
#include <stacks/updPlus/model/UdpPlusHeader.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/factory/StackFactory.h>
#include <virtualStack/fastInspection/FlowIdGenerator.h>

namespace VSSnippetMultiSocket
{
    std::unique_ptr<LatencyMeter> runPerf(const size_t timeInSeconds,
                                          const double dataRateInMbps,
                                          const size_t payloadSize,
                                          VirtualStack &virtualStack,
                                          DummyNorthboundDevice &dummyDevice,
                                          StackEnum secondStack)
	{
		Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", timeInSeconds);

		auto tmpSource = NetworkExtensions::getIPv4SockAddr(virtualStack.getSettings().SouthboundInterfaceIPv4Address.value[0], 10000);
		auto tmpDestination = NetworkExtensions::getIPv4SockAddr(virtualStack.getSettings().SouthboundInterfaceIPv4Address.value[1],
																 11000);

        StopWatch tim;

		const auto tmpTemplatePacket = PacketFactory::createUdpPacket(dummyDevice.getPool(), tmpSource, tmpDestination);
		auto udpHeaderSize = PacketFactory::getUdpPacketHeaderSize();
		tmpTemplatePacket->appendDataScalarAfterEnd(1);

		auto tmpPaket = dummyDevice.getPool().request();
		tmpTemplatePacket->copyInto(*tmpPaket);

        const size_t packetSize = payloadSize >= (udpHeaderSize + sizeof(size_t)) ? payloadSize : dummyDevice.getPool().request()->freeSpaceForAppend();
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};

        auto meter = std::make_unique<LatencyMeter>(packetSize);

        meter->connectionEstablishmentWatch.start();
		dummyDevice.externalIntoNorthbound(std::move(tmpPaket));
		while(!dummyDevice.availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // #### Add additional stack
		auto& remoteControl = virtualStack.getRemoteControl();
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

		meter->connectionEstablishmentWatch.stop();

		dummyDevice.externalOutOfNorthbound();
        Logger::Log(Logger::INFO, "Connection Established");

        auto& pool = dummyDevice.getPool();

        meter->clearInTime();
        auto sendKey = meter->getNewInTime("toNorthbound");

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        tim.start();

        while(!tim.stop().hasElapsed(timeInSecondsHighRes))
		{
            if(dummyDevice.canSend() && pool.canRequest() && bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                tmpPaket = pool.request();
                tmpTemplatePacket->copyInto(*tmpPaket);
                tmpPaket->setSize(packetSize);

                LatencyMeter::addInTime(sendKey);
                dummyDevice.externalIntoNorthbound(std::move(tmpPaket));
            }

            while(dummyDevice.availableExternal())
            {
                auto recvPacket = dummyDevice.externalOutOfNorthbound();
                meter->addOutTime();
            }
		};

        virtualStack.stop();

        return meter;
	}
	
	std::unique_ptr<LatencyMeter> run(size_t runtimeInSeconds,
             const double dataRateInMbps,
             const size_t payloadSize,
             StackEnum stack,
             StackEnum secondStack,
             size_t stackSendBufferSize)
	{
		Logger::setMinLogLevel(Logger::DEBUG);

		auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(stack, true);
		settingsStream.AddStringVector("SouthboundInterfaces", {"lo","lo"})
				.AddSizeTVector("SouthboundInterfacesMTU", {1500, 1500})
				.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.2", "127.0.0.3"})
                .AddSizeT("SizeOfStackBuffer", stackSendBufferSize)
                .AddSizeT("SizeOfFromNorthboundBuffer", stackSendBufferSize);

		auto virtualStackSettings = DefaultVirtualStackSettings::Default(settingsStream);
		if(virtualStackSettings->SettingsReadFailed())
			return nullptr;

		StopWatch initTimer;
		initTimer.start();
		VirtualStackLoader<DummyNorthboundDevice> vsLoader;
		if (!vsLoader.Initialize(std::move(virtualStackSettings)))
			return nullptr;
		Logger::Log(Logger::DEBUG, "Time to initialize: ", initTimer.stop().getDifferenceInSeconds());

		auto& dummyDevice = *vsLoader.northboundDevice;

		return runPerf(runtimeInSeconds, dataRateInMbps, payloadSize, *vsLoader.virtualStack, dummyDevice, secondStack);
	}
}
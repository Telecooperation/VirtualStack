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
#include <virtualStack/fastInspection/FlowIdGenerator.h>

class VSServerClient
{
public:
    static void storeTime(Storage& storage)
    {
        storage.replaceDataScalarBeforeEnd(static_cast<size_t>(1));
        storage.replaceDataScalarBeforeEnd(static_cast<uint8_t>(0), sizeof(size_t));
        storage.replaceDataScalarBeforeEnd(StopWatch::getHighResolutionTime(), sizeof(uint8_t) + sizeof(size_t));
    }

    static size_t getTimeCount(Storage& storage)
    {
        return storage.toTypeAutomatic<size_t>(storage.size() - sizeof(size_t));
    }

    static uint8_t readStackIndex(Storage& storage, size_t index = 0)
    {
        //size_t for timeCount and long for every time
        return storage.toTypeAutomatic<uint8_t>(storage.size() - sizeof(size_t) + sizeof(long) - ((index + 1) * (sizeof(uint8_t) + sizeof(long))));
    }

    static long readTime(Storage& storage, size_t index = 0)
    {
        //size_t for timeCount and long for every time
        return storage.toTypeAutomatic<long>(storage.size() - sizeof(size_t) - ((index + 1) * (sizeof(uint8_t) + sizeof(long))));
    }

    static void sender(const std::string& recvIp,
                       const size_t timeInSeconds,
                       const double dataRateInMbps,
                       const size_t payloadSize,
                       VirtualStack& virtualStack,
                       DummyNorthboundDevice& dummyDevice,
                       StackEnum additionalStack, const std::string& connectOverIp)
    {
        Logger::Log(Logger::INFO, "Start measurement with runtime in seconds: ", timeInSeconds);

        auto tmpSource = NetworkExtensions::getIPv4SockAddr(virtualStack.getSettings().getDefaultIPv4(), 10000);
        auto tmpDestination = NetworkExtensions::getIPv4SockAddr(recvIp, 11000);

        const auto tmpTemplatePacket = PacketFactory::createUdpPacket(dummyDevice.getPool(), tmpSource, tmpDestination);
        auto udpHeaderSize = PacketFactory::getUdpPacketHeaderSize();

        //sizeof(long) is for timeStamp
        auto& pool = dummyDevice.getPool();
        const size_t packetSize = payloadSize >= (udpHeaderSize + sizeof(long)) ? payloadSize : pool.request()->freeSpaceForAppend();
        tmpTemplatePacket->setSize(packetSize);
        BandwidthLimiter bwLimiter{dataRateInMbps, packetSize};

        auto tmpPaket = pool.request();
        tmpTemplatePacket->copyInto(*tmpPaket);
        storeTime(*tmpPaket);

        dummyDevice.externalIntoNorthbound(std::move(tmpPaket));
        //Add additional stack for multihoming
        while(!dummyDevice.availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        dummyDevice.externalOutOfNorthbound();
        Logger::Log(Logger::INFO, "Connection Established");

        if(additionalStack != StackEnum::Invalid)
        {
            auto flowId = FlowIdGenerator::createFlowId(TransportProtocolEnum::UDP, tmpDestination, 10000, 11000);
            /*auto res = */virtualStack.getRemoteControl().createStack(flowId, additionalStack, connectOverIp).get().message.get();
//            Logger::Log(Logger::DEBUG, "AdditionalStack: ", StackEnumHelper::toString(additionalStack), ", overIp: ", connectOverIp, ", flowId: ", flowId, ", res: ", ConfigurationStateWrapper::toString(res));
        }

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        StopWatch tim;
        while(!tim.stop().hasElapsed(timeInSecondsHighRes))
        {
            if(dummyDevice.canSend() && pool.canRequest() && bwLimiter.canSend())
            {
                bwLimiter.hasSent();

                tmpPaket = pool.request();
                tmpTemplatePacket->copyInto(*tmpPaket);
                tmpPaket->setSize(packetSize);

                storeTime(*tmpPaket);

                dummyDevice.externalIntoNorthbound(std::move(tmpPaket));
            }
        }

        virtualStack.stop();
    }

	static std::vector<std::unique_ptr<LatencyResult>> receiver(const size_t timeInSeconds, VirtualStack& virtualStack, DummyNorthboundDevice& dummyDevice)
	{
        Logger::Log(Logger::INFO, "Waiting for first packet");

        std::vector<std::vector<std::pair<uint8_t, int64_t>>> packetDelayList{};
        packetDelayList.reserve(10);

		while(!dummyDevice.availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

		auto connectionPacketRecvTime = StopWatch::getHighResolutionTime();
        auto connectionPacket = dummyDevice.externalOutOfNorthbound();
        auto connectionPacketSendTime = readTime(*connectionPacket);
        const size_t connectionEstablishment = static_cast<size_t>(connectionPacketRecvTime - connectionPacketSendTime);
        const size_t packetSize = connectionPacket->size();

        Logger::Log(Logger::INFO, "Received first packet");

        //swap sender/receiver?
        ReverseSourceDest::process(connectionPacket);
		dummyDevice.externalIntoNorthbound(std::move(connectionPacket));
        Logger::Log(Logger::INFO, "Sent first packet back");
        Logger::Log(Logger::INFO, "Connection Established");

        auto timeInSecondsHighRes = std::chrono::seconds(timeInSeconds);

        StopWatch tim;
		bool waitForFirstData = true;
        while(waitForFirstData || !tim.stop().hasElapsed(timeInSecondsHighRes))
		{
            if(dummyDevice.availableExternal())
            {
                //start runtime after first packet recevied
                if(waitForFirstData)
                {
                    waitForFirstData = false;
                    tim.start();
                }

                auto recvPacket = dummyDevice.externalOutOfNorthbound();
                //add
                
                auto currentTime = StopWatch::getHighResolutionTime();

                auto timeCount = getTimeCount(*recvPacket);

                if(packetDelayList.size() < timeCount)
                {
                    for (size_t i = packetDelayList.size(); i < timeCount; ++i)
                    {
                        packetDelayList.emplace_back();
                        packetDelayList[i].reserve(50000000);
                    }
                }

                long sendTime = 0;
                {
                    auto stackIndex = readStackIndex(*recvPacket, 0);
                    auto timeSnapshot = readTime(*recvPacket, 0);
                    auto delay = currentTime - timeSnapshot;
                    packetDelayList[0].push_back(std::make_pair(stackIndex, delay));
                    sendTime = timeSnapshot;
                }

                for (size_t i = 1; i < timeCount; ++i)
                {
                    auto stackIndex = readStackIndex(*recvPacket, i);
                    auto timeSnapshot = readTime(*recvPacket, i);
                    auto delay =  timeSnapshot - sendTime;
                    packetDelayList[i].push_back(std::make_pair(stackIndex, delay));
                }
            }
		};

        virtualStack.stop();

        std::vector<std::unique_ptr<LatencyResult>> analyseResultList{};
        analyseResultList.reserve(packetDelayList.size());

        //transform lists?

        for (size_t j = 0; j < packetDelayList.size(); ++j)
        {
            std::map<uint8_t, std::vector<int64_t>> delayListMap{};
            for (auto& val : packetDelayList[j])
            {
                auto keyIt = delayListMap.find(val.first);
                if(keyIt == delayListMap.end())
                {
                    auto insertIt = delayListMap.emplace(val.first, std::vector<int64_t>{});
                    keyIt = insertIt.first;
                    keyIt->second.reserve(packetDelayList[j].size());
                }
                keyIt->second.push_back(val.second);
            }

            for(auto& val : delayListMap)
            {
                std::string name =  "runner_" + std::to_string(j) + "_" + std::to_string(val.first);
                auto analyseResult = LatencyResult::analyse(j,name , connectionEstablishment, timeInSeconds, packetSize,
                                                            std::move(val.second));
                analyseResultList.emplace_back(std::move(analyseResult));
            }
        }
        return analyseResultList;
	}
};

#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <common/Helper/PacketFactory.h>
#include <gtest/gtest.h>
#include <southbound/SouthboundControl.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/device/northboundDevices/LoopbackNorthboundDevice.h>
#include <virtualStack/stackEngine/endpoints/UdpEndpoint.h>
#include <virtualStack/fastInspection/FlowIdGenerator.h>
#include <virtualStack/fastInspection/Classifier.h>
#include <algorithm>

namespace
{
    inline void waitForStackResult(const StackCreationHandler& stackCreationHandler)
    {
        while(!stackCreationHandler.hasNewStacks())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    template <typename T>
    inline std::pair<std::vector<T>, std::vector<T>> splitVector(std::vector<T>& list, std::function<bool(const T&)> predicate)
    {
        std::vector<T> first{};
        std::vector<T> second{};
        for(auto& el : list)
        {
            if (predicate(el))
                first.push_back(std::move(el));
            else
                second.push_back(std::move(el));
        }
        return std::make_pair<std::vector<T>, std::vector<T>>(std::move(first), std::move(second));
    };

    void sendOnePacket(VirtualStackLoader<DummyNorthboundDevice> &sender, const sockaddr_storage &senderIp,
                       VirtualStackLoader<DummyNorthboundDevice> &recv, const sockaddr_storage &recvIp)
    {
        //s1-    -s3
        //  }-r1-|
        //s2-    -s4
        const size_t data = 0x111F1111;
        auto sendStorage = PacketFactory::createUdpPacket(sender.northboundDevice->getPool(), senderIp, recvIp);
        size_t sendStorageHeaderSize = sendStorage->size();
        sendStorage->appendDataAutomaticAfterEnd(&data);

        sender.northboundDevice->externalIntoNorthbound(std::move(sendStorage));

        while (!recv.northboundDevice->availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto recvStorage = recv.northboundDevice->externalOutOfNorthbound();
        auto recvData = recvStorage->toTypeAutomatic<size_t>(sendStorageHeaderSize);
        ASSERT_EQ(data, recvData);
    }

    TEST(Southbound, RoutingMatrixCollision)
    {
        for (size_t i = 0; i < 20; ++i)
        {
            auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);

            settingsStream.AddBool("IsRouter", false);
            settingsStream.AddStringVector("RoutingTable", {"127.0.0.0", "127.0.1.1", "127.0.1.0", "127.0.1.1"});
            settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.1"});
            settingsStream.AddString("ManagementBindAddress", "127.0.0.1");
            auto s1Settings = DefaultVirtualStackSettings::Default(settingsStream);

            settingsStream.AddBool("IsRouter", false);
            settingsStream.AddStringVector("RoutingTable", {"127.0.0.0", "127.0.1.1", "127.0.1.0", "127.0.1.1"});
            settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.2"});
            settingsStream.AddString("ManagementBindAddress", "127.0.0.2");
            auto s2Settings = DefaultVirtualStackSettings::Default(settingsStream);

            settingsStream.AddBool("IsRouter", true);
            settingsStream.AddStringVector("RoutingTable", {});
            settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.1.1"});
            settingsStream.AddString("ManagementBindAddress", "127.0.1.1");
            auto r1Settings = DefaultVirtualStackSettings::Default(settingsStream);

            settingsStream.AddBool("IsRouter", false);
            settingsStream.AddStringVector("RoutingTable", {"127.0.1.0", "127.0.1.1", "127.0.0.0", "127.0.1.1"});
            settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.1.3"});
            settingsStream.AddString("ManagementBindAddress", "127.0.1.3");
            auto s3Settings = DefaultVirtualStackSettings::Default(settingsStream);

            SouthboundCallbacks callbacks{
                    [](std::unique_ptr<NewStackResult> result)
                    {
                        if (!result->socket)
                            Logger::Log(Logger::WARNING, "FAILED Request: ", result->socket.get(), ", with Stack: ",
                                        StackEnumHelper::toString(result->request->stack));
                    },
                    [](std::unique_ptr<NewStackResult> result)
                    {
                        if (!result->socket)
                            Logger::Log(Logger::WARNING, "FAILED Respond: ", result->socket.get(), ", with Stack: ",
                                        StackEnumHelper::toString(result->request->stack));
                    }
            };

            auto vsObjectFactory = VirtualStackLoader<DummyNorthboundDevice>::createVSObjectFactory(*s1Settings);
            auto packetPool = vsObjectFactory->getStorageSendPool(16, "PacketPool");

            StackCreationHandler s1StackCreationHandler{*vsObjectFactory, *s1Settings};
            StackCreationHandler s2StackCreationHandler{*vsObjectFactory, *s2Settings};
            StackCreationHandler s3StackCreationHandler{*vsObjectFactory, *s3Settings};
            StackCreationHandler r1StackCreationHandler{*vsObjectFactory, *r1Settings};

            s1StackCreationHandler.start();
            s2StackCreationHandler.start();
            s3StackCreationHandler.start();
            r1StackCreationHandler.start();

            const sockaddr_storage s1Ip = NetworkExtensions::getIPv4SockAddr(s1Settings->getDefaultIPv4(), 11000);
            const sockaddr_storage s2Ip = NetworkExtensions::getIPv4SockAddr(s2Settings->getDefaultIPv4(), 11000);
            const sockaddr_storage s3Ip = NetworkExtensions::getIPv4SockAddr(s3Settings->getDefaultIPv4(), 11000);

            auto s1Packet = PacketFactory::createUdpPacket(*packetPool, s1Ip, s3Ip);
            auto s2Packet = PacketFactory::createUdpPacket(*packetPool, s2Ip, s3Ip);

            auto s1InspectionStruct = Classifier::process(s1Packet);
            auto s2InspectionStruct = Classifier::process(s2Packet);

            FlowIdGenerator::process(s1Packet);
            FlowIdGenerator::process(s2Packet);

            ASSERT_EQ(s1InspectionStruct->flowId, s2InspectionStruct->flowId);

            s1StackCreationHandler.add(s1InspectionStruct->flowId, std::move(s1Packet));
            waitForStackResult(s1StackCreationHandler);
            auto s1StackResult = s1StackCreationHandler.getNextNewStack();
            ASSERT_FALSE(s1StackCreationHandler.hasNewStacks());

            s2StackCreationHandler.add(s2InspectionStruct->flowId, std::move(s2Packet));
            waitForStackResult(s2StackCreationHandler);
            auto s2StackResult = s2StackCreationHandler.getNextNewStack();
            ASSERT_FALSE(s2StackCreationHandler.hasNewStacks());

            //flowid to s3 is equal for clients
            ASSERT_EQ(s1StackResult->flowId, s2StackResult->flowId);

            ASSERT_EQ(s1StackResult->partnerFlowId, FlowIdGenerator::createFlowId(TransportProtocolEnum::UDP, s1Ip, 11000, 11000));
            ASSERT_EQ(s2StackResult->partnerFlowId, FlowIdGenerator::createFlowId(TransportProtocolEnum::UDP, s2Ip, 11000, 11000));

            std::vector<std::unique_ptr<StackCreationResult>> r1StackResults{};
            waitForStackResult(r1StackCreationHandler);
            r1StackResults.push_back(r1StackCreationHandler.getNextNewStack());
            waitForStackResult(r1StackCreationHandler);
            r1StackResults.push_back(r1StackCreationHandler.getNextNewStack());
            waitForStackResult(r1StackCreationHandler);
            r1StackResults.push_back(r1StackCreationHandler.getNextNewStack());
            waitForStackResult(r1StackCreationHandler);
            r1StackResults.push_back(r1StackCreationHandler.getNextNewStack());
            ASSERT_FALSE(r1StackCreationHandler.hasNewStacks());

            decltype(r1StackResults) r1RequestStackResults;
            decltype(r1StackResults) r1RespondStackResults;
            std::tie(r1RequestStackResults, r1RespondStackResults) = splitVector<std::unique_ptr<StackCreationResult>>(
                    r1StackResults, [](const std::unique_ptr<StackCreationResult> &el) { return el->byRequest; });

            //check if splitVector was successful
            ASSERT_EQ(r1RequestStackResults.size(), 2);
            ASSERT_EQ(r1RespondStackResults.size(), 2);

            //check connections from s1->s3 and s2->s3 have their own flowid
            ASSERT_NE(r1RequestStackResults[0]->flowId, r1RequestStackResults[1]->flowId);
            ASSERT_NE(r1RequestStackResults[0]->partnerFlowId, r1RequestStackResults[1]->partnerFlowId);

            waitForStackResult(s3StackCreationHandler);
            auto s3StackResult_1 = s3StackCreationHandler.getNextNewStack();
            waitForStackResult(s3StackCreationHandler);
            auto s3StackResult_2 = s3StackCreationHandler.getNextNewStack();
            ASSERT_FALSE(s3StackCreationHandler.hasNewStacks());

            bool s3_1_IsFromS1 = NetworkExtensions::getAddress(s3StackResult_1->newStackResult->request->origin) ==
                                 s1Settings->getDefaultIPv4();
            auto &s3FromS1 = s3_1_IsFromS1 ? s3StackResult_1 : s3StackResult_2;
            auto &s3FromS2 = !s3_1_IsFromS1 ? s3StackResult_1 : s3StackResult_2;

            ASSERT_NE(s3FromS1->flowId, s3FromS2->flowId);
            ASSERT_EQ(s3FromS1->partnerFlowId, s3FromS2->partnerFlowId);
        }
    }
}
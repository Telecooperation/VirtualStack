
#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <common/Helper/PacketFactory.h>
#include <gtest/gtest.h>
#include <southbound/SouthboundControl.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/device/northboundDevices/LoopbackNorthboundDevice.h>
#include <virtualStack/stackEngine/endpoints/UdpEndpoint.h>

namespace
{
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

    TEST(Southbound, RoutingMatrix)
    {
        auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);

        for (size_t i = 0; i < 1; ++i)
        {
//        Logger::Log(Logger::DEBUG, "Southbound::Routing Iteration: ", i);

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

            settingsStream.AddBool("IsRouter", false);
            settingsStream.AddStringVector("RoutingTable", {"127.0.1.0", "127.0.1.1", "127.0.0.0", "127.0.1.1"});
            settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.1.4"});
            settingsStream.AddString("ManagementBindAddress", "127.0.1.4");
            auto s4Settings = DefaultVirtualStackSettings::Default(settingsStream);


//    Logger::Log(Logger::DEBUG, "Send Paket from: ", s1Settings->getDefaultSendIPv4(), " over: ",
//                r1Settings->getDefaultSendIPv4(), " to: ",
//                s2Settings->getDefaultSendIPv4());

            const sockaddr_storage s1Ip = NetworkExtensions::getIPv4SockAddr(s1Settings->getDefaultIPv4(), 11000);
            const sockaddr_storage s2Ip = NetworkExtensions::getIPv4SockAddr(s2Settings->getDefaultIPv4(), 12000);
            const sockaddr_storage s3Ip = NetworkExtensions::getIPv4SockAddr(s3Settings->getDefaultIPv4(), 13000);
            const sockaddr_storage s4Ip = NetworkExtensions::getIPv4SockAddr(s4Settings->getDefaultIPv4(), 14000);

            VirtualStackLoader<DummyNorthboundDevice> s1{};
            VirtualStackLoader<DummyNorthboundDevice> s2{};
            VirtualStackLoader<DummyNorthboundDevice> s3{};
            VirtualStackLoader<DummyNorthboundDevice> s4{};
            VirtualStackLoader<LoopbackNorthboundDevice> r1{};

            s1.Initialize(std::move(s1Settings));
            s2.Initialize(std::move(s2Settings));
            s3.Initialize(std::move(s3Settings));
            s4.Initialize(std::move(s4Settings));
            r1.Initialize(std::move(r1Settings));

            sendOnePacket(s1, s1Ip, s3, s3Ip);
            sendOnePacket(s1, s1Ip, s2, s2Ip);
            sendOnePacket(s1, s1Ip, s4, s4Ip);
            sendOnePacket(s2, s2Ip, s4, s4Ip);
            sendOnePacket(s3, s3Ip, s2, s2Ip);
        }
    }
}
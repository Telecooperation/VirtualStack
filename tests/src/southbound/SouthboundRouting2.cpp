#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <gtest/gtest.h>
#include <southbound/SouthboundControl.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/stackEngine/endpoints/UdpEndpoint.h>
#include <virtualStack/device/northboundDevices/LoopbackNorthboundDevice.h>
#include <common/Helper/PacketFactory.h>

TEST(Southbound, Routing2)
{
    auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);

    for (size_t i = 0; i < 1; ++i)
    {
        //s1--r1--r2--s2
//        Logger::Log(Logger::DEBUG, "Southbound::Routing2 Iteration: ", i);
        settingsStream.AddBool("IsRouter", false);
        settingsStream.AddStringVector("RoutingTable", {"127.0.0.0", "127.0.0.1", "127.0.1.0", "127.0.0.1"});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.2"});
        settingsStream.AddString("ManagementBindAddress", "127.0.0.2");
        auto s1Settings = DefaultVirtualStackSettings::Default(settingsStream);

        settingsStream.AddBool("IsRouter", true);
        settingsStream.AddStringVector("RoutingTable", {"127.0.1.0", "127.0.1.1"});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.1"});
        settingsStream.AddString("ManagementBindAddress", "127.0.0.1");
        auto r1Settings = DefaultVirtualStackSettings::Default(settingsStream);

        settingsStream.AddBool("IsRouter", true);
        settingsStream.AddStringVector("RoutingTable", {"127.0.0.0", "127.0.0.1"});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.1.1"});
        settingsStream.AddString("ManagementBindAddress", "127.0.1.1");
        auto r2Settings = DefaultVirtualStackSettings::Default(settingsStream);

        settingsStream.AddBool("IsRouter", false);
        settingsStream.AddStringVector("RoutingTable", {"127.0.1.0", "127.0.1.1", "127.0.0.0", "127.0.1.1"});
        settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.1.2"});
        settingsStream.AddString("ManagementBindAddress", "127.0.1.2");
        auto s2Settings = DefaultVirtualStackSettings::Default(settingsStream);

//    Logger::Log(Logger::DEBUG, "Send Paket from: ", s1Settings->getDefaultSendIPv4(), " over: ",
//                r1Settings->getDefaultSendIPv4(), "and", r2Settings->getDefaultSendIPv4(), " to: ",
//                s2Settings->getDefaultSendIPv4());

        const sockaddr_storage source = NetworkExtensions::getIPv4SockAddr(s1Settings->getDefaultIPv4(), 11000);
        const sockaddr_storage destination = NetworkExtensions::getIPv4SockAddr(s2Settings->getDefaultIPv4(), 12000);

        VirtualStackLoader<DummyNorthboundDevice> s1{};
        VirtualStackLoader<DummyNorthboundDevice> s2{};
        VirtualStackLoader<LoopbackNorthboundDevice> r1{};
        VirtualStackLoader<LoopbackNorthboundDevice> r2{};

        s1.Initialize(std::move(s1Settings));
        s2.Initialize(std::move(s2Settings));
        r1.Initialize(std::move(r1Settings));
        r2.Initialize(std::move(r2Settings));

        const size_t data = 0x11111111;
        auto s1Storage = PacketFactory::createUdpPacket(s1.northboundDevice->getPool(), source, destination);
        size_t s1StorageHeaderSize = s1Storage->size();
        s1Storage->appendDataAutomaticAfterEnd(&data);

        auto s2Storage = PacketFactory::createUdpPacket(s2.northboundDevice->getPool(), destination, source);
        size_t s2StorageHeaderSize = s2Storage->size();
        s2Storage->appendDataAutomaticAfterEnd(&data);


        //send
        s1.northboundDevice->externalIntoNorthbound(std::move(s1Storage));

        while (!s2.northboundDevice->availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto s2RecvStorage = s2.northboundDevice->externalOutOfNorthbound();
        auto s2RecvData = s2RecvStorage->toTypeAutomatic<size_t>(s1StorageHeaderSize);
        ASSERT_EQ(data, s2RecvData);

        //Create same data and send it back to s1 to test that existing StackEngines and Stacks are used

        s2.northboundDevice->externalIntoNorthbound(std::move(s2Storage));

        while (!s1.northboundDevice->availableExternal())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto s1RecvStorage = s1.northboundDevice->externalOutOfNorthbound();
        auto s1RecvData = s1RecvStorage->toTypeAutomatic<size_t>(s2StorageHeaderSize);
        ASSERT_EQ(data, s1RecvData);
    }
}

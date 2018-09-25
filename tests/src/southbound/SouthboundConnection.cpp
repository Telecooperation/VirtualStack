#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <gtest/gtest.h>
#include <southbound/SouthboundControl.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/fastInspection/FlowIdGenerator.h>
#include <virtualStack/stackEngine/endpoints/UdpEndpoint.h>

TEST(Southbound, ConnectionTest) {
    auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);

    settingsStream.AddString("SouthboundInterfaceIPv4Address", "127.0.0.1");
    settingsStream.AddString("ManagementBindAddress", "127.0.0.1");
    auto virtualStackSettingsS1 = DefaultVirtualStackSettings::Default(settingsStream);

    settingsStream.AddString("SouthboundInterfaceIPv4Address", "127.0.0.2");
    settingsStream.AddString("ManagementBindAddress", "127.0.0.2");
    auto virtualStackSettingsS2 = DefaultVirtualStackSettings::Default(settingsStream);
    if(virtualStackSettingsS1->SettingsReadFailed() || virtualStackSettingsS2->SettingsReadFailed())
        return;

    for (int i = 0; i < 10; ++i)
    {
        auto s1VSObjectFactory = VirtualStackLoader<DummyNorthboundDevice>::createVSObjectFactory(*virtualStackSettingsS1);
        auto s2VSObjectFactory = VirtualStackLoader<DummyNorthboundDevice>::createVSObjectFactory(*virtualStackSettingsS2);

        std::atomic<size_t> openRequests{0};
        std::atomic<size_t> openResponds{0};

        SouthboundCallbacks callbacks{
                [&openRequests](std::unique_ptr<NewStackResult> result) {
                    if(!result->socket)
                        Logger::Log(Logger::WARNING, "FAILED Request: ", result->socket.get(), ", with Stack: ", StackEnumHelper::toString(result->request->stack));
                    --openRequests;
                },
                [&openResponds](std::unique_ptr<NewStackResult> result) {
                    if(!result->socket)
                        Logger::Log(Logger::WARNING, "FAILED Respond: ", result->socket.get(), ", with Stack: ", StackEnumHelper::toString(result->request->stack));
                    --openResponds;
                }
        };

        SouthboundControl s1{callbacks, *s1VSObjectFactory, *virtualStackSettingsS1};
        SouthboundControl s2{callbacks, *s2VSObjectFactory, *virtualStackSettingsS2};

        s1.start();
        s2.start();

        const uint16_t s1Port = 11000u;
        const uint16_t s2Port = 12000u;
        auto s1Source = NetworkExtensions::getIPv4SockAddr(virtualStackSettingsS1->getDefaultIPv4(), s1Port);
        auto s2Source = NetworkExtensions::getIPv4SockAddr(virtualStackSettingsS2->getDefaultIPv4(), s2Port);

        s1.requestCreateStack(s2Source, SouthboundControl::createNewStackRequest(s2Source, s1Port, s2Port, TransportProtocolEnum::UDP, StackEnum::TCPIPv4));
        openRequests = openResponds += 1;
        s2.requestCreateStack(s1Source,  SouthboundControl::createNewStackRequest(s1Source, s2Port, s1Port, TransportProtocolEnum::UDP, StackEnum::TCPIPv4));
        openRequests = openResponds += 1;
        s2.requestCreateStack(s1Source, SouthboundControl::createNewStackRequest( s1Source, s2Port, s1Port, TransportProtocolEnum::UDP, StackEnum::TCPIPv4));
        openRequests = openResponds += 1;
        s2.requestCreateStack(s1Source,  SouthboundControl::createNewStackRequest(s1Source, s2Port, s1Port, TransportProtocolEnum::UDP, StackEnum::UDPIPv4));
        openRequests = openResponds += 1;
        s1.requestCreateStack(s2Source,  SouthboundControl::createNewStackRequest(s2Source, s1Port, s2Port, TransportProtocolEnum::UDP, StackEnum::UDPIPv4));
        openRequests = openResponds += 1;

        //Logger::Log(Logger::DEBUG, "Wait: ", i);
        while (openRequests > 0 || openResponds > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

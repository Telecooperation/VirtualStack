#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <gtest/gtest.h>
#include <southbound/SouthboundControl.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/stackEngine/endpoints/UdpEndpoint.h>

TEST(Southbound, Loopback) {
    auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);

    settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.1"});
    auto virtualStackSettingsS1 = DefaultVirtualStackSettings::Default(settingsStream);

    if(virtualStackSettingsS1->SettingsReadFailed())
        return;

    for (int i = 0; i < 1; ++i)
    {
        auto pool1 = VirtualStackLoader<DummyNorthboundDevice>::createVSObjectFactory(*virtualStackSettingsS1);

        std::atomic<size_t> openRequests{0};
        std::atomic<size_t> openResponds{0};

        SouthboundCallbacks callbacks{
                [&openRequests](std::unique_ptr<NewStackResult> result) {
                    if(!result->socket)
                        Logger::Log(Logger::DEBUG, "FAILED Request: ", result->socket.get(), ", with Stack: ", StackEnumHelper::toString(result->request->stack));
                    --openRequests;
                },
                [&openResponds](std::unique_ptr<NewStackResult> result) {
                    if(!result->socket)
                        Logger::Log(Logger::DEBUG, "FAILED Respond: ", result->socket.get(), ", with Stack: ", StackEnumHelper::toString(result->request->stack));
                    --openResponds;
                }
        };

        SouthboundControl s1{callbacks, *pool1, *virtualStackSettingsS1};

        s1.start();

        const uint16_t s1Port = 11000u;
        const uint16_t s2Port = 12000u;
        auto s2Source = NetworkExtensions::getIPv4SockAddr(virtualStackSettingsS1->getDefaultIPv4(), s2Port);

        s1.requestCreateStack(s2Source,  SouthboundControl::createNewStackRequest(s2Source, s1Port, s2Port, TransportProtocolEnum::UDP, StackEnum::TCPIPv4));
        openRequests = openResponds += 1;
        s1.requestCreateStack(s2Source,  SouthboundControl::createNewStackRequest(s2Source, s1Port, s2Port, TransportProtocolEnum::UDP, StackEnum::TCPIPv4));
        openRequests = openResponds += 1;
        s1.requestCreateStack(s2Source,  SouthboundControl::createNewStackRequest(s2Source, s1Port, s2Port, TransportProtocolEnum::UDP, StackEnum::TCPIPv4));
        openRequests = openResponds += 1;

//        Logger::Log(Logger::DEBUG, "Wait: ", i);
        while(openRequests > 0 || openResponds > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

}

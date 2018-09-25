#pragma once

#include <ns3/csma-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/internet-module.h>
#include <ns3/core-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/tap-bridge-module.h>

namespace tapTest {
    using namespace ns3;

    const std::string simName = "tapTest";
    NS_LOG_COMPONENT_DEFINE (simName);

    void TapTest() {
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
        GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

        CsmaHelper csma;
        csma.SetChannelAttribute ("DataRate", StringValue("100GBps"));
        csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (1)));

        PointToPointHelper p2p;
        InternetStackHelper internet;

        p2p.SetDeviceAttribute("DataRate", StringValue("100GBps"));
        p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(1)));

        NodeContainer tapNodes;
        tapNodes.Create(2);
        NodeContainer innerNodes;
        innerNodes.Create(2);

        Names::Add("tapIn", tapNodes.Get(0));
        Names::Add("tapOut", tapNodes.Get(1));
        Names::Add("innerNodeIn", innerNodes.Get(0));
        Names::Add("innerNodeOut", innerNodes.Get(1));

        NodeContainer csmaIn;
        csmaIn.Add(tapNodes.Get(0));
        csmaIn.Add(innerNodes.Get(0));

        NodeContainer csmaOut;
        csmaOut.Add(innerNodes.Get(1));
        csmaOut.Add(tapNodes.Get(1));

        auto csmaInDev = csma.Install(csmaIn);
        auto innerDev = p2p.Install(innerNodes);
        auto csmaOutDev = csma.Install(csmaOut);

        internet.Install(tapNodes);
        internet.Install(innerNodes);

        Ipv4AddressHelper ipv4AddressHelper;
        ipv4AddressHelper.SetBase("10.0.0.0", "255.255.255.0");
        auto csmaInIp = ipv4AddressHelper.Assign(csmaInDev);

        ipv4AddressHelper.SetBase("10.0.1.0", "255.255.255.0");
        auto innerIp = ipv4AddressHelper.Assign(innerDev);

        ipv4AddressHelper.SetBase("10.0.2.0", "255.255.255.0");
        auto cmsaOutIp = ipv4AddressHelper.Assign(csmaOutDev);

        std::cout << "tapIn: " << csmaInIp.GetAddress(0) << std::endl;
        std::cout << "tapOut: " << cmsaOutIp.GetAddress(1) << std::endl;

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();

        TapBridgeHelper tapBridge;
        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue("left"));
        tapBridge.Install(tapNodes.Get(0), csmaInDev.Get(0));

        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue("right"));
        tapBridge.Install(tapNodes.Get(1), csmaOutDev.Get(1));

//        csma.EnablePcapAll("sim/csma_" + simName, false);
//        p2p.EnablePcapAll("sim/p2p_" + simName, false);


        Simulator::Stop(Seconds(6000));
        Simulator::Run();

        Simulator::Destroy();

        /*
         * #!/bin/bash

ip link set dev left netns left
ip netns exec left ip link set left up
ip netns exec left ip addr add 10.0.0.1/24 dev left
ip netns exec left ip route add 10.0.1.0/24 via 10.0.0.2
ip netns exec left ip route add 10.0.2.0/24 via 10.0.0.2

ip link set dev right netns right
ip netns exec right ip link set right up
ip netns exec right ip addr add 10.0.2.2/24 dev right
ip netns exec right ip route add 10.0.1.0/24 via 10.0.2.1
ip netns exec right ip route add 10.0.0.0/24 via 10.0.2.1


         */
    }
}
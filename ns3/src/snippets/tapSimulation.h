#pragma once

#include <ns3/tap-bridge-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3Helper.h"

namespace tapSimulation {
    using namespace ns3;
    using namespace ns3Helper;

    const std::string simName = "tapSimulation";
    NS_LOG_COMPONENT_DEFINE (simName);

    void TapSimulation() {
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
        GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

        NodeContainer leftCsma, rightCsma;
        leftCsma.Create(2);
        rightCsma.Create(2);

        Names::Add("tapIn", leftCsma.Get(0));
        Names::Add("tapInCsma", leftCsma.Get(1));
        Names::Add("tapOutCsma", rightCsma.Get(0));
        Names::Add("tapOut", rightCsma.Get(1));

        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

        auto leftDevices = csma.Install(leftCsma);
        auto rightDevices = csma.Install(rightCsma);

        //PointToPointHelper p2p;
        NodeContainer inner;
        inner.Add(leftCsma.Get(1));
        inner.Add(rightCsma.Get(0));

        auto csmaBridge = csma.Install(inner);


        InternetStackHelper stack;
        stack.Install(leftCsma);
        stack.Install(rightCsma);

        //tapIn -> csma -> p2p -> p2p -> csma -> tapOut

        Ipv4AddressHelper addresses;
        addresses.SetBase("10.1.1.0", "255.255.255.0");
        auto leftInterface = addresses.Assign(leftDevices);

        addresses.SetBase("10.1.2.0", "255.255.255.0");
        auto csmaToP2pInterface = addresses.Assign(csmaBridge);

        addresses.SetBase("10.1.5.0", "255.255.255.0");
        auto rightInterface = addresses.Assign(rightDevices);

        std::cout << "tapIn: " << leftInterface.GetAddress(0) << std::endl;
        std::cout << "tapOut: " << rightInterface.GetAddress(1) << std::endl;

        std::cout << "2.1: " << csmaBridge.Get(1)->GetAddress() << std::endl;
        std::cout << "2.2: " << csmaBridge.Get(0)->GetAddress() << std::endl;


        TapBridgeHelper tapBridge;
//        tapBridge.SetAttribute("Gateway", Ipv4AddressValue(leftInterface.GetAddress(1)));
        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue("tapIn"));
        tapBridge.Install(leftCsma.Get(0), leftDevices.Get(0));

//        tapBridge.SetAttribute("Gateway", Ipv4AddressValue(rightInterface.GetAddress(0)));
        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue("tapOut"));
        tapBridge.Install(rightCsma.Get(1), rightDevices.Get(1));

        csma.EnablePcapAll("sim/csma_" + simName, false);
        //p2p.EnablePcapAll("sim/p2p_" + simName, false);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
            LinuxStackHelper::PopulateRoutingTables();
        ns3Helper::NS3Helper::printGlobalRoutes("sim/" + simName);

        FlowMonitorHelper flowmonHelper;
        flowmonHelper.InstallAll();

//        NS_LOG_INFO ("Create Applications.");
//        uint32_t packetSize = 1024;
//        Time interPacketInterval = Seconds (1.0);
//        V4PingHelper ping ("10.1.5.2");
//
//        ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
//        ping.SetAttribute ("Size", UintegerValue (packetSize));
//        if (true)
//        {
//            ping.SetAttribute ("Verbose", BooleanValue (true));
//        }
//        ApplicationContainer apps = ping.Install (leftCsma.Get(1));
//        apps.Start (Seconds (1.0));
//        apps.Stop (Seconds (200.0));

        Simulator::Stop(Seconds(6000));
        Simulator::Run();

        flowmonHelper.SerializeToXmlFile("sim/" + simName + ".flowmon", false, false);

        Simulator::Destroy();
    }
}
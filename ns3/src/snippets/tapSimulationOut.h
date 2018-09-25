#pragma once

#include <ns3/internet-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/core-module.h>
#include <ns3/applications-module.h>
#include <ns3/tap-bridge-module.h>
#include <ns3/log.h>
#include "ns3Helper.h"

namespace tapSimulationOut
{
    using namespace ns3;
    using namespace ns3Helper;

    const std::string simName = "tapSimulationOut";
    NS_LOG_COMPONENT_DEFINE (simName);

    void TapSimulationOut()
    {
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
        GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

        NodeContainer rightCsma;
        rightCsma.Create(2);

        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

        auto rightDevices = csma.Install(rightCsma);

        InternetStackHelper stack;
        stack.Install(rightCsma);

        Ipv4AddressHelper addresses;
        addresses.SetBase("10.1.1.0", "255.255.255.0");
        auto rightInterface = addresses.Assign(rightDevices);

        std::cout << "inner: " << rightInterface.GetAddress(0) << std::endl;
        std::cout << "tapOut: " << rightInterface.GetAddress(1) << std::endl;


        TapBridgeHelper tapBridge;
//        tapBridge.SetAttribute("Gateway", Ipv4AddressValue(leftInterface.GetAddress(1)));
        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue("tapOut"));
        tapBridge.Install(rightCsma.Get(1), rightDevices.Get(1));

        csma.EnablePcapAll("sim/csma_" + simName, false);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
        ns3Helper::NS3Helper::printGlobalRoutes("sim/" + simName);

        FlowMonitorHelper flowmonHelper;
        flowmonHelper.InstallAll();

        NS_LOG_INFO ("Create Applications.");
        uint32_t packetSize = 1024;
        Time interPacketInterval = Seconds (1.0);
        V4PingHelper ping ("10.1.1.2");

        ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
        ping.SetAttribute ("Size", UintegerValue (packetSize));
        if (true)
        {
            ping.SetAttribute ("Verbose", BooleanValue (true));
        }
        ApplicationContainer apps = ping.Install (rightCsma.Get(0));
        apps.Start (Seconds (1.0));
        apps.Stop (Seconds (5.0));


        Simulator::Stop(Seconds(600.));
        Simulator::Run();

        flowmonHelper.SerializeToXmlFile("sim/" + simName + ".flowmon", false, false);

        Simulator::Destroy();
    }
}
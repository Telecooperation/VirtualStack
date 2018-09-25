//
// Network topology
//
//       n2----n3
//      /        \ 5 Mb/s, 2ms
//     /          \          1.5Mb/s, 10ms
//   n0           n4-------------------------n5--n1
//     \         /
//      \     / 5 Mb/s, 2ms
//     lteGateway

#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

#include "ns3Helper.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wimax-module.h"
#include "ns3/csma-module.h"
#include "ns3/uan-module.h"
#include "ns3/wave-module.h"

using namespace ns3;
using namespace ns3Helper;


namespace tcpBaselineLTE_DCE {

    const std::string simName = "tcpBaselineLteDCE";
    NS_LOG_COMPONENT_DEFINE (simName);



    AnimationInterface
    dumpNetAnim(NodeContainer &nodes, Ptr<Node> lteGateway, std::string simName,
                NodeContainer &enbNodes) {
        BaseStationNetDevice b;
        SubscriberStationNetDevice s;
        CsmaNetDevice c;
        UanNetDevice u;
        WaveNetDevice tt;

        MobilityHelper mobilityNetAnim;

        mobilityNetAnim.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobilityNetAnim.Install(nodes);
        mobilityNetAnim.Install(enbNodes);
        mobilityNetAnim.Install(lteGateway);


        AnimationInterface anim(simName + ".xml");
        anim.EnablePacketMetadata(true);

        anim.SetConstantPosition(nodes.Get(0), 0, 50);
        anim.UpdateNodeDescription(nodes.Get(0), "Client");
        anim.UpdateNodeColor(nodes.Get(0), 0x0, 0x0, 0xFF);

        anim.SetConstantPosition(nodes.Get(2), 25, 25);
        anim.UpdateNodeDescription(nodes.Get(2), "Wifi-Router");
        anim.UpdateNodeColor(nodes.Get(2), 0xFF, 0xFF, 0x0);

        anim.SetConstantPosition(nodes.Get(3), 50, 25);
        anim.UpdateNodeDescription(nodes.Get(3), "Router");

        anim.SetConstantPosition(nodes.Get(4), 75, 50);
        anim.UpdateNodeDescription(nodes.Get(4), "Router");

        anim.SetConstantPosition(nodes.Get(5), 100, 50);
        anim.UpdateNodeDescription(nodes.Get(5), "Router");

        anim.SetConstantPosition(nodes.Get(1), 125, 50);
        anim.UpdateNodeDescription(nodes.Get(1), "Server");
        anim.UpdateNodeColor(nodes.Get(1), 0x00, 0xFF, 0x00);

        anim.SetConstantPosition(enbNodes.Get(0), 25, 75);
        anim.UpdateNodeDescription(enbNodes.Get(0), "LTE-ENB");
        anim.UpdateNodeColor(enbNodes.Get(0), 0xAA, 0xAA, 0xAA);

        anim.SetConstantPosition(lteGateway, 50, 75);
        anim.UpdateNodeDescription(lteGateway, "LTE-Gateway");

        return anim;
    }

    void tcpBaselineLteDce() {
        // Users may find it convenient to turn on explicit debugging
        // for selected modules; the below lines suggest how to do this
        LogComponentEnable(simName.c_str(), LOG_LEVEL_INFO);

        NS3Helper::setGlobalDataRateAndDelay("1Gbps", "1ms");
        //GlobalRoutingTable gets updated on ifDown
        Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

        //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

        // Allow the user to override any of the defaults and the above
        // DefaultValue::Bind ()s at run-time, via command-line arguments

        DceManagerHelper dceManager;
        dceManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
                                   "Library", StringValue("liblinux.so"));
        dceManager.SetTaskManagerAttribute("FiberManagerType",
                                           StringValue("UcontextFiberManager"));

        const std::string socketType = LinuxTcpSocketFactory::GetTypeId().GetName();

        LinuxStackHelper stack;
        PointToPointHelper p2p;

        NS_LOG_INFO ("Create network nodes and install InternetStack");
        NodeContainer nodes;
        nodes.Create(6);

        dceManager.Install(nodes);
        stack.Install(nodes);

        NS_LOG_INFO ("Create p2p-channels");
        auto ueDeviceNode = nodes.Get(0);
        NetDeviceContainer d0d2 = p2p.Install(nodes.Get(0), nodes.Get(2));
        NetDeviceContainer d2d3 = p2p.Install(nodes.Get(2), nodes.Get(3));
        NetDeviceContainer d3d4 = p2p.Install(nodes.Get(3), nodes.Get(4));
        NetDeviceContainer d4d5 = p2p.Install(nodes.Get(4), nodes.Get(5));
        NetDeviceContainer d5d1 = p2p.Install(nodes.Get(5), nodes.Get(1));

        // Later, we add IP addresses.
        NS_LOG_INFO ("Assign IP Addresses.");
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.0.1.0", "255.255.255.0");
        //Ipv4InterfaceContainer i0i2 = ipv4.Assign(d0d2);
        ipv4.SetBase("10.0.3.0", "255.255.255.0");
        Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);
        ipv4.SetBase("10.0.4.0", "255.255.255.0");
        Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);
        ipv4.SetBase("10.0.5.0", "255.255.255.0");
        Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
        ipv4.SetBase("10.0.6.0", "255.255.255.0");
        Ipv4InterfaceContainer i5i1 = ipv4.Assign(d5d1);

        //std::cout << "i0i2: " << i0i2.GetAddress(0) << ", " << i0i2.GetAddress(1) << std::endl;
        std::cout << "i2i3: " << i2i3.GetAddress(0) << ", " << i2i3.GetAddress(1) << std::endl;
        std::cout << "i3i4: " << i3i4.GetAddress(0) << ", " << i3i4.GetAddress(1) << std::endl;
        std::cout << "i4i5: " << i4i5.GetAddress(0) << ", " << i4i5.GetAddress(1) << std::endl;
        std::cout << "i5i1: " << i5i1.GetAddress(0) << ", " << i5i1.GetAddress(1) << std::endl;

        NS3Helper::setAndPrintGlobalRoutes("sim/" + simName);

        NodeContainer enbNodes;
        Ipv4InterfaceContainer ueDeviceNodeIpIface;
        NetDeviceContainer lteGateway_gatewayPartner;
        Ipv4InterfaceContainer iLteGateway_iGatewayPartner;
        Ptr<PointToPointEpcHelper> epcHelper;

        NS3Helper::addLte(ueDeviceNode, nodes.Get(4), enbNodes, ueDeviceNodeIpIface,
               lteGateway_gatewayPartner, iLteGateway_iGatewayPartner, epcHelper);
        NS3Helper::addLteRoutes(nodes, ueDeviceNode, lteGateway_gatewayPartner);

        auto lteGateway = lteGateway_gatewayPartner.Get(0);

        NS3Helper::printGlobalRoutes("sim/" + simName);

        for (size_t i = 0; i < nodes.GetN(); ++i)
        {
            NS3Helper::copyLocalRoutesIntoIpRoute(nodes.Get(i));
            NS3Helper::copyGlobalRoutesIntoIpRoute(nodes.Get(i));
        }

        NS3Helper::copyLocalRoutesIntoIpRoute(lteGateway->GetNode());
        NS3Helper::copyGlobalRoutesIntoIpRoute(lteGateway->GetNode());

        NS_LOG_INFO ("Create Applications");
        auto transportProtocol = TcpSocketFactory::GetTypeId().GetName();

        // Create the OnOff application to send UDP datagrams of size
        // 210 bytes at a rate of 448 Kb/s

        uint16_t port = 9;   // Discard port (RFC 863)
        OnOffHelper onoff(transportProtocol, Address(InetSocketAddress(i5i1.GetAddress(1), port)));

        onoff.SetAttribute("PacketSize", UintegerValue (1300));
        onoff.SetConstantRate(DataRate("1Gbps"));
        ApplicationContainer apps = onoff.Install(nodes.Get(0));
        apps.Start(Seconds(0.0));
        apps.Stop(Seconds(10.0));

        // Create a packet sink to receive these packets
        PacketSinkHelper sink(transportProtocol, Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
        apps = sink.Install(nodes.Get(1));
        apps.Start(Seconds(0.0));
        apps.Stop(Seconds(10.0));

        //Create a similar flow from n3 to n1, starting at time 1.1 seconds
//        onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(ueDeviceNodeIpIface.GetAddress(0), port)));
//        apps = onoff.Install(nodes.Get(2));
//        apps.Start(Seconds(1.0));
//        apps.Stop(Seconds(5.0));
//
//        //Create a packet sink to receive these packets
//        apps = sink.Install(nodes.Get(0));
//        apps.Start(Seconds(1.0));
//        apps.Stop(Seconds(5.0));


        Names::Add("Client", nodes.Get(0));
        Names::Add("Wifi-Router", nodes.Get(2));
        Names::Add("Router-3", nodes.Get(3));
        Names::Add("Router-4", nodes.Get(4));
        Names::Add("Router-5", nodes.Get(5));
        Names::Add("Server", nodes.Get(1));
        Names::Add("LTE-ENB", enbNodes.Get(0));
        Names::Add("LTE-Gateway", lteGateway);

        AsciiTraceHelper ascii;
        p2p.EnableAsciiAll(ascii.CreateFileStream(simName + ".tr"));
        p2p.EnablePcapAll("sim/" + simName);

        // Flow Monitor
        FlowMonitorHelper flowmonHelper;
        flowmonHelper.InstallAll();

        auto netAnim = dumpNetAnim(nodes, lteGateway->GetNode(), "sim/" + simName, enbNodes);

        NS_LOG_INFO ("Run Simulation.");
        Simulator::Stop(Seconds(11));
        Simulator::Run();
        NS_LOG_INFO ("Done.");

        auto txInMByte = static_cast<double>(apps.Get(0)->GetObject<PacketSink>()->GetTotalRx()) / (1ul << 20);
        std::cout << txInMByte << ", overTime: " << txInMByte / 9.0;

        flowmonHelper.SerializeToXmlFile("sim/" + simName + ".flowmon", false, false);
        Ptr<PacketSink> pktsink;
        pktsink = apps.Get(0)->GetObject<PacketSink>();
        std::cout << "Total Rx: " << pktsink->GetTotalRx() << " bytes" << std::endl;

        Simulator::Destroy();
    }
}

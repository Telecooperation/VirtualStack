/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// Network topology
//
//       n2----n3
//      /        \ 5 Mb/s, 2ms
//     /          \          1.5Mb/s, 10ms
//   n0           n5-------------------------n6--n1
//     \         /
//      \     / 5 Mb/s, 2ms
//        n4
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n3, and from n3 to n1
// - FTP/TCP flow from n0 to n3, starting at time 1.2 to time 1.35 sec.
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues 
// - Tracing of queues and packet receptions to file "simple-global-routing.tr"

#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <ns3/dce-module.h>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3Helper.h"


namespace TCPBaselineDCE
{
    const std::string simName = "TCPBaselineDCE";
    NS_LOG_COMPONENT_DEFINE (simName);
    using namespace ns3;
    using namespace ns3Helper;

    void tcpBaselineDce()
    {
        // Users may find it convenient to turn on explicit debugging
        // for selected modules; the below lines suggest how to do this
        LogComponentEnable(simName.c_str(), LOG_LEVEL_INFO);

        //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

        // Allow the user to override any of the defaults and the above
        // DefaultValue::Bind ()s at run-time, via command-line arguments

        NS_LOG_INFO ("Create nodes.");
        NodeContainer c;
        c.Create(7);
        NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
        NodeContainer n0n4 = NodeContainer(c.Get(0), c.Get(4));
        NodeContainer n2n3 = NodeContainer(c.Get(2), c.Get(3));
        NodeContainer n3n5 = NodeContainer(c.Get(3), c.Get(5));
        NodeContainer n4n5 = NodeContainer(c.Get(4), c.Get(5));
        NodeContainer n5n6 = NodeContainer(c.Get(5), c.Get(6));
        NodeContainer n6n1 = NodeContainer(c.Get(6), c.Get(1));

        DceManagerHelper dceManager;
        dceManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
                                   "Library", StringValue("liblinux.so"));
        dceManager.SetTaskManagerAttribute("FiberManagerType",
                                           StringValue("UcontextFiberManager"));

        const std::string socketType = LinuxTcpSocketFactory::GetTypeId().GetName(); // "ns3::LinuxTcpSocketFactory";
//    dceManager.SetLoader ("ns3::CoojaLoaderFactory");


        dceManager.Install(c);

        LinuxStackHelper stack;
        stack.Install(c);


        //
//       n2----n3
//      /        \ 5 Mb/s, 2ms
//     /          \          1.5Mb/s, 10ms
//   n0           n5-------------------------n6--n1
//     \         /
//      \     / 5 Mb/s, 2ms
//        n4
//

        // We create the channels first without any IP addressing information
        NS_LOG_INFO ("Create channels.");
        PointToPointHelper p2p;

        NetDeviceContainer d0d2 = p2p.Install(n0n2);
        //NetDeviceContainer d0d4 = p2p.Install(n0n4);
        NetDeviceContainer d2d3 = p2p.Install(n2n3);
        NetDeviceContainer d3d5 = p2p.Install(n3n5);
        NetDeviceContainer d4d5 = p2p.Install(n4n5);
        NetDeviceContainer d5d6 = p2p.Install(n5n6);
        NetDeviceContainer d6d1 = p2p.Install(n6n1);


        const float internetLatencyInMS = 50;
        const float hopsInInternet = 2;
        const uint32_t simTime = 15;

        NS3Helper::setDataRateAndDelay(d0d2.Get(0), "1000Mbps", "0.1ms");
        NS3Helper::setDataRateAndDelay(d0d2.Get(1), "1000Mbps", "0.1ms");

        NS3Helper::setDataRateAndDelay(d2d3.Get(0), "1000Mbps", "0.1ms");
        NS3Helper::setDataRateAndDelay(d2d3.Get(1), "1000Mbps", "0.1ms");

        NS3Helper::setDataRateAndDelay(d3d5.Get(0), "1.3Mbps", std::to_string(internetLatencyInMS / hopsInInternet).append("ms"));
        NS3Helper::setDataRateAndDelay(d3d5.Get(1), "15.3Mbps", std::to_string(internetLatencyInMS / hopsInInternet).append("ms"));

//        NS3Helper::setDataRateAndDelay(d4d5.Get(0), "5.7Mbps", std::to_string(internetLatencyInMS / hopsInInternet).append("ms"));
//        NS3Helper::setDataRateAndDelay(d4d5.Get(1), "24.1Mbps", std::to_string(internetLatencyInMS / hopsInInternet).append("ms"));

        NS3Helper::setDataRateAndDelay(d5d6.Get(0), "1000Mbps", std::to_string(internetLatencyInMS / hopsInInternet).append("ms"));
        NS3Helper::setDataRateAndDelay(d5d6.Get(1), "1000Mbps", std::to_string(internetLatencyInMS / hopsInInternet).append("ms"));

        NS3Helper::setDataRateAndDelay(d6d1.Get(0), "10Gbps", "1ms");
        NS3Helper::setDataRateAndDelay(d6d1.Get(1), "10Gbps", "1ms");

        // Later, we add IP addresses.
        NS_LOG_INFO ("Assign IP Addresses.");
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.0.1.0", "255.255.255.0");
        Ipv4InterfaceContainer i0i2 = ipv4.Assign(d0d2);
        ipv4.SetBase("10.0.2.0", "255.255.255.0");
        //Ipv4InterfaceContainer i0i4 = ipv4.Assign(d0d4);
        ipv4.SetBase("10.0.3.0", "255.255.255.0");
        Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);
        ipv4.SetBase("10.0.4.0", "255.255.255.0");
        Ipv4InterfaceContainer i3i5 = ipv4.Assign(d3d5);
        ipv4.SetBase("10.0.5.0", "255.255.255.0");
        Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
        ipv4.SetBase("10.0.6.0", "255.255.255.0");
        Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);
        ipv4.SetBase("10.0.7.0", "255.255.255.0");
        Ipv4InterfaceContainer i6i1 = ipv4.Assign(d6d1);

        // Create router nodes, initialize routing database and set up the routing
        // tables in the nodes.
        NS3Helper::setAndPrintGlobalRoutes("sim/" + simName);
        for (size_t i = 0; i < c.GetN(); ++i)
        {
            NS3Helper::copyLocalRoutesIntoIpRoute(c.Get(i));
            NS3Helper::copyGlobalRoutesIntoIpRoute(c.Get(i));
        }

        //NS3Helper::setPacketLossRate(c.Get(1), 0.10);

        float internetPacketLoss = 0.0033;
        int hopCount = 5;

        NS3Helper::setPacketLossRate(c.Get(2), internetPacketLoss / hopCount);
        NS3Helper::setPacketLossRate(c.Get(3), internetPacketLoss / hopCount);
        NS3Helper::setPacketLossRate(c.Get(4), internetPacketLoss / hopCount);
        NS3Helper::setPacketLossRate(c.Get(5), internetPacketLoss / hopCount);
        NS3Helper::setPacketLossRate(c.Get(6), internetPacketLoss / hopCount);


        // Create the OnOff application to send UDP datagrams of size
        // 210 bytes at a rate of 448 Kb/s
        NS_LOG_INFO ("Create Applications.");
        uint16_t port = 9;   // Discard port (RFC 863)
        OnOffHelper onoff(socketType,
                          Address(InetSocketAddress(i0i2.GetAddress(0), port)));
        onoff.SetAttribute("PacketSize", UintegerValue(1300));
        onoff.SetConstantRate(DataRate("1Gbps"));
        ApplicationContainer apps = onoff.Install(c.Get(1));
        apps.Start(Seconds(1));
        apps.Stop(Seconds(simTime + 1));

        // Create a packet sink to receive these packets
        PacketSinkHelper sink(socketType,
                              Address(InetSocketAddress(Ipv4Address::GetAny(), port)));

        apps = sink.Install(c.Get(0));
        apps.Start(Seconds(1));
        apps.Stop(Seconds(simTime + 1));

        // Create a similar flow from n3 to n1, starting at time 1.1 seconds
//    onoff.SetAttribute("Remote",
//                       AddressValue(InetSocketAddress(i1i2.GetAddress(0), port)));
//    apps = onoff.Install(c.Get(3));
//    apps.Start(Seconds(1.1));
//    apps.Stop(Seconds(10.0));

        // Create a packet sink to receive these packets
//    apps = sink.Install(c.Get(1));
//    apps.Start(Seconds(1.1));
//    apps.Stop(Seconds(10.0));

        AsciiTraceHelper ascii;
        //p2p.EnableAsciiAll(ascii.CreateFileStream("sim/" + simName + ".tr"));
        p2p.EnablePcapAll("sim/" + simName);

        // Flow Monitor
        FlowMonitorHelper flowmonHelper;
        flowmonHelper.Install(c);


        NS_LOG_INFO ("Run Simulation.");
        Simulator::Stop(Seconds(simTime + 2));
        Simulator::Run();
        NS_LOG_INFO ("Done.");

        flowmonHelper.SerializeToXmlFile("sim/" + simName + ".flowmon", false, false);

        auto pktsink = apps.Get(0)->GetObject<PacketSink>();

        std::cout << "Total Rx: " << (pktsink->GetTotalRx() / static_cast<double>(1 << 20)) << " MBytes" << std::endl;
        std::cout << "Speed: " << ((8 * pktsink->GetTotalRx() / simTime) / static_cast<double>(1 << 20)) << " Mbps" << std::endl;

        Simulator::Destroy();
    }
}

#include "ns3/applications-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/dce-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3Helper.h"
#include <ns3/bs-net-device.h>
#include <ns3/csma-module.h>
#include <ns3/mobility-helper.h>
#include <ns3/uan-net-device.h>
#include <ns3/wave-net-device.h>

using namespace ns3;

namespace dceCradleMPTCP
{
    void setPos(Ptr<Node> n, int x, int y, int z)
    {
        Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel>();
        n->AggregateObject(loc);
        Vector locVec2(x, y, z);
        loc->SetPosition(locVec2);
    }

    int mainMPTCP(int argc, char *argv[])
    {
        uint32_t nRtrs = 2;
        CommandLine cmd;
        cmd.AddValue("nRtrs", "Number of routers. Default 2", nRtrs);
        cmd.Parse(argc, argv);

        NodeContainer nodes, routers;
        nodes.Create(2);
        routers.Create(nRtrs);

        Names::Add("Client", nodes.Get(0));
        Names::Add("Server", nodes.Get(1));
        Names::Add("Router1", routers.Get(0));
        Names::Add("Router2", routers.Get(1));

        DceManagerHelper dceManager;
        dceManager.SetTaskManagerAttribute("FiberManagerType",
                                           StringValue("UcontextFiberManager"));

        dceManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
                                   "Library", StringValue("liblinux.so"));
        LinuxStackHelper stack;
        stack.Install(nodes);
        stack.Install(routers);

        dceManager.Install(nodes);
        dceManager.Install(routers);

        PointToPointHelper pointToPoint;
        NetDeviceContainer devices1, devices2;
        Ipv4AddressHelper address1, address2;
        std::ostringstream cmd_oss;
        address1.SetBase("10.1.0.0", "255.255.255.0");
        address2.SetBase("10.2.0.0", "255.255.255.0");

        for (uint32_t i = 0; i < nRtrs; i++)
        {
            // Left link
            pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
            pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));
            devices1 = pointToPoint.Install(nodes.Get(0), routers.Get(i));
            // Assign ip addresses
            Ipv4InterfaceContainer if1 = address1.Assign(devices1);
            address1.NewNetwork();

            // Right link
            pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
            pointToPoint.SetChannelAttribute("Delay", StringValue("1ns"));
            devices2 = pointToPoint.Install(nodes.Get(1), routers.Get(i));
            // Assign ip addresses
            Ipv4InterfaceContainer if2 = address2.Assign(devices2);
            address2.NewNetwork();

            auto mid = 50;
            double steps = mid / nRtrs;
            int negate = i < (nRtrs / 2.0) ? -1 : 1;
            std::cout << "r " << i << " steps: " << steps << " negate: " << negate << " pos: " << mid + (negate * i * steps) << std::endl;
            setPos(routers.Get(i), 50, mid + (negate * i * steps), 0);

            //0: 50 + (-1 * 0 * 50) = 0
            //1: 50 + (1 * 1 * 50) = 100
        }

        setPos(nodes.Get(0), 0, 50, 0);
        setPos(nodes.Get(1), 100, 50, 0);


        std::cout << "Node 0: " << nodes.Get(0)->GetObject<Ipv4>()->GetAddress(0, 0) << std::endl;
        std::cout << "Node 1: " << nodes.Get(1)->GetObject<Ipv4>()->GetAddress(0, 0) << std::endl;
        std::cout << "Route 0: " << routers.Get(0)->GetObject<Ipv4>()->GetAddress(0, 0) << std::endl;
        std::cout << "Route 1: " << routers.Get(1)->GetObject<Ipv4>()->GetAddress(0, 0) << std::endl;
        ns3Helper::NS3Helper::setAndPrintGlobalRoutes("sim/dceCradleMPTCP");
        ns3Helper::NS3Helper::copyGlobalRoutesIntoIpRoute(nodes.Get(0));
        ns3Helper::NS3Helper::copyGlobalRoutesIntoIpRoute(nodes.Get(1));
        ns3Helper::NS3Helper::copyGlobalRoutesIntoIpRoute(routers.Get(0));
        ns3Helper::NS3Helper::copyGlobalRoutesIntoIpRoute(routers.Get(1));

        // Schedule Up/Down (XXX: didn't work...)
        LinuxStackHelper::RunIp(nodes.Get(0), Seconds(0), "link set dev sim0 multipath on");
        LinuxStackHelper::RunIp(nodes.Get(0), Seconds(0), "link set dev sim1 multipath on");
        LinuxStackHelper::RunIp(nodes.Get(1), Seconds(0.2), "link set dev sim0 multipath on");
        LinuxStackHelper::RunIp(nodes.Get(1), Seconds(0.2), "link set dev sim1 multipath on");
//        LinuxStackHelper::RunIp(nodes.Get(1), Seconds(1.0), "link set dev sim0 multipath off");
//        LinuxStackHelper::RunIp(nodes.Get(1), Seconds(15.0), "link set dev sim0 multipath on");
//        LinuxStackHelper::RunIp(nodes.Get(1), Seconds(30.0), "link set dev sim0 multipath off");


        // debug
        stack.SysctlSet(nodes, ".net.mptcp.mptcp_debug", "1");

        // Launch iperf client on node 0
        ApplicationContainer apps;
        OnOffHelper onoff = OnOffHelper("ns3::LinuxTcpSocketFactory",
                                        InetSocketAddress("10.2.0.1", 9));
        onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        onoff.SetAttribute("PacketSize", StringValue("1024"));
        onoff.SetAttribute("DataRate", StringValue("10Mbps"));
        apps = onoff.Install(nodes.Get(0));
        apps.Start(Seconds(5.0));

        // server on node 1
        PacketSinkHelper sink = PacketSinkHelper("ns3::LinuxTcpSocketFactory",
                                                 InetSocketAddress(Ipv4Address::GetAny(), 9));
        apps = sink.Install(nodes.Get(1));
        apps.Start(Seconds(3.9999));

        pointToPoint.EnablePcapAll("sim/mptcp-dce-cradle", false);

        apps.Start(Seconds(4));

        AnimationInterface anim("sim/dceCradleMPTCP" ".xml");
        anim.EnablePacketMetadata(true);

        Simulator::Stop(Seconds(10.0));
        Simulator::Run();
        Simulator::Destroy();

        return 0;
    }
}
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"


using namespace ns3;


int tcpBaselineLte_example ()
{
    uint16_t numberOfNodes = 1;
    uint16_t numberOfUeNodes = 3;

    double interPacketInterval = 100;

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper (epcHelper);

    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (1);
    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    InternetStackHelper internet;
    internet.Install (remoteHostContainer);

    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create (numberOfNodes);
    ueNodes.Create (numberOfUeNodes);

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (enbNodes);
    mobility.Install (ueNodes);

    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
    // Assign IP address to UEs, and install applications
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        Ptr<Node> ueNode = ueNodes.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    lteHelper->Attach (ueLteDevs, enbLteDevs.Get (0));


    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        Ptr<NetDevice> ueDevice = ueLteDevs.Get (u);
        EpsBearer bearer (EpsBearer::GBR_CONV_VOICE);
        lteHelper->ActivateDedicatedEpsBearer (ueDevice, bearer, EpcTft::Default ());
    }


    uint16_t dlPort = 1234;
    uint16_t ulPort = 2000;
    uint16_t otherPort = 3000;
    ApplicationContainer clientApps;
    ApplicationContainer serverApps;
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        ++ulPort;
        ++otherPort;
        PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
        PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
        PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
        serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));
        serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
        serverApps.Add (packetSinkHelper.Install (ueNodes.Get (u)));

        UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
        dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
        dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));

        UdpClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));

        UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
        client.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
        client.SetAttribute ("MaxPackets", UintegerValue (1000000));

        clientApps.Add (dlClient.Install (remoteHost));
        clientApps.Add (ulClient.Install (ueNodes.Get (u)));
        if (u + 1 < ueNodes.GetN ())
        {
            clientApps.Add (client.Install (ueNodes.Get (u + 1)));
        }
        else
        {
            clientApps.Add (client.Install (ueNodes.Get (0)));
        }
    }

    serverApps.Start (Seconds (0.030));
    clientApps.Start (Seconds (0.030));

    Ptr<NetDevice> ueDevice = ueLteDevs.Get (0);
    Ptr<NetDevice> enbDevice = enbLteDevs.Get (0);


    Time deActivateTime (Seconds (1.5));
    Simulator::Schedule (deActivateTime, &LteHelper::DeActivateDedicatedEpsBearer, lteHelper, ueDevice, enbDevice, 2);

    Simulator::Stop (Seconds (3.0));


    AsciiTraceHelper ascii;
    p2ph.EnableAsciiAll(ascii.CreateFileStream("tcpBaselineLTE" ".tr"));
    p2ph.EnablePcapAll("TCPBaselineWlan");

    // Flow Monitor
    FlowMonitorHelper flowmonHelper;
    flowmonHelper.InstallAll();

    Simulator::Run ();

    Simulator::Destroy ();

    flowmonHelper.SerializeToXmlFile("tcpBaselineLTE" ".flowmon", false, false);


    return 0;

}
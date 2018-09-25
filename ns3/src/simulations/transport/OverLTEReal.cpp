#include "OverLTEReal.h"
//#include "../../snippets/ns3Helper.h"
#include "../SequentialDeterministicErrorModel.h"
#include <ns3/core-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/dce-module.h>
#include <ns3/internet-module.h>


//#https://github.com/direct-code-execution/ns-3-dce/blob/master/example/dce-mptcp-lte-wifi.cc

void OverLTEReal::setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver)
{
    lteHelper = ns3::CreateObject<ns3::LteHelper>();
    epcHelper = ns3::CreateObject<ns3::PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);
    auto lteGateway = epcHelper->GetPgwNode();

    lteGateway_gatewayPartner = p2p.Install(lteGateway, receiver);

    ipv4.SetBase("10.0.2.0", "255.255.255.0");
    auto iLteGateway_iGatewayPartner = ipv4.Assign(lteGateway_gatewayPartner);

//    std::cout << "iLteGateway_i4: " << iLteGateway_iGatewayPartner.GetAddress(0) << ", "
//              << iLteGateway_iGatewayPartner.GetAddress(1)
//              << std::endl;

//    auto ls = lteGateway->GetObject<Ipv4>();
//    auto nInterface = ls->GetNInterfaces();
//    for(size_t i = 0; i < nInterface; ++i)
//    {
//        auto nAddresses = ls->GetNAddresses(i);
//        for(size_t y = 0; y < nAddresses; ++y)
//        {
//            std::cout << i << " : " << ls->GetAddress(i, y)<< std::endl;
//        }
//    }

    enbNodes.Create(1);
    ueNodes.Add(sender);

//    ns3::Ptr<ns3::ListPositionAllocator> positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator>();
//    positionAlloc->Add(ns3::Vector(0.0, 0.0, 0.0));
//    positionAlloc->Add(ns3::Vector(1.0, 1.0, 0.0));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
//    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(enbNodes);
    mobility.Install(ueNodes);

    enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

    auto ueIpIface = epcHelper->AssignUeIpv4Address(ns3::NetDeviceContainer(ueLteDevs));
//    std::cout << "ueIpInt: " << ueIpIface.GetAddress(0) << std::endl;

    lteHelper->Attach(ueLteDevs, enbLteDevs.Get(0));

    auto ueDevice = ueLteDevs.Get(0);

    ns3::GbrQosInformation qos;
    qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
    qos.gbrUl = 132;
    qos.mbrDl = qos.gbrDl;
    qos.mbrUl = qos.gbrUl;

    enum ns3::EpsBearer::Qci q = ns3::EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
    ns3::EpsBearer bearer(q);
//    bearer.arp.priorityLevel = 15 - (0 + 1);
//    bearer.arp.preemptionCapability = true;
//    bearer.arp.preemptionVulnerability = true;

    lteHelper->ActivateDedicatedEpsBearer(ueDevice, bearer, ns3::EpcTft::Default());

//    ns3::Ipv4StaticRoutingHelper ipv4RoutingHelper;
//    auto ueNode = sender;
//    auto ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<ns3::Ipv4Linux>());
//    auto enbInterface = ueNode->GetObject<ns3::Ipv4Linux>()->GetInterfaceForPrefix("7.0.0.0", "255.0.0.0");
//    ueStaticRouting->SetDefaultRoute("7.0.0.1", enbInterface);

//    auto ls = lteGateway->GetObject<Ipv4>();
//    auto nInterface = ls->GetNInterfaces();
//    for(size_t i = 0; i < nInterface; ++i)
//    {
//        auto nAddresses = ls->GetNAddresses(i);
//        for(size_t y = 0; y < nAddresses; ++y)
//        {
//            std::cout << i << " : " << ls->GetAddress(i, y)<< std::endl;
//        }
//    }
    //###### Additional settings

    //has to be set as settings above destory the value
    ueLteDevs.Get(0)->SetAttribute("Mtu", ns3::UintegerValue(1500));

    netDeviceContainer = ns3::NetDeviceContainer();
    netDeviceContainer.Add(ueLteDevs.Get(0));
    netDeviceContainer.Add(lteGateway_gatewayPartner.Get(1));

    interfaceContainer = ns3::Ipv4InterfaceContainer();
    interfaceContainer.Add(sender->GetObject<ns3::Ipv4>(), getLteInterface(sender));
    interfaceContainer.Add(receiver->GetObject<ns3::Ipv4>(), 0);

    ns3::LinuxStackHelper::RunIp (sender, ns3::Seconds (0.1), "route add default via 7.0.0.1 dev sim0");
    ns3::LinuxStackHelper::RunIp (receiver, ns3::Seconds (0.1), "route add default via 10.0.2.1 dev sim0");

    ns3::LinuxStackHelper::RunIp(sender, ns3::Seconds(0.3), "link");


//    p2p.EnablePcap("receiver", lteGateway_gatewayPartner.Get(1));
//    p2p.EnablePcap("ltegateway", lteGateway_gatewayPartner.Get(0));
//    std::cout << "sender: " << interfaceContainer.GetAddress(0) << std::endl;
//    std::cout << "receiver: " << interfaceContainer.GetAddress(1) << std::endl;
}

void OverLTEReal::setPacketLoss(double packetLoss)
{
    auto packetLossModel = ns3::CreateObject<SequentialDeterministicErrorModel>(packetLoss);
    lteGateway_gatewayPartner.Get(1)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
}

void OverLTEReal::setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate)
{
    sendDataRate = ns3::StringValue(senderDataRate);
    lteGateway_gatewayPartner.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDataRate));
    lteGateway_gatewayPartner.Get(1)->SetAttribute("DataRate", ns3::StringValue(sendDataRate));

    lteHelper->SetEnbDeviceAttribute ("DlBandwidth", ns3::UintegerValue (200));
    lteHelper->SetEnbDeviceAttribute ("UlBandwidth", ns3::UintegerValue (200));
}

void OverLTEReal::setDelay(const std::string &delay)
{
    lteGateway_gatewayPartner.Get(0)->GetChannel()->SetAttribute("Delay", ns3::StringValue(delay));
}

ns3::StringValue OverLTEReal::getSendDataRate()
{
//    ns3::StringValue dataRate;
//    lteGateway_gatewayPartner.Get(0)->GetAttribute("DataRate", dataRate);
    return sendDataRate;
}

uint32_t OverLTEReal::getLteInterface(ns3::Ptr<ns3::Node> node)
{
    return node->GetObject<ns3::Ipv4>()->GetInterfaceForPrefix("7.0.0.0", "255.0.0.0");
}

//void OverLTEReal::setPacketLoss(double packetLoss)
//{
//
//}
//
//void OverLTEReal::setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate)
//{
//    sendDataRate = ns3::StringValue(senderDataRate);
//
////    lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (downlinkRb));
////    lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (downlinkRb));
//
//    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
//    pointToPoint.SetDeviceAttribute ("Mtu", UintegerValue (1500));
//    pointToPoint.SetChannelAttribute ("Delay", StringValue ("0.00001ms"));
//}
//
//void OverLTEReal::setDelay(const std::string &delay)
//{
//
//}
//
//ns3::StringValue OverLTEReal::getSendDataRate()
//{
//    if(sendDataRate.Get().empty())
//        return ns3::StringValue("1Gbps");
//    return sendDataRate;
//}



//    Ipv4GlobalRoutingHelper globalRoutingHelper;
//    auto routingProtocol = sender->GetObject<Ipv4>()->GetRoutingProtocol();
//    auto routing = globalRoutingHelper.GetRouting<Ipv4GlobalRouting>(routingProtocol);
//    routing->AddNetworkRouteTo(ns3::Ipv4Address("10.0.1.0"), ns3::Ipv4Mask("255.255.255.0"), epcHelper->GetUeDefaultGatewayAddress(), getLteInterface(sender));
//
//    routingProtocol = receiver->GetObject<Ipv4>()->GetRoutingProtocol();
//    routing = globalRoutingHelper.GetRouting<Ipv4GlobalRouting>(routingProtocol);
//    routing->AddNetworkRouteTo(ns3::Ipv4Address("7.0.0.0"), ns3::Ipv4Mask("255.0.0.0"), iLteGateway_iGatewayPartner.GetAddress(0), 0);
//
//    ns3::LinuxStackHelper::RunIp(sender, ns3::Seconds(0.1), "route add default via 7.0.0.1 dev sim0");
//    ns3::LinuxStackHelper::RunIp(receiver, ns3::Seconds(0.1), "route add default via 10.0.2.1 dev sim0");
//    ns3::LinuxStackHelper::RunIp(receiver, ns3::Seconds(0.8), "rule show");

// setup ip routes
//    std::ostringstream cmd_oss;
//    cmd_oss.str ("");
//    cmd_oss << "rule add from " << interfaceContainer.GetAddress (0, 0) << " table " << 1;
//    LinuxStackHelper::RunIp (sender, Seconds (0.1), cmd_oss.str ().c_str ());
//    cmd_oss.str ("");
//    cmd_oss << "route add default via " << "7.0.0.1 "  << " dev sim" << 0 << " table " << 1;
//    LinuxStackHelper::RunIp (sender, Seconds (0.2), cmd_oss.str ().c_str ());
//
//
//    // setup ip routes
//    cmd_oss.str ("");
//    cmd_oss << "rule add from " << interfaceContainer.GetAddress (1, 0) << " table " << (1);
//    LinuxStackHelper::RunIp (receiver, Seconds (0.1), cmd_oss.str ().c_str ());
//    cmd_oss.str ("");
//    cmd_oss << "route add 10.0.2.0/24 dev sim" << 0 << " scope link table " << (1);
//    LinuxStackHelper::RunIp (receiver, Seconds (0.2), cmd_oss.str ().c_str ());
//    cmd_oss.str ("");
//    cmd_oss << "route add default via " << "10.0.2.1 "  << " dev sim" << 0 << " table " << 1;
//    LinuxStackHelper::RunIp (receiver, Seconds (0.2), cmd_oss.str ().c_str ());

//    Ipv4StaticRoutingHelper ipv4RoutingHelper;
//    auto ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(bridgeNode->GetObject<Ipv4>());
//    auto enbInterface = bridgeNode->GetObject<Ipv4>()->GetInterfaceForPrefix("7.0.0.0", "255.0.0.0");
//    ueStaticRouting->SetDefaultRoute("7.0.0.1", enbInterface);

//    ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(lteGateway->GetObject<Ipv4>());
//    enbInterface = lteGateway->GetObject<Ipv4>()->GetInterfaceForPrefix("10.0.0.5", "255.255.255.252");
//    ueStaticRouting->SetDefaultRoute("10.0.0.5", enbInterface);

// default route
//    LinuxStackHelper::RunIp (sender, Seconds (0.1), "route add default via 10.0.1.2 dev sim0");

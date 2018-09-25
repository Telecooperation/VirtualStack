#pragma once

#include <ns3/dce-module.h>
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/uan-module.h"
#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/wimax-module.h"

using namespace ns3;

namespace ns3Helper
{
    const std::string simName = "ns3Helper";
    NS_LOG_COMPONENT_DEFINE (simName);

    class NS3Helper
    {
    public:
        static void setGlobalDataRateAndDelay(std::string dataRate, std::string delay)
        {
            Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue(dataRate));
            Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue(delay));
        }

        static void setDataRateAndDelay(Ptr<NetDevice> dev, std::string dataRate, std::string delay)
        {
            dev->SetAttribute("DataRate", StringValue(dataRate));
            auto channel = dev->GetChannel();
            if(!channel)
                return;
            channel->SetAttribute("Delay",  StringValue(delay));
        }

        static void setPacketLossRate(Ptr<Node> node, double packetLoss, uint32_t netDevice = 0)
        {
            auto em = CreateObject<RateErrorModel>();
            em->SetAttribute("ErrorRate", DoubleValue(packetLoss));
            em->SetAttribute("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));
            node->GetDevice(netDevice)->SetAttribute("ReceiveErrorModel", PointerValue(em));
        }

        static void printGlobalRoutes(std::string simName)
        {
            Ipv4GlobalRoutingHelper globalRouting;
            auto routingStream = Create<OutputStreamWrapper>(simName + ".routes", std::ios::out);
            globalRouting.PrintRoutingTableAllAt(Seconds(0.1), routingStream);
        }

        static void setAndPrintGlobalRoutes(std::string simName)
        {
            // Create router nodes, initialize routing database and set up the routing
            // tables in the nodes.
            NS_LOG_INFO ("Create global routing table");
            Ipv4GlobalRoutingHelper::PopulateRoutingTables();
            //Simulator::Schedule (Seconds(4), &Ipv4::SetDown, nodes.Get(0)->GetObject<Ipv4>(), 1);

            printGlobalRoutes(simName);
        }

        static void copyGlobalRoutesIntoIpRoute(Ptr<Node> node)
        {
            auto ipv4 = node->GetObject<Ipv4>();
            if (!ipv4)
                return;

            auto rp = ipv4->GetRoutingProtocol();
            auto globalRp = rp->GetObject<Ipv4GlobalRouting>();

            if(!globalRp)
                return;

            auto routeCount = globalRp->GetNRoutes();

//        std::cout << "### Node: " << Names::FindName(node) << std::endl;

            std::ostringstream cmd_oss;
            for (size_t i = 0; i < routeCount; ++i)
            {
                cmd_oss.str("");
                auto route = globalRp->GetRoute(i);
                cmd_oss << "route add "<< route->GetDest() << "/" << route->GetDestNetworkMask().GetPrefixLength() << " via " << route->GetGateway() << " dev sim" << route->GetInterface();
                //LinuxStackHelper::RunIp(node, Seconds(0.2), cmd_oss.str().c_str());

                std::cout << cmd_oss.str() << std::endl;
            }

            LinuxStackHelper::RunIp(node, Seconds(0.3), "route show");
        }

        static void copyLocalRoutesIntoIpRoute(Ptr<Node> node)
        {
            auto ipv4 = node->GetObject<Ipv4>();
            if (!ipv4)
                return;

            auto rp = ipv4->GetRoutingProtocol();
            auto staticRp = rp->GetObject<Ipv4StaticRouting>();

            if(!staticRp)
                return;

            auto routeCount = staticRp->GetNRoutes();

//        std::cout << "### Node: " << Names::FindName(node) << std::endl;

            std::ostringstream cmd_oss;
            for (size_t i = 0; i < routeCount; ++i)
            {
                cmd_oss.str("");
                auto route = staticRp->GetRoute(i);
                cmd_oss << "route add "<< route.GetDest() << "/" << route.GetDestNetworkMask().GetPrefixLength() << " via " << route.GetGateway() << " dev sim" << route.GetInterface();
//                LinuxStackHelper::RunIp(node, Seconds(0.2), cmd_oss.str().c_str());

            std::cout << cmd_oss.str() << std::endl;
            }

            LinuxStackHelper::RunIp(node, Seconds(0.3), "route show");
        }

        static void addLte(Ptr<Node> ueNode,
                    Ptr<Node> gatewayPartner, NodeContainer &enbNodes,
                    Ipv4InterfaceContainer &ueIpIface,
                    NetDeviceContainer &lteGateway_gatewayPartner, Ipv4InterfaceContainer &iLteGateway_iGatewayPartner,
                    Ptr<PointToPointEpcHelper> &epcHelper)
        {
            PointToPointHelper p2p;
            Ipv4AddressHelper ipv4;

            NS_LOG_INFO ("Create LTE Helpers");
            auto lteHelper = CreateObject<LteHelper>();
            epcHelper = CreateObject<PointToPointEpcHelper>();
            lteHelper->SetEpcHelper(epcHelper);
            auto lteGateway = epcHelper->GetPgwNode();

            NS_LOG_INFO ("Create channel: LTE-Gateway to Network");
            lteGateway_gatewayPartner = p2p.Install(lteGateway, gatewayPartner);

            ipv4.SetBase("10.0.2.0", "255.255.255.0");
            iLteGateway_iGatewayPartner = ipv4.Assign(lteGateway_gatewayPartner);

            std::cout << "iLteGateway_i4: " << iLteGateway_iGatewayPartner.GetAddress(0) << ", "
                      << iLteGateway_iGatewayPartner.GetAddress(1)
                      << std::endl;

            NS_LOG_INFO ("Create LTE Nodes");
            NodeContainer ueNodes;
            enbNodes.Create(1);
            ueNodes.Add(ueNode);

            NS_LOG_INFO ("Configure LTE Nodes");
            Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
            positionAlloc->Add(Vector(0.0, 0.0, 0.0));
            positionAlloc->Add(Vector(1.0, 1.0, 0.0));

            MobilityHelper mobility;
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.SetPositionAllocator(positionAlloc);
            mobility.Install(enbNodes);
            mobility.Install(ueNodes);

            auto enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
            auto ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

            NS_LOG_INFO ("Assign IP for UE to ENB");
            ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
            std::cout << "ueIpInt: " << ueIpIface.GetAddress(0) << std::endl;

            NS_LOG_INFO ("Connect UE to ENB");
            lteHelper->Attach(ueLteDevs, enbLteDevs.Get(0));

            NS_LOG_INFO ("Set Bearer for UE");
            auto ueDevice = ueLteDevs.Get(0);

            GbrQosInformation qos;
            qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
            qos.gbrUl = 132;
            qos.mbrDl = qos.gbrDl;
            qos.mbrUl = qos.gbrUl;

            enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
            EpsBearer bearer (q, qos);
            bearer.arp.priorityLevel = 15 - (0 + 1);
            bearer.arp.preemptionCapability = true;
            bearer.arp.preemptionVulnerability = true;

            lteHelper->ActivateDedicatedEpsBearer(ueDevice, bearer, EpcTft::Default());
        }

        static void wlan(Ptr<Node> device, Ptr<Node> accessPoint, NetDeviceContainer &apDevice, NetDeviceContainer &staDevices)
        {
//            std::string tcpVariant = "TcpNewReno";             /* TCP variant type. */
//            tcpVariant = std::string("ns3::") + tcpVariant;
//            // Select TCP variant
//            if (tcpVariant.compare("ns3::TcpWestwoodPlus") == 0)
//            {
//                // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
//                Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpWestwood::GetTypeId()));
//                // the default protocol type in ns3::TcpWestwood is WESTWOOD
//                Config::SetDefault("ns3::TcpWestwood::ProtocolType", EnumValue(TcpWestwood::WESTWOODPLUS));
//            } else
//            {
//                TypeId tcpTid;
//                NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe(tcpVariant, &tcpTid),
//                                     "TypeId " << tcpVariant << " not found");
//                Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName(tcpVariant)));
//            }

            std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */

            /* No fragmentation and no RTS/CTS */
            Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("999999"));
            Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("999999"));

            WifiMacHelper wifiMac;
            WifiHelper wifiHelper;
            wifiHelper.SetStandard(WifiPhyStandard::WIFI_PHY_STANDARD_80211n_2_4GHZ);

            /* Set up Legacy Channel */
            YansWifiChannelHelper wifiChannel;

            wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
            wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5e9));

            /* Setup Physical Layer */
            YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
            wifiPhy.SetChannel(wifiChannel.Create());
            wifiPhy.Set("TxPowerStart", DoubleValue(10.0));
            wifiPhy.Set("TxPowerEnd", DoubleValue(10.0));
            wifiPhy.Set("TxPowerLevels", UintegerValue(1));
            wifiPhy.Set("TxGain", DoubleValue(0));
            wifiPhy.Set("RxGain", DoubleValue(0));
            wifiPhy.Set("RxNoiseFigure", DoubleValue(10));
            wifiPhy.Set("CcaMode1Threshold", DoubleValue(-79));
            wifiPhy.Set("EnergyDetectionThreshold", DoubleValue(-79 + 3));
            wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
            wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                               "DataMode", StringValue(phyRate),
                                               "ControlMode", StringValue("HtMcs0"));

            Ptr<Node> apWifiNode = accessPoint;
            Ptr<Node> staWifiNode = device;

            /* Configure AP */
            Ssid ssid = Ssid("network");
            wifiMac.SetType("ns3::ApWifiMac",
                            "Ssid", SsidValue(ssid));

            //NetDeviceContainer apDevice;
            apDevice = wifiHelper.Install(wifiPhy, wifiMac, apWifiNode);

            /* Configure STA */
            wifiMac.SetType("ns3::StaWifiMac",
                            "Ssid", SsidValue(ssid));

            //NetDeviceContainer staDevices;
            staDevices = wifiHelper.Install(wifiPhy, wifiMac, staWifiNode);


            MobilityHelper mobility;
            Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
            positionAlloc->Add(Vector(0.0, 0.0, 0.0));
            positionAlloc->Add(Vector(1.0, 1.0, 0.0));

            mobility.SetPositionAllocator(positionAlloc);
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(apWifiNode);
            mobility.Install(staWifiNode);
        }

        static void addStaticRoutesToLTEGateway(Ptr<NetDevice> device,
                                         Ptr<NetDevice> from,
                                         std::vector<Ptr<Node>> &nodesToVisit)
        {
            auto deviceNode = device->GetNode();
            auto fromNode = from->GetNode();

            auto deviceIpv4 = deviceNode->GetObject<Ipv4>();
            auto fromIpv4 = fromNode->GetObject<Ipv4>();

            auto deviceInterface = deviceIpv4->GetInterfaceForDevice(device);
            auto fromInterface = fromIpv4->GetInterfaceForDevice(from);

//        std::cout << "Node: " << deviceNode->GetId() << " -> Interface: " << deviceInterface << " IP: " << deviceIpv4->GetAddress(deviceInterface, 0)
//                  << " , from: " << from->GetNode()->GetId() << " -> Interface: " << fromInterface << std::endl;

            auto fromIpv4Address = fromIpv4->GetAddress(fromInterface, 0);

            Ipv4StaticRoutingHelper ipv4RoutingHelper;
            auto staticRoutes = ipv4RoutingHelper.GetStaticRouting(deviceIpv4);
            staticRoutes->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), fromIpv4Address.GetLocal(),
                                            deviceInterface);


            for (size_t i = 0; i < deviceNode->GetNDevices(); ++i)
            {
                auto item = deviceNode->GetDevice(i);
                if (item == device)
                    continue;

                auto channel = item->GetChannel();
                if (channel == nullptr)
                    continue;

                for (size_t devIdx = 0; devIdx < channel->GetNDevices(); ++devIdx)
                {
                    auto toDevice = channel->GetDevice(devIdx);
                    if (toDevice == item)
                        continue;
                    auto it = std::find(nodesToVisit.begin(), nodesToVisit.end(), toDevice->GetNode());
                    if (it == nodesToVisit.end())
                        continue;

                    auto nextNodesToVisit = nodesToVisit;
                    auto newIt = nextNodesToVisit.begin() + std::distance(nodesToVisit.begin(), it);
                    nextNodesToVisit.erase(newIt);

                    addStaticRoutesToLTEGateway(toDevice, item, nextNodesToVisit);
                }
            }
        }

        static void addLteRoutes(NodeContainer &nodesWithUeNode,
                          Ptr<Node> ueNode,
                          NetDeviceContainer &lteGateway_gatewayPartner, bool forceOverLte = true)
        {
            NS_LOG_INFO ("Add static route from n4 to LTE on Interface 1");
            Ipv4StaticRoutingHelper ipv4RoutingHelper;

            NS_LOG_INFO ("Add static route from ueNode to LTE-ENB");
            auto ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
            auto enbInterface = ueNode->GetObject<Ipv4>()->GetInterfaceForPrefix("7.0.0.0", "255.0.0.0");

            if(forceOverLte)
                ueStaticRouting->SetDefaultRoute("7.0.0.1", enbInterface);

            auto lteGatewayDevice = lteGateway_gatewayPartner.Get(0);
            auto ltePartnerDevice = lteGateway_gatewayPartner.Get(1);
            auto lteGatewayNode = lteGatewayDevice->GetNode();
            auto lteGatewayIpv4 = lteGatewayNode->GetObject<Ipv4>();

            //6 = lteGateway
            //7 = enb

            NS_LOG_INFO("Compute global routes for LTE-Gateway with ENB disabled");
            auto gatewayEnbInterface = lteGatewayIpv4->GetInterfaceForPrefix("7.0.0.0", "255.0.0.0");
            lteGatewayIpv4->SetDown(gatewayEnbInterface);
            Ipv4GlobalRoutingHelper::RecomputeRoutingTables();
            lteGatewayIpv4->SetUp(gatewayEnbInterface);

            NS_LOG_INFO("Add static routes for nodes to LTE-Gateway");
            std::vector<Ptr<Node>> addRouteNodes{nodesWithUeNode.Begin(), nodesWithUeNode.End()};
            addRouteNodes.erase(std::find(addRouteNodes.begin(), addRouteNodes.end(), ueNode));
            addStaticRoutesToLTEGateway(ltePartnerDevice, lteGatewayDevice, addRouteNodes);
        }
    };
}
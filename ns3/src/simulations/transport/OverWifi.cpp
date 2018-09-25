#include "OverWifi.h"
#include "../SequentialDeterministicErrorModel.h"

#include <ns3/core-module.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/point-to-point-module.h>


void OverWifi::setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver)
{
    std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */

    /* No fragmentation and no RTS/CTS */
    ns3::Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", ns3::StringValue("999999"));
    ns3::Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", ns3::StringValue("999999"));

    wifiHelper.SetStandard(ns3::WifiPhyStandard::WIFI_PHY_STANDARD_80211ac);

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", ns3::DoubleValue(5e9));

    /* Setup Physical Layer */
    wifiPhy = ns3::YansWifiPhyHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());
//    wifiPhy.Set("MaxSupportedTxSpatialStreams", ns3::UintegerValue(4));
//    wifiPhy.Set("MaxSupportedRxSpatialStreams", ns3::UintegerValue(4));
//    wifiPhy.Set("Antennas", ns3::UintegerValue(4));

//    wifiPhy.Set("TxPowerStart", ns3::DoubleValue(10.0));
//    wifiPhy.Set("TxPowerEnd", ns3::DoubleValue(10.0));
//    wifiPhy.Set("TxPowerLevels", ns3::UintegerValue(1));
//    wifiPhy.Set("TxGain", ns3::DoubleValue(0));
//    wifiPhy.Set("RxGain", ns3::DoubleValue(0));
//    wifiPhy.Set("RxNoiseFigure", ns3::DoubleValue(10));
//    wifiPhy.Set("CcaMode1Threshold", ns3::DoubleValue(-79));
//    wifiPhy.Set("EnergyDetectionThreshold", ns3::DoubleValue(-79 + 3));
//    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
    wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                       "DataMode", ns3::StringValue(phyRate),
                                       "ControlMode", ns3::StringValue("HtMcs0"));

    ns3::Ptr<ns3::Node> staWifiNode = sender;
    ns3::Ptr<ns3::Node> apWifiNode = ns3::CreateObject<ns3::Node>();
    ns3::NodeContainer container{apWifiNode, receiver};

    /* Configure AP */
    ns3::Ssid ssid = ns3::Ssid("network");
    wifiMac.SetType("ns3::ApWifiMac",
                    "Ssid", ns3::SsidValue(ssid));

    //NetDeviceContainer apDevice;
    apDevice = wifiHelper.Install(wifiPhy, wifiMac, apWifiNode);

    /* Configure STA */
    wifiMac.SetType("ns3::StaWifiMac",
                    "Ssid", ns3::SsidValue(ssid));

    //NetDeviceContainer staDevices;
    staDevices = wifiHelper.Install(wifiPhy, wifiMac, staWifiNode);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    ns3::Ptr<ns3::ListPositionAllocator> positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
    positionAlloc->Add (ns3::Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (ns3::Vector (1.0, 0.0, 0.0)); //place device 1m from router

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apWifiNode);
    mobility.Install(staWifiNode);

    netDeviceContainer = ns3::NetDeviceContainer(staDevices, apDevice);

    ns3::InternetStackHelper internetStackHelper;
    //ns3::LinuxStackHelper internetStackHelper;
    internetStackHelper.Install(container.Get(0));

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0");
    interfaceContainer = ipv4.Assign(netDeviceContainer);



    ns3::PointToPointHelper p2p;

    p2pDevices = p2p.Install(apWifiNode, receiver);
    p2pDevices.Get(0)->SetAttribute("DataRate", ns3::StringValue("100Gbps"));
    p2pDevices.Get(1)->SetAttribute("DataRate", ns3::StringValue("100Gbps"));


    ipv4.SetBase("10.0.2.0", "255.255.255.0");
    interfaceContainer = ipv4.Assign(p2pDevices);



    ns3::Ipv4StaticRoutingHelper ipv4RoutingHelper;
    auto staticRoutes = ipv4RoutingHelper.GetStaticRouting(apWifiNode->GetObject<ns3::Ipv4>());
    staticRoutes->AddNetworkRouteTo(ns3::Ipv4Address("10.0.2.0"),ns3::Ipv4Mask("255.255.255.0"), p2pDevices.Get(0)->GetIfIndex());
    //staticRoutes->AddHostRouteTo(ns3::Ipv4Address("10.0.1.1"), apDevice.Get(0)->GetIfIndex());
    staticRoutes->AddNetworkRouteTo(ns3::Ipv4Address("10.0.1.0"),ns3::Ipv4Mask("255.255.255.0"), 1);



    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    p2p.EnablePcapAll("p2p");

    wifiPhy.EnablePcap("accesspoint", apDevice.Get(0));
    wifiPhy.EnablePcap("client", staDevices.Get(0));
}

void OverWifi::setPacketLoss(double packetLoss)
{
    /*auto randVar = ns3::CreateObject<ns3::UniformRandomVariable>();
    randVar->SetAttribute("Min", ns3::DoubleValue(0));
    randVar->SetAttribute("Max", ns3::DoubleValue(1));

    wifiChannel.AddPropagationLoss("ns3::RandomPropagationDelayModel","Variable",ns3::PointerValue(randVar));*/
    auto packetLossModel = ns3::CreateObject<SequentialDeterministicErrorModel>(packetLoss);
    p2pDevices.Get(1)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
    p2pDevices.Get(0)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
}

void OverWifi::setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate)
{
    sendDataRate = ns3::StringValue(senderDataRate);
}

void OverWifi::setDelay(const std::string &delay)
{
    /*ns3::TimeValue time;
    time.DeserializeFromString(delay, ns3::MakeTimeChecker());
    auto delayTime = time.Get();

    auto timeInSec = delayTime.GetSeconds();

    auto randVar = ns3::CreateObject<ns3::ConstantRandomVariable>();
    randVar->SetAttribute("Constant", ns3::DoubleValue(timeInSec));
    //randVar->SetAttribute("Max", ns3::DoubleValue(1));

    //distance from device to router is 1m
    wifiChannel.SetPropagationDelay("ns3::RandomPropagationDelayModel", "Variable", ns3::PointerValue(randVar));*/
    p2pDevices.Get(0)->GetChannel()->SetAttribute("Delay", ns3::StringValue(delay));
    p2pDevices.Get(1)->GetChannel()->SetAttribute("Delay", ns3::StringValue(delay));
}

ns3::StringValue OverWifi::getSendDataRate()
{
    if(sendDataRate.Get().empty())
        return ns3::StringValue("1Gbps");
    return sendDataRate;
}

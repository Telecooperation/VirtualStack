#include "../SequentialDeterministicErrorModel.h"
#include "OverLTE.h"
#include <ns3/core-module.h>
#include <ns3/csma-helper.h>
#include <ns3/dce-module.h>

OverLTE::OverLTE()
{
    delayTime = ns3::CreateObject<ns3::NormalRandomVariable>();
    delayTime->SetAttribute ("Mean", ns3::DoubleValue (0.0065));
    delayTime->SetAttribute ("Variance", ns3::DoubleValue (0.001*0.001));
    delayTime->SetAttribute ("Bound", ns3::DoubleValue (0.003));
}

void OverLTE::setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver)
{
    /*netDeviceContainer = p2p.Install(sender, receiver);

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0");
    interfaceContainer = ipv4.Assign(netDeviceContainer);

    lteEvent = ns3::Simulator::Schedule(ns3::Seconds(0.5),
                                        &OverLTE::BlockLTEConnection, this);
    p2p.EnablePcap("receiver", netDeviceContainer.Get(1));*/


    auto lteDownSpeed = ns3::StringValue("20Mbps");
    auto lteUpSpeed = ns3::StringValue("20Mbps");
    auto lteStaticDownDelay = ns3::StringValue("50ms");
    auto lteStaticUpDelay = ns3::StringValue("30ms");
    auto lteLoss = 0;

    ns3::NodeContainer container;
    container.Add(sender);
    container.Create(1);

    ns3::InternetStackHelper internetStackHelper;
    //ns3::LinuxStackHelper internetStackHelper;
    internetStackHelper.Install(container.Get(1));

    ns3::CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", ns3::StringValue("100Gbps"));
    csmaNetDevice = csma.Install(container);

    //container.Add(receiver);

    ns3::PointToPointHelper p2p;
    p2pNetDevice = p2p.Install(container.Get(1), receiver);

    netDeviceContainer.Add(csmaNetDevice.Get(0));
    netDeviceContainer.Add(p2pNetDevice.Get(1));

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0");
    auto csmaInterface = ipv4.Assign(csmaNetDevice);
    auto p2pInterface = ipv4.Assign(p2pNetDevice);

    interfaceContainer.Add(csmaInterface.Get(0));
    interfaceContainer.Add(p2pInterface.Get(1));

    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    ns3::Ipv4StaticRoutingHelper ipv4RoutingHelper;
    auto staticRoutes = ipv4RoutingHelper.GetStaticRouting(container.Get(1)->GetObject<ns3::Ipv4>());
    staticRoutes->AddHostRouteTo(ns3::Ipv4Address("10.0.1.1"), csmaNetDevice.Get(1)->GetIfIndex());

    //ns3::Ipv4GlobalRoutingHelper globalRouting;
    //auto routingStream = ns3::Create<ns3::OutputStreamWrapper>("blah" ".routes", std::ios::out);
//    globalRouting.PrintRoutingTableAllAt(ns3::Seconds(0.1), routingStream);

    lteEvent = ns3::Simulator::Schedule(ns3::Seconds(1.02),
                                       &OverLTE::UnblockConnection, this);
    //csma.EnablePcapAll("cmsa");
    //p2p.EnablePcapAll("p2p");
    this->senderDatarate = lteUpSpeed.Get();
    this->receiverDatarate = lteDownSpeed.Get();
    p2pNetDevice.Get(0)->SetAttribute("DataRate", ns3::StringValue(lteUpSpeed));
    p2pNetDevice.Get(1)->SetAttribute("DataRate", ns3::StringValue(lteDownSpeed));
    p2pNetDevice.Get(0)->GetChannel()->SetAttribute("Delay", lteStaticUpDelay);
    p2pNetDevice.Get(1)->GetChannel()->SetAttribute("Delay", lteStaticDownDelay);
}

void OverLTE::setPacketLoss(double packetLoss)
{
    /*auto packetLossModel = ns3::CreateObject<SequentialDeterministicErrorModel>(packetLoss);
    netDeviceContainer.Get(1)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
    netDeviceContainer.Get(0)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));*/
}

void OverLTE::setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate)
{
    /*//netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDataRate));
    p2pNetDevice.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDataRate));
    p2pNetDevice.Get(1)->SetAttribute("DataRate", ns3::StringValue(receiverDataRate));
    //netDeviceContainer.Get(0)->GetChannel()->SetAttribute("DataRate", ns3::StringValue(senderDataRate));*/


    //this->senderDatarate = senderDataRate;
    //this->receiverDatarate = receiverDataRate;
}

void OverLTE::setDelay(const std::string &delay)
{
    /*ns3::TimeValue time;
    time.DeserializeFromString(delay, ns3::MakeTimeChecker());
    defaultDelay = time.Get();

    //netDeviceContainer.Get(0)->GetChannel()->SetAttribute("Delay", time);
    p2pNetDevice.Get(0)->GetChannel()->SetAttribute("Delay", time);*/
}

ns3::StringValue OverLTE::getSendDataRate()
{
    ns3::StringValue dataRate;
    //netDeviceContainer.Get(0)->GetAttribute("DataRate", dataRate);
    return senderDatarate;
    //return dataRate;
}
//sender -> cmsa/p2p -> p2p

void OverLTE::BlockLTEConnection()
{
    //netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue("10kbps"));
    //netDeviceContainer.Get(1)->SetAttribute("DataRate", ns3::StringValue("10kbps"));
    csmaNetDevice.Get(1)->SetAttribute("ReceiveEnable", ns3::BooleanValue(false));

    auto newDelay = ns3::Seconds(delayTime->GetValue()*0.0001);
    //std::cout << ns3::Simulator::Now() << " Blocking for " << newDelay << " seconds" <<  std::endl;// << ": Sent complete packet" << std::endl;
    lteEvent = ns3::Simulator::Schedule(newDelay,
                                        &OverLTE::UnblockConnection, this);
}

void OverLTE::UnblockConnection()
{
    //netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDatarate));
    csmaNetDevice.Get(1)->SetAttribute("ReceiveEnable", ns3::BooleanValue(true));
    //netDeviceContainer.Get(1)->SetAttribute("DataRate", ns3::StringValue(receiverDatarate));
    //std::cout << ns3::Simulator::Now() << " Unblocking Sender: " << senderDatarate <<  std::endl;// << ": Sent complete packet" << std::endl;
    lteEvent = ns3::Simulator::Schedule(ns3::MilliSeconds(10),
                                        &OverLTE::BlockLTEConnection, this);
}

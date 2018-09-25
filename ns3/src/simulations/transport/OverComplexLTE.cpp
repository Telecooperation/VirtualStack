#include "../SequentialDeterministicErrorModel.h"
#include "OverComplexLTE.h"
#include <ns3/core-module.h>
#include <ns3/csma-helper.h>
#include <ns3/dce-module.h>

OverComplexLTE::OverComplexLTE()
{
    delayTime = ns3::CreateObject<ns3::NormalRandomVariable>();
    delayTime->SetAttribute ("Mean", ns3::DoubleValue (0.0065));
    delayTime->SetAttribute ("Variance", ns3::DoubleValue (0.001*0.001));
    delayTime->SetAttribute ("Bound", ns3::DoubleValue (0.003));
}

void setP2POptions(ns3::NetDeviceContainer &netDeviceContainer, ns3::StringValue &bandwidth, ns3::StringValue &delay, double packetLoss)
{
    netDeviceContainer.Get(0)->SetAttribute("DataRate", bandwidth);
    netDeviceContainer.Get(1)->SetAttribute("DataRate", bandwidth);
    netDeviceContainer.Get(0)->GetChannel()->SetAttribute("Delay", delay);
    netDeviceContainer.Get(1)->GetChannel()->SetAttribute("Delay", delay);

    auto packetLossModel = ns3::CreateObject<ns3::RateErrorModel>();
    packetLossModel->SetUnit(ns3::RateErrorModel::ERROR_UNIT_PACKET);
    packetLossModel->SetRate(packetLoss);


    auto randVar = ns3::CreateObject<ns3::UniformRandomVariable>();
    randVar->SetAttribute("Min", ns3::DoubleValue(0));
    randVar->SetAttribute("Max", ns3::DoubleValue(1));
    packetLossModel->SetRandomVariable(randVar);


    netDeviceContainer.Get(0)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
    netDeviceContainer.Get(1)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
}

void OverComplexLTE::setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver)
{
    //sender <-csma-> lteEnb <-p2p-> lteExit <p2p> cdnExit <-p2p-> receiver
    //lte:  40Mbps 0ms, Internet: 1Gbps 15ms, CDN: 10Gbps (quelle sagt 30ms, modellbedingt muss die im inet hocken)
    //wifi: 240Mbit 4ms, Internet: 15.3Mbit 30ms, CDN: 10Gbps
    //TODO: swap sender and receiver

    /*auto lteSpeed = ns3::StringValue("40Mbps");
    auto lteDelay = ns3::StringValue("30ms");
    auto lteLoss = 0;

    auto inetSpeed = ns3::StringValue("1Gbps");
    auto inetDelay = ns3::StringValue("10ms");
    auto inetLoss = 0.001;

    auto cdnSpeed = ns3::StringValue("10Gbps");
    auto cdnDelay = ns3::StringValue("1ms");
    auto cdnLoss = 0;*/
    auto lteSpeed = ns3::StringValue("10Mbps");
    auto lteDelay = ns3::StringValue("1ms");
    auto lteLoss = 0.001;

    auto inetSpeed = ns3::StringValue("10Mbps");
    auto inetDelay = ns3::StringValue("100ms");
    auto inetLoss = 0;

    auto cdnSpeed = ns3::StringValue("10Mbps");
    auto cdnDelay = ns3::StringValue("1ms");
    auto cdnLoss = 0;

    //sender <-csma-> lteEnb
    ns3::NodeContainer container;
    container.Add(sender);
    container.Create(1);
    auto lteEnb = container.Get(1);

    ns3::CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", ns3::StringValue("100Gbps"));
    csmaNetDevice = csma.Install(container);
    ns3::Names::Add("sender-csma", csmaNetDevice.Get(0));
    ns3::Names::Add("lteEnb-csma", csmaNetDevice.Get(1));

    ns3::InternetStackHelper internetStackHelper;
    internetStackHelper.Install(lteEnb);

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0");
    auto csmaInterface = ipv4.Assign(csmaNetDevice);


    //lteEnb <-p2p-> lteExit
    ns3::NodeContainer lteContainer;
    lteContainer.Add(lteEnb);
    lteContainer.Create(1);
    auto lteExit = lteContainer.Get(1);

    ns3::InternetStackHelper cdnInternetStackHelper;
    cdnInternetStackHelper.Install(lteExit);

    ns3::PointToPointHelper ltep2p;
    auto ltep2pNetDevice = ltep2p.Install(lteEnb, lteExit);
    ns3::Names::Add("lteEnb-p2p", ltep2pNetDevice.Get(0));
    ns3::Names::Add("lteExit-p2p", ltep2pNetDevice.Get(1));

    ipv4.SetBase("10.0.2.0", "255.255.255.0");
    auto ltep2pInterface = ipv4.Assign(ltep2pNetDevice);


    //lteExit <-p2p-> cdnExit
    ns3::NodeContainer testContainer;
    testContainer.Add(lteExit);
    testContainer.Create(1);
    auto cdnExit = testContainer.Get(1);

    ns3::InternetStackHelper cdnExitInternetStackHelper;
    cdnExitInternetStackHelper.Install(cdnExit);

    ns3::PointToPointHelper inetTestp2p;
    auto inetTestp2pNetDevice = inetTestp2p.Install(lteExit, cdnExit);
    //ns3::Names::Add("lteEnb-p2p", inetp2pNetDevice.Get(0));
    //ns3::Names::Add("lteExit-p2p", inetp2pNetDevice.Get(1));

    ipv4.SetBase("10.0.3.0", "255.255.255.0");
    auto inetTestp2pInterface = ipv4.Assign(inetTestp2pNetDevice);

    //lteExit <-csma-> receiver
    ns3::NodeContainer cdnContainer;
    cdnContainer.Add(cdnExit);
    cdnContainer.Add(receiver);

    ns3::PointToPointHelper cdnp2p;
    auto cdnp2pNetDevice = cdnp2p.Install(cdnExit, receiver);
    //ns3::Names::Add("lteEnb-p2p", cdnp2pNetDevice.Get(0));
    //ns3::Names::Add("lteExit-p2p", cdnp2pNetDevice.Get(1));

    netDeviceContainer.Add(csmaNetDevice.Get(0));
    netDeviceContainer.Add(cdnp2pNetDevice.Get(1));

    ipv4.SetBase("10.0.4.0", "255.255.255.0");
    auto cdnp2pInterface = ipv4.Assign(cdnp2pNetDevice);
    interfaceContainer.Add(csmaInterface.Get(0));
    interfaceContainer.Add(cdnp2pInterface.Get(1));


    ns3::Ipv4StaticRoutingHelper ipv4RoutingHelper;
    //auto staticRoutes = ipv4RoutingHelper.GetStaticRouting(inetContainer.Get(0)->GetObject<ns3::Ipv4>());
    //staticRoutes->AddHostRouteTo(ns3::Ipv4Address("10.0.1.1"), csmaNetDevice.Get(1)->GetIfIndex());

    //auto staticRoutes = ipv4RoutingHelper.GetStaticRouting(cdnContainer.Get(0)->GetObject<ns3::Ipv4>());
    //staticRoutes->AddHostRouteTo(ns3::Ipv4Address("10.0.1.1"), ns3::Ipv4Address("10.0.2.1"), inetp2pNetDevice.Get(1)->GetIfIndex());

    //staticRoutes = ipv4RoutingHelper.GetStaticRouting(cdnContainer.Get(0)->GetObject<ns3::Ipv4>());
    //staticRoutes->AddHostRouteTo(ns3::Ipv4Address("10.0.3.2"), csma2NetDevice.Get(0)->GetIfIndex());




    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    ns3::Ipv4GlobalRoutingHelper globalRouting;
    auto routingStream = ns3::Create<ns3::OutputStreamWrapper>("blah" ".routes", std::ios::out);
    globalRouting.PrintRoutingTableAllAt(ns3::Seconds(0.1), routingStream);

    lteEvent = ns3::Simulator::Schedule(ns3::Seconds(1.02),
                                       &OverComplexLTE::UnblockConnection, this);
    csma.EnablePcapAll("cmsa");
    ltep2p.EnablePcapAll("ltep2p");
    cdnp2p.EnablePcapAll("cdn");

    setP2POptions(ltep2pNetDevice, lteSpeed, lteDelay, lteLoss);
    setP2POptions(inetTestp2pNetDevice, inetSpeed, inetDelay, inetLoss);
    setP2POptions(cdnp2pNetDevice, cdnSpeed, cdnDelay, cdnLoss);

}



void OverComplexLTE::setPacketLoss(double packetLoss)
{
    auto packetLossModel = ns3::CreateObject<SequentialDeterministicErrorModel>(packetLoss);
    //netDeviceContainer.Get(1)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
    //netDeviceContainer.Get(0)->SetAttribute("ReceiveErrorModel", ns3::PointerValue(packetLossModel));
}

void OverComplexLTE::setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate)
{
    //netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDataRate));
    //p2pNetDevice.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDataRate));
    //p2pNetDevice.Get(1)->SetAttribute("DataRate", ns3::StringValue(receiverDataRate));
    //netDeviceContainer.Get(0)->GetChannel()->SetAttribute("DataRate", ns3::StringValue(senderDataRate));


    this->senderDatarate = senderDataRate;
    this->receiverDatarate = receiverDataRate;
}

void OverComplexLTE::setDelay(const std::string &delay)
{
    ns3::TimeValue time;
    time.DeserializeFromString(delay, ns3::MakeTimeChecker());
    defaultDelay = time.Get();

    //netDeviceContainer.Get(0)->GetChannel()->SetAttribute("Delay", time);
    //p2pNetDevice.Get(0)->GetChannel()->SetAttribute("Delay", time);
}

ns3::StringValue OverComplexLTE::getSendDataRate()
{
    ns3::StringValue dataRate;
    //netDeviceContainer.Get(0)->GetAttribute("DataRate", dataRate);
    return senderDatarate;
    //return dataRate;
}
//sender -> cmsa/p2p -> p2p

void OverComplexLTE::BlockLTEConnection()
{
    //netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue("10kbps"));
    //netDeviceContainer.Get(1)->SetAttribute("DataRate", ns3::StringValue("10kbps"));
    csmaNetDevice.Get(1)->SetAttribute("ReceiveEnable", ns3::BooleanValue(false));

    auto newDelay = ns3::Seconds(delayTime->GetValue());
//    std::cout << ns3::Simulator::Now() << " Blocking for " << newDelay << " seconds" <<  std::endl;// << ": Sent complete packet" << std::endl;
    lteEvent = ns3::Simulator::Schedule(newDelay,
                                        &OverComplexLTE::UnblockConnection, this);
}

void OverComplexLTE::UnblockConnection()
{
    //netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDatarate));
    csmaNetDevice.Get(1)->SetAttribute("ReceiveEnable", ns3::BooleanValue(true));
    //netDeviceContainer.Get(1)->SetAttribute("DataRate", ns3::StringValue(receiverDatarate));
//    std::cout << ns3::Simulator::Now() << " Unblocking Sender: " << senderDatarate <<  std::endl;// << ": Sent complete packet" << std::endl;
    lteEvent = ns3::Simulator::Schedule(ns3::MilliSeconds(10),
                                        &OverComplexLTE::BlockLTEConnection, this);
}

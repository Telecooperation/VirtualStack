#include "../SequentialDeterministicErrorModel.h"
#include "OverCable.h"
#include <ns3/core-module.h>

void OverCable::setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver)
{
    netDeviceContainer = p2p.Install(sender, receiver);

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0");
    interfaceContainer = ipv4.Assign(netDeviceContainer);
//    static int pcapCounter = 0;
//    p2p.EnablePcap("receiver" + std::to_string(pcapCounter++), netDeviceContainer.Get(1));
}
void OverCable::setPacketLoss(double packetLoss)
{
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

void OverCable::setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate)
{
    netDeviceContainer.Get(0)->SetAttribute("DataRate", ns3::StringValue(senderDataRate));
    netDeviceContainer.Get(1)->SetAttribute("DataRate", ns3::StringValue(receiverDataRate));
}

void OverCable::setDelay(const std::string &delay)
{
    netDeviceContainer.Get(0)->GetChannel()->SetAttribute("Delay", ns3::StringValue(delay));
    netDeviceContainer.Get(1)->GetChannel()->SetAttribute("Delay", ns3::StringValue(delay));
}

ns3::StringValue OverCable::getSendDataRate()
{
    ns3::StringValue dataRate;
    netDeviceContainer.Get(0)->GetAttribute("DataRate", dataRate);
    return dataRate;
}

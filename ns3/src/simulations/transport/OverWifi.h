#pragma once


#include "BaseOverTransport.h"
#include <ns3/wifi-module.h>
#include <ns3/mobility-module.h>

class OverWifi final : public BaseOverTransport
{
public:
    void setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver) override;

    void setPacketLoss(double packetLoss) override;

    void setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate) override;

    void setDelay(const std::string &delay) override;

    ns3::StringValue getSendDataRate() override;

private:
    ns3::WifiMacHelper wifiMac;
    ns3::WifiHelper wifiHelper;
    ns3::YansWifiChannelHelper wifiChannel;
    ns3::YansWifiPhyHelper wifiPhy;
    ns3::MobilityHelper mobility;

    ns3::NetDeviceContainer apDevice;
    ns3::NetDeviceContainer staDevices;
    ns3::NetDeviceContainer p2pDevices;

    ns3::StringValue sendDataRate;
};



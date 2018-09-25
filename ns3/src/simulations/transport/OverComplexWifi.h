#pragma once


#include "BaseOverTransport.h"
#include <ns3/wifi-module.h>
#include <ns3/mobility-module.h>
#include <ns3/point-to-point-helper.h>

class OverComplexWifi final : public BaseOverTransport
{
public:
    OverComplexWifi();

    void setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver) override;

    void setPacketLoss(double packetLoss) override;

    void setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate) override;

    void setDelay(const std::string &delay) override;

    ns3::StringValue getSendDataRate() override;

private:
    ns3::PointToPointHelper p2p;
    ns3::NetDeviceContainer p2pNetDevice;
    ns3::NetDeviceContainer csmaNetDevice;
    ns3::EventId lteEvent;

    const size_t updateFrequencyPerMs = 1;
    ns3::Ptr<ns3::RandomVariableStream>  delayTime;
    ns3::Time defaultDelay;

    void BlockLTEConnection();

    void UnblockConnection();

    std::string senderDatarate, receiverDatarate;



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



#pragma once


#include <ns3/point-to-point-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "BaseOverTransport.h"

class OverLTEReal final : public BaseOverTransport
{
public:
    void setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver) override;

    void setPacketLoss(double packetLoss) override;

    void setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate) override;

    void setDelay(const std::string &delay) override;

    ns3::StringValue getSendDataRate() override;

private:
    inline uint32_t getLteInterface(ns3::Ptr<ns3::Node> node);

    ns3::StringValue sendDataRate;
    ns3::PointToPointHelper p2p;
    ns3::Ipv4AddressHelper ipv4;

    ns3::Ptr<ns3::LteHelper> lteHelper;
    ns3::Ptr<ns3::PointToPointEpcHelper> epcHelper;
    ns3::NetDeviceContainer lteGateway_gatewayPartner;

    ns3::NodeContainer ueNodes;
    ns3::NodeContainer enbNodes;

    ns3::MobilityHelper mobility;
    ns3::NetDeviceContainer enbLteDevs;
    ns3::NetDeviceContainer ueLteDevs;
};



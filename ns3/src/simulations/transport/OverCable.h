#pragma once


#include <ns3/point-to-point-module.h>
#include "BaseOverTransport.h"

class OverCable final : public BaseOverTransport
{
public:
    void setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver) override;

    void setPacketLoss(double packetLoss) override;

    void setDataRate(const std::string &senderDataRate, const std::string &receiverDataRate) override;

    void setDelay(const std::string &delay) override;

    ns3::StringValue getSendDataRate() override;

private:
    ns3::PointToPointHelper p2p;
};



#pragma once


#include <ns3/point-to-point-module.h>
#include <ns3/csma-helper.h>
#include "BaseOverTransport.h"

class OverComplexLTE final : public BaseOverTransport
{
public:
    OverComplexLTE();

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
};



#pragma once


#include <ns3/net-device-container.h>
#include <ns3/string.h>
#include <ns3/internet-module.h>

class BaseOverTransport
{
public:
    virtual ~BaseOverTransport();
    virtual void setup(ns3::Ptr<ns3::Node> sender, ns3::Ptr<ns3::Node> receiver) = 0;

    virtual void setPacketLoss(double packetLoss) = 0;
    virtual void setDataRate(const std::string &senderDataRate,
                             const std::string &receiverDataRate) = 0;
    virtual void setDelay(const std::string &delay) = 0;

    virtual ns3::StringValue getSendDataRate() = 0;
    ns3::NetDeviceContainer& getNetDeviceContainer();
    ns3::Ipv4InterfaceContainer& getInterfaceContainer();

protected:
    ns3::NetDeviceContainer netDeviceContainer;
    ns3::Ipv4InterfaceContainer interfaceContainer;
};



#pragma once


#include <ns3/error-model.h>
#include <ns3/packet.h>

class SequentialDeterministicErrorModel : public ns3::ErrorModel
{
public:
    explicit SequentialDeterministicErrorModel(double packetLoss);

private:
    bool DoCorrupt(ns3::Ptr<ns3::Packet> p) override;
    void DoReset() override;

private:
    int packetToDrop;
    int packetCounter;
};



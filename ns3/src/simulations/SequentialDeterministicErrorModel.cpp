#include "SequentialDeterministicErrorModel.h"

SequentialDeterministicErrorModel::SequentialDeterministicErrorModel(double packetLoss) : packetToDrop(-1), packetCounter(0)
{
    if(packetLoss > 0)
        packetToDrop = static_cast<int>(1.0 / packetLoss);
}

bool SequentialDeterministicErrorModel::DoCorrupt(ns3::Ptr<ns3::Packet> p)
{
    if(packetToDrop == -1) //do not drop packets
        return false;

    ++packetCounter;
    auto dropPacket = packetCounter == packetToDrop;
    if(dropPacket)
        packetCounter = 0;
    return dropPacket;
}

void SequentialDeterministicErrorModel::DoReset()
{
    packetCounter = 0;
}

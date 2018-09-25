#pragma once

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <memory>
#include <LatencyMeter.h>

class SimulationRunCapture
{
public:
    //liste von sentPackets mit Zeit
    //liste von empfangenenPackets mit Zeit

    //1300\t0.5s\t0,7
    //1300\t0,5s\t0,8

    explicit SimulationRunCapture();

    void setPacketSize(uint32_t packetSize);
    void setRuntime(size_t seconds);

    static void addSent(SimulationRunCapture* obj, size_t seqNum, ns3::Time time);
    static void addReceived(SimulationRunCapture* obj,
                            ns3::Ptr<const ns3::Packet> packet,
                            ns3::Time,
                            const ns3::Address &address);

    void saveToFile(const std::string& fullFilename);

    ns3::Callback<void, size_t, ns3::Time> getAddSentCallback();
    ns3::Callback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&> getAddReceivedCallback();
private:
    size_t _packetSize;

//    std::vector<std::pair<ns3::Time, ns3::Time>> sendRecvList;
    ns3::Time _connectionStart;
    ns3::Time _connectionEstablished;
    std::map<size_t, ns3::Time> _sendList;
    std::map<size_t, ns3::Time> _recvList;

    std::unique_ptr<uint8_t[]> _packetBuffer;
    uint32_t _packetBufferSize;

    LatencyMeter _meter;
    size_t _meterSendKey;
    size_t _runtime;
};



#include <memory>
#include "SimulationRunCapture.h"

SimulationRunCapture::SimulationRunCapture() :
        _packetSize(0),
        _connectionStart(-1),
        _connectionEstablished(-1),
        _sendList(),
        _recvList(),
        _packetBufferSize(0),
        _meter(0),
        _meterSendKey(0)
{

}


void SimulationRunCapture::addSent(SimulationRunCapture* obj, size_t seqNum, ns3::Time time)
{
//    std::cout << "S:" << time << ": " << seqNum << std::endl;
    if(seqNum == 0)
    {
        if(obj->_connectionStart.IsNegative())
            obj->_connectionStart = time;
        else
            obj->_connectionEstablished = time;
        return;
    }

    //obj->_sendList.emplace(seqNum, time);
    obj->_meter.addInTime(obj->_meterSendKey, static_cast<size_t>(time.GetNanoSeconds()));
}

void SimulationRunCapture::addReceived(SimulationRunCapture* obj,
                                       ns3::Ptr<const ns3::Packet> packet,
                                       ns3::Time time,
                                       const ns3::Address &address)
{
    auto packetCopy = packet->Copy();
    uint32_t bytesLeft = 0;
    do
    {
        uint32_t bufferRemainSize = static_cast<uint32_t>(obj->_packetSize - obj->_packetBufferSize);

        uint32_t packetSize = packetCopy->GetSize();
        uint32_t toReadBytes = std::min(bufferRemainSize, packetSize);

        bytesLeft = packetSize - toReadBytes;

        packetCopy->CopyData(obj->_packetBuffer.get() + obj->_packetBufferSize, toReadBytes);
        packetCopy->RemoveAtStart(toReadBytes);

        obj->_packetBufferSize += toReadBytes;

        if(obj->_packetBufferSize == obj->_packetSize)
        {
            obj->_packetBufferSize = 0;
            size_t seqNum = 0;
            memcpy(&seqNum, obj->_packetBuffer.get(), sizeof(seqNum));
//            for(size_t i=0; i<obj->_packetSize; ++i)
//            {
//                std::cout << static_cast<int>(obj->_packetBuffer[i]);
//            }
//            std::cout << std::endl;
            //obj->_recvList.emplace(seqNum, time);
            obj->_meter.addOutTime(static_cast<size_t>(time.GetNanoSeconds()));
        }

    } while(bytesLeft > 0);
}

ns3::Callback<void, size_t, ns3::Time> SimulationRunCapture::getAddSentCallback()
{
    return ns3::MakeBoundCallback(&SimulationRunCapture::addSent, this);
}

ns3::Callback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address &> SimulationRunCapture::getAddReceivedCallback()
{
    return ns3::MakeBoundCallback(&SimulationRunCapture::addReceived, this);
}

void SimulationRunCapture::saveToFile(const std::string &fullFilename)
{
    //1300\t0.5s\t0,7
    //1300\t0,5s\t0,8

    auto latencyResults = _meter.analyse(_runtime);
    std::ofstream bargraphFile{fullFilename + ".vsbar"};

    latencyResults[0]->dumpBargraphMetadata(bargraphFile, 1000);
    bargraphFile.close();

    std::ofstream metadataFile{fullFilename + ".vsmeta"};
    latencyResults[0]->printValues(metadataFile, "\t");
    latencyResults[0]->printAsMicroSeconds(metadataFile);
    metadataFile.close();

    /*std::ofstream tmpFile(fullFilename);
    tmpFile << R"("packetSize","seqNum","sendTime","recvTime")" << std::endl;
    tmpFile << "\"" << 0 << R"(",")" << 0 << R"(",")" << _connectionStart.GetNanoSeconds()  <<R"(",")" << _connectionEstablished.GetNanoSeconds() << "\"" << std::endl;
    for(const auto& el : _sendList)
    {
        tmpFile << "\"" << _packetSize << R"(",")" << el.first << R"(",")" << el.second.GetNanoSeconds() << R"(",")"
                << _recvList[el.first].GetNanoSeconds() << "\"" << std::endl;
    }
    tmpFile.close();*/
}

void SimulationRunCapture::setPacketSize(uint32_t packetSize)
{
    _packetSize = packetSize;
    _packetBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[packetSize]{});
    _packetBufferSize = 0;
    _meter = LatencyMeter{_packetSize};
    _meter.clearInTime();
    _meterSendKey = _meter.getNewInTime("toNorthbound");
}

void SimulationRunCapture::setRuntime(size_t seconds)
{
    _runtime = seconds;
}

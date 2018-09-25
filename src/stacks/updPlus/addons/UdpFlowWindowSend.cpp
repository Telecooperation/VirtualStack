#include "../../../common/Helper/Logger.h"
#include "UdpFlowWindowSend.h"

UdpFlowWindowSend::UdpFlowWindowSend(const VirtualStackSettings& settings, size_t freeSlotsCount, size_t controlPacketReservationSize) :
        _controlPacketReservationSize(controlPacketReservationSize),
        _sentPackets(0),
        _sendQuota(freeSlotsCount),
        _lastRemoteReceivedCount(0)
{}

bool UdpFlowWindowSend::sendDataAllowed() const
{
    return _sendQuota > _controlPacketReservationSize;
}

bool UdpFlowWindowSend::sendControlAllowed() const
{
    return _sendQuota > 0;
}

void UdpFlowWindowSend::sentOnePacket()
{
    if(_sendQuota == 0)
    {
        Logger::Log(Logger::DEBUG, "UdpFlowWindow: sent a packet but the quota didnt allow it");
        return;
    }

    --_sendQuota;
    ++_sentPackets;
}

void UdpFlowWindowSend::updateSendQuota(Storage& storage)
{
    if(storage.size() < 2*sizeof(size_t))
    {
        Logger::Log(Logger::WARNING, "UpdFlowWindow malformed");
        return;
    }

    auto maxReceiveWindowSize = storage.toTypeAutomatic<size_t>();
    auto recvReleasedCount = storage.toTypeAutomatic<size_t>(sizeof(size_t));
    storage.incrementStartIndex(sizeof(size_t) * 2);

    if(recvReleasedCount < _lastRemoteReceivedCount)
        return;

    if(recvReleasedCount > _sentPackets)
    {
        Logger::Log(Logger::ERROR, std::this_thread::get_id(), ": UdpFlowWindow: Received invalid flowWindowUpdate. ReleaseCount > sentPackets");
        return;
    }

    _lastRemoteReceivedCount = recvReleasedCount;
    auto diff = _sentPackets - recvReleasedCount;
    _sendQuota = maxReceiveWindowSize - diff;
}
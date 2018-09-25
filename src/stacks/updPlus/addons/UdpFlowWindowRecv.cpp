#include "../../../common/Helper/Logger.h"
#include "UdpFlowWindowRecv.h"

UdpFlowWindowRecv::UdpFlowWindowRecv(const VirtualStackSettings& settings, size_t maxFreeSlotsCount,
                             size_t freeSlotsCount) :
        _maxFreeSlotsCount(maxFreeSlotsCount),
        _lastFreeSlotsCount(freeSlotsCount),
        _lastReceivedCount(0),
        _releasedSlotsCounter(0),
        _receivedCounter(0)
{}

void UdpFlowWindowRecv::receiveOnePacket()
{
    ++_receivedCounter;
}

void UdpFlowWindowRecv::storeWindowUpdate(Storage& storage, size_t freeSlotsCount)
{
    if(storage.freeSpaceForAppend() < 2*sizeof(size_t))
        return;

    ssize_t diffFreeSlots = static_cast<ssize_t>(freeSlotsCount) - static_cast<ssize_t>(_lastFreeSlotsCount);
    ssize_t diffReceived = static_cast<ssize_t>(_receivedCounter) - static_cast<ssize_t>(_lastReceivedCount);
    ssize_t currentReleaseCount = diffFreeSlots + diffReceived;
    if (currentReleaseCount < 0) //discard snapshot
    {
        Logger::Log(Logger::ERROR, "UdpFlowWindow->storeWindowUpdate: currentReleaseCount was negative");
        return;
    }

    _releasedSlotsCounter += static_cast<size_t>(currentReleaseCount);

    storage.appendDataScalarAfterEnd(_maxFreeSlotsCount);
    storage.appendDataScalarAfterEnd(_releasedSlotsCounter);

    _lastReceivedCount = _receivedCounter;
    _lastFreeSlotsCount = freeSlotsCount;
}

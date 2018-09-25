#pragma once

#include "../../../VirtualStackSettings.h"
#include "../../../common/DataStructures/Model/Storage.h"

class UdpFlowWindowRecv
{
public:
    explicit UdpFlowWindowRecv(const VirtualStackSettings& settings,
                           size_t maxFreeSlotsCount,
                           size_t freeSlotsCount);

    void receiveOnePacket();
    void storeWindowUpdate(Storage& storage, size_t freeSlotsCount);

private:
    //recv-side
    const size_t _maxFreeSlotsCount;
    size_t _lastFreeSlotsCount;
    size_t _lastReceivedCount;
    size_t _releasedSlotsCounter;
    size_t _receivedCounter;
};



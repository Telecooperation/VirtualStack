#pragma once

#include "../../../VirtualStackSettings.h"
#include "../../../common/DataStructures/Model/Storage.h"

class UdpFlowWindowSend
{
public:
    explicit UdpFlowWindowSend(const VirtualStackSettings& settings, size_t freeSlotsCount,
                           size_t controlPacketReservationSize = 0);

    bool sendDataAllowed() const;
    bool sendControlAllowed() const;

    void sentOnePacket();
    void updateSendQuota(Storage& storage);
private:
    //send-side
    const size_t _controlPacketReservationSize;
    size_t _sentPackets;
    size_t _sendQuota;
    size_t _lastRemoteReceivedCount;
};



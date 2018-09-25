#pragma once

#include "StopWatch.h"
#include <chrono>
#include <cmath>
#include <cstddef>

class BandwidthLimiter
{
public:
    explicit BandwidthLimiter(double dataRateInMbps, const size_t bytesPerPacket) :
            _watch(),
            _timeElapseNs(calcTimePerPacket(dataRateInMbps, bytesPerPacket)),
            _timeElapseTimePoint(std::chrono::nanoseconds(_timeElapseNs))
    {
    }

    bool canSend()
    {
        return _watch.stop().hasElapsed(_timeElapseTimePoint);
    }

    void hasSent()
    {
        _watch.addToStart(_timeElapseTimePoint);
    }

private:
    inline size_t calcTimePerPacket(double dataRateInMbps, const size_t bytesPerPacket)
    {
        //calculate in BytePerSecond as on hasSent will sent a bytesPerPacket size of payload
        const auto dataRateInMBps = dataRateInMbps / 8.0;
        const auto packetsPerSecond = dataRateInMBps / bytesPerPacket * 1024.0 * 1024.0;
        const auto nanosecondsPerPacket = 1e9 / packetsPerSecond;
        return static_cast<size_t>(std::ceil(nanosecondsPerPacket));
    }

    StopWatch _watch;
    const size_t _timeElapseNs;
    const std::chrono::nanoseconds _timeElapseTimePoint;
};
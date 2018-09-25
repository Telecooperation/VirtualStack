#pragma once


#include "../../../common/DataStructures/VS/IStack.h"

class MetricMiddleware
{
public:
    struct MetricData
    {
        size_t packetsToStack = 0;
        size_t packetsOnHold = 0;

        void oneSent(size_t stackSentBufferSize)
        {
            ++packetsToStack;
            packetsOnHold += stackSentBufferSize;
        }
    };

    void addToSend(const Storage& storage, const IStack& stack);
private:
    std::map<size_t, MetricData> _metrics;
};



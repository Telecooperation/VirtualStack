#pragma once

#include <cstddef>

struct MetricData
{
    size_t packetsToStack = 0;

    void oneSent()
    {
        ++packetsToStack;
    }
};
#pragma once

#include "../../interface/IScheduler.h"

class WeightedRoundRobinScheduler final : public IScheduler
{
public:
    explicit WeightedRoundRobinScheduler(const FixedSizeArray<StackDetails>& stackDetails);

    void process(const Storage &packet) override;
private:
    const uint8_t _firstStackWeight;
    uint8_t _currentWeight;
};



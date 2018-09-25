#pragma once

#include "../../interface/IScheduler.h"

class RoundRobinScheduler final : public IScheduler
{
public:
    explicit RoundRobinScheduler(const FixedSizeArray<StackDetails>& stackDetails);

    void process(const Storage &packet) override;
};



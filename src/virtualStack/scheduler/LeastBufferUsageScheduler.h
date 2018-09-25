#pragma once

#include "../../interface/IScheduler.h"

class LeastBufferUsageScheduler final : public IScheduler
{
public:
    explicit LeastBufferUsageScheduler(const FixedSizeArray<StackDetails>& stackDetails);

    void process(const Storage &packet) override;
};



#pragma once

#include "../../interface/IScheduler.h"

class PassThroughScheduler final : public IScheduler
{
public:
    explicit PassThroughScheduler(const FixedSizeArray<StackDetails>& stackDetails);

    void process(const Storage &packet) override;
};



#pragma once

#include "MetricData.h"
#include "../common/DataStructures/VS/IStack.h"

class StackDetails
{
public:
    bool isInactive;
    MetricData metricData;

    explicit StackDetails(std::unique_ptr<IStack> &&pStack, size_t pStackId) :
            isInactive(false),
            metricData(),
            _stack(std::move(pStack)),
            stackId(pStackId)
    {

    }

    StackDetails() : isInactive(true), metricData(), _stack(), stackId(0)
    {

    }

    bool isValid()
    {
        return _stack != nullptr;
    }

    IStack& get()
    {
        return *_stack;
    }

    const IStack& get() const
    {
        return *_stack;
    }

    size_t getStackId() const
    {
        return stackId;
    }

    ALLOW_MOVE_SEMANTICS_ONLY(StackDetails);
private:
    std::unique_ptr<IStack> _stack;
    size_t stackId;
};
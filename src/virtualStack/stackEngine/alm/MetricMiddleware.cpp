#include "MetricMiddleware.h"

void MetricMiddleware::addToSend(const Storage &storage, const IStack &stack)
{
    auto it = _metrics.find(stack.getStackId());
    if(it == _metrics.end())
    {
        _metrics.emplace(std::pair<size_t, MetricData>(stack.getStackId(), {}));
    }
    it->second.oneSent(stack.getPushFreeSlots());
}

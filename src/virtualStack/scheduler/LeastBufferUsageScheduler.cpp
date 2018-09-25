#include "LeastBufferUsageScheduler.h"

LeastBufferUsageScheduler::LeastBufferUsageScheduler(const FixedSizeArray<StackDetails>& stackDetails) : IScheduler(stackDetails)
{}

void LeastBufferUsageScheduler::process(const Storage &packet)
{
    auto val = std::min_element(_stackDetails.begin(), _stackDetails.end(), [](const StackDetails& left, const StackDetails& right)
    {
        if(left.isInactive)
            return false;
        if(right.isInactive)
            return true;

        return left.get().getPushSlotsInUse() < right.get().getPushSlotsInUse();
    });

    _activeStackIndex = static_cast<size_t>(std::distance(_stackDetails.begin(), val));
}

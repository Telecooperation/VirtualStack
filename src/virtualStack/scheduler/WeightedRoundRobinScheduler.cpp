#include "WeightedRoundRobinScheduler.h"

WeightedRoundRobinScheduler::WeightedRoundRobinScheduler(const FixedSizeArray<StackDetails>& stackDetails) :
        IScheduler(stackDetails),
        _firstStackWeight(4)
{}

void WeightedRoundRobinScheduler::process(const Storage &packet)
{
    skipInactiveStacks();

    if(_activeStackIndex == 0)
    {
        ++_currentWeight;
        if(_currentWeight > _firstStackWeight)
        {
            ++_activeStackIndex;
            _currentWeight = 0;
        }
    }
    else
    {
        ++_activeStackIndex;
    }
}



#include "RoundRobinScheduler.h"

RoundRobinScheduler::RoundRobinScheduler(const FixedSizeArray<StackDetails>& stackDetails) : IScheduler(stackDetails)
{}

void RoundRobinScheduler::process(const Storage &packet)
{
    ++_activeStackIndex; //sanatizing and skipping inactive stacks is done by baseclass
}

#include "PassThroughScheduler.h"

PassThroughScheduler::PassThroughScheduler(const FixedSizeArray<StackDetails>& stackDetails) : IScheduler(stackDetails)
{}

void PassThroughScheduler::process(const Storage &packet)
{
}
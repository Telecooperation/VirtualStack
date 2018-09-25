#include "IScheduler.h"

IScheduler::IScheduler(const FixedSizeArray<StackDetails> &stackDetails) :
        _stackDetails(stackDetails),
        _activeStackIndex(0)
{}

IScheduler::~IScheduler()
{

}

size_t IScheduler::getActiveStack()
{
    skipInactiveStacks();
    return _activeStackIndex;
}



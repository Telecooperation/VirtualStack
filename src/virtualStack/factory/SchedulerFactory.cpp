#include "SchedulerFactory.h"
#include "../stackEngine/StackEngine.h"
#include "../scheduler/PassThroughScheduler.h"
#include "../scheduler/RoundRobinScheduler.h"
#include "../scheduler/WeightedRoundRobinScheduler.h"
#include "../scheduler/LeastBufferUsageScheduler.h"

UniquePtrArray<IScheduler> SchedulerFactory::createSchedulerList(const StackEngine& stackEngine)
{
    //#####################
    //ACHTUNG: Änderungen der Reihenfolge müssen auch in SchedulerTypeEnum gemacht werden
    //ACHTUNG: Änderungen der Reihenfolge müssen auch in SchedulerTypeEnum gemacht werden
    //ACHTUNG: Änderungen der Reihenfolge müssen auch in SchedulerTypeEnum gemacht werden
    //#####################

    auto &stacks = stackEngine.getStacks();
    return
            {
                    std::make_unique<LeastBufferUsageScheduler>(stacks),
                    std::make_unique<WeightedRoundRobinScheduler>(stacks),
                    std::make_unique<RoundRobinScheduler>(stacks),
                    std::make_unique<PassThroughScheduler>(stacks)
            };
}

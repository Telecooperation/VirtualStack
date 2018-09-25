#pragma once


#include "../../VirtualStackSettings.h"
#include "../../common/DataStructures/Container/UniquePtrArray.h"
#include "../../interface/IScheduler.h"

class StackEngine;

class SchedulerFactory
{
public:
	static UniquePtrArray<IScheduler> createSchedulerList(const StackEngine& stackEngine);
};



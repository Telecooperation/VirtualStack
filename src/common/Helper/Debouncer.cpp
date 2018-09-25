#include "../../VirtualStackSettings.h"
#include "Debouncer.h"
#include "Logger.h"
#include <chrono>
#include <thread>

Debouncer::Debouncer(UtilizationPlan plan, const VirtualStackSettings &settings) :
        _threadholdSleep(getThreshold(plan, settings)),
        _sleepInNanoseconds(settings.DebouncerSleepInNanoseconds),
        _counter(0)
{}

void Debouncer::sleep()
{
	++_counter;
	if (_counter > _threadholdSleep)
		std::this_thread::sleep_for(std::chrono::nanoseconds(_sleepInNanoseconds));
}

void Debouncer::reset()
{
	_counter = 0;
}

size_t Debouncer::getThreshold(UtilizationPlan plan, const VirtualStackSettings &settings)
{
    switch (plan)
	{
		case UtilizationPlan::Low:
			return settings.UtilizationPlanLowThreshold;
		case UtilizationPlan::Mid:
			return settings.UtilizationPlanMidThreshold;
		case UtilizationPlan::High:
			return settings.UtilizationPlanHighThreshold;
		case UtilizationPlan::Invalid:
		{
			Logger::Log(Logger::ERROR, "Debouncer::getThreshold called with plan UtilizationPlan::Invalid");
			return settings.UtilizationPlanLowThreshold;
		}
	}

    Logger::Log(Logger::ERROR, "Debouncer::getThreshold called with non exiting plan");
    return settings.UtilizationPlanLowThreshold;
}

#pragma once

#include "ClassMacros.h"
#include "UtilizationPlan.h"
#include <cstdint>
#include <cstdio>

class VirtualStackSettings;

/**
 * Debounce thread sleeps that dont do anything
 */
class Debouncer final
{
private:
public:
    explicit Debouncer(UtilizationPlan plan, const VirtualStackSettings& settings);
	
	void sleep();
	void reset();
	
	ALLOW_MOVE_SEMANTICS_ONLY(Debouncer);
private:
    static size_t getThreshold(UtilizationPlan plan, const VirtualStackSettings& settings);
	const size_t _threadholdSleep;
	const size_t _sleepInNanoseconds;
	size_t _counter;
};
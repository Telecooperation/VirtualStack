#pragma once

#include "ClassMacros.h"
#include <chrono>

class StopWatch
{
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point stopTime;

public:
	StopWatch() : startTime(std::chrono::high_resolution_clock::now()), stopTime(startTime) {}
	StopWatch& start()
	{
		startTime = std::chrono::high_resolution_clock::now();
		return *this;
	}

	StopWatch& stop(){
		stopTime = std::chrono::high_resolution_clock::now();
		return *this;
	}

    template <typename T>
    void addToStart(const T& timePoint)
    {
        startTime += timePoint;
    }

	double getDifferenceInSeconds() const
	{
		return getDifferenceInNanoSeconds() / 1e9;
	}
	
	long getDifferenceInNanoSeconds() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
				stopTime - startTime).count();
	}

    template <typename T>
	bool hasElapsed(const T& timePoint) const
	{
		return (stopTime - startTime) >= timePoint;
	}

	float progress(const std::chrono::high_resolution_clock::duration& timePoint) const
	{
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint - (stopTime - startTime)).count();
//		Logger::Log(Logger::DEBUG, duration, ", tp:", timePoint.count(), ", diff:", (stopTime - startTime).count());
		if (duration <= 0)
			return 1.0f;

		return 1.0f - (static_cast<float>(duration) / timePoint.count());
	}
	
	inline static long getHighResolutionTime()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}

	ALLOW_MOVE_SEMANTICS_ONLY(StopWatch);
};

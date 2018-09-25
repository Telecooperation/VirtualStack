
#pragma once

#include <time.h>

/**
 * Functions for DateTime access
 */
class DateTime
{
public:
	/**
	 * clock_gettime(CLOCK_MONOTONIC_COARSE) costs about 9 clock-cycles based on benchmark: https://stackoverflow.com/questions/6498972/faster-equivalent-of-gettimeofday
	 */
	static constexpr clockid_t CLOCK_TYPE = CLOCK_MONOTONIC_COARSE;

	/**
	 * Get the current time in seconds based on unix-timestamp (UTC). Costs about 3 clock-cycles.
	 * Attention: Is not monotone because of system-time changes caused by for example ntp
	 * @return The current time in seconds
	 */
	static long NowInSec()
	{
		time_t t = time(nullptr);
		return t;
	}

	/**
	 * Get a monotonic relative time value in seconds. The reference for "Relative" is system dependent. Costs about 9 clock-cycles
	 * @return A monotonic value which changes every second
	 */
	static long RelativeTimeInSec()
	{
		timespec time;
		clock_gettime(CLOCK_TYPE, &time);
		return time.tv_sec;
	}

	/**
	 * Get a monotonic relative time value in nanosecond. The reference for "Relative" is system dependent. Costs about 9 clock-cycles
	 * @return A monotonic value which changes every nanosecond
	 */
	static long RelativeTimeInNanoSec()
	{
		timespec time;
		clock_gettime(CLOCK_TYPE, &time);
		return time.tv_nsec;
	}

	static long RelativeTime()
	{
		return RelativeTimeInSec() * 1000000000 + RelativeTimeInNanoSec();
	}

	/**
	 * Get the precision of the relativeTime functions in nanoseconds
	 * @return The precision in nanoseconds
	 */
	static long RelativeTimeResolutionInNanoSec()
	{
		timespec time;
		clock_getres(CLOCK_TYPE, &time);
		return time.tv_nsec;
	}
};

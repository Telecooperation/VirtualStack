#pragma once

#include <cstdint>

//per definition enums start with 0 and the next enum value is lastEnumValue + 1
//dont change the values as they are directly used for array index access
enum class SchedulerTypeEnum : uint8_t
{
    LeastBufferUsage = 0,
    WeightedRoundRobin,
    RoundRobin,
    PassThrough,
};
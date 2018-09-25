#pragma once

#include <cstdint>

enum class RemoteControlResultEnum : uint8_t
{
	Ok,
    Failed,
	StackNotFound,
    StackEngineNotFound
};
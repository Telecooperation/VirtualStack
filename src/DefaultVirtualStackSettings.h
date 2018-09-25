
#pragma once

#include "VirtualStackSettings.h"
#include "common/Helper/SettingsStreamFactory.h"

class DefaultVirtualStackSettings
{
public:
    static std::unique_ptr<VirtualStackSettings> Default(StackEnum defaultStack = StackEnum::TCPIPv4, bool isLocalhost = false, bool isRouter = false);

    static VirtualStackSettings Default2(StackEnum defaultStack = StackEnum::TCPIPv4, bool isLocalhost = false, bool isRouter = false);

    static std::unique_ptr<VirtualStackSettings> Default(const SettingsStreamFactory& factory);

    static SettingsStreamFactory DefaultFactory(StackEnum defaultStack = StackEnum::TCPIPv4, bool isLocalhost = false, bool isRouter = false);
};


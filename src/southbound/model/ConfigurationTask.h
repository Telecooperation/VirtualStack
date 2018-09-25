#pragma once

#include "../configuration/base/BaseSouthboundConfiguration.h"
#include <future>

struct ConfigurationTask
{
public:
    const sockaddr_storage destination;
    std::unique_ptr<BaseSouthboundConfiguration> configuration;
    std::promise<ConfigurationState> promise;

    ConfigurationTask(const sockaddr_storage &dest,
                      std::unique_ptr<BaseSouthboundConfiguration> config) :
            destination(dest), //make copy to prevent changes because of later changes
            configuration(std::move(config)),
            promise()
    {
        configuration->linkPromise(promise);
    }

    bool hasStarted() const
    {
        if (configuration == nullptr)
            return true;

        return configuration->getState() != ConfigurationState::NotInitialized;
    }

    bool isFinished() const
    {
        if (configuration == nullptr)
            return true;

        return configuration->isFinished();
    }

    bool hasFailed() const
    {
        if (configuration == nullptr)
            return true;

        return isFinished() && !configuration->isValid();
    }

    void cancel()
    {
        if (configuration != nullptr)
            configuration->stop();
        promise.set_value(ConfigurationState::Canceled);
    }

    ALLOW_MOVE_SEMANTICS_ONLY(ConfigurationTask);
};
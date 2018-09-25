#pragma once

#include "../../../VirtualStackSettings.h"
#include "BaseSouthboundConfiguration.h"
#include <functional>
#include <future>
#include <sys/socket.h>

/**
 * Extended baseClass for a new configuration
 * It adds start() and implementation method
 * @tparam IResult The result of a configuration
 */
template <typename IResult = void>
class ExtendedSouthboundConfiguration : public BaseSouthboundConfiguration
{
public:
	explicit ExtendedSouthboundConfiguration(const NetworkMessageEnum networkMessageEnum) :
            BaseSouthboundConfiguration(networkMessageEnum)
    {}

	void start(const sockaddr_storage& managementSource, const sockaddr_storage& managementDestination) override
	{
		//if it is running, ignore start()
		if (_runFuture.valid())
			return;

        source = managementSource;
        destination = managementDestination;

        _state = ConfigurationState::Running;
		_runFuture = std::async(std::launch::async, &ExtendedSouthboundConfiguration::runConfiguration, this);
	}

protected:
    sockaddr_storage source;
    sockaddr_storage destination;

	std::future<void> _runFuture;

	virtual IResult run() = 0;
    virtual void onComplete(IResult&& result) = 0;

	void runConfiguration()
	{
		auto result = run();
		onComplete(std::move(result));
		if(_linkedPromise != nullptr)
		    _linkedPromise->set_value(_state);
		_onCompleteDone = true;
	}
};
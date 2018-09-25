#include "../VirtualStackSettings.h"
#include "../common/Allocator/VsObjectFactory.h"
#include "../common/Helper/DomainExtensions.h"
#include "SouthboundControl.h"
#include "configuration/NewStack/Request/NewStackRequest.h"
#include "configuration/NewStack/Request/NewStackRequestConfiguration.h"

SouthboundControl::SouthboundControl(const SouthboundCallbacks& callbacks,
                                     VsObjectFactory& vsObjectFactory,
                                     const VirtualStackSettings& settings) :
        _callbacks(callbacks),
        _socketFactory(*vsObjectFactory.socketFactory),
        _settings(settings),
        _controller(callbacks, vsObjectFactory, settings)
{}

void SouthboundControl::start()
{
    _controller.start();
}

void SouthboundControl::stop()
{
    _controller.stop();
}

ConfigurationState SouthboundControl::getState() const
{
    return _controller.getCurrentState();
}

std::future<ConfigurationState> SouthboundControl::requestCreateStack(const sockaddr_storage &nextHop,
                                                                      std::unique_ptr<NewStackRequest> &&request)
{
    auto newStackConfiguration = std::make_unique<NewStackRequestConfiguration>(
            std::move(request),
            _callbacks.OnCreateNewStackRequest,
            _socketFactory,
            _settings);

    return _controller.addConfiguration(nextHop, std::move(newStackConfiguration));
}

std::unique_ptr<NewStackRequest> SouthboundControl::createNewStackRequest(const sockaddr_storage &northboundDestination,
                                                                          const uint16_t northboundSourcePort,
                                                                          const uint16_t northboundDestinationPort,
                                                                          const TransportProtocolEnum northboundProtocol,
                                                                          const StackEnum stack,
                                                                          const flowid_t* overrideFlowId)
{
    auto stackRequest = std::make_unique<NewStackRequest>();
    stackRequest->northboundSourcePort = northboundSourcePort;
    stackRequest->northboundDestinationPort = northboundDestinationPort;
    stackRequest->northboundProtocol = northboundProtocol;
    stackRequest->internetProtocol = DomainExtensions::getInternetProtocol(northboundDestination);
    stackRequest->stack = stack;
    stackRequest->destination = northboundDestination;
    stackRequest->overrideOriginFlowId = overrideFlowId != nullptr;
    if(stackRequest->overrideOriginFlowId)
        stackRequest->customOriginFlowId = overrideFlowId->destination;

    return stackRequest;
}

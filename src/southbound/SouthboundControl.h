#pragma once

#include "../VirtualStackSettings.h"
#include "../interface/IEndpoint.h"
#include "NetworkMessageController.h"
#include "SouthboundCallbacks.h"
#include "model/ConfigurationState.h"

class SouthboundControl
{
public:
    SouthboundControl(const SouthboundCallbacks& callbacks,
                      VsObjectFactory& vsObjectFactory,
                      const VirtualStackSettings& settings);

    void start();
    void stop();

    ConfigurationState getState() const;

    static std::unique_ptr<NewStackRequest> createNewStackRequest(const sockaddr_storage &northboundDestination,
                                                                  const uint16_t northboundSourcePort,
                                                                  const uint16_t northboundDestinationPort,
                                                                  const TransportProtocolEnum northboundProtocol,
                                                                  const StackEnum stack,
                                                                  const flowid_t* overrideFlowId = nullptr);

    std::future<ConfigurationState> requestCreateStack(const sockaddr_storage &nextHop,
                                                       std::unique_ptr<NewStackRequest> &&request);
private:
    const SouthboundCallbacks& _callbacks;
    ISocketFactory& _socketFactory;
    const VirtualStackSettings& _settings;
    NetworkMessageController _controller;

};



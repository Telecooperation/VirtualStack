#pragma once

#include "../../../../common/DataStructures/Container/unique_fd.h"
#include "../../../../interface/ISocketFactory.h"
#include "../../base/ExtendedSouthboundConfiguration.h"
#include "../NewStackResult.h"
#include "NewStackRequest.h"

class NewStackRequestConfiguration final : public ExtendedSouthboundConfiguration<UniqueSocket>
{
public:
	explicit NewStackRequestConfiguration(std::unique_ptr<NewStackRequest> request,
										  const std::function<void(std::unique_ptr<NewStackResult>)>& onComplete,
										  ISocketFactory& socketFactory,
										  const VirtualStackSettings &settings);

private:
	UniqueSocket run() override;
    void onComplete(UniqueSocket &&socket) override;

    std::unique_ptr<NewStackRequest> _request;
	ISocketFactory& _socketFactory;
	sockaddr_storage _sourceByPartner;
	const VirtualStackSettings &_settings;
	const std::function<void(std::unique_ptr<NewStackResult>)> _onComplete;
};



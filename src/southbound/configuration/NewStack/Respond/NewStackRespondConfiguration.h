#pragma once


#include "../../../../common/DataStructures/Container/unique_fd.h"
#include "../../base/ExtendedSouthboundConfiguration.h"
#include "../NewStackResult.h"
#include "../../../../interface/ISocketFactory.h"

class NewStackRespondConfiguration final : public ExtendedSouthboundConfiguration<UniqueSocket>
{
public:
	explicit NewStackRespondConfiguration(const std::function<void(std::unique_ptr<NewStackResult>)>& onComplete,
										  ISocketFactory& socketFactory,
                                          const VirtualStackSettings& settings);

private:
	UniqueSocket run() override;
	void onComplete(UniqueSocket &&socket) override;

    std::unique_ptr<NewStackRequest> _request;
	ISocketFactory& _socketFactory;
	const VirtualStackSettings& _settings;
	const std::function<void(std::unique_ptr<NewStackResult>)> _onComplete;
};



#pragma once

#include "../../../interface/ISocket.h"
#include "../../model/ConfigurationState.h"
#include "../../model/ISerializeable.h"
#include "../../protocol/NetworkMessageEnum.h"
#include "Request/NewStackRequest.h"

class NewStackResult
{
public:
	NewStackResult(NetworkMessageEnum pNetworkMessageEnum,
				   ConfigurationState pConfigurationState,
				   UniqueSocket pSocket,
				   const sockaddr_storage &pPartner,
				   std::unique_ptr<NewStackRequest> pRequest) :
			networkMessageEnum(pNetworkMessageEnum),
            configurationState(pConfigurationState),
			socket(std::move(pSocket)),
			partner(pPartner),
			request(std::move(pRequest)) {}

	NetworkMessageEnum networkMessageEnum;
	ConfigurationState configurationState;
	UniqueSocket socket;
	sockaddr_storage partner;
	sockaddr_storage sourceByPartner;
	std::unique_ptr<NewStackRequest> request;

	bool isValid() const
	{
		return socket && socket->isValid();
	}

	ALLOW_MOVE_SEMANTICS_ONLY(NewStackResult);
};



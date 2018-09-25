#pragma once

#include "../../../common/DataStructures/Container/RingBufferMove.h"
#include "../../../common/Helper/ClassMacros.h"
#include "../../../model/InspectionStruct.h"
#include "../../model/ConfigurationState.h"
#include "../../model/IReceiveFromNetwork.h"
#include "../../model/ISerializeable.h"
#include "../../protocol/NetworkMessageEnum.h"
#include <future>
#include <sys/socket.h>

/**
 * Baseclass for a new configuration
 * A configuration describes a messagetype like createNewStack or DeleteStack to be send to another VirtualStack
 * Its divided into request and respond where an instance of a configuration only serves on of the two.
 *
 */
class BaseSouthboundConfiguration
{
public:
	explicit BaseSouthboundConfiguration(const NetworkMessageEnum networkMessageEnum);
	
	virtual ~BaseSouthboundConfiguration();
	
	ConfigurationState getState() const;

	virtual void start(const sockaddr_storage& managementSource,
					   const sockaddr_storage& managementDestination) {}

	void stop();
	
	bool isValid() const;

	bool requestsFinish() const;

	void acceptFinish();

	bool isFinished() const;
	
	bool hasSendToNetwork() const;
	
	const ISerializeable* getSendToNetwork();
	
	void addReceiveFromNetwork(std::unique_ptr<IReceiveFromNetwork> message);

    void linkPromise(std::promise<ConfigurationState>& promise);

	const NetworkMessageEnum networkMessageEnum;
	
	ALLOW_MOVE_SEMANTICS_ONLY(BaseSouthboundConfiguration);
protected:
	void requestToFinish();
	RingBufferMove<const ISerializeable*> _sendToNetwork;
	RingBufferMove<std::unique_ptr<IReceiveFromNetwork>> _receiveFromNetwork;
    std::atomic<ConfigurationState> _state;
    std::promise<ConfigurationState>* _linkedPromise;
    std::atomic<bool> _onCompleteDone;
private:
    enum class FinishState
    {
        NotInitialized,
        Ok,
        Canceled,
        RequestsFinish
    };

    std::atomic<FinishState> _finishState;
};



#pragma once


#include "../VirtualStackSettings.h"
#include "../common/Allocator/VsObjectFactory.h"
#include "../common/DataStructures/Container/RingBuffer.h"
#include "../common/DataStructures/Container/unique_fd.h"
#include "../common/DataStructures/VS/IStack.h"
#include "../interface/IKernel.h"
#include "../model/InspectionStruct.h"
#include "SouthboundCallbacks.h"
#include "SouthboundSessionController.h"
#include "configuration/base/BaseSouthboundConfiguration.h"
#include "model/ConfigurationTask.h"
#include "model/ISerializeable.h"
#include <future>
#include <map>

class NetworkMessageController final
{
public:
	NetworkMessageController(const SouthboundCallbacks& callbacks,
							 VsObjectFactory& vsObjectFactory,
							 const VirtualStackSettings& settings);
	
	void start();
	void stop();
	
	std::future<ConfigurationState> addConfiguration(const sockaddr_storage& destination,
													 std::unique_ptr<BaseSouthboundConfiguration> configuration);

	ConfigurationState getCurrentState() const { return _currentState; }
	virtual ~NetworkMessageController();
	
	ALLOW_MOVE_SEMANTICS_ONLY(NetworkMessageController);
private:
	void loop();
	inline void processConfigurationBuffer();
	std::mutex _pendingConfigurationLock;

	const SouthboundCallbacks& _callbacks;
	VsObjectFactory& _vsObjectFactory;
	const VirtualStackSettings& _settings;

	ConfigurationState _currentState;
	RingBuffer<ConfigurationTask> _pendingConfigurations;
	
	std::map<flowid_t, std::unique_ptr<SouthboundSessionController>> _flowIdToSessionController;
	
	std::atomic<bool> _stopThread;
	std::unique_ptr<std::thread> _thread;

	inline UniqueSocket createTcpIpv4Socket();
	inline bool configureAsAcceptSocket(UniqueSocket &socket);

	bool handleIncomingManagementConnections(IKernel& kernel);
};



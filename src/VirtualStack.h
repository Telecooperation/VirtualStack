#pragma once


#include "common/DataStructures/Container/RingBufferMove.h"
#include "common/DataStructures/Container/unique_fd.h"
#include "common/Helper/ClassMacros.h"
#include "common/Helper/Debouncer.h"
#include "interface/IEndpoint.h"
#include "interface/INorthboundDevice.h"
#include "model/InspectionStruct.h"
#include "stacks/StackEnum.h"
#include "virtualStack/StackCreationHandler.h"
#include "virtualStack/remote/VirtualStackRemoteControl.h"
#include <map>

class VirtualStack
{
	friend class VirtualStackRemoteControl;
public:
	VirtualStack(std::unique_ptr<INorthboundDevice> northboundDevice, VsObjectFactory& vsObjectFactory,
                 RingBufferMove<StoragePoolPtr>& inspectionEvents, const VirtualStackSettings &settings);
	
	bool start();
	void stop();
	
	const VirtualStackSettings& getSettings() { return _virtualStackSettings; }
	
	VirtualStackRemoteControl& getRemoteControl() { return _remoteControl; }
	
	~VirtualStack();
	
	ALLOW_MOVE_SEMANTICS_ONLY(VirtualStack);
private:
	inline void removeStackEngine(const StackEngine& stackEngine);
	void loop();
	inline StackEngine *getStackEngineByFlowId(flowid_t flowId);
	void processStackCreationResult(std::unique_ptr<StackCreationResult> &&result);
    inline std::unique_ptr<StackEngine> createStackEngine(flowid_t flowId, flowid_t issuerFlowId,
                                                          TransportProtocolEnum transportProtocol,
                                                          const sockaddr_storage &source,
                                                          const sockaddr_storage &destination);
    std::vector<flowid_t> getAllFlowIds() const;
	
	const VirtualStackSettings& _virtualStackSettings;
	VsObjectFactory& _vsObjectFactory;
	
	RingBufferMove<StoragePoolPtr>& _inspectionEvents;

	std::unique_ptr<INorthboundDevice> _northboundDevice;
	std::map<flowid_t, std::unique_ptr<StackEngine>> _stackEngines; //macht das umsortieren leichter, wenn StackEngine nur ein Pointer ist

	StackCreationHandler _stackCreationHandler;
	VirtualStackRemoteControl _remoteControl;
	
	std::atomic<bool> _threadRunning;
	std::unique_ptr<std::thread> _thread;
};



#pragma once


#include "../../common/DataStructures/Container/RingBufferMove.h"
#include "../../model/InspectionStruct.h"
#include "../../southbound/SouthboundControl.h"
#include "../stackEngine/StackEngine.h"
#include "RemoteControlResult.h"
#include "RemoteControlResultEnum.h"
#include <future>

class VirtualStack;

class VirtualStackRemoteControl final
{
	friend class VirtualStack;
public:
	explicit VirtualStackRemoteControl(VirtualStack& virtualStack);

	std::future<RemoteControlResult<std::future<ConfigurationState>>>  createStack(flowid_t flowId, const StackEnum stack, const std::string& nextHopIp = "");

    std::future<RemoteControlResult<std::vector<flowid_t>>> getFlowIds();

    std::future<RemoteControlResultEnum> addStack(std::unique_ptr<StackCreationResult> &&stackCreation);

	std::future<RemoteControlResultEnum> killStack(flowid_t flowId, size_t stackId);
	
	std::future<RemoteControlResultEnum> deactivateStack(flowid_t flowId, size_t stackId);
	
	std::future<RemoteControlResultEnum> activateStack(flowid_t flowId, size_t stackId);

	/// Switch the active scheduler for a flow
	/// \param flowId The flowId
	/// \param scheduler The new scheduler, can be the same as it is currently
	/// \return A future with the result
	std::future<RemoteControlResultEnum> switchScheduler(flowid_t flowId, const SchedulerTypeEnum scheduler);
private:
	VirtualStack& _virtualStack;
	std::map<flowid_t, std::unique_ptr<StackEngine>>& _stackEngines;
	StackCreationHandler& _stackCreationHandler;

    RingBufferMove<std::function<void()>> _tasksQueue;
	
	void processOneRemoteControlMessage();
};



#pragma once


#include "../../VirtualStackSettings.h"
#include "../../common/DataStructures/Container/FixedSizeArray.h"
#include "../../common/DataStructures/Container/UniquePtrArray.h"
#include "../../common/DataStructures/Model/Storage.h"
#include "../../common/DataStructures/VS/IStack.h"
#include "../../interface/IEndpoint.h"
#include "../../interface/IScheduler.h"
#include "../../interface/INorthboundDevice.h"
#include "../../model/StackDetails.h"
#include "../../stacks/StackEnum.h"
#include "../factory/SchedulerFactory.h"
#include "../scheduler/SchedulerTypeEnum.h"
#include "StackEngineReceiveController.h"
#include "alm/SequenceNumberMiddleware.h"
#include <memory>

class StackEngine
{
public:
	StackEngine(flowid_t pFlowId, INorthboundDevice& northboundDevice,
                const VirtualStackSettings& settings,
                VsObjectFactory& objectFactory,
                std::unique_ptr<IEndpoint>&& endpoint);

	IEndpoint& getEndpoint() const { return *_endpoint.get(); }
	
	bool process(StoragePoolPtr&& storage, bool isRetry = false);
	
	size_t addStack(std::unique_ptr<IStack> stack);
	
	void killStack(size_t stackIndex, bool force = false);

    bool deactivateStack(size_t stackIndex);

    bool activateStack(size_t stackIndex);
	
	void switchScheduler(const SchedulerTypeEnum scheduler);
	
	const IStack* getStack(size_t stackId) const;

	void stop();

    bool isValid() const;
    const FixedSizeArray<StackDetails>& getStacks() const;

	const flowid_t flowId;
	
	ALLOW_MOVE_SEMANTICS_ONLY(StackEngine);
private:
    inline size_t getStackIndex(size_t stackId) const;

    size_t _nextStackId;
    UniquePtrArray<IScheduler> _schedulers;
    FixedSizeArray<StackDetails> _stackDetails;
    SequenceNumberMiddleware _seqNumMiddleware;

    std::unique_ptr<IEndpoint> _endpoint;
    std::unique_ptr<StackEngineReceiveController> _receiveController;

    StackDetails* _stackDetailPtr;
    IScheduler* _activeScheduler;
};



#include "StackEngine.h"

StackEngine::StackEngine(flowid_t pFlowId,
                         INorthboundDevice &northboundDevice,
                         const VirtualStackSettings &settings,
                         VsObjectFactory &objectFactory,
                         std::unique_ptr<IEndpoint> &&endpoint) :
        flowId(pFlowId),
        _nextStackId(1),
        _schedulers(SchedulerFactory::createSchedulerList(*this)),
        _stackDetails(settings.StackEngineMaxStacksCount),
        _seqNumMiddleware(settings),
        _endpoint(std::move(endpoint)),
        _receiveController(new StackEngineReceiveController(settings, objectFactory, _seqNumMiddleware,
                                                            *_endpoint, northboundDevice)),
        _stackDetailPtr(_stackDetails.getRawData()),
        _activeScheduler(_schedulers[settings.DefaultSchedulerIndex])
{

}

bool StackEngine::process(StoragePoolPtr&& storage, bool isRetry)
{
	if (_stackDetails.getSize() == 0)
		return false;

    if(!isRetry)
	    _endpoint->processNBIToStack(*storage);

	_activeScheduler->process(*storage);
	auto stackIndex = _activeScheduler->getActiveStack();
	auto& stackDetails = _stackDetails[stackIndex];
    auto& stack = stackDetails.get();

    if (stack.isFull())
		return false;

	if (stack.isConnectionClosed())
	{
		killStack(stackIndex, true);
		return false;
	}

    if(stack.stackInfo.IsReliable)
        _seqNumMiddleware.addSequenceNumber(storage);

    stackDetails.metricData.oneSent();

    stack.push(std::move(storage));

	return true;
}

void StackEngine::killStack(size_t stackId, bool force)
{
	if (_stackDetails.getSize() == 0)
		return;

	if(_stackDetails.getSize() <= 1 && !force)
	{
		Logger::Log(Logger::ERROR, "Cannot kill stack: At least one stack per StackEngine must be active at all time");
		return;
	}

    auto stackIndex = getStackIndex(stackId);
	if(stackIndex == _stackDetails.getSize())
    {
        Logger::Log(Logger::ERROR, "Cannot kill stack: Stack not found for StackId: ", stackId);
        return;
    }

	auto el = _stackDetails.remove(stackIndex);

	//teile dem recieveController mit, dass er den Stack nicht mehr beobachten muss
	_receiveController->killStack(stackId);
}

bool StackEngine::deactivateStack(size_t stackId)
{
    auto stackIndex = getStackIndex(stackId);
    if(stackIndex == _stackDetails.getSize())
    {
        Logger::Log(Logger::ERROR, "Cannot deactivate stack: Stack not found for StackId: ", stackId);
        return false;
    }
    _stackDetails[stackIndex].isInactive = true;
    return true;
}

bool StackEngine::activateStack(size_t stackId)
{
	auto stackIndex = getStackIndex(stackId);
    if(stackIndex == _stackDetails.getSize())
    {
        Logger::Log(Logger::ERROR, "Cannot activate stack: Stack not found for StackId: ", stackId);
        return false;
    }
    _stackDetails[stackIndex].isInactive = false;
    return true;
}

void StackEngine::switchScheduler(const SchedulerTypeEnum scheduler)
{
	auto schedulerIndex = static_cast<uint8_t>(scheduler);
	_activeScheduler = _schedulers[schedulerIndex];
}

size_t StackEngine::addStack(std::unique_ptr<IStack> stack)
{
	if(_stackDetails.isFull())
	{
		Logger::Log(Logger::ERROR, "StackEngine: Tried to create a stack, but no space left for a new stack.");
		return std::numeric_limits<size_t>::max();
	}

	//_stacksPtr[_stackCount].swap(stack);
	auto& el = *_stackDetails.add(StackDetails{std::move(stack), _nextStackId});
	_receiveController->addStack(el.get());

	//start the added stack
	el.get().start(_nextStackId);

    ++_nextStackId;

	return el.getStackId();
}

const IStack* StackEngine::getStack(size_t stackId) const
{
    auto stackIndex = getStackIndex(stackId);
    if(stackIndex == _stackDetails.getSize())
    {
        Logger::Log(Logger::ERROR, "Cannot get stack: Stack not found for StackId: ", stackId);
        return nullptr;
    }

	auto el = _stackDetails.get(stackIndex);
	if(el == nullptr)
	{
		Logger::Log(Logger::ERROR, "StackEngine: requested Stack was not found with stackId: ", stackId);
		return nullptr;
	}

	return &el->get();
}

void StackEngine::stop() {
	for (size_t i = 0; i < _stackDetails.getSize(); ++i) {
        _stackDetails[i].get().stop();
	}
	_receiveController->stop();
	_stackDetails.reset();
}

bool StackEngine::isValid() const
{
    return _stackDetails.getSize() > 0;
}

const FixedSizeArray<StackDetails>& StackEngine::getStacks() const
{
    return _stackDetails;
}

size_t StackEngine::getStackIndex(size_t stackId) const
{
    for (size_t i = 0; i < _stackDetails.getSize(); ++i)
    {
        if(_stackDetails[i].getStackId() == stackId)
            return i;
    }
    return _stackDetails.getSize();
}

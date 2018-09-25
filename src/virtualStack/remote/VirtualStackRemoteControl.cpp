#include "../../VirtualStack.h"


VirtualStackRemoteControl::VirtualStackRemoteControl(VirtualStack& virtualStack) :
		_virtualStack(virtualStack),
		_stackEngines(_virtualStack._stackEngines),
		_stackCreationHandler(_virtualStack._stackCreationHandler),
		_tasksQueue(_virtualStack._virtualStackSettings.SizeOfRemoteControlCommandsBuffer)
{}

void VirtualStackRemoteControl::processOneRemoteControlMessage()
{
	if(!_tasksQueue.available())
		return;

	auto task = _tasksQueue.pop();
    task();

}

std::future<RemoteControlResult<std::future<ConfigurationState>>> VirtualStackRemoteControl::createStack(flowid_t flowId, const StackEnum stack, const std::string& nextHopIp)
{
    auto promise = std::make_shared<std::promise<RemoteControlResult<std::future<ConfigurationState>>>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){
		auto tmpStackEngine = _stackEngines.find(flowId);
		if(tmpStackEngine == _stackEngines.end())
        {
            promise->set_value(RemoteControlResult<std::future<ConfigurationState>>(RemoteControlResultEnum::StackEngineNotFound));
            return;
        }

		auto tmpCreateStackResult = _stackCreationHandler.add(tmpStackEngine->second->getEndpoint(), stack, nextHopIp);
        promise->set_value(RemoteControlResult<std::future<ConfigurationState>>(std::move(tmpCreateStackResult), RemoteControlResultEnum::Ok));
	});

	return future;
}

std::future<RemoteControlResult<std::vector<flowid_t>>> VirtualStackRemoteControl::getFlowIds()
{
    auto promise = std::make_shared<std::promise<RemoteControlResult<std::vector<flowid_t>>>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){

        promise->set_value(RemoteControlResult<std::vector<flowid_t>>(_virtualStack.getAllFlowIds(),
                                               RemoteControlResultEnum::Ok));
    });

    return future;
}

std::future<RemoteControlResultEnum> VirtualStackRemoteControl::addStack(std::unique_ptr<StackCreationResult> &&stackCreation)
{
    //lambda does not support move semantics. so we release the pointer, pass it raw and secure it again in a unique_ptr
    auto stackCreationPtr = stackCreation.release();
    auto promise = std::make_shared<std::promise<RemoteControlResultEnum>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){
        _virtualStack.processStackCreationResult(std::unique_ptr<StackCreationResult>(stackCreationPtr));
        promise->set_value(RemoteControlResultEnum::Ok);
    });

    return future;
}

std::future<RemoteControlResultEnum> VirtualStackRemoteControl::killStack(flowid_t flowId, size_t stackId)
{
    auto promise = std::make_shared<std::promise<RemoteControlResultEnum>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){


        auto tmpStackEngine = _stackEngines.find(flowId);
        if(tmpStackEngine == _stackEngines.end())
        {
            promise->set_value(RemoteControlResultEnum::StackNotFound);
            return;
        }

        tmpStackEngine->second->killStack(stackId);
        promise->set_value(RemoteControlResultEnum::Ok);
    });

    return future;
}

std::future<RemoteControlResultEnum> VirtualStackRemoteControl::deactivateStack(flowid_t flowId, size_t stackId)
{
    auto promise = std::make_shared<std::promise<RemoteControlResultEnum>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){

        auto tmpStackEngine = _stackEngines.find(flowId);
        if(tmpStackEngine == _stackEngines.end())
        {
            promise->set_value(RemoteControlResultEnum::StackNotFound);
            return;
        }

        auto result = tmpStackEngine->second->deactivateStack(stackId) ? RemoteControlResultEnum::Ok : RemoteControlResultEnum::Failed;
        promise->set_value(result);
    });

    return future;
}

std::future<RemoteControlResultEnum> VirtualStackRemoteControl::activateStack(flowid_t flowId, size_t stackId)
{
    auto promise = std::make_shared<std::promise<RemoteControlResultEnum>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){

        auto tmpStackEngine = _stackEngines.find(flowId);
        if(tmpStackEngine == _stackEngines.end())
        {
            promise->set_value(RemoteControlResultEnum::StackNotFound);
            return;
        }

        auto result = tmpStackEngine->second->activateStack(stackId) ? RemoteControlResultEnum::Ok : RemoteControlResultEnum::Failed;
        promise->set_value(result);
    });

    return future;
}

std::future<RemoteControlResultEnum> VirtualStackRemoteControl::switchScheduler(flowid_t flowId, const SchedulerTypeEnum scheduler)
{
    auto promise = std::make_shared<std::promise<RemoteControlResultEnum>>();
    auto future = promise->get_future();

    _tasksQueue.push([=](){

        auto tmpStackEngine = _stackEngines.find(flowId);
        if(tmpStackEngine == _stackEngines.end())
        {
            promise->set_value(RemoteControlResultEnum::StackNotFound);
            return;
        }

        tmpStackEngine->second->switchScheduler(scheduler);
        promise->set_value(RemoteControlResultEnum::Ok);
    });

    return future;
}

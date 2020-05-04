#include "VirtualStack.h"
#include "virtualStack/Splitter.h"
#include "virtualStack/factory/EndpointFactory.h"
#include "virtualStack/factory/StackFactory.h"
#include "virtualStack/fastInspection/Classifier.h"
#include "virtualStack/fastInspection/FlowIdGenerator.h"
#include "virtualStack/fastInspection/PacketFilter.h"
#include "virtualStack/stackEngine/endpoints/RouterEndpoint.h"
#include "common/Helper/StopWatch.h"

VirtualStack::VirtualStack(std::unique_ptr<INorthboundDevice> northboundDevice, VsObjectFactory& vsObjectFactory,
                           RingBufferMove<StoragePoolPtr>& inspectionEvents, const VirtualStackSettings &settings) :
		_virtualStackSettings(settings),
		_vsObjectFactory(vsObjectFactory),
		_inspectionEvents(inspectionEvents),
		_northboundDevice(std::move(northboundDevice)),
		_stackEngines(),
		_stackCreationHandler(_vsObjectFactory, _virtualStackSettings),
		_remoteControl(*this),
		_threadRunning(false)
{
}

VirtualStack::~VirtualStack()
{
    stop();
}

bool VirtualStack::start()
{
	if(_threadRunning)
		return true;
	
	if(_virtualStackSettings.SettingsReadFailed())
	{
		Logger::Log(Logger::ERROR, "Could not start VirtualStack, settings are invalid");
		return false;
	}
	if(!_northboundDevice->isDeviceOpen())
	{
		Logger::Log(Logger::ERROR, "Could not start VirtualStack, northboundDevice is not open");
		return false;
	}

	_threadRunning = true;
	_thread = std::make_unique<std::thread>(&VirtualStack::loop, this);
	_stackCreationHandler.start();

	return true;
}

void VirtualStack::stop()
{
	_threadRunning = false;
	_stackCreationHandler.stop();

    if (_thread && _thread->joinable())
    {
        _thread->join();
        _thread.reset();
    }
}

void VirtualStack::loop()
{
    Debouncer debouncer{_virtualStackSettings.VirtualStackUtilizationPlan, _virtualStackSettings};

    StoragePoolPtr storage{};
    InspectionStruct* inspectionStruct = nullptr;
    StackEngine *stackEngine = nullptr;

	while(_threadRunning)
	{
		_remoteControl.processOneRemoteControlMessage();

		if(_stackCreationHandler.hasNewStacks())
            processStackCreationResult(_stackCreationHandler.getNextNewStack());

		//if we dont have a previous storage
        //this is a workaround as we dont want to discard packets currently
        //pending... can be removed if we want to discard packets in the future

        bool isFirstRun = !storage;

        if(isFirstRun)
        {
            if (!_northboundDevice->available())
            {
                debouncer.sleep();
                continue; //kein sleep wegen latenz
            }

            debouncer.reset();
            storage = _northboundDevice->pop();
            if (!storage)
                continue;

  //          auto timeCount = storage->toTypeAutomatic<size_t>(storage->size() - sizeof(size_t));
  //          storage->replaceDataScalarBeforeEnd(static_cast<size_t>(1 + timeCount));

//            auto appendPosition = sizeof(size_t) + timeCount*(sizeof(uint8_t) + sizeof(long));
 //           storage->replaceDataScalarBeforeEnd(static_cast<uint8_t>(0), appendPosition);
  //          storage->replaceDataScalarBeforeEnd(StopWatch::getHighResolutionTime(), appendPosition + sizeof(uint8_t));

            //data
            inspectionStruct = Classifier::process(storage);
            if (PacketFilter::filterOut(_virtualStackSettings, *inspectionStruct))
            {
                //sendToRawStack();
                storage.reset();
                continue;
            }

            FlowIdGenerator::process(storage);

            //Activate if deepInspection us used
            //if(Splitter::dispatchToDeepInspection(_inspectionEvents, pendingStorage))
            //  continue;

            stackEngine = getStackEngineByFlowId(inspectionStruct->flowId);

            if (stackEngine == nullptr)
            {
                _stackCreationHandler.add(inspectionStruct->flowId, std::move(storage));
                continue;
            }
        }

        if (stackEngine->process(std::move(storage), !isFirstRun))
            continue;

        //the stackEngine is full currently
        if(stackEngine->isValid())
            continue;

		removeStackEngine(*stackEngine);
		storage.reset(); //discard packet as we dont have a valid stackEngine for it
	}
}

void VirtualStack::processStackCreationResult(std::unique_ptr<StackCreationResult> &&result)
{
	if(result->stack == nullptr)
		return;

    auto& stack = result->stack;
    const flowid_t& flowId = result->flowId;
    auto& storageBuffer = result->storageBuffer;
    auto& newStackResult = result->newStackResult;
	auto& newStackRequest = newStackResult->request;
	
	auto stackEngineIterator = _stackEngines.find(flowId);
	if(stackEngineIterator == _stackEngines.end())
	{
//        Logger::Log(Logger::DEBUG, _virtualStackSettings.getDefaultIPv4(), ": New StackEngine for Flow: ", flowId, ", to flow: ", result->partnerFlowId, " with Endpoint: ", static_cast<int>(newStackRequest->northboundProtocol), ", byRequest: ", result->byRequest);
		//create new stack and add it to stackEngine
		auto tmpStackEngine = createStackEngine(flowId, result->partnerFlowId, newStackRequest->northboundProtocol,
												NetworkExtensions::getIPv4SockAddr(_virtualStackSettings.getDefaultIPv4(), newStackRequest->northboundSourcePort),
                                                newStackRequest->destination);
        if(!tmpStackEngine)
            return;
		
		auto tmpResult = _stackEngines.emplace(flowId, std::move(tmpStackEngine));
		if(!tmpResult.second)
		{
			Logger::Log(Logger::ERROR, _virtualStackSettings.getDefaultIPv4(), "-> Discard StackEngine: Could not be added to VirtualStack loop");
			return;
		}
		stackEngineIterator = tmpResult.first;
	}
	
	auto& stackEngine = stackEngineIterator->second;
	stackEngine->addStack(std::move(stack));

    if (storageBuffer)
    {
        while (storageBuffer->available())
			stackEngine->process(storageBuffer->pop());
    }
}

StackEngine* VirtualStack::getStackEngineByFlowId(flowid_t flowId)
{
	auto tmpStackEnginePosition = _stackEngines.find(flowId);
	if(tmpStackEnginePosition != _stackEngines.end())
		return tmpStackEnginePosition->second.get();

	return nullptr;
}

std::unique_ptr<StackEngine> VirtualStack::createStackEngine(flowid_t flowId, flowid_t issuerFlowId,
															 TransportProtocolEnum transportProtocol, const sockaddr_storage& source,
															 const sockaddr_storage& destination)
{
	//erstelle einen geeigneten Endpoint
	std::unique_ptr<IEndpoint> endpoint = EndpointFactory::createEndpoint(flowId, issuerFlowId, transportProtocol, source, destination);
	if(!endpoint)
    {
        Logger::Log(Logger::ERROR, "VirtualStack::createStackEngine-> createEndpoint failed");
        return std::unique_ptr<StackEngine>();
    }

	//Erstelle eine StackEngine und für sie den stackEngines hinzu
	return std::make_unique<StackEngine>(flowId, *_northboundDevice, _virtualStackSettings, _vsObjectFactory, std::move(endpoint));
}

void VirtualStack::removeStackEngine(const StackEngine& stackEngine) {
    if(_virtualStackSettings.IsRouter)
    {
        //Wenn für die eine StackEngine keine Stacks mehr da sind, muss auch die andere StackEngine gelöscht werden
        //Beide StackEngines bilden eine P2P Verbindung zwischen den äußeren Verbindungspartnern
        auto otherFlowId = static_cast<RouterEndpoint&>(stackEngine.getEndpoint()).flowId;
        _stackEngines.erase(otherFlowId);
    }
    _stackEngines.erase(stackEngine.flowId);
}

std::vector<flowid_t> VirtualStack::getAllFlowIds() const
{
    std::vector<flowid_t> retval;
    for (auto const& element : _stackEngines) {
        retval.push_back(element.first);
    }
    return retval;
}

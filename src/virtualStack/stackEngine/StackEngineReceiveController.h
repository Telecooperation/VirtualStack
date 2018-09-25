#pragma once

#include "../../VirtualStackSettings.h"
#include "../../common/DataStructures/VS/IStack.h"
#include "../../common/Helper/Debouncer.h"
#include "../../interface/IEndpoint.h"
#include "../../interface/INorthboundDevice.h"
#include "alm/SequenceNumberMiddleware.h"

class StackEngineReceiveController
{
public:
	StackEngineReceiveController(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory,
                                 SequenceNumberMiddleware& sequenceNumberMiddleware,
                                 const IEndpoint& endpoint, INorthboundDevice& northboundDevice) :
            _settings(settings),
            _seqNumMiddleware(sequenceNumberMiddleware),
			_endpoint(endpoint),
			_northboundDevice(northboundDevice),
			_stacksSize(0),
			_stacks(new IStack*[settings.StackEngineMaxStacksCount]{}),
			_threadStallState(ThreadStallState::Running),
			_stopThread(false),
			_workerThread(&StackEngineReceiveController::loop, this)
	{ }
	
	void addStack(IStack& stack);
	void killStack(size_t stackIndex);

	void stop();
	
	~StackEngineReceiveController();
	
	ALLOW_MOVE_SEMANTICS_ONLY(StackEngineReceiveController);
private:
	enum class ThreadStallState : uint8_t
	{
		Running,
		AcceptStall,
		Stall
	};

	inline void synchronizeWithExternalRequests();
	void loop();

    const VirtualStackSettings& _settings;

    SequenceNumberMiddleware& _seqNumMiddleware;
	const IEndpoint& _endpoint;
	INorthboundDevice& _northboundDevice;
	
	std::atomic<size_t> _stacksSize;
	std::unique_ptr<IStack*[]> _stacks;

	std::atomic<ThreadStallState> _threadStallState;
	std::atomic<bool> _stopThread; //basierend auf MESI Protokoll müssen wir die änderung irgendwann sehen wegen Threadcontextswitch
	std::thread _workerThread;
};



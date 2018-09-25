
#include "StackEngineReceiveController.h"
#include "../../common/Helper/StopWatch.h"

void StackEngineReceiveController::loop()
{
	Debouncer debouncer{_settings.StackEngineReceiveControllerUtilizationPlan, _settings};

	while(!_stopThread)
	{
		if(_stacksSize == 0)
		{
			debouncer.sleep();
			continue;
		}

		for (size_t i = 0; i < _stacksSize; ++i)
		{
			synchronizeWithExternalRequests();

			//Event could delete last running stack, or reorder stackList
            //Break if stacksSize == 0 or i >= stacksSize because or reordering on delete
			if(i >= _stacksSize)
                break;

			IStack& stack = *(_stacks[i]);

			if(!stack.available())
			{
				debouncer.sleep();
				continue;
			}
			debouncer.reset();

            auto nextData = stack.pop();

            if(!nextData)
                continue;

            if(stack.stackInfo.IsReliable)
                _seqNumMiddleware.addStorage(std::move(nextData));

            //if neither packetizer nor seqNum accepted nextData, it shall be processed raw
            auto toNorthboundData = nextData ? std::move(nextData) : _seqNumMiddleware.getNextAvailable();
            while(toNorthboundData)
            {
				//wieder das protokoll an den header packen
                _endpoint.processStackToNBI(*toNorthboundData);

                while (_northboundDevice.isFull())
                {
                    //Can block if killStack is called, as it is a blocking call
                    if (_stopThread)
                        return;
                    debouncer.sleep();

                    synchronizeWithExternalRequests();
                }
                debouncer.reset();


				auto timeCount = toNorthboundData->toTypeAutomatic<size_t>(toNorthboundData->size() - sizeof(size_t));
				toNorthboundData->replaceDataScalarBeforeEnd(static_cast<size_t>(1 + timeCount));

				auto appendPosition = sizeof(size_t) + timeCount*(sizeof(uint8_t) + sizeof(long));
				toNorthboundData->replaceDataScalarBeforeEnd(0, appendPosition);
				toNorthboundData->replaceDataScalarBeforeEnd(StopWatch::getHighResolutionTime(), appendPosition + sizeof(uint8_t));

                //TODO: possible blocking as the buffer might be filled by another stackengine
                _northboundDevice.push(std::move(toNorthboundData));

                toNorthboundData = _seqNumMiddleware.getNextAvailable();
            }
		}
	}
}

StackEngineReceiveController::~StackEngineReceiveController()
{
	stop();
}

void StackEngineReceiveController::addStack(IStack& stack)
{
    //hier ist kein locking nötig, da wir nur hinzufügen, und der aktuelle durchlauf nur auf alles kleiner dem neuen eintrag prüft
    _stacks[_stacksSize] = &stack;
    ++_stacksSize;
}

void StackEngineReceiveController::killStack(size_t stackIndex)
{
	//check if _stacksIncomingBufferSize > 1 is made in StackEngine
	_threadStallState = ThreadStallState::Stall;
	while(!_stopThread && _threadStallState.load() != ThreadStallState::AcceptStall)
		continue;

	if (_stopThread)
		return;

	std::swap(_stacks[stackIndex], _stacks[_stacksSize - 1]);
	--_stacksSize;
	_threadStallState = ThreadStallState::Running;
}

void StackEngineReceiveController::stop() {
	_stopThread = true;

	if (_workerThread.joinable())
        _workerThread.join();
}

void StackEngineReceiveController::synchronizeWithExternalRequests() {
	if (_threadStallState.load() != ThreadStallState::Stall)
		return;

	_threadStallState = ThreadStallState::AcceptStall;

	while (!_stopThread && _threadStallState.load() == ThreadStallState::AcceptStall)
		std::this_thread::sleep_for(std::chrono::microseconds(1));
}

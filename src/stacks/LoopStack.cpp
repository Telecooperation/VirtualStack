#include "LoopStack.h"

LoopStack::LoopStack(const VirtualStackSettings &settings) :
		IStack(StackEnum::SoftwareLoop, settings),
		_threadRunning(false)
{
	
}

void LoopStack::loop()
{
	while(_threadRunning)
	{
		if(_toStackBuffer.available() && !_fromStackBuffer.isFull())
		{
			_fromStackBuffer.push(_toStackBuffer.pop());
		}
	}
}

void LoopStack::start()
{
	if(_threadRunning)
		return;

	_threadRunning = true;
	_thread = std::make_unique<std::thread>(&LoopStack::loop, this);
}

LoopStack::~LoopStack()
{
	stop();
}

void LoopStack::stop()
{
    IStack::stop();
    _threadRunning = false;

    if (_thread && _thread->joinable())
    {
        _thread->join();
        _thread.reset();
    }
}


#include "GenericStack.h"
#include "../LatencyMeter.h"
#include "../model/PosixSocketFactory.h"
#include "../common/Helper/StopWatch.h"

GenericStack::GenericStack(const StackEnum stackType, std::unique_ptr<IKernel> kernel,
                           const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory) :
		IStack(stackType, settings),
		_kernel(std::move(kernel)),
        _kernelPtr(*_kernel),
		_threadRunning(false)
{
}

void GenericStack::loop()
{
	if(!_kernelPtr.start())
	{
		Logger::Log(Logger::ERROR, "Could not start ", stackInfo.Name, " stack");
		return;
	}

    auto utilizationPlan = _isManagement ? _settings.SouthboundStackUtilizationPlan : _settings.GenericStackUtilizationPlan;
    Debouncer debouncer{utilizationPlan, _settings};

	while(_threadRunning)
    {
        if (!_kernelPtr.isValid())
            break;

        if (_toStackBuffer.available())
        {
            auto payload = _toStackBuffer.pop();
//            LatencyMeter::addInTime(sendKey);

            if (!_isManagement)
            {
                auto timeCount = payload->toTypeAutomatic<size_t>(payload->size() - sizeof(size_t));
                payload->replaceDataScalarBeforeEnd(static_cast<size_t>(1 + timeCount));

                auto appendPosition = sizeof(size_t) + timeCount*(sizeof(uint8_t) + sizeof(long));
                payload->replaceDataScalarBeforeEnd(stackIndex, appendPosition);
                payload->replaceDataScalarBeforeEnd(StopWatch::getHighResolutionTime(), appendPosition + sizeof(uint8_t));
            }
            _kernelPtr.sendPacket(*payload);
            debouncer.reset();
        }

        if(_kernelPtr.dataAvailable() && !_fromStackBuffer.isFull() && _kernelPtr.canReceive())
        {
            debouncer.reset();
            auto recvPacket = _kernelPtr.receivePacket();

            if (!recvPacket || !_kernelPtr.isValid()) //connection has been closed
                break;

            if (!_isManagement)
            {
                auto timeCount = recvPacket->toTypeAutomatic<size_t>(recvPacket->size() - sizeof(size_t));
                recvPacket->replaceDataScalarBeforeEnd(static_cast<size_t>(1 + timeCount));

                auto appendPosition = sizeof(size_t) + timeCount * (sizeof(uint8_t) + sizeof(long));
                recvPacket->replaceDataScalarBeforeEnd(stackIndex, appendPosition);
                recvPacket->replaceDataScalarBeforeEnd(StopWatch::getHighResolutionTime(),
                                                       appendPosition + sizeof(uint8_t));
            }

            _fromStackBuffer.push(std::move(recvPacket));
        }
        debouncer.sleep();
    }

    if (_kernel)
        _kernel->stop();

    _isConnectionClosed = true;
	_threadRunning = false;
}

void GenericStack::start()
{
	if(_threadRunning || !_kernel)
		return;

	_threadRunning = true;
	_thread = std::make_unique<std::thread>(&GenericStack::loop, this);
}

void GenericStack::stop()
{
    IStack::stop();

    _threadRunning = false;


    if (_kernel)
        _kernelPtr.stop();

    if (!_thread || !_thread->joinable())
        return;

    _thread->join();
    _thread.reset();
    _kernel.reset();
}

GenericStack::~GenericStack()
{
    stop();
}
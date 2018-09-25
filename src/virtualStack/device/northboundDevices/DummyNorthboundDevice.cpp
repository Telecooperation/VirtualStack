#include "DummyNorthboundDevice.h"


DummyNorthboundDevice::DummyNorthboundDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory) :
		INorthboundDevice(settings, vsObjectFactory),
		_outOfVirtualStackPushMutex(),
		_outOfVirtualStackBuffer(settings.SizeOfToNorthboundBuffer, "DummyNorthboundDevice.cpp::_outOfVirtualStackBuffer"),
		_intoVirtualStackBuffer(settings.SizeOfFromNorthboundBuffer, "DummyNorthboundDevice.cpp::_intoVirtualStackBuffer")
{
	_deviceOpened = true;
}

void DummyNorthboundDevice::push(StoragePoolPtr packet)
{
	std::lock_guard<std::mutex> lock(_outOfVirtualStackPushMutex);
    _outOfVirtualStackBuffer.push(std::move(packet));
}

StoragePoolPtr DummyNorthboundDevice::pop()
{
	return _intoVirtualStackBuffer.pop();
}

bool DummyNorthboundDevice::available()
{
	return _intoVirtualStackBuffer.available();
}

void DummyNorthboundDevice::externalIntoNorthbound(StoragePoolPtr packet)
{
	_intoVirtualStackBuffer.push(std::move(packet));
}

StoragePoolPtr DummyNorthboundDevice::externalOutOfNorthbound()
{
	return _outOfVirtualStackBuffer.pop();
}

bool DummyNorthboundDevice::availableExternal()
{
	return _outOfVirtualStackBuffer.available();
}

bool DummyNorthboundDevice::canSend()
{
	return !_intoVirtualStackBuffer.isFull();
}

DummyNorthboundDevice::~DummyNorthboundDevice() {
	_intoVirtualStackBuffer.stop();
	_outOfVirtualStackBuffer.stop();
}

bool DummyNorthboundDevice::isFull() {
	std::lock_guard<std::mutex> lock(_outOfVirtualStackPushMutex);
	return _outOfVirtualStackBuffer.isFull();
}

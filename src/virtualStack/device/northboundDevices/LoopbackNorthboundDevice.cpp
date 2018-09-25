#include "LoopbackNorthboundDevice.h"

LoopbackNorthboundDevice::LoopbackNorthboundDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory) :
        INorthboundDevice(settings, vsObjectFactory),
        _pushMutex(),
        _storageBuffer(settings.SizeOfToNorthboundBuffer, "LoopbackNorthboundDevice.cpp::_storageBuffer")
{}

void LoopbackNorthboundDevice::push(StoragePoolPtr packet) {
    std::lock_guard<std::mutex> lock(_pushMutex);
    _storageBuffer.push(std::move(packet));
}

StoragePoolPtr LoopbackNorthboundDevice::pop() {
    return _storageBuffer.pop();
}

bool LoopbackNorthboundDevice::available() {
    return _storageBuffer.available();
}

bool LoopbackNorthboundDevice::isFull() {
    std::lock_guard<std::mutex> lock(_pushMutex);
    return _storageBuffer.isFull();
}

bool LoopbackNorthboundDevice::isDeviceOpen() {
    return true;
}

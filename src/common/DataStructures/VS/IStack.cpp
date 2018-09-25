
#include "IStack.h"

IStack::~IStack()
{
    stop();
};

void IStack::start(size_t stackId, bool isManagement)
{
    _stackId = stackId;
    _isManagement = isManagement;
    start();
}

void IStack::stop()
{
    _toStackBuffer.stop();
    _fromStackBuffer.stop();
}

StoragePoolPtr IStack::pop()
{
    return _fromStackBuffer.pop();
}

void IStack::push(StoragePoolPtr storage)
{
    return _toStackBuffer.push(std::move(storage));
}

bool IStack::isFull()
{
    return _toStackBuffer.isFull();
}

bool IStack::available()
{
    return _fromStackBuffer.available();
}

size_t IStack::getStackId() const
{
    return _stackId;
}

bool IStack::isManagementStack() const
{
    return _isManagement;
}

size_t IStack::getPushFreeSlots() const
{
    return _toStackBuffer.getElementsPushableCount();
}

size_t IStack::getPushSlotsInUse() const
{
    return _toStackBuffer.getElementsAvailableCount();
}










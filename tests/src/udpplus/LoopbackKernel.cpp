#include "LoopbackKernel.h"

LoopbackKernel::LoopbackKernel(const VirtualStackSettings &settings, VsObjectFactory &vsObjectFactory) :
        _pool(vsObjectFactory.getKernelPool(settings.SizeOfKernelBuffer, "LoopbackKernel")),
        _buffer(settings.SizeOfKernelBuffer)
{

}

bool LoopbackKernel::dataAvailable()
{
    return _buffer.available();
}

StoragePoolPtr LoopbackKernel::receivePacket()
{
    return _buffer.pop();
}

bool LoopbackKernel::sendPacket(Storage &content)
{
    if(!_pool->canRequest())
        return false;

    auto contentCopy = _pool->request();
    content.copyInto(*contentCopy);
    _buffer.push(std::move(contentCopy));

    return true;
}

bool LoopbackKernel::isValid() const
{
    return true;
}

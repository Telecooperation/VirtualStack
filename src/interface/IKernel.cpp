#include "IKernel.h"

IKernel::~IKernel() { }

bool IKernel::start()
{
    if (!_kernel)
        return true;

    return _kernel->start();
}

bool IKernel::stop()
{
    if (!_kernel)
        return true;

    return _kernel->stop();
}

bool IKernel::dataAvailable()
{
	return _kernel->dataAvailable();
}

bool IKernel::sendPacketBypassed(Storage &content)
{
	if(_kernel)
		return _kernel->sendPacketBypassed(content);

	return sendPacket(content);
}

size_t IKernel::getCompleteHeaderSize() const
{
	size_t tmpResult = getKernelHeaderSize();
	if(_kernel)
		tmpResult += _kernel->getCompleteHeaderSize();
	return tmpResult;
}

size_t IKernel::getKernelHeaderSize() const
{
	return 0;
}

UniqueSocket &IKernel::getUnderlyingSocket()
{
    return _kernel->getUnderlyingSocket();
}

bool IKernel::configureAsListen()
{
    return _kernel->configureAsListen();
}

UniqueSocket IKernel::accept(sockaddr_storage &sockaddr)
{
    return _kernel->accept(sockaddr);
}

bool IKernel::isValid() const {
	return _kernel->isValid();
}

size_t IKernel::getCanReceiveSize()
{
    return _kernel->getCanReceiveSize();
}

bool IKernel::canReceive() const
{
    return _kernel->canReceive();
}

size_t IKernel::getMaxCanReceiveSize()
{
    return _kernel->getMaxCanReceiveSize();
}

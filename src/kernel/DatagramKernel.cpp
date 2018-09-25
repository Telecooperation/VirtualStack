#include "DatagramKernel.h"

bool DatagramKernel::dataAvailable()
{
	if (!_socketPtr.isValid())
		return false;

    auto res = _epoll->waitNonBlocking();
    if(res == ISocketEpoll::EpollResult::SocketError)
    {
        _socket->close();
        return false;
    }

    return res == ISocketEpoll::EpollResult::DataAvailable;
}

StoragePoolPtr DatagramKernel::receivePacket()
{
	auto tmpStorage = _receiveStoragePool->request();
	ssize_t recvBytes = _socketPtr.receiveBlocking(tmpStorage->data(), tmpStorage->freeSpaceForAppend(), false);

    if(recvBytes == 0) //0 bytes mean the connection is closed
        stop();

    if(!_socketPtr.isValid() || recvBytes < 0)
        return StoragePoolPtr();

	tmpStorage->setSize(static_cast<size_t>(recvBytes));
	return tmpStorage;
}

bool DatagramKernel::sendPacket(Storage &content)
{
	if (!_socketPtr.isValid())
		return false;

//	auto tmpSourceSockAddr = SocketExtensions::getSourceSockaddrStorage(_socket);
//	auto tmpDestSockAddr = SocketExtensions::getDestinationSockaddrStorage(_socket);

//	Logger::Log(Logger::DEBUG, "Source: ", NetworkExtensions::getAddress(tmpSourceSockAddr), ", Port: ", NetworkExtensions::getPort(tmpSourceSockAddr));
//	Logger::Log(Logger::DEBUG, "Dest: ", NetworkExtensions::getAddress(tmpDestSockAddr), ", Port: ", NetworkExtensions::getPort(tmpDestSockAddr));

    _socketPtr.sendBlocking(content.data(), content.size());

	return true;
}

DatagramKernel::~DatagramKernel()
{
	stop();
}

UniqueSocket& DatagramKernel::getUnderlyingSocket()
{
	return _socket;
}

bool DatagramKernel::configureAsListen()
{
	return _socketPtr.setAsListenSocket();
}

UniqueSocket DatagramKernel::accept(sockaddr_storage &sockaddr)
{
	return _socketFactory.acceptFromListenSocket(_socket, sockaddr);
}

bool DatagramKernel::canReceive() const
{
    return _receiveStoragePool->canRequest();
}

bool DatagramKernel::stop()
{
    if(_socket)
        _socketPtr.close();

    _isStopped = true;
    return true;
}

size_t DatagramKernel::getCanReceiveSize()
{
    return _receiveStoragePool->getFreeSlotsCount();
}

size_t DatagramKernel::getMaxCanReceiveSize()
{
    return _receiveStoragePool->getCapacity();
}

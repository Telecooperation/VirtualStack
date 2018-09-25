#include "StreamKernel.h"

bool StreamKernel::dataAvailable()
{
	if (!_socketPtr.isValid())
		return false;

    auto res = _epoll->waitNonBlocking();
    if(res == ISocketEpoll::EpollResult::None)
        return false;

    auto bytesAvailable = _socketPtr.getBytesAvailable();
    if(bytesAvailable == 0 && res == ISocketEpoll::EpollResult::SocketError)
    {
        _socket->close();
        return false;
    }

    if(bytesAvailable < sizeof(size_t))
        return false;

    if(_currentPacketExpectedSize == 0)
        _socketPtr.receiveBlocking(reinterpret_cast<uint8_t*>(&_currentPacketExpectedSize), sizeof(size_t), true);

    return bytesAvailable >= _currentPacketExpectedSize;
}

StoragePoolPtr StreamKernel::receivePacket()
{
    //available was not called before, so we block until we got a full packet
    if(_currentPacketExpectedSize == 0)
    {
        while (_socketPtr.isValid() && !dataAvailable())
            continue;

        if (!_socketPtr.isValid())
            return StoragePoolPtr();
    }

    auto content = _receiveStoragePool->request();
    if(_currentPacketExpectedSize > content->freeSpaceForAppend())
    {
        Logger::Log(Logger::ERROR, "StreamKernel: Expected packetSize > freeSpaceForAppend. Closing connection");
        stop();
        return StoragePoolPtr();
    }

    auto bytesRead = _socketPtr.receiveBlocking(content->data(), _currentPacketExpectedSize, true);
    if(!_socketPtr.isValid() || bytesRead < 0)
    {
        stop();
        return StoragePoolPtr();
    }

    auto recvBytes = static_cast<size_t>(bytesRead);

    if(recvBytes == 0) //0 bytes mean the connection is closed
        _socketPtr.close();

    _currentPacketExpectedSize = 0;
	content->setSize(static_cast<size_t>(recvBytes));
	return content;
}

bool StreamKernel::sendPacket(Storage &content)
{
	if (!_socketPtr.isValid())
		return false;

//	auto tmpSourceSockAddr = SocketExtensions::getSourceSockaddrStorage(_socket);
//	auto tmpDestSockAddr = SocketExtensions::getDestinationSockaddrStorage(_socket);

//	Logger::Log(Logger::DEBUG, "Source: ", NetworkExtensions::getAddress(tmpSourceSockAddr), ", Port: ", NetworkExtensions::getPort(tmpSourceSockAddr));
//	Logger::Log(Logger::DEBUG, "Dest: ", NetworkExtensions::getAddress(tmpDestSockAddr), ", Port: ", NetworkExtensions::getPort(tmpDestSockAddr));

    size_t contentSize = content.size();

    //_socketPtr.sendMoreFlag(reinterpret_cast<const uint8_t *>(&contentSize), sizeof(size_t));
    content.prependDataAutomaticBeforeStart(&contentSize);
    _socketPtr.sendBlocking(content.data(), content.size());

	return true;
}

StreamKernel::~StreamKernel()
{
	stop();
}

UniqueSocket& StreamKernel::getUnderlyingSocket()
{
	return _socket;
}

bool StreamKernel::configureAsListen()
{
	return _socketPtr.setAsListenSocket();
}

UniqueSocket StreamKernel::accept(sockaddr_storage &sockaddr)
{
	return _socketFactory.acceptFromListenSocket(_socket, sockaddr);
}

bool StreamKernel::canReceive() const
{
    return _receiveStoragePool->canRequest();
}

bool StreamKernel::stop()
{
    //_socket.reset(); //close socket so blocking reads or writes cancel
    if(_socket)
        _socket->close();

    _isStopped = true;
    return true;
}

size_t StreamKernel::getCanReceiveSize()
{
    return _receiveStoragePool->getFreeSlotsCount();
}

size_t StreamKernel::getMaxCanReceiveSize()
{
    return _receiveStoragePool->getCapacity();
}

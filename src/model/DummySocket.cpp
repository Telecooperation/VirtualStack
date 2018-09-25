#include "DummySocket.h"

DummySocket::DummySocket() = default;

ssize_t DummySocket::receiveBlocking(uint8_t *buffer, size_t bufferSize, bool isStreambased)
{
    return 0;
}

ssize_t DummySocket::receiveNonBlocking(uint8_t *buffer, size_t bufferSize)
{
    return 0;
}

size_t DummySocket::sendBlocking(uint8_t *buffer, size_t size)
{
    return 0;
}

ssize_t DummySocket::sendNonBlocking(uint8_t *buffer, size_t size)
{
    return 0;
}

size_t DummySocket::getBytesAvailable()
{
    return 0;
}

bool DummySocket::connectSocket(const sockaddr_storage &connectAddress)
{
    return false;
}

bool DummySocket::setAsListenSocket()
{
    return false;
}

bool DummySocket::setNoDelay()
{
    return false;
}

bool DummySocket::isSocketClosed()
{
    return false;
}

bool DummySocket::isValid()
{
    return true;
}

bool DummySocket::setSocketReuse()
{
    return false;
}

bool DummySocket::bindSocket(const sockaddr_storage &bindAddress)
{
    return false;
}

bool DummySocket::bindToNetworkDevice(const std::string &interfaceName)
{
    return false;
}

sockaddr_storage DummySocket::getSource()
{
    return sockaddr_storage();
}

sockaddr_storage DummySocket::getDestination()
{
    return sockaddr_storage();
}

uint16_t DummySocket::getPort()
{
    return 0;
}

void DummySocket::close()
{

}

DummySocket::operator bool() const
{
    return true;
}

int DummySocket::getSystemFd() const
{
    return 0;
}

TransportProtocolEnum DummySocket::getTransportProtocol() const
{
    return TransportProtocolEnum::NONE;
}

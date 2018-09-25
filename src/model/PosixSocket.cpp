#include "../common/Helper/DomainExtensions.h"
#include "../common/Helper/Logger.h"
#include "../common/Helper/NetworkExtensions.h"
#include "PosixSocket.h"
#include <netinet/tcp.h>
#include <sys/ioctl.h>

#define SCTP_NODELAY	3		/* Get/set nodelay option. */

PosixSocket::PosixSocket(unique_fd&& socket, TransportProtocolEnum transportProtocol) : _socket(std::move(socket)),
                                                                                        _lastErrorCode(0),
                                                                                        _transportProtocol(transportProtocol)
{
    setNoDelay();
    setNonBlocking();
}

bool PosixSocket::failOnError() const
{
    switch (errno)
    {
        case EAGAIN:    /* empty rx queue */
        case ETIMEDOUT: /* recv timeout */
            return false;
        default:
        {
            return true;
        }
    }
}

ssize_t PosixSocket::receiveBlocking(uint8_t *buffer, size_t bufferSize, bool isStreambased)
{
    // if the socket is streambased, we read everything until we are done.
    // if it is not stream based, stop reading after first recv
    ssize_t bytesReceived = 0;
    do
    {
        auto res = ::recv(_socket, buffer + bytesReceived, bufferSize - static_cast<size_t>(bytesReceived), 0);
        if (res < 0)
        {
            if (!failOnError())
                continue;

            _socket.close();

            if(errno != _lastErrorCode)
            {
                Logger::Log(Logger::LogLevel::ERROR, "PosixSocket: receive() FATAL-ERROR errno: ", errno,
                            " , message: ",
                            strerror(errno));
                _lastErrorCode = errno;
            }
            return -1;
        }

        //Dont delete: It seperates between datagram and streambased recv.
        //Datagram with 0 means to read again!
        if(res == 0)
            continue;

        bytesReceived += res;
        if (!isStreambased || bytesReceived == static_cast<ssize_t>(bufferSize))
            return bytesReceived;
    } while (true);
}

ssize_t PosixSocket::receiveNonBlocking(uint8_t *buffer, size_t bufferSize)
{
    // if the socket is streambased, we read everything until we are done.
    // if it is not stream based, stop reading after first recv
    auto res = ::recv(_socket, buffer, bufferSize, 0);
    if (res < 0)
    {
        if (!failOnError())
            return 0;

        _socket.close();

        if(errno != _lastErrorCode)
        {
            Logger::Log(Logger::LogLevel::ERROR, "PosixSocket: receive() FATAL-ERROR errno: ", errno,
                        " , message: ",
                        strerror(errno));
            _lastErrorCode = errno;
        }
        return -1;
    }

    return res;
}

size_t PosixSocket::sendBlocking(uint8_t *buffer, size_t size)
{
    size_t bytesSent = 0;
    do
    {
        auto res = ::send(_socket, buffer + bytesSent, size - bytesSent, 0);
        if (res < 0)
        {
            if (!failOnError())
                continue;

            _socket.close();
            if(errno != _lastErrorCode)
            {
                Logger::Log(Logger::LogLevel::ERROR, "PosixSocket: send() FATAL-ERROR errno: ", errno,
                            " , message: ",
                            strerror(errno));
                _lastErrorCode = errno;
            }
            return 0;
        }

        bytesSent += static_cast<size_t>(res);
        if (bytesSent == size)
            return bytesSent;
    } while (true);
}

ssize_t PosixSocket::sendNonBlocking(uint8_t *buffer, size_t size)
{
    auto res = ::send(_socket, buffer, size, 0);
    if (res < 0)
    {
        if (!failOnError())
            return 0;

        _socket.close();
        if(errno != _lastErrorCode)
        {
            Logger::Log(Logger::LogLevel::ERROR, "PosixSocket: send() FATAL-ERROR errno: ", errno,
                        " , message: ",
                        strerror(errno));
            _lastErrorCode = errno;
        }
        return -1;
    }

    return res;
}

size_t PosixSocket::getBytesAvailable()
{
    size_t bytesAvailable = 0u;
    if (ioctl(_socket, FIONREAD, &bytesAvailable) == 0)
        return bytesAvailable;

    switch (errno)
    {
        case EAGAIN:    /* empty rx queue */
        case ETIMEDOUT: /* recv timeout */
        case ENOTCONN:  /* not connected yet */
            {
                if(errno != _lastErrorCode)
                {
                    Logger::Log(Logger::LogLevel::ERROR, "PosixSocket: getBytesAvailable() failed with errno: ", errno,
                                " , message: ", strerror(errno));
                    _lastErrorCode = errno;
                }
                break;
            }
        default:
        {
            _socket.reset();
            break;
        }
    }

    return 0;
}

bool PosixSocket::setNoDelay()
{
    int proto = DomainExtensions::convertToSystem(_transportProtocol);
    int noDelayOpt = 0;

    switch(_transportProtocol)
    {
        case TransportProtocolEnum::TCP:
        {
            noDelayOpt = TCP_NODELAY;
            break;
        }
        case TransportProtocolEnum::SCTP:
        {
            noDelayOpt = SCTP_NODELAY;
            break;
        }
        default:
            return false;
    }

    int flag = 1;
    if(setsockopt(_socket, proto, noDelayOpt, &flag, sizeof(flag)) < 0)
    {
        Logger::Log(Logger::LogLevel::ERROR, "Setting setNoDelay failed -> Error: ", errno, ", Message: ",
                    strerror(errno));
        return false;
    }

    return true;
}

bool PosixSocket::connectSocket(const sockaddr_storage &connectAddress)
{
    auto ret = connect(_socket, reinterpret_cast<const sockaddr *>(&connectAddress), sizeof(sockaddr_storage));
    if(ret == 0)
        return true;

    switch(errno)
    {
        case EISCONN:
            return true;
        case EALREADY:
        case EINPROGRESS:
        {
            if(isWriteable(5000))
                return hasErrors();
            return false;
        }
        default:
        {
            if(errno != _lastErrorCode)
            {
                Logger::Log(Logger::LogLevel::ERROR, "connect() for Socket was not successful -> Error: ", errno,
                            ", Message: ",
                           strerror(errno));
                _lastErrorCode = errno;
            }
            return false;
        }
    }
}

bool PosixSocket::setAsListenSocket()
{
    if(::listen(_socket, 5) < 0)
    {
        Logger::Log(Logger::LogLevel::ERROR, "listen() for Socket was not successful -> Error: ", errno, ", Message: ",
                    strerror(errno));
        return false;
    }
    return true;
}

bool PosixSocket::isSocketClosed()
{
    uint8_t tmpPeekBuffer;

    ssize_t r;
    interrupted:;
    r = ::recv(_socket, &tmpPeekBuffer, 1, MSG_DONTWAIT | MSG_PEEK);

    if (r < 0) {
        switch (errno) {
            case EINTR:     goto interrupted;
            case EAGAIN:    break; /* empty rx queue */
            case ETIMEDOUT: break; /* recv timeout */
            case ENOTCONN:  break; /* not connected yet */
            default:
            {
                return true;
            }
        }
    }
    return r == 0;
}

bool PosixSocket::isValid()
{
    return _socket;
}

bool PosixSocket::setSocketReuse()
{
    int flag = 1;
    if(setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))    /* length of option value */
       < 0)
    {
        Logger::Log(Logger::LogLevel::ERROR, "Setting Socket-Reuse failed -> Error: ", errno, ", Message: ",
                    strerror(errno));
        return false;
    }

    return true;
}

bool PosixSocket::bindSocket(const sockaddr_storage &bindAddress)
{
    if (bind(_socket, reinterpret_cast<const sockaddr *>(&bindAddress), sizeof(sockaddr_storage)) >= 0)
        return true;

    std::string tmpError(strerror(errno));

    auto address = NetworkExtensions::getAddress(bindAddress);
    auto port = NetworkExtensions::getPort(bindAddress);
    Logger::Log(Logger::LogLevel::ERROR, "bind() for Socket to: ",
                address,
                " on Port: ", port,
                " was not successful -> Error: ", errno, ", Message: ",
                tmpError);
    return false;
}

bool PosixSocket::bindToNetworkDevice(const std::string &interfaceName)
{
    int result = setsockopt(_socket, SOL_SOCKET, SO_BINDTODEVICE, interfaceName.c_str(), static_cast<socklen_t>(interfaceName.length()));
    if(result < 0)
    {
        Logger::Log(Logger::ERROR, "Error binding socket to southbound device: \"", interfaceName ,"\" with error: ", strerror(errno));
        return false;
    }
    return true;
}

sockaddr_storage PosixSocket::getSource()
{
    sockaddr_storage serv_addr{};
    bzero(&serv_addr, sizeof(serv_addr));

    socklen_t len = sizeof(serv_addr);
    if (getsockname(_socket, reinterpret_cast<sockaddr *>(&serv_addr), &len) < 0) {
        Logger::Log(Logger::ERROR, "Failed to get Socket Source: ", strerror(errno));
        return serv_addr;
    }

    return serv_addr;
}

sockaddr_storage PosixSocket::getDestination()
{
    sockaddr_storage serv_addr{};
    bzero(&serv_addr, sizeof(serv_addr));

    socklen_t len = sizeof(serv_addr);
    if (getpeername(_socket, reinterpret_cast<sockaddr *>(&serv_addr), &len) < 0) {
        Logger::Log(Logger::ERROR, "Failed to get Socket Destination: ", strerror(errno));
        return serv_addr;
    }

    return serv_addr;
}

uint16_t PosixSocket::getPort()
{
    return NetworkExtensions::getPort(getSource());
}

PosixSocket::operator bool() const
{
    return _socket;
}

int PosixSocket::getSystemFd() const
{
    return _socket;
}

void PosixSocket::close()
{
    _socket.reset();
}

TransportProtocolEnum PosixSocket::getTransportProtocol() const
{
    return _transportProtocol;
}

bool PosixSocket::setNonBlocking()
{
    /* set socket to non-blocking i/o */
    int flag = 1;
    if (ioctl(_socket, FIONBIO, &flag) < 0)
    {
        Logger::Log(Logger::ERROR, "Failed to set Socket NonBlocking: ", strerror(errno));
        return false;
    }
    return true;
}

bool PosixSocket::isWriteable(int timeoutMs) const
{
    unique_fd epollFd{epoll_create(1)};
    epoll_event epollEvent{};
    bzero(&epollEvent, sizeof(epollEvent));
    epollEvent.events = EPOLLOUT | EPOLLERR;
    epollEvent.data.fd = _socket.get();

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent) != 0)
    {
        Logger::Log(Logger::ERROR, "PosixSocket isWriteable(): failed to ADD socket to epoll: ", strerror(errno));
        return false;
    }

    auto res = epoll_wait(epollFd, &epollEvent, 1, timeoutMs);
    return res != 0 && (epollEvent.events & EPOLLOUT) != 0;
}

bool PosixSocket::hasErrors() const
{
    int error = 0;
    socklen_t len = sizeof(error);
    if(getsockopt(_socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
    {
        //Logger::Log(Logger::ERROR, "PosixSocket hasErrors(): socket has errors: ", strerror(errno));
        return true;
    }
    return false;
}

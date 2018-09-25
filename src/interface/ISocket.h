#pragma once


#include "../common/Helper/ClassMacros.h"
#include "../model/TransportProtocolEnum.h"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sys/socket.h>

class ISocket
{
public:
    ISocket();

    virtual ~ISocket();

    virtual ssize_t receiveBlocking(uint8_t *buffer, size_t bufferSize, bool isStreambased) = 0;
    virtual ssize_t receiveNonBlocking(uint8_t *buffer, size_t bufferSize) = 0;

    virtual size_t sendBlocking(uint8_t* buffer, size_t size) = 0;
    virtual ssize_t sendNonBlocking(uint8_t* buffer, size_t size) = 0;

    virtual size_t getBytesAvailable() = 0;

    virtual bool connectSocket(const sockaddr_storage &connectAddress) = 0;
    virtual bool setAsListenSocket() = 0;

    virtual bool setNoDelay() = 0;

    /**
     * Ask the network if the socket is closed
     * @return True if socket is closed
     */
    virtual bool isSocketClosed() = 0;
    /**
     * Check if the socket is valid. Does not check if it has been closed remotely
     * @return True if socket is valid
     */
    virtual bool isValid() = 0;

    virtual bool setSocketReuse() = 0;
    virtual bool bindSocket(const sockaddr_storage &bindAddress) = 0;
    virtual bool bindToNetworkDevice(const std::string& interfaceName) = 0;

    virtual sockaddr_storage getSource() = 0;
    virtual sockaddr_storage getDestination() = 0;
    virtual uint16_t getPort() = 0;

    virtual int getSystemFd() const = 0;

    virtual TransportProtocolEnum getTransportProtocol() const = 0;

    virtual void close() = 0;

    virtual operator bool() const = 0;

    ALLOW_MOVE_SEMANTICS_ONLY(ISocket);
};

typedef std::unique_ptr<ISocket> UniqueSocket;



#pragma once


#include "../common/DataStructures/Container/unique_fd.h"
#include "../interface/ISocket.h"
#include "PosixSocketEpoll.h"

class PosixSocket final : public ISocket
{
public:
    explicit PosixSocket(unique_fd&& _socket, TransportProtocolEnum transportProtocol);

    ssize_t receiveBlocking(uint8_t *buffer, size_t bufferSize, bool isStreambased) override;
    ssize_t receiveNonBlocking(uint8_t *buffer, size_t bufferSize) override;

    size_t sendBlocking(uint8_t *buffer, size_t size) override;
    ssize_t sendNonBlocking(uint8_t *buffer, size_t size) override;

    size_t getBytesAvailable() override;

    bool connectSocket(const sockaddr_storage &connectAddress) override;

    bool setAsListenSocket() override;

    bool setNoDelay() override;

    bool isSocketClosed() override;

    bool isValid() override;

    bool setSocketReuse() override;

    bool bindSocket(const sockaddr_storage &bindAddress) override;

    bool bindToNetworkDevice(const std::string &interfaceName) override;

    sockaddr_storage getSource() override;

    sockaddr_storage getDestination() override;

    uint16_t getPort() override;

    void close() override;

    operator bool() const override;

    int getSystemFd() const override;

    bool setNonBlocking();

    TransportProtocolEnum getTransportProtocol() const override;
private:
    inline bool failOnError() const;
    inline bool isWriteable(int timeoutMs) const;
    inline bool hasErrors() const;
    unique_fd _socket;
    int _lastErrorCode;
    const TransportProtocolEnum  _transportProtocol;
};



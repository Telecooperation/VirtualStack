#pragma once


#include "../interface/ISocketFactory.h"

class PosixSocketFactory final : public ISocketFactory
{
public:
    UniqueSocket createTCPSocket(InternetProtocolEnum internetProtocol) override;

    UniqueSocket createUDPSocket(InternetProtocolEnum internetProtocol) override;

    UniqueSocket createUDPLITESocket(InternetProtocolEnum internetProtocol) override;

    UniqueSocket createSCTPSocket(InternetProtocolEnum internetProtocol) override;

    UniqueSocket createDCCPSocket(InternetProtocolEnum internetProtocol) override;

    UniqueSocket holePunching(UniqueSocket &&fd, const bool isInitiator, const TransportProtocolEnum transportProtocol,
                              sockaddr_storage &destination, const bool isLocalhost = false) override;

    UniqueSocket createSocketForHolePunching(const InternetProtocolEnum internetProtocol,
                                             const TransportProtocolEnum transportProtocol,
                                             const std::string &ip) override;

    UniqueSocket listenAndAcceptSocket(UniqueSocket &socket, sockaddr_storage &acceptedSocketAddress) override;

    UniqueSocket acceptFromListenSocket(UniqueSocket &socket, sockaddr_storage &acceptedSocketAddress) override;

    std::unique_ptr<ISocketEpoll> createEpoll(const ISocket  &socket) override;
};



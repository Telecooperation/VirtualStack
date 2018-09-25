#pragma once

#include "../model/InternetProtocolEnum.h"
#include "../model/TransportProtocolEnum.h"
#include "ISocket.h"
#include "ISocketEpoll.h"

class ISocketFactory
{
public:
    ISocketFactory();

    virtual ~ISocketFactory();

    virtual UniqueSocket createSocket(InternetProtocolEnum internetProtocol, TransportProtocolEnum transportProtocol);
    virtual UniqueSocket createTCPSocket(InternetProtocolEnum internetProtocol) = 0;
    virtual UniqueSocket createUDPSocket(InternetProtocolEnum internetProtocol) = 0;
    virtual UniqueSocket createUDPLITESocket(InternetProtocolEnum internetProtocol) = 0;
    virtual UniqueSocket createSCTPSocket(InternetProtocolEnum internetProtocol) = 0;
    virtual UniqueSocket createDCCPSocket(InternetProtocolEnum internetProtocol) = 0;

    virtual UniqueSocket holePunching(UniqueSocket &&fd, const bool isInitiator, const TransportProtocolEnum transportProtocol,
                                  sockaddr_storage &destination, const bool isLocalhost = false) = 0;

    virtual UniqueSocket createSocketForHolePunching(const InternetProtocolEnum internetProtocol,
                                                 const TransportProtocolEnum transportProtocol,
                                                 const std::string& ip) = 0;

    virtual UniqueSocket listenAndAcceptSocket(UniqueSocket &socket, sockaddr_storage &acceptedSocketAddress) = 0;

    virtual UniqueSocket acceptFromListenSocket(UniqueSocket &socket, sockaddr_storage &acceptedSocketAddress) = 0;

    virtual UniqueSocket createConnection(InternetProtocolEnum internetProtocol,
                                          TransportProtocolEnum transportProtocol,
                                          bool isInitiator,
                                          sockaddr_storage &destination,
                                          sockaddr_storage *source,
                                          const std::string *interfaceName);

    virtual UniqueSocket createConnection(UniqueSocket&& socket,
                                          bool isInitiator,
                                          sockaddr_storage &destination,
                                          sockaddr_storage *source,
                                          const std::string *interfaceName);

    virtual std::unique_ptr<ISocketEpoll> createEpoll(const ISocket &socket) = 0;

private:
    inline UniqueSocket genericReliableConnection(UniqueSocket &&fd,
                                           const bool isInitiator,
                                           sockaddr_storage &destination,
                                           sockaddr_storage *source,
                                           const std::string *interfaceName);
};



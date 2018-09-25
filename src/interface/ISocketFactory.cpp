#include "../common/Helper/DomainExtensions.h"
#include "../common/Helper/Logger.h"
#include "ISocketFactory.h"
#include <thread>

ISocketFactory::ISocketFactory() = default;

ISocketFactory::~ISocketFactory() = default;

UniqueSocket ISocketFactory::createSocket(InternetProtocolEnum internetProtocol,
                                          TransportProtocolEnum transportProtocol)
{
    switch (transportProtocol)
    {
        case TransportProtocolEnum::TCP:
            return createTCPSocket(internetProtocol);
        case TransportProtocolEnum::UDP:
            return createUDPSocket(internetProtocol);
        case TransportProtocolEnum::UDPLITE:
            return createUDPLITESocket(internetProtocol);
        case TransportProtocolEnum::SCTP:
            return createSCTPSocket(internetProtocol);
        case TransportProtocolEnum::DCCP:
            return createDCCPSocket(internetProtocol);
        default:
            return nullptr;
    }
}


UniqueSocket ISocketFactory::createConnection(InternetProtocolEnum internetProtocol,
                                              TransportProtocolEnum transportProtocol,
                                              bool isInitiator,
                                              sockaddr_storage &destination,
                                              sockaddr_storage *source,
                                              const std::string *interfaceName = nullptr)
{
    if(transportProtocol == TransportProtocolEnum::UDP || transportProtocol == TransportProtocolEnum::UDPLITE)
        isInitiator = true; //UDP does not have listenAndAccept

    auto fd = createSocket(internetProtocol, transportProtocol);
    return genericReliableConnection(std::move(fd), isInitiator, destination, source, interfaceName);
}

UniqueSocket ISocketFactory::createConnection(UniqueSocket&& socket,
                                              bool isInitiator,
                                              sockaddr_storage &destination,
                                              sockaddr_storage *source,
                                              const std::string *interfaceName = nullptr)
{
    if(!socket)
        return UniqueSocket();

    auto transportProtocol = socket->getTransportProtocol();
    if(transportProtocol == TransportProtocolEnum::UDP || transportProtocol == TransportProtocolEnum::UDPLITE)
        isInitiator = true; //UDP does not have listenAndAccept

    return genericReliableConnection(std::move(socket), isInitiator, destination, source, interfaceName);
}

UniqueSocket ISocketFactory::genericReliableConnection(UniqueSocket &&fd,
                                                       const bool isInitiator,
                                                       sockaddr_storage &destination,
                                                       sockaddr_storage *source,
                                                       const std::string *interfaceName)
{
    if (!fd)
        return std::move(fd);

    fd->setSocketReuse();
    if (source != nullptr)
        fd->bindSocket(*source);

    if (interfaceName != nullptr && !interfaceName->empty())
        fd->bindToNetworkDevice(*interfaceName);

    if (!isInitiator)
        return listenAndAcceptSocket(fd, destination);

    bool tmpConnectResult = false;
    for (size_t i = 0; i < 500; ++i)
    {
        tmpConnectResult = fd->connectSocket(destination);
        if(tmpConnectResult)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!tmpConnectResult)
        return nullptr;

    return std::move(fd);
}

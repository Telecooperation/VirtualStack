#include "../common/Helper/DomainExtensions.h"
#include "../common/Helper/NetworkExtensions.h"
#include "PosixSocket.h"
#include "PosixSocketEpoll.h"
#include "PosixSocketFactory.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/ioctl.h>

UniqueSocket PosixSocketFactory::createTCPSocket(InternetProtocolEnum internetProtocol)
{
    unique_fd sock{socket(DomainExtensions::convertToSystem(internetProtocol), SOCK_STREAM, IPPROTO_TCP)};
    if (sock)
        return std::make_unique<PosixSocket>(std::move(sock), TransportProtocolEnum::TCP);

    Logger::Log(Logger::ERROR, "createTCPSocket failed with errno: ", errno, ", message: ", strerror(errno));
    return UniqueSocket();
}

UniqueSocket PosixSocketFactory::createUDPSocket(InternetProtocolEnum internetProtocol)
{
    unique_fd sock{socket(DomainExtensions::convertToSystem(internetProtocol), SOCK_DGRAM, IPPROTO_UDP)};

    if (sock)
        return std::make_unique<PosixSocket>(std::move(sock), TransportProtocolEnum::UDP);

    Logger::Log(Logger::ERROR, "createUDPSocket failed with errno: ", errno, ", message: ", strerror(errno));
    return UniqueSocket();
}

UniqueSocket PosixSocketFactory::createUDPLITESocket(InternetProtocolEnum internetProtocol)
{
    unique_fd sock{socket(DomainExtensions::convertToSystem(internetProtocol), SOCK_DGRAM, IPPROTO_UDPLITE)};

    if (sock)
        return std::make_unique<PosixSocket>(std::move(sock), TransportProtocolEnum::UDPLITE);

    Logger::Log(Logger::ERROR, "createUDPLITESocket failed with errno: ", errno, ", message: ", strerror(errno));
    return UniqueSocket();
}

UniqueSocket PosixSocketFactory::createSCTPSocket(InternetProtocolEnum internetProtocol)
{
    unique_fd sock{socket(DomainExtensions::convertToSystem(internetProtocol), SOCK_STREAM, IPPROTO_SCTP)};
    if (sock)
        return std::make_unique<PosixSocket>(std::move(sock), TransportProtocolEnum::SCTP);

    Logger::Log(Logger::ERROR, "createSCTPSocket failed with errno: ", errno, ", message: ", strerror(errno));
    return UniqueSocket();
}

UniqueSocket PosixSocketFactory::createDCCPSocket(InternetProtocolEnum internetProtocol)
{
    unique_fd sock{socket(DomainExtensions::convertToSystem(internetProtocol), SOCK_DCCP, IPPROTO_DCCP)};
    if (sock)
        return std::make_unique<PosixSocket>(std::move(sock), TransportProtocolEnum::DCCP);

    Logger::Log(Logger::ERROR, "createDCCPocket failed with errno: ", errno, ", message: ", strerror(errno));
    return UniqueSocket();
}

UniqueSocket PosixSocketFactory::holePunching(UniqueSocket &&fd, const bool isInitiator,
                                              const TransportProtocolEnum transportProtocol,
                                              sockaddr_storage &destination,
                                              const bool isLocalhost)
{
    if (transportProtocol == TransportProtocolEnum::RAW)
    {
        Logger::Log(Logger::ERROR, "Hole punching for raw socket is not supported");
        return UniqueSocket();
    }
    fd->setSocketReuse();

//	auto ip = NetworkExtensions::getAddress(destination);
//	auto destPort = NetworkExtensions::getPort(destination);
//	auto sourcePort = fd->getPort();
    bool mayAccept = transportProtocol == TransportProtocolEnum::TCP ||
                    transportProtocol == TransportProtocolEnum::SCTP ||
                    transportProtocol == TransportProtocolEnum::DCCP;

    // TCP holepunch works if both use connect() but on localhost this wont work.
    // So for localhost we skip holepunch and use default listen/accept
    if(mayAccept && !isInitiator && isLocalhost)
    {
//        Logger::Log(Logger::LogLevel::INFO, "HolePunch-Listen Dest: IP: ", ip, " PORT: ", port);
        fd = listenAndAcceptSocket(fd, destination);
    }
    else {
//        Logger::Log(Logger::LogLevel::INFO, "HolePunch-Connect Dest: IP: ", ip, " Source-PORT: ", sourcePort, " Dest-PORT: ", destPort);
        bool tmpConnectResult = false;
        for (size_t i = 0; i < 2000; ++i)
        {
            tmpConnectResult = fd->connectSocket(destination);
            if(tmpConnectResult)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
//            Logger::Log(Logger::DEBUG, "Retry connect");
        }
        if(!tmpConnectResult)
            return UniqueSocket();
    }

    return std::move(fd);
}

UniqueSocket PosixSocketFactory::createSocketForHolePunching(const InternetProtocolEnum internetProtocol,
                                                             const TransportProtocolEnum transportProtocol,
                                                             const std::string &ip)
{
    UniqueSocket newSocket = createSocket(internetProtocol, transportProtocol);
    if (!newSocket)
        return UniqueSocket();

    auto manSocketSource = NetworkExtensions::getSockAddr(internetProtocol, ip, 0);
    if(!newSocket->bindSocket(manSocketSource))
        return UniqueSocket();

    return newSocket;
}

UniqueSocket PosixSocketFactory::listenAndAcceptSocket(UniqueSocket &socket, sockaddr_storage &acceptedSocketAddress)
{
    socket->setAsListenSocket();
    return acceptFromListenSocket(socket, acceptedSocketAddress);
}

UniqueSocket PosixSocketFactory::acceptFromListenSocket(UniqueSocket &socket, sockaddr_storage &acceptedSocketAddress)
{
    if(!socket)
        return UniqueSocket();

    auto poll = createEpoll(*socket);
    auto ret = poll->waitTimeout(5000);
    if (ret != ISocketEpoll::EpollResult::DataAvailable)
    {
        Logger::Log(Logger::WARNING,
                    "Canceled acceptFromListenSocket for Socket because of no connection made for socket or timeout: ",
                    socket->getSystemFd());
        return UniqueSocket();
    }

    socklen_t acceptedAddressSize = sizeof(acceptedSocketAddress);
    unique_fd clientSocket{::accept(socket->getSystemFd(), reinterpret_cast<struct sockaddr *>(&acceptedSocketAddress), &acceptedAddressSize)};
    if (clientSocket < 0)
    {
        std::string tmpError(strerror(errno));
        Logger::Log(Logger::LogLevel::ERROR, "accept() for Socket was not successful -> Error: ", errno, ", Message: ",
                    tmpError);
        return UniqueSocket();
    }
    return std::make_unique<PosixSocket>(std::move(clientSocket), socket->getTransportProtocol());
}

std::unique_ptr<ISocketEpoll> PosixSocketFactory::createEpoll(const ISocket &socket)
{
    return std::make_unique<PosixSocketEpoll>(socket);
}

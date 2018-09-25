#include "../common/Helper/Logger.h"
#include "PosixSocketEpoll.h"
#include <cstring>
#include <sys/epoll.h>

PosixSocketEpoll::PosixSocketEpoll(const ISocket& socket) :
        ISocketEpoll(socket),
        _epollFd(epoll_create(1)),
        _initEpollEvent(),
        _runningEpollEvent()
{
    bzero(&_initEpollEvent, sizeof(_initEpollEvent));
    bzero(&_runningEpollEvent, sizeof(_runningEpollEvent));
    _initEpollEvent.events = EPOLLIN | EPOLLERR;
    _initEpollEvent.data.fd = socket.getSystemFd();

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _initEpollEvent.data.fd, &_initEpollEvent) != 0) {
		Logger::Log(Logger::ERROR, "EPollController, could not add epollEvent to the epollfd: ", strerror(errno));
        _epollFd.reset();
	}
}

PosixSocketEpoll::~PosixSocketEpoll()
{
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, _initEpollEvent.data.fd, &_initEpollEvent);
//    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, _epollEvent.data.fd, &_epollEvent) != 0) {
//        Logger::Log(Logger::ERROR, "EPollController, could not delete epollEvent of the epollfd: ", strerror(errno));
//    }
}

bool PosixSocketEpoll::isValid()
{
    return _epollFd;
}

ISocketEpoll::EpollResult PosixSocketEpoll::waitNonBlocking()
{
    int val = epoll_wait(_epollFd, &_runningEpollEvent, 1, 0);
    if(val == 0)
        return ISocketEpoll::EpollResult::None;

    return getEpollResult();
}

ISocketEpoll::EpollResult PosixSocketEpoll::waitTimeout(uint32_t timeoutInMilliseconds)
{
    int val = epoll_wait(_epollFd, &_runningEpollEvent, 1, static_cast<int>(timeoutInMilliseconds));
    if(val == 0)
        return ISocketEpoll::EpollResult::None;

    return getEpollResult();
}

ISocketEpoll::EpollResult PosixSocketEpoll::getEpollResult()
{
    auto val = _runningEpollEvent.events;
    if(val == 0)
        return ISocketEpoll::EpollResult::None;
    if((val & (EPOLLHUP | EPOLLERR)) > 0)
        return ISocketEpoll::EpollResult::SocketError;
    if((val & EPOLLIN) > 0)
        return ISocketEpoll::EpollResult::DataAvailable;
    return ISocketEpoll::EpollResult::None;
}

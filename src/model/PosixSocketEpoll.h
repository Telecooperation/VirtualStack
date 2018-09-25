#pragma once


#include "../common/DataStructures/Container/unique_fd.h"
#include "../interface/ISocketEpoll.h"
#include <sys/epoll.h>

class PosixSocketEpoll final : public ISocketEpoll
{
public:
    explicit PosixSocketEpoll(const ISocket& socket);

    ~PosixSocketEpoll() override;

    bool isValid() override;

    EpollResult waitNonBlocking() override;

    EpollResult waitTimeout(uint32_t timeoutInMilliseconds) override;

    ALLOW_MOVE_SEMANTICS_ONLY(PosixSocketEpoll);
private:
    inline EpollResult getEpollResult();

    unique_fd _epollFd;
    epoll_event _initEpollEvent;
    epoll_event _runningEpollEvent;
};



#pragma once


#include "ISocket.h"

class ISocketEpoll
{
public:
    enum class EpollResult
    {
        None,
        DataAvailable,
        SocketError
    };


    ISocketEpoll(const ISocket&);

    virtual ~ISocketEpoll();

    /**
     * Check if epoll was created successfully
     * @return True if successfull
     */
    virtual bool isValid() = 0;

    /**
     * Check if new data is available without blocking
     * @return True if data is available
     */
    virtual EpollResult waitNonBlocking() = 0;

    /**
     * Check if new data is available with timeout
     * @return True if data is available
     */
    virtual EpollResult waitTimeout(uint32_t timeoutInMilliseconds) = 0;
};



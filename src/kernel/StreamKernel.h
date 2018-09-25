#pragma once

#include "../common/Allocator/VsObjectFactory.h"
#include "../common/DataStructures/Container/RingBufferMove.h"
#include "../common/DataStructures/Container/unique_fd.h"
#include "../interface/IKernel.h"
#include "../interface/ISocket.h"
#include "../VirtualStackSettings.h"

class StreamKernel final : public IKernel
{
public:
	StreamKernel(UniqueSocket socket, const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory):
            _isStopped(false),
			_socketFactory(*vsObjectFactory.socketFactory),
			_receiveStoragePool(vsObjectFactory.getKernelPool(settings.SizeOfKernelBuffer, "StreamKernel")),
			_socket(std::move(socket)),
            _socketPtr(*_socket),
            _epoll(_socketFactory.createEpoll(_socketPtr)),
            _currentPacketExpectedSize(0)
	{
	}

    bool stop() override;

    bool dataAvailable() override;

	StoragePoolPtr receivePacket() override;

	bool sendPacket(Storage &content) override;

	UniqueSocket& getUnderlyingSocket() override;

	bool configureAsListen() override;

	bool isValid() const override { return _socket && _socket->isValid(); }

	UniqueSocket accept(sockaddr_storage &sockaddr) override;

	bool canReceive() const override;

	~StreamKernel() override;

    size_t getCanReceiveSize() override;

    size_t getMaxCanReceiveSize() override;

    ALLOW_MOVE_SEMANTICS_ONLY(StreamKernel);
private:
    std::atomic<bool> _isStopped;
	ISocketFactory& _socketFactory;
	PoolRef _receiveStoragePool;
	UniqueSocket _socket;
    ISocket& _socketPtr;
    std::unique_ptr<ISocketEpoll> _epoll;

    size_t _currentPacketExpectedSize;
};



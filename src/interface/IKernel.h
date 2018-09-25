#pragma once

#include "../common/DataStructures/Model/Storage.h"
#include "ISocket.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sys/socket.h>
#include <vector>

/**
 * Provides the Interface for a Kernel to be used in the pipeline of sending data.
 * This is only a peace of the pipeline but not the base kernel which will actually send and receive the payload @see INetworkKernel
 */
class IKernel{
public:
    /**
     * Start the Kernel
     */
	virtual bool start();

    /**
     * Stop the Kernel
     */
    virtual bool stop();

	/**
	 * Get the size in bytes this Kernel will add as a header to the data send through it
	 * @return Size in bytes occupied by this kernel in a received Storage
	 */
	virtual size_t getKernelHeaderSize() const;
	
	/**
	 * Traverse through all kernels in this kernel chain and accumulate their kerneHeaderSize
	 * @return Accumulated size in bytes occupied by all kernels in a received Storage
	 */
	size_t getCompleteHeaderSize() const;
	
	/**
	 * Sends a payload through the pipeline without doing anything until it reaches the bottom kernel.
	 * @param content The payload to send
	 * @return True if send was successful, false otherwise
	 */
	virtual bool sendPacketBypassed(Storage& content);
	/**
	 * Get whether bytes are currently available to be received
	 * @return true iff bytes are available to be received
	 */
	virtual bool dataAvailable();
	
	/**
	 * Receives data from its unterlying Kernel. If this kernel is the last Kernel, it has to supply the data
	 * The received data will only be the payload without the header which will be stripped in this method
	 * @return The raw Payload without the header of this kernel
	 */
	virtual StoragePoolPtr receivePacket() = 0;
	/**
	 * Sends a payload through the pipeline of all underlying kernels. Modifications on content are possible
	 * @param content The payload to send
	 * @return True if send was successful, false otherwise
	 */
	virtual bool sendPacket(Storage &content) = 0;
	
	/**
	 * Get the underlying socket of this kernel
	 * @return The socket or nullptr if not existing
	 */
	virtual UniqueSocket&  getUnderlyingSocket();
	
	/**
	 * Configures the kernel to be a listenkernel for incoming connections
	 * @return True if successfull
	 */
	virtual bool configureAsListen();
	
	/**
	 * Listens for incoming connections and creates a new socket out of it
	 * @return the socket
	 */
	virtual UniqueSocket accept(sockaddr_storage& sockaddr);

	/***
	 * Get if the Socket is valid
	 * @return true if socket is valid
	 */
	virtual bool isValid() const;

	/***
	 * Check if data can be received despite its availability
	 * @return True if data can be received
	 */
	virtual bool canReceive() const;

    /***
     * Get the size of how many receives can be done
     * @return size
     */
    virtual size_t getCanReceiveSize();

    /***
     * Get the max size of how many receives can be done
     * @return size
     */
    virtual size_t getMaxCanReceiveSize();
	
	virtual ~IKernel();
	IKernel() = default;
	ALLOW_MOVE_SEMANTICS_ONLY(IKernel);
protected:
	/**
	 * The underlying kernel for sending and receiving payload through. If it is the last Kernel @see INetworkKernel
	 */
	std::unique_ptr<IKernel> _kernel;
};

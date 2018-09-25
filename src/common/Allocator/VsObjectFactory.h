
#pragma once

#include "../../interface/ISocketFactory.h"
#include "../DataStructures/Model/Storage.h"
#include "IPool.h"
#include "Pool.h"
#include <map>
#include <memory>
#include <mutex>

/// Typedef for custom destructor for Pools of type Storage
typedef std::unique_ptr<Pool<Storage>, std::function<void(IPool*)>> PoolRef;

/// Container for all requested pools and ISocketFactory for VirtualStack.
/// It keeps all memory in pools alive. If a pool does not have memory in use anymore it will be freed
/// The purpose is that we only delete the pool and its memory if and only if noone uses memory of a pool anymore
class VsObjectFactory
{
public:
    /// Create instance of VsObjectFactory
    /// \param pSocketFactory A domain specific instance of ISocketFactory
    /// \param storageSize The max size of storage
    /// \param storageHeaderPadding The max size of header in a storage
    /// \param storageAlmSize The max size of all alms in the pipeline
	VsObjectFactory(std::unique_ptr<ISocketFactory>&& pSocketFactory,
					size_t storageSize,
					size_t storageHeaderPadding,
                    size_t storageAlmSize) :
			socketFactory(std::move(pSocketFactory)),
			_storageSize(storageSize),
			_storageHeaderPadding(storageHeaderPadding),
            _storageAlmSize(storageAlmSize),
            _requestedPoolsMutex(),
            _requestedPoolsKey(0),
            _requestedPools()
	{
	}

    /// Get a storagepool for sending storages
    /// \param poolSize The size of elements in the pool
    /// \param debugName The name for log messages
    /// \return The requested pool
    PoolRef getStorageSendPool(size_t poolSize, const std::string& debugName);

    /// Get a storagepool for receiving storages
    /// \param poolSize The size of elements in the pool
    /// \param debugName The name for log messages
    /// \return The requested pool
    PoolRef getKernelPool(size_t poolSize, const std::string& debugName);

	const std::unique_ptr<ISocketFactory> socketFactory;
private:
    /// Destructor function call for pool destruction
    /// \param vsObjectFactory The VSObjectFactory
    /// \param poolKey A unique key for the pool
    /// \param pool The pool itself
    static void poolDeleteFn(VsObjectFactory* vsObjectFactory, size_t poolKey, IPool* pool);

    /// Deletes the pool from the poolsList
    /// \param vsObjectFactory VSObjectFactory
    /// \param poolKey The unique poolKey of the pool
    /// \return True if successful
    static bool deletePool(VsObjectFactory* vsObjectFactory, size_t poolKey);

    /// Get a storagePool from given parameters
    /// \param poolSize The size of the pool
    /// \param headerSize The headerSize for the pool, so the Storages will have a incremented startIndex
    /// \param debugName The name for log messages
    /// \return The created storagePool
    inline PoolRef getStoragePool(size_t poolSize, size_t headerSize, const std::string& debugName);

	const size_t _storageSize;
	const size_t _storageHeaderPadding;
	const size_t _storageAlmSize;

    std::mutex _requestedPoolsMutex;
    size_t _requestedPoolsKey;
	std::map<size_t, std::unique_ptr<IPool>> _requestedPools;

};





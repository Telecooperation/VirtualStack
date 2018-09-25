#include "VsObjectFactory.h"

PoolRef VsObjectFactory::getStorageSendPool(size_t poolSize, const std::string &debugName)
{
    return getStoragePool(poolSize, _storageHeaderPadding + _storageAlmSize, debugName);
}

PoolRef VsObjectFactory::getKernelPool(size_t poolSize, const std::string &debugName)
{
    //without almSize so we can receive what a alm has added to the payload
    return getStoragePool(poolSize, _storageHeaderPadding, debugName);
}

PoolRef VsObjectFactory::getStoragePool(size_t poolSize, size_t headerSize, const std::string &debugName)
{
    //create a StoragePool and set the reset-function of the pool to the Storage::reset
    auto pool = std::make_unique<Pool<Storage>>(poolSize, debugName, [] (Storage& item) { item.reset(); }, _storageSize, headerSize);
    auto rawPoolPtr = pool.get();

    _requestedPoolsMutex.lock();
    auto newPoolKey = _requestedPoolsKey;
    ++_requestedPoolsKey;
    _requestedPools.emplace(newPoolKey, std::move(pool));
    _requestedPoolsMutex.unlock();

    //Return the pool but place a custom destructor for the pool itself so we only delete it if noone uses storage of this pool anymore
    return PoolRef(rawPoolPtr, std::bind(&VsObjectFactory::poolDeleteFn, this, newPoolKey, std::placeholders::_1));
}

void VsObjectFactory::poolDeleteFn(VsObjectFactory* vsObjectFactory, size_t poolKey, IPool* pool)
{
    if(vsObjectFactory == nullptr || pool == nullptr)
        return;

    std::lock_guard<std::mutex> lock(vsObjectFactory->_requestedPoolsMutex);
    if(!deletePool(vsObjectFactory, poolKey))
        pool->setOnLastReleaseCallback(std::bind(&VsObjectFactory::deletePool, vsObjectFactory, poolKey));
}

bool VsObjectFactory::deletePool(VsObjectFactory* vsObjectFactory, size_t poolKey)
{
    if(vsObjectFactory == nullptr)
        return true;

    auto poolIter = vsObjectFactory->_requestedPools.find(poolKey);
    if(poolIter == vsObjectFactory->_requestedPools.end())
    {
        //if this occurs, one tries to call a destructor multiple times
        Logger::Log(Logger::WARNING, "Tried to delete already deleted Pool");
        return true;
    }

    //we return false, because if it has a releaseCallback it will delete itself after noones uses the pool anymore
    if(poolIter->second->isInUse() || poolIter->second->hasReleaseCallback())
        return false;

    vsObjectFactory->_requestedPools.erase(poolKey);
    return true;
}


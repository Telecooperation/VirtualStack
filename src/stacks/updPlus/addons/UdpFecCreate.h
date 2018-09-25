#pragma once

#include "../../../common/Allocator/VsObjectFactory.h"
#include "../../../VirtualStackSettings.h"
#include "../../../common/DataStructures/Model/Storage.h"
#include "../../../interface/IKernel.h"
#include "../model/UdpPlusHeader.h"

class UdpFecCreate
{
public:
    //addToStorage
    //isFecReady
    //getFec

    //add

    UdpFecCreate(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory);

    void addToStorage(Storage &storageWithHeader, IKernel& kernel);

private:
    inline void resetFecBuffer();
    PoolRef _pool;

    const uint8_t _fecGroupSize;
    uint8_t _currentFecGroupIndex;
    size_t _currentFecGroup;
    StoragePoolPtr _fecBuffer;
    Storage& _fecBufferRef;
    UdpPlusFecHeader _storageHeader;
    UdpPlusFecHeader _fecHeader;
    const UdpPlusHeader _udpPlusHeader;
};



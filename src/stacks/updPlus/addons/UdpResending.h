#pragma once


#include "../../../VirtualStackSettings.h"
#include "../../../common/Allocator/VsObjectFactory.h"
#include "../../../common/DataStructures/Model/Storage.h"
#include "../../../interface/IKernel.h"

class UdpResending
{
public:
    UdpResending(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory);

    bool bufferStorage(const StoragePoolPtr& storage, size_t seqNum);
    void processNackInStorage(Storage& nackPayload, IKernel& kernel);
    bool canBuffer() const;
    size_t getCapacity() const;

    virtual ~UdpResending();

    ALLOW_MOVE_SEMANTICS_ONLY(UdpResending);
private:
    inline size_t toIndex(size_t val);
    PoolRef _resendingPool;
    const size_t _resendingBufferSize;
    std::unique_ptr<StoragePoolPtr[]> _resendingBuffer;

    size_t _firstMissingSeqNum;
    size_t _largestSentSeqNum;
    size_t _packetsResentCounter;
};



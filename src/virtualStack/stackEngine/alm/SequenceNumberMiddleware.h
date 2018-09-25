#pragma once


#include "../../../VirtualStackSettings.h"
#include "../../../common/DataStructures/Model/Storage.h"

class SequenceNumberMiddleware
{
public:
    explicit SequenceNumberMiddleware(const VirtualStackSettings& settings);
    ~SequenceNumberMiddleware() = default;

    void addSequenceNumber(StoragePoolPtr& storage);

    void addStorage(StoragePoolPtr&& storage);

    bool isAvailable();
    StoragePoolPtr getNextAvailable();

    ALLOW_MOVE_SEMANTICS_ONLY(SequenceNumberMiddleware);
private:
    inline size_t toIndex(size_t seqNum);
    size_t _seqNum;

    const size_t _bufferedStorageSize;
    std::unique_ptr<StoragePoolPtr[]> _bufferedStorage;
    size_t _firstMissingSeqNum;
};



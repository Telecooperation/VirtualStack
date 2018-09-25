#pragma once


#include "../../../VirtualStackSettings.h"
#include "../../../common/DataStructures/Model/Storage.h"
#include "../../../common/Allocator/VsObjectFactory.h"

class UdpSequenceNumber
{
public:
    explicit UdpSequenceNumber(const VirtualStackSettings& settings);
    ~UdpSequenceNumber() = default;

    size_t getNextSequenceNumber();
    bool addStorage(StoragePoolPtr&& storage, size_t sequenceNumber);

    /**
     * Fills the remaining space with NACKs
     * @param storage The storage
     */
    void fillWithNAcks(Storage& storage);

    bool isAvailable();
    StoragePoolPtr getNextAvailable();

    ALLOW_MOVE_SEMANTICS_ONLY(UdpSequenceNumber);
private:
    inline size_t toIndex(size_t seqNum);
    size_t _seqNum;

    const size_t _bufferedStorageSize;
    std::unique_ptr<StoragePoolPtr[]> _bufferedStorage;
    size_t _firstMissingSeqNum;
    size_t _largestReceivedSeqNum;
    bool _isNackPossible;

};



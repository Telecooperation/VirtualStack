#pragma once

#include "../../../VirtualStackSettings.h"
#include "../../../common/Allocator/VsObjectFactory.h"
#include "../../../common/DataStructures/Container/RingBufferMove.h"
#include "../../../common/DataStructures/Model/Storage.h"
#include "../../../interface/IKernel.h"


class UdpFecRestore
{
public:
    struct FecRestoreInfo
    {
        size_t groupNumber = 0;
        StoragePoolPtr buffer{};
        size_t seenFlag = 0; //bitflag map to check if a packet of this group has been received already
        bool containsFecPacket = false;
        uint8_t counter = 0;

        inline void reset(size_t groupNum = 0)
        {
            counter = 0;
            groupNumber = groupNum;
            seenFlag = 0;
            containsFecPacket = false;
            //dont reset buffer as it has to be nulled for reuse
        }

        void setSeen(uint8_t groupIndex)
        {
            seenFlag |= (1ul << groupIndex);
        }

        bool alreadySeen(uint8_t groupIndex) const
        {
            return (seenFlag & (1ul << groupIndex)) != 0;
        }

        operator bool() const noexcept {
            return buffer;
        }
    };

    UdpFecRestore(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory);

    bool receivePacket(Storage &storage);
    bool canRestore() const;
    StoragePoolPtr getNextRestored();
private:
    inline bool packetOfGroupCanBeRestored(const FecRestoreInfo& fecRestoreInfo) const;
    inline uint8_t getGroupIndex(const Storage& storage) const;
    inline size_t toIndex(size_t val);

    PoolRef _fecPool;

    const size_t _fecGroupSize;
    const size_t _fecRestoreBufferSize;
    std::unique_ptr<FecRestoreInfo[]> _fecRestoreBuffer;
    StoragePoolPtr _restoredPacket;
};



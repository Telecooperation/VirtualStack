#include "../model/UdpPlusHeader.h"
#include "UdpFecRestore.h"

UdpFecRestore::UdpFecRestore(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory) :
        _fecPool(vsObjectFactory.getKernelPool(settings.SizeOfUdpPlusFecRestoreBuffer, "UdpFecRestore")),
        _fecGroupSize(settings.SizeOfUdpPlusFecGroupSize),
        _fecRestoreBufferSize(_fecPool->getCapacity()),
        _fecRestoreBuffer(new FecRestoreInfo[_fecPool->getCapacity()]{}),
        _restoredPacket()
{
    assert(_fecGroupSize <= (sizeof(FecRestoreInfo::seenFlag) * 8));
}

bool UdpFecRestore::receivePacket(Storage &storage)
{
    if(storage.size() < sizeof(UdpPlusFecHeader))
        return false;

    auto fecHeader = storage.toTypeAutomatic<UdpPlusFecHeader>();
    storage.incrementStartIndex(sizeof(UdpPlusFecHeader));

    auto &fecRestoreInfo = _fecRestoreBuffer[toIndex(fecHeader.fecGroup)];

    //check if storage has been initialized
    if(!fecRestoreInfo || fecRestoreInfo.groupNumber < fecHeader.fecGroup)
    {
        fecRestoreInfo.reset(fecHeader.fecGroup);
        if(!fecRestoreInfo.buffer) //only nullify if storage is valid
        {
            if(!_fecPool->canRequest()) //we cannot allocate a new pool for this fec_group, so discard it
                return fecHeader.isFecPacket(); //we dont increment fecGroupCount, so we wont get incomplete fecRestoredPackets

            fecRestoreInfo.buffer = _fecPool->request();
        }

        fecRestoreInfo.buffer->nullifyData();
    }

    //packet is from older group, so we can discard it
    if(fecRestoreInfo.groupNumber > fecHeader.fecGroup)
        return fecHeader.isFecPacket();

    //check if fec-packet was seend
    if(fecHeader.isFecPacket())
    {
        if(fecRestoreInfo.containsFecPacket)
            return true; //discard fec packet
        fecRestoreInfo.containsFecPacket = true;
    }
    else //check if datapacket was seen
    {
        auto groupIndex = getGroupIndex(storage);
        if(fecRestoreInfo.alreadySeen(groupIndex))
            return false;
                //increment packets seen for this fec group. set special mark if we have seen the FEC packet of this group
        ++fecRestoreInfo.counter;
        fecRestoreInfo.setSeen(groupIndex);
    }

    //neu
    //teil eine unvollständigen gruppe
    //teil einer abgeschlossenen/wiederhergestellten gruppe


    if(fecRestoreInfo.buffer->size() < storage.size())
        fecRestoreInfo.buffer->setSize(storage.size());

    //to fec with current storage and buffer
    auto* fecRef = fecRestoreInfo.buffer->data();
    const auto* storageRef = storage.constData();
    const auto storSize = storage.size();

    for (size_t i = 0; i < storSize; ++i)
        fecRef[i] ^= storageRef[i];

    //time to restore
    if(packetOfGroupCanBeRestored(fecRestoreInfo))
        _restoredPacket = std::move(fecRestoreInfo.buffer);

    return fecHeader.isFecPacket();
}

size_t UdpFecRestore::toIndex(size_t val)
{
    return val % _fecRestoreBufferSize;
}

StoragePoolPtr UdpFecRestore::getNextRestored()
{
    return std::move(_restoredPacket);
}

bool UdpFecRestore::canRestore() const
{
    return _restoredPacket;
}

bool UdpFecRestore::packetOfGroupCanBeRestored(const FecRestoreInfo& fecRestoreInfo) const
{
    return fecRestoreInfo.containsFecPacket && fecRestoreInfo.counter == (_fecGroupSize - 1);
}

uint8_t UdpFecRestore::getGroupIndex(const Storage &storage) const
{
    //peek udpDataHeader for seqNum
    auto dataHeader = storage.toTypeAutomatic<UdpPlusDataHeader>();

    return static_cast<uint8_t>((dataHeader.seqNum - 1) % _fecGroupSize);
}

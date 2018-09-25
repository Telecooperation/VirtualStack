#include "UdpSequenceNumber.h"
#include "../../../common/Allocator/VsObjectFactory.h"

UdpSequenceNumber::UdpSequenceNumber(const VirtualStackSettings &settings) :
        _seqNum(1),
        _bufferedStorageSize(settings.SizeOfUdpSequenceNumberBuffer),
        _bufferedStorage(new StoragePoolPtr[_bufferedStorageSize]{}),
        _firstMissingSeqNum(1),
        _largestReceivedSeqNum(0),
        _isNackPossible(false)
{

}

size_t UdpSequenceNumber::getNextSequenceNumber()
{
    return _seqNum++;
}

bool UdpSequenceNumber::addStorage(StoragePoolPtr &&storage, size_t sequenceNumber)
{
    if(sequenceNumber < _firstMissingSeqNum) //skip seen packets
    {
        storage.reset();
        return false;
    }

    size_t bufferIndex = toIndex(sequenceNumber);

    //discard packets that are to new
    if((sequenceNumber - _firstMissingSeqNum) >= _bufferedStorageSize)
    {
//        Logger::Log(Logger::DEBUG, "UdpSequenceNumber: SeqNum overflow: Missing: ", _firstMissingSeqNum);
        storage.reset();
        return false;
    }

    //discard the same packet. the stored packets is the same as the now discarded! ensured by checks above
    if(_bufferedStorage[bufferIndex])
        return false;

    _largestReceivedSeqNum = std::max(sequenceNumber, _largestReceivedSeqNum);
    _bufferedStorage[bufferIndex] = std::move(storage);
    _isNackPossible = true;

    return true;
}

bool UdpSequenceNumber::isAvailable()
{
    return _bufferedStorage[toIndex(_firstMissingSeqNum)];
}

StoragePoolPtr UdpSequenceNumber::getNextAvailable()
{
    if(!isAvailable())
        return StoragePoolPtr();

    auto storage = std::move(_bufferedStorage[toIndex(_firstMissingSeqNum)]);
    ++_firstMissingSeqNum;

    return storage;
}

size_t UdpSequenceNumber::toIndex(size_t seqNum)
{
    return seqNum % _bufferedStorageSize;
}

void UdpSequenceNumber::fillWithNAcks(Storage &storage)
{
    //no packets have been sent until now
    if(!_isNackPossible)
    {
//        Logger::Log(Logger::DEBUG, "UdpSequenceNumber-Nack: No packets have been sent");
        return;
    }

    //storage is too small for largestObserved
    if(storage.freeSpaceForAppend() < sizeof(size_t))
    {
        Logger::Log(Logger::ERROR, "UdpSequenceNumber-Nack: Storage too small");
        return;
    }

    //we received packets but not the first one. the storage needs to have space for two size_ts but it doenst
    //prevents number underflow caused by storing (_firstMissingSeqNum - 1) which results in (0 - 1)
    if(_firstMissingSeqNum <= 1 && (storage.freeSpaceForAppend() < (sizeof(size_t) * 2)))
    {
        Logger::Log(Logger::ERROR, "UdpSequenceNumber-Nack: Storage too small");
        return;
    }

    //TODO: part is ignored right now
    //size_t largestReceivedNumIndex = toIndex(_largestReceivedSeqNum);
    size_t seqNum = _firstMissingSeqNum;

    //as long as theres space
    while (storage.freeSpaceForAppend() >= sizeof(size_t) * 2)
    {
        //we walked all over the buffer, nothing more left
        if(seqNum >= _largestReceivedSeqNum)
        {
            storage.appendDataScalarAfterEnd(_largestReceivedSeqNum);
            return;
        }

        auto& val = _bufferedStorage[toIndex(seqNum)];
        if(!val)
        {
            storage.appendDataScalarAfterEnd(seqNum);
//            Logger::Log(Logger::DEBUG, "Request Missing: ", seqNum");
        }

        ++seqNum;
    }

    storage.appendDataScalarAfterEnd(seqNum - 1);
}

#include "SequenceNumberMiddleware.h"

SequenceNumberMiddleware::SequenceNumberMiddleware(const VirtualStackSettings &settings) :
        _seqNum(0),
        _bufferedStorageSize(settings.SizeOfStackEngineSequenceNumberBuffer * settings.SizeOfStackEngineSequenceNumberBuffer * settings.StackEngineMaxStacksCount),
        _bufferedStorage(new StoragePoolPtr[_bufferedStorageSize]{}),
        _firstMissingSeqNum(0)
{

}

void SequenceNumberMiddleware::addSequenceNumber(StoragePoolPtr &storage)
{
    storage->prependDataAutomaticBeforeStart(&_seqNum);
    ++_seqNum;
}

void SequenceNumberMiddleware::addStorage(StoragePoolPtr &&storage)
{
    if(!storage || storage->size() == 0)
        return;

    //TODO: need packetizer so tcp as bytestream protocol wont split packets into several receives
    auto sequenceNumber = storage->toTypeAutomatic<size_t>();

    if(sequenceNumber < _firstMissingSeqNum) //skip seen packets
        return;

    size_t bufferIndex = toIndex(sequenceNumber);
    //todo: sequencenumber greater than bufferSize
    //probably invalidate all data before as buffer is too small

    // (sequenceNumber - _firstMissingSeqNum) >= _bufferedStorageSize ---> buffer too small
    if((sequenceNumber - _firstMissingSeqNum) >= _bufferedStorageSize)
    {
        Logger::Log(Logger::ERROR, "SequenceNumberMiddleware: Reset SequenceNumberBuffer because of overflow");
        //we have overflow, invalidate whole buffer (memset/bzero is not possible because of StoragePoolPtr destructor)
        for (size_t i = 0; i < _bufferedStorageSize; ++i)
        {
            _bufferedStorage[i].reset();
        }
        _firstMissingSeqNum = sequenceNumber;
    }

    storage->incrementStartIndex(sizeof(size_t)); //to remove the seqNum
    _bufferedStorage[bufferIndex] = std::move(storage);
}

bool SequenceNumberMiddleware::isAvailable()
{
    return _bufferedStorage[toIndex(_firstMissingSeqNum)];
}

StoragePoolPtr SequenceNumberMiddleware::getNextAvailable()
{
    if(!isAvailable())
        return StoragePoolPtr();

    auto storage = std::move(_bufferedStorage[toIndex(_firstMissingSeqNum)]);
    ++_firstMissingSeqNum;

    return storage;
}

size_t SequenceNumberMiddleware::toIndex(size_t seqNum)
{
    return seqNum % _bufferedStorageSize;
}

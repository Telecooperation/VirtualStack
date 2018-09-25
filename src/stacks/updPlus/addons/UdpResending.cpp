#include "UdpResending.h"

UdpResending::UdpResending(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory) :
        _resendingPool(vsObjectFactory.getStorageSendPool(settings.SizeOfUdpPlusResendingBuffer, "UdpPlusResendingBuffer")),
        _resendingBufferSize(_resendingPool->getCapacity()),
        _resendingBuffer(new StoragePoolPtr[_resendingBufferSize]{}),
        _firstMissingSeqNum(1),
        _largestSentSeqNum(0),
        _packetsResentCounter(0)
{}

bool UdpResending::bufferStorage(const StoragePoolPtr &storage, size_t seqNum)
{
    if(!storage || !_resendingPool->canRequest())
    {
        Logger::Log(Logger::ERROR, "UdpPlusResending: Resendbuffer was empty. Could not copy storage into it with seqNum: ", seqNum);
        return false;
    }

    _largestSentSeqNum = std::max(seqNum, _largestSentSeqNum);
    auto storageCopy = _resendingPool->request();
    storage->copyInto(*storageCopy);

    _resendingBuffer[toIndex(seqNum)] = std::move(storageCopy);
    return true;
}

void UdpResending::processNackInStorage(Storage& nackPayload, IKernel &kernel)
{
    //does not contain any nack because we did not sent any data yet
    if(nackPayload.size() < sizeof(size_t))
    {
        if(_largestSentSeqNum == 0)
            return;

        auto& toSendBuffer = _resendingBuffer[toIndex(_largestSentSeqNum)]; //toIndex
        if(!toSendBuffer)
        {
            Logger::Log(Logger::ERROR, "Resent->  Packet was wrongly deleted: ", _largestSentSeqNum);
            return;
        }

        if(!kernel.isValid())
            return;

//        Logger::Log(Logger::DEBUG, "Resent: ", missingPacket);
        kernel.sendPacket(*toSendBuffer);
        ++_packetsResentCounter;
        return;
    }

    if(nackPayload.size() % sizeof(size_t) != 0)
    {
        Logger::Log(Logger::DEBUG, "UdpPlus: Nack was malformed. Not a multiple of size_t");
        return;
    }

    const bool noPacketsMissing = nackPayload.size() == sizeof(size_t);
    size_t missingPacket = 0;
    auto firstMissingPacket = nackPayload.toTypeAutomatic<size_t>();
    while(nackPayload.size() > sizeof(size_t)) //Resend missing packets before largest seen
    {
        missingPacket = nackPayload.toTypeAutomatic<size_t>();
        nackPayload.incrementStartIndex(sizeof(missingPacket));

        //seqNum was too old and already received and acknowledged
        if(missingPacket < _firstMissingSeqNum)
        {
//            Logger::Log(Logger::DEBUG, "Resent-> Packet was too old: ", missingPacket, ", with FirstSeqNum: ", _firstMissingSeqNum);
            continue;
        }

        if(missingPacket >= (_firstMissingSeqNum + _resendingBufferSize))
        {
            Logger::Log(Logger::ERROR, "Resent-> Packet has not been sent yet - ResendingBufferOverflow: ", missingPacket);
            return; //receiver requested packet which was not sent yet
        }

        //packet was really not sent
        if(missingPacket > _largestSentSeqNum)
        {
            Logger::Log(Logger::ERROR, "Resent-> Packet has not been sent yet: ", missingPacket);
            return; //receiver requested packet which was not sent yet
        }

        auto& toSendBuffer = _resendingBuffer[toIndex(missingPacket)]; //toIndex
        if(!toSendBuffer)
        {
            Logger::Log(Logger::ERROR, "Resent->  Packet was wrongly deleted: ", missingPacket);
            continue;
        }

        if(!kernel.isValid())
            return;

//        Logger::Log(Logger::DEBUG, "Resent: ", missingPacket);
        kernel.sendPacket(*toSendBuffer);
        ++_packetsResentCounter;
    }

    //clean up resending buffer
    if(noPacketsMissing) //when the nack contains only one size_t, firstMissingPacket is equal to largestSeen, which is not missing
        ++firstMissingPacket;

    for(size_t i = _firstMissingSeqNum; i < firstMissingPacket; ++i)
    {
//        Logger::Log(Logger::DEBUG, "Resending-> Delete packet: ", i);
        _resendingBuffer[toIndex(i)].reset();
    }
    _firstMissingSeqNum = firstMissingPacket;
}

size_t UdpResending::toIndex(size_t val)
{
    return val % _resendingBufferSize;
}

UdpResending::~UdpResending()
{
    //Logger::Log(Logger::DEBUG, "Resent ", _packetsResentCounter, " packets");
}

bool UdpResending::canBuffer() const
{
    auto nextSeqNum = _largestSentSeqNum + 1;
    return nextSeqNum < (_firstMissingSeqNum + _resendingBufferSize);
}

size_t UdpResending::getCapacity() const
{
    return _resendingBufferSize;
}

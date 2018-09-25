#include "UdpFecCreate.h"
#include <cstddef>

UdpFecCreate::UdpFecCreate(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory) :
        _pool(vsObjectFactory.getKernelPool(2, "UdpFecCreateBuffer")),
        _fecGroupSize(settings.SizeOfUdpPlusFecGroupSize.as<uint8_t>()),
        _currentFecGroupIndex(0),
        _currentFecGroup(1ul),
        _fecBuffer(_pool->request()),
        _fecBufferRef(*_fecBuffer),
        _storageHeader(false),
        _fecHeader(true),
        _udpPlusHeader(true)
{
    resetFecBuffer();
}

void UdpFecCreate::addToStorage(Storage &storageWithHeader, IKernel &kernel)
{
    //what about initial startIndex

    auto* fecRef = _fecBufferRef.data();
    const auto* storageRef = storageWithHeader.constData();
    const auto storSize = storageWithHeader.size();

    for (size_t i = 0; i < storSize; ++i)
        fecRef[i] ^= storageRef[i];

    if(_fecBufferRef.size() < storageWithHeader.size()) //Header steht vor dem initialStartIndex -> für fecBuffer/Paket muss der Startindex um sizeof(udpplusheader) vorgezogen werden
        _fecBufferRef.setSize(storageWithHeader.size());

    _storageHeader.fecGroup = _currentFecGroup;
    storageWithHeader.prependDataScalarBeforeStart(_storageHeader);

    ++_currentFecGroupIndex;

    if (_currentFecGroupIndex != _fecGroupSize)
        return;

    _fecHeader.fecGroup = _currentFecGroup;
    _fecBufferRef.prependDataScalarBeforeStart(_fecHeader);
    _fecBufferRef.prependDataScalarBeforeStart(_udpPlusHeader);

    kernel.sendPacket(_fecBufferRef);

    resetFecBuffer();

    ++_currentFecGroup;
    _currentFecGroupIndex = 0;
}

void UdpFecCreate::resetFecBuffer()
{
    _fecBufferRef.nullifyData();
    _fecBufferRef.reset();
    _fecBufferRef.decrementEndIndexSafe(sizeof(UdpPlusDataHeader));
    _fecBufferRef.decrementEndIndexSafe(sizeof(UdpPlusHeader));
}

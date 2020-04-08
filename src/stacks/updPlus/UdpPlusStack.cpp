#include "../../common/Helper/Debouncer.h"
#include "UdpPlusStack.h"

UdpPlusStack::UdpPlusStack(StackEnum stackType,
                           std::unique_ptr<IKernel> kernel,
                           const VirtualStackSettings &settings, VsObjectFactory& vsObjectFactory) :
        IStack(stackType, settings),
        ack(stackUsesAck()),
        flow(stackUsesFlow()),
        fec(stackUsesFec()),
        _threadRunning(false),
        _thread(),
        _kernel(std::move(kernel)),
        _controlTimer(),
        _packetsReceivedCounter(0),
        _nackStoragePool(vsObjectFactory.getStorageSendPool(2, "UdpPlusNackBuffer")),
        _sequenceNumber(settings),
        _resending(settings, vsObjectFactory),
        _fecCreate(settings, vsObjectFactory),
        _fecRestore(settings, vsObjectFactory),
        _sendFlowWindow(settings, std::min(settings.SizeOfUdpPlusInitialFlowWindowSize.value, _kernel->getCanReceiveSize()), _settings.SizeOfUdpPlusControlFrameFlowWindowReserve),
        _recvFlowWindow(settings, _kernel->getMaxCanReceiveSize(), _kernel->getCanReceiveSize()),
        _controlStorage(_nackStoragePool->request()),
        _controlStoragePtr(*_controlStorage),
        _controlUdpPlusHeader(false),
        _dataUdpPlusHeader(true),
        _udpPlusDataHeader()
{

}

void UdpPlusStack::loop()
{
    if (!_kernel->start())
    {
        Logger::Log(Logger::ERROR, "Could not start ", stackInfo.Name, " stack");
        return;
    }

    const auto controlTimerElapsed = std::chrono::milliseconds(_settings.SizeOfUdpPlusControlFrameTimeThreshold);
    const auto packetsReceivedThreshold = _settings.SizeOfUdpPlusControlFrameRecvThreshold;

    //size_t packetsSentCounter = 0;
    _controlTimer.start();

    auto utilizationPlan = _isManagement ? _settings.SouthboundStackUtilizationPlan
                                              : _settings.UdpPlusUtilizationPlan;
    Debouncer debouncer{utilizationPlan, _settings};

    while (_threadRunning)
    {
        /*
         * Default: Ack, Flow, Fec
         * NoFec: Ack, Flow
         * NoFlowFec: Ack
         * NoFlow: Ack, Fec
         * OnlyFec: Fec
         * OnlyFlow: Flow
         * OnlyFlowFec: Flow, Fec
         */

        //Needed by: Ack
        if(ack)
        {
            while (_sequenceNumber.isAvailable() && !_fromStackBuffer.isFull())
                _fromStackBuffer.push(_sequenceNumber.getNextAvailable());
        }

        if (!_kernel->isValid())
            break;

        //Depends on: Ack or Flow
        //ControlPacket: WindowUpdate and NACK
        if ((ack || flow) && (_controlTimer.stop().hasElapsed(controlTimerElapsed) ||
                              _packetsReceivedCounter > packetsReceivedThreshold))
        {
            _controlStoragePtr.reset();
            _packetsReceivedCounter = 0;

            //Needed by: Flow
            if (flow)
                _recvFlowWindow.storeWindowUpdate(_controlStoragePtr, _kernel->getCanReceiveSize());

            //Needed by: Ack
            if (ack)
                _sequenceNumber.fillWithNAcks(_controlStoragePtr);
            _controlStoragePtr.prependDataAutomaticBeforeStart(&_controlUdpPlusHeader);

            _kernel->sendPacket(_controlStoragePtr);

            _controlTimer.start();
        }

        //Depends on: Always
        //Payload
        if ((_toStackBuffer.available() && (!flow || _sendFlowWindow.sendDataAllowed()) &&
             (!ack || _resending.canBuffer())))
        {
            auto payload = _toStackBuffer.pop();
            if (!payload)
                continue;

            //Needed by: Ack
            updatePayloadHeader(payload);
            payload->prependDataAutomaticBeforeStart(&_udpPlusDataHeader);

/*            if (!_isManagement)
            {
                auto timeCount = payload->toTypeAutomatic<size_t>(payload->size() - sizeof(size_t));
                payload->replaceDataScalarBeforeEnd(static_cast<size_t>(1 + timeCount));

                auto appendPosition = sizeof(size_t) + timeCount * (sizeof(uint8_t) + sizeof(long));
                payload->replaceDataScalarBeforeEnd(stackIndex, appendPosition);
                payload->replaceDataScalarBeforeEnd(StopWatch::getHighResolutionTime(),
                                                    appendPosition + sizeof(uint8_t));
            }
*/

            //Needed by: Fec
            //FEC, might send an additional storage which shall not be counted as its released immediately after receive
            if (fec)
                _fecCreate.addToStorage(*payload, *_kernel);

            payload->prependDataAutomaticBeforeStart(&_dataUdpPlusHeader);

            //Needed by: Ack
            if (ack)
                _resending.bufferStorage(payload, _udpPlusDataHeader.seqNum);



            if (_kernel->sendPacket(*payload))
            {
                //++packetsSentCounter;
                //Needed by: Flow
                if (flow)
                    _sendFlowWindow.sentOnePacket();
            }

            debouncer.reset();
        }

        if (_fromStackBuffer.isFull() || !_kernel->canReceive() || !_kernel->dataAvailable())
        {
            //nothing to receive
            debouncer.sleep();
            continue;
        }

        //wenn ein fec da ist, nimm es. aber merke. FEC sind nur datenpackete
        //wenn kein fec, sondern direkt vom kernel, dann header anschaun, für control oder daten

        auto recvPacket = _kernel->receivePacket();
        if (!recvPacket || !_kernel->isValid()) //connection has been closed
        {
            _threadRunning = false;
            break;
        }

        auto udpPlusHeader = recvPacket->toTypeAutomatic<UdpPlusHeader>();
        recvPacket->incrementStartIndex(sizeof(UdpPlusHeader));

        //Depends on: Ack or Flow [muss nicht gegaurded werden, weil wenn beides aus, dann bekomm ich nie Controlpackete]
        if (!udpPlusHeader.isDataPacket()) //contains windowUpdate
        {
            handleRecvControlPacket(*recvPacket);
            continue;
        }

        //Depends on: Fec
        if (!fec || !_fecRestore.receivePacket(*recvPacket))
            handleRecvDataPacket(std::move(recvPacket));

        //Depends on: Fec
        if (fec && _fecRestore.canRestore())
        {
            auto restored = _fecRestore.getNextRestored();
            if (!restored)
                continue;

            handleRestoredDataPacket(*restored);
            handleRecvDataPacket(std::move(restored));
        }

        debouncer.reset();
    }

    if (_kernel)
        _kernel->stop();

    _isConnectionClosed = true;
    _threadRunning = false;
}

void UdpPlusStack::start()
{
    if (_threadRunning || !_kernel)
        return;

    _threadRunning = true;
    _thread = std::make_unique<std::thread>(&UdpPlusStack::loop, this);
}

void UdpPlusStack::updatePayloadHeader(const StoragePoolPtr &storage)
{
    _udpPlusDataHeader.size = static_cast<uint16_t>(storage->size());
    if (ack)
        _udpPlusDataHeader.seqNum = _sequenceNumber.getNextSequenceNumber();
}

void UdpPlusStack::handleRecvDataPacket(StoragePoolPtr&& storage)
{
    if (storage->size() < sizeof(UdpPlusDataHeader))
        return;

    auto dataHeader = storage->toTypeAutomatic<UdpPlusDataHeader>();
    storage->incrementStartIndex(sizeof(UdpPlusDataHeader));

    if (!ack)
        _fromStackBuffer.push(std::move(storage));

    if (!ack || _sequenceNumber.addStorage(std::move(storage), dataHeader.seqNum))
    {
        if (flow)
            _recvFlowWindow.receiveOnePacket();
        ++_packetsReceivedCounter;
    }
}

void UdpPlusStack::handleRecvControlPacket(Storage &storage)
{
    if (flow)
        _sendFlowWindow.updateSendQuota(storage);
    if (ack)
        _resending.processNackInStorage(storage, *_kernel);
}

void UdpPlusStack::handleRestoredDataPacket(Storage &storage)
{
    if (storage.size() < sizeof(UdpPlusDataHeader))
        return;

    auto restoredPacketHeader = storage.toTypeAutomatic<UdpPlusDataHeader>();
    auto restoredPacketSize = std::min(storage.size(),
                                       restoredPacketHeader.size + sizeof(UdpPlusDataHeader));
    assert(restoredPacketSize >= sizeof(UdpPlusDataHeader));

    storage.setSize(restoredPacketSize);
}

void UdpPlusStack::stop()
{
    IStack::stop();

    _threadRunning = false;

    if(_kernel)
        _kernel->stop();

    if (!_thread || !_thread->joinable())
        return;

    _thread->join();
    _thread.reset();
}

UdpPlusStack::~UdpPlusStack()
{
    stop();
}

bool UdpPlusStack::stackUsesAck() const
{
    switch (stackInfo.Stack)
    {
        case StackEnum::UDPPlusIPv4:
        case StackEnum::UDPPlusNoFlowIPv4:
        case StackEnum::UDPPlusNoFlowFecIPv4:
        case StackEnum::UDPPlusNoFecIPv4:
            return true;
        default:
            return false;
    }
}

bool UdpPlusStack::stackUsesFlow() const
{
    switch (stackInfo.Stack)
    {
        case StackEnum::UDPPlusIPv4:
        case StackEnum::UDPPlusNoFecIPv4:
        case StackEnum::UDPPlusOnlyFlowIPv4:
        case StackEnum::UDPPlusOnlyFlowFecIPv4:
            return true;
        default:
            return false;
    }
}

bool UdpPlusStack::stackUsesFec() const
{
    switch (stackInfo.Stack)
    {
        case StackEnum::UDPPlusIPv4:
        case StackEnum::UDPPlusNoFlowIPv4:
        case StackEnum::UDPPlusOnlyFecIPv4:
        case StackEnum::UDPPlusOnlyFlowFecIPv4:
            return true;
        default:
            return false;
    }
}

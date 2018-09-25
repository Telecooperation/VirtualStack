#pragma once


#include "../../VirtualStackSettings.h"
#include "../../common/DataStructures/VS/IStack.h"
#include "../../common/Helper/StopWatch.h"
#include "../../interface/IKernel.h"
#include "addons/UdpFlowWindowRecv.h"
#include "addons/UdpFlowWindowSend.h"
#include "addons/UdpResending.h"
#include "addons/UdpSequenceNumber.h"
#include "model/UdpPlusHeader.h"
#include "addons/UdpFecCreate.h"
#include "addons/UdpFecRestore.h"

class UdpPlusStack final : public IStack
{
public:
    UdpPlusStack(StackEnum stackType, std::unique_ptr<IKernel> kernel,
                 const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory);

    ~UdpPlusStack() override;


    void stop() override;
private:
    void start() override;
    inline bool stackUsesAck() const;
    inline bool stackUsesFlow() const;
    inline bool stackUsesFec() const;

    inline void handleRestoredDataPacket(Storage& storage);
    inline void handleRecvDataPacket(StoragePoolPtr&& storage);
    inline void handleRecvControlPacket(Storage& storage);
    inline void updatePayloadHeader(const StoragePoolPtr &storage);

    const bool ack;
    const bool flow;
    const bool fec;

    std::atomic<bool> _threadRunning;
    std::unique_ptr<std::thread> _thread;

    std::unique_ptr<IKernel> _kernel;
    StopWatch _controlTimer;
    size_t _packetsReceivedCounter;

    PoolRef _nackStoragePool;
    UdpSequenceNumber _sequenceNumber;
    UdpResending _resending;
    UdpFecCreate _fecCreate;
    UdpFecRestore _fecRestore;
    UdpFlowWindowSend _sendFlowWindow;
    UdpFlowWindowRecv _recvFlowWindow;

    StoragePoolPtr _controlStorage;
    Storage& _controlStoragePtr;

    const UdpPlusHeader _controlUdpPlusHeader;
    const UdpPlusHeader _dataUdpPlusHeader;
    UdpPlusDataHeader _udpPlusDataHeader;

    void loop();

};



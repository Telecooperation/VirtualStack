
#pragma once


#include "../../../common/DataStructures/Container/RingBufferMove.h"
#include "../../../interface/INorthboundDevice.h"

class LoopbackNorthboundDevice final : public INorthboundDevice {
public:
    LoopbackNorthboundDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory);

    StoragePoolPtr pop() override;

    bool available() override;

    bool isFull() override;

    bool isDeviceOpen() override;

    void push(StoragePoolPtr packet) override;

private:
    std::mutex _pushMutex;
    RingBufferMove<StoragePoolPtr> _storageBuffer;
};



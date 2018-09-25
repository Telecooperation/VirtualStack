#pragma once

#include <common/DataStructures/Container/RingBufferMove.h>
#include <gmock/gmock-generated-function-mockers.h>
#include <interface/IKernel.h>
#include <common/Allocator/VsObjectFactory.h>
#include <VirtualStackSettings.h>

class LoopbackKernel final : public IKernel
{
public:
    LoopbackKernel(const VirtualStackSettings& settings, VsObjectFactory& vsObjectFactory);

    bool dataAvailable() override;

    StoragePoolPtr receivePacket() override;

    bool sendPacket(Storage &content) override;

    bool isValid() const override;

private:
    PoolRef _pool{};
    RingBufferMove<StoragePoolPtr> _buffer;
};
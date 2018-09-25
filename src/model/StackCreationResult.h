
#pragma once

#include "../common/DataStructures/VS/IStack.h"
#include "../southbound/configuration/NewStack/NewStackResult.h"
#include "InspectionStruct.h"

struct StackCreationResult
{
    explicit StackCreationResult(
                        bool pByRequest,
                        std::unique_ptr<IStack>&& pStack,
                        std::unique_ptr<RingBufferMove<StoragePoolPtr>>&& pStorageBuffer,
                        std::unique_ptr<NewStackResult>&& pNewStackResult) :
            byRequest(pByRequest),
            stack(std::move(pStack)),
            storageBuffer(std::move(pStorageBuffer)),
            newStackResult(std::move(pNewStackResult))
    {}

    flowid_t flowId;
    flowid_t partnerFlowId;
    const bool byRequest;
    std::unique_ptr<IStack> stack;
    std::unique_ptr<RingBufferMove<StoragePoolPtr>> storageBuffer;
    std::unique_ptr<NewStackResult> newStackResult;

    ALLOW_MOVE_SEMANTICS_ONLY(StackCreationResult);
};
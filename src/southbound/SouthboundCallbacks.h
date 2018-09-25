#pragma once

#include "../common/Helper/ClassMacros.h"
#include "configuration/NewStack/NewStackResult.h"
#include "configuration/NewStack/Request/NewStackRequest.h"
#include <functional>
#include <utility>

class SouthboundCallbacks
{
public:
    SouthboundCallbacks(std::function<void(std::unique_ptr<NewStackResult>)> &&onCreateNewStackRequest,
                        std::function<void(std::unique_ptr<NewStackResult>)> &&onCreateNewStackRespond) :
            OnCreateNewStackRequest(std::move(onCreateNewStackRequest)),
            OnCreateNewStackRespond(std::move(onCreateNewStackRespond))
    {}

    const std::function<void(std::unique_ptr<NewStackResult>)> OnCreateNewStackRequest;
    const std::function<void(std::unique_ptr<NewStackResult>)> OnCreateNewStackRespond;

    ALLOW_MOVE_SEMANTICS_ONLY(SouthboundCallbacks);
};



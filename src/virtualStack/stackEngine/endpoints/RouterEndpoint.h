#pragma once


#include "../../../interface/IEndpoint.h"

class RouterEndpoint final : public IEndpoint
{
public:
    RouterEndpoint(const flowid_t& flowId, const flowid_t& partnerFlowId);

    void processNBIToStack(Storage &packet) const override;

    void processStackToNBI(Storage &packet) const override;

    ~RouterEndpoint() override;
};



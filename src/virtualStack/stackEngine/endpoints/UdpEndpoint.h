#pragma once


#include "../../../interface/IEndpoint.h"

class UdpEndpoint final : public IEndpoint
{
public:
	UdpEndpoint(const flowid_t& flowId, const flowid_t& partnerFlowId,
                const sockaddr_storage &northboundSource,
				const sockaddr_storage &northboundDest);
	
	//virtual void initialize() override;
	
	void processNBIToStack(Storage &packet) const override;
	
	void processStackToNBI(Storage &packet) const override;
	
	~UdpEndpoint() override;
};



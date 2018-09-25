#pragma once


#include <sys/socket.h>

#include "../common/DataStructures/Model/Storage.h"
#include "../model/TransportProtocolEnum.h"
#include "../common/Helper/NetworkExtensions.h"

class IEndpoint
{
public:
	IEndpoint(const flowid_t& pFlowId, const flowid_t& pPartnerFlowId, const TransportProtocolEnum protoEnum,
			  const sockaddr_storage &northboundSource, const sockaddr_storage &northboundDest)	:
			flowId(pFlowId),
            partnerFlowId(pPartnerFlowId),
			protocolEnum(protoEnum),
			_northboundSourceAddr(northboundSource),
			_northboundDestinationAddr(northboundDest),
			_northboundSourcePort(NetworkExtensions::getPort(_northboundSourceAddr)),
			_northboundDestinationPort(NetworkExtensions::getPort(_northboundDestinationAddr))
	{}
	
	//virtual void initialize() = 0;
	//Paket entpacken, payload an ALM weitergeben,
	virtual void processNBIToStack(Storage &packet) const = 0;
	
	//Payload in Paket verpacken, an das NBI schicken, hier auch eventuelle Kommunikation an das NBI, z.b. ACKs
	virtual void processStackToNBI(Storage &packet) const = 0;
	
	virtual ~IEndpoint();

	const flowid_t flowId;
	const flowid_t partnerFlowId;

	const TransportProtocolEnum protocolEnum;
	
	const sockaddr_storage &getNorthboundSourceAddr() const;
	
	const sockaddr_storage &getNorthboundDestinationAddr() const;
	
	uint16_t  getNorthboundSourcePort() const;
	uint16_t  getNorthboundDestinationPort() const;
	
	ALLOW_MOVE_SEMANTICS_ONLY(IEndpoint);
protected:
	//This sockaddr_storage and ports are in hostorder! (not networkorder)
	const sockaddr_storage _northboundSourceAddr;
	const sockaddr_storage _northboundDestinationAddr;
	const uint16_t _northboundSourcePort;
	const uint16_t _northboundDestinationPort;
};



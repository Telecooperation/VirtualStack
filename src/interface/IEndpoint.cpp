#include "IEndpoint.h"

IEndpoint::~IEndpoint()
{
	
}

const sockaddr_storage &IEndpoint::getNorthboundSourceAddr() const
{
	return _northboundSourceAddr;
}

const sockaddr_storage &IEndpoint::getNorthboundDestinationAddr() const
{
	return _northboundDestinationAddr;
}

uint16_t IEndpoint::getNorthboundSourcePort() const
{
	return _northboundSourcePort;
}

uint16_t IEndpoint::getNorthboundDestinationPort() const
{
	return _northboundDestinationPort;
}

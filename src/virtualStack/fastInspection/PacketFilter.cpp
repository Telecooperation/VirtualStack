#include "../../model/InspectionStruct.h"
#include "PacketFilter.h"

bool PacketFilter::filterOutIPv6(const VirtualStackSettings& settings, const InspectionStruct& inspectionStruct)
{
	return inspectionStruct.internetProtocol == InternetProtocolEnum::IPv6;
}

bool PacketFilter::filterOutRawProtocol(const InspectionStruct &inspectionStruct)
{
	return inspectionStruct.northboundTransportProtocol == TransportProtocolEnum::RAW;
}

bool PacketFilter::filterOut(const VirtualStackSettings &settings, const InspectionStruct &inspectionStruct)
{
	return filterOutIPv6(settings, inspectionStruct) || filterOutRawProtocol(inspectionStruct);
}
